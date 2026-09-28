"""Restricted zero-linked ARM ELF -> YOLK. Not a mod ABI, installer or sandbox.

Only our freestanding ARM-mode link profile is supported. Link with --emit-relocs;
hidden/hand-coded absolute addresses cannot be discovered by an ELF packer. Inputs
must come from a trusted build. Native code itself is not made safe by validation.
"""
import argparse
import hashlib
import json
from pathlib import Path
import struct

# B-039: the image magic follows the ABI generation of the entry symbol. YOLK (EggLoader, `egg_module_entry`) is what the live
# loader ships; FWM1 (`fw_module_entry`) is the pre-rename form, still read so the pinned rounds before the rename keep their images.
MAGICS = (b"YOLK", b"FWM1")
ENTRY_SYMBOLS = {"egg_module_entry": b"YOLK", "fw_module_entry": b"FWM1"}
HEADER, MAX_MEMORY, MAX_RELOCS = 64, 1024 * 1024, 65536
MAX_FILE = HEADER + MAX_MEMORY + 4 * MAX_RELOCS
MAX_ELF = 16 * 1024 * 1024


class ImageError(ValueError):
    pass


def require(condition, message):
    if not condition:
        raise ImageError(message)


def word(blob, offset):
    return struct.unpack_from("<I", blob, offset)[0]


def align(n, boundary):
    return (n + boundary - 1) & -boundary


def inspect(blob, abi=1):
    require(HEADER <= len(blob) <= MAX_FILE, "image file size")
    h = struct.unpack_from("<4s15I", blob)
    magic, version, actual_abi, arch, size, initialized, memory, rx, entry, count, table = h[:11]
    require(magic in MAGICS and version == 1 and arch == 1 and size == len(blob), "image header")
    require(0 < abi <= 0xFFFFFFFF and actual_abi == abi, "image ABI")
    require(not any(h[11:]), "reserved header words")
    require(memory <= MAX_MEMORY and count <= MAX_RELOCS, "image resource limit")
    require(0 < rx <= initialized <= memory and not rx % 4096 and not memory % 4096
            and not initialized % 4 and not entry % 4 and entry < rx, "image layout")
    require(table == HEADER + initialized and table + 4 * count == size, "relocation table size")
    offsets = [word(blob, table + 4 * i) for i in range(count)]
    previous = -1
    for offset in offsets:
        require(not offset % 4 and previous < offset <= initialized - 4, "relocation offset/order")
        require(word(blob, HEADER + offset) < memory, "relocation target")
        previous = offset
    return {"image_bytes": initialized, "memory_bytes": memory, "rx_bytes": rx,
            "entry_offset": entry, "relocations": offsets, "abi": actual_abi}


def from_elf(blob, abi=1):
    """Pack only .text/.rodata/.data/.bss, internal ARM calls and ABS32 pointers.

    No imports, Thumb, dynamic linking, TLS, constructors or MOVW/MOVT fixups.
    Nonallocated debug sections are ignored; every allocated relocation is checked.
    This deliberately rejects ELF features rather than guessing their semantics.
    """
    require(52 <= len(blob) <= MAX_ELF, "ELF file size")
    h = struct.unpack_from("<16sHHIIIIIHHHHHH", blob)
    ident, kind, machine, version, entry, phoff, shoff, flags, eh, phsz, phn, shsz, shn, names = h
    require(ident[:9] == b"\x7fELF\x01\x01\x01\x00\x00" and not any(ident[9:]), "ELF identity")
    require((kind, machine, version, flags, eh) == (2, 40, 1, 0x05000200, 52), "ELF ARM EABI5 soft-float profile")
    require(shsz == 40 and 1 < shn <= 256 and 0 < names < shn, "ELF section header format")
    require(phsz == 32 and 0 < phn <= 4, "ELF program header format")

    def span(offset, size):
        require(0 <= offset <= len(blob) and 0 <= size <= len(blob) - offset, "ELF truncated range")
        return blob[offset:offset + size]

    span(shoff, shn * shsz)
    span(phoff, phn * phsz)
    sections = [struct.unpack_from("<10I", blob, shoff + 40 * i) for i in range(shn)]
    require(not any(sections[0]), "ELF null section")
    require(sections[names][1] == 3, "ELF section string table")
    string_tables = {}

    def string(table, offset):
        require(table[1] == 3, "ELF string table type")
        if table not in string_tables:
            string_tables[table] = span(table[4], table[5])
        data = string_tables[table]
        require(offset < len(data), "ELF string offset")
        end = data.find(b"\x00", offset, offset + 256)
        require(end >= 0, "ELF name unterminated or exceeds 255 bytes")
        try:
            return data[offset:end].decode("ascii")
        except UnicodeDecodeError as exc:
            raise ImageError("ELF non-ASCII name") from exc

    allocated = {}
    allowed = {".text": (1, 6), ".rodata": (1, 2), ".data": (1, 3), ".bss": (8, 3)}
    for i, section in enumerate(sections[1:], 1):
        name, typ, sf, addr, off, size, link, info, alignment, entsize = section
        if typ != 8:
            span(off, size)
        if sf & 2:
            label = string(sections[names], name)
            # SHF_MERGE (0x10) and SHF_STRINGS (0x20) are linker hints on string
            # literals; the section is still plain read-only data.
            require(label in allowed and (typ, sf & ~0x30 if label == ".rodata" else sf) == allowed[label],
                    "unsupported allocated section: " + label)
            require(label not in allocated and size <= MAX_MEMORY and addr + size <= MAX_MEMORY,
                    "ELF section duplicate/limit")
            require(0 < alignment <= 4096 and not alignment & (alignment - 1) and not addr % alignment,
                    "ELF section alignment")
            allocated[label] = (i, section)
    require(".text" in allocated, "ELF missing text")
    text = allocated[".text"][1]
    require(text[3] == 0 and text[5] > 0 and text[5] % 4 == 0, "ELF text layout")
    ordered = sorted(allocated.values(), key=lambda pair: (pair[1][3], pair[0]))
    last = 0
    for _, section in ordered:
        require(section[3] >= last, "ELF overlapping sections")
        last = section[3] + section[5]
    rx_end = max(s[3] + s[5] for _, s in allocated.values() if not s[2] & 1)
    rx = align(rx_end, 4096)
    for label in (".data", ".bss"):
        if label in allocated:
            require(allocated[label][1][3] >= rx, "ELF writable data overlaps RX pages")
    initialized = align(max([rx] + [s[3] + s[5] for _, s in allocated.values() if s[1] == 1]), 4)
    if ".bss" in allocated:
        require(allocated[".bss"][1][3] >= initialized, "ELF BSS before initialized data")
    memory = align(max(last, initialized), 4096)
    require(memory <= MAX_MEMORY and entry % 4 == 0 and entry < text[5], "ELF memory/entry limit")
    programs = [struct.unpack_from("<8I", blob, phoff + 32 * i) for i in range(phn)]
    for typ, off, va, pa, filesz, memsz, pf, alignment in programs:
        require(typ == 1 and pf in (5, 6) and va == pa and filesz <= memsz
                and va + memsz <= memory and alignment == 4096 and off % 4096 == va % 4096,
                "unsupported ELF load segment")
        require((pf == 5 and va + memsz <= rx) or (pf == 6 and va >= rx), "ELF segment permissions")
        span(off, filesz)
    for _, s in allocated.values():
        require(any(p[2] <= s[3] and s[3] + s[5] <= p[2] + p[5] and
                    p[6] == (6 if s[2] & 1 else 5) and
                    (s[1] == 8 or (s[4] - p[1] == s[3] - p[2] and s[4] + s[5] <= p[1] + p[4]))
                    for p in programs), "ELF section not covered by load segment")
    tables = [i for i, s in enumerate(sections) if s[1] == 2]
    require(len(tables) == 1, "ELF needs one symbol table (keep --emit-relocs)")
    symindex = tables[0]
    symtab = sections[symindex]
    require(symtab[9] == 16 and symtab[5] % 16 == 0 and 16 <= symtab[5] <= 16 * MAX_RELOCS
            and 0 < symtab[6] < shn, "ELF symbol table layout")
    symbols = [struct.unpack_from("<IIIBBH", blob, symtab[4] + i) for i in range(0, symtab[5], 16)]
    require(not any(symbols[0]), "ELF null symbol")
    alloc_indices = {i for i, _ in allocated.values()}
    entry_found = False; magic = None
    for symbol in symbols[1:]:
        name, value, size, info, other, si = symbol
        label = string(sections[symtab[6]], name)
        require(si != 0, "ELF unresolved import: " + label)
        if si in alloc_indices:
            s = sections[si]
            require(s[3] <= value <= s[3] + s[5] and size <= s[3] + s[5] - value, "ELF symbol range")
        if info & 15 == 2:
            require(si == allocated[".text"][0] and value % 4 == 0, "ELF non-ARM function")
        if label in ENTRY_SYMBOLS:
            require(info & 15 == 2 and value == entry and size > 0, "ELF entry symbol")
            require(not entry_found, "ELF defines two entry symbols"); entry_found = True; magic = ENTRY_SYMBOLS[label]
    require(entry_found, "ELF missing egg_module_entry function")
    image = bytearray(initialized)
    for _, s in allocated.values():
        if s[1] == 1:
            image[s[3]:s[3] + s[5]] = span(s[4], s[5])
    fixups, seen = [], set()
    runtime_tables = 0
    for s in sections:
        if s[1] not in (4, 9):
            continue
        require(0 < s[7] < shn, "ELF relocation target section")
        if s[7] not in alloc_indices:
            continue  # Debug relocations don't run or enter the image.
        require(s[1] == 9 and s[9] == 8 and s[5] % 8 == 0 and s[6] == symindex,
                "ELF requires REL relocations and main symbol table")
        target_section = sections[s[7]]
        require(target_section[1] == 1, "ELF cannot relocate BSS contents")
        runtime_tables += 1
        require(s[5] // 8 + len(seen) <= MAX_RELOCS, "ELF relocation limit")
        for at in range(s[4], s[4] + s[5], 8):
            offset, info = struct.unpack_from("<II", blob, at)
            typ, si = info & 255, info >> 8
            require(offset % 4 == 0 and target_section[3] <= offset
                    and offset + 4 <= target_section[3] + target_section[5]
                    and offset not in seen and si < len(symbols), "ELF relocation site/symbol")
            seen.add(offset)
            value = word(image, offset)
            if typ == 40:  # R_ARM_V4BX: already linked BX, null symbol by ABI.
                require(si == 0 and s[7] == allocated[".text"][0] and
                        value & 0x0FFFFFF0 == 0x012FFF10, "ELF V4BX instruction")
                continue
            symbol = symbols[si]
            require(si != 0 and symbol[5] in alloc_indices, "ELF relocation must stay internal")
            if typ == 2:  # R_ARM_ABS32, linked at zero: stored word is image offset.
                require(any(sec[3] <= value < sec[3] + sec[5] for _, sec in allocated.values()),
                        "ELF absolute pointer outside sections")
                fixups.append(offset)
            elif typ in (28, 29):  # R_ARM_CALL / R_ARM_JUMP24 stay relative.
                require(s[7] == allocated[".text"][0] and symbol[5] == allocated[".text"][0]
                        and value >> 28 != 15 and (value >> 24) & 15 == (11 if typ == 28 else 10),
                        "ELF unsupported branch instruction")
                displacement = value & 0xFFFFFF
                if displacement & 0x800000:
                    displacement -= 0x1000000
                target = offset + 8 + displacement * 4
                require(0 <= target < text[5], "ELF branch leaves text")
            else:
                raise ImageError("unsupported ARM relocation: " + str(typ))
    require(runtime_tables > 0 and seen, "ELF has no runtime relocation records; keep --emit-relocs")
    fixups.sort()
    table = HEADER + initialized
    require(type(abi) is int and 0 < abi <= 0xFFFFFFFF, "image ABI")
    header = struct.pack("<4s15I", magic, 1, abi, 1, table + 4 * len(fixups), initialized,
                         memory, rx, entry, len(fixups), table, 0, 0, 0, 0, 0)
    result = header + image + b"".join(struct.pack("<I", p) for p in fixups)
    inspect(result, abi)
    return bytes(result)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("elf", type=Path)
    parser.add_argument("out", type=Path, help="new output file; never overwrites")
    args = parser.parse_args(argv)
    try:
        with args.elf.open("rb") as stream:
            blob = stream.read(MAX_ELF + 1)
        packed = from_elf(blob)
        with args.out.open("xb") as stream:
            stream.write(packed)
    except (OSError, ImageError) as exc:
        parser.exit(2, str(exc) + "\n")
    print(json.dumps({**inspect(packed), "sha256": hashlib.sha256(packed).hexdigest(),
                      "native_runtime_ready": False}, sort_keys=True, indent=2))


if __name__ == "__main__":
    main()
