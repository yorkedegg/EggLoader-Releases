# Make an egg

Start small: this example prints a message when it loads and another when block
initialization runs. No game dump, NuMC source, or private repo needed to build it.

## You'll need

- Python 3.10 or newer.
- [devkitARM](https://devkitpro.org/wiki/Getting_Started), installed through devkitPro.
- A copy of this repository, including `sdk/` and `examples/`.

Use **Code → Download ZIP** on the repo page, or clone it. The source archive
attached to the older 0.0.1 tag predates this starter.

From the repository folder:

```sh
python3 sdk/build.py examples/hello_egg --out build/hello_egg.egg
```

The script looks in `$DEVKITARM/bin`, then `$DEVKITPRO/devkitARM/bin`, then
`/opt/devkitpro/devkitARM/bin`, then your PATH. No extra Python packages needed.

For the [supported 0.0.1 setup](INSTALL.md), close Minecraft and copy the result
to `/eggloader/eggs/hello_egg.egg`. Start with just this egg after you've tested
the supplied smoke egg. Stay at the title screen. Look in `/eggloader/plugin.log`
for `hello_egg: loaded` and `hello_egg: blocks event`.

This starter is checked offline against the compiled loader; that isn't a new
console test of your mod.

## Change it

Edit [hello_egg.c](examples/hello_egg/hello_egg.c) and rebuild. Your entry point
receives an `EggHost` with logging and event registration functions. Check its
magic, size and API before using it. Return zero on success; failed entry or
ordinary event callbacks stop loading. Don't block or do slow work in callbacks.

The builder won't overwrite an existing egg. For the next build, use a fresh
output path such as `--out build/test2/hello_egg.egg`.

To rename it, change `name`, `module` and `assets` in `egg.json`, the C/asset
filenames, and the `module` field inside the assets JSON. The output filename
must match: `your_name.egg`. Use lowercase letters, digits and underscores.

An egg from this starter contains exactly three files:

```text
egg.json                    # name, version, API, filenames
hello_egg.yolk               # compiled ARM module
hello_egg.assets.json        # empty assets for 0.0.1
```

`version` is a positive integer; `api` is the minimum host API you need.
This example uses API 15; the released host provides API 16. The current
code-only package reader accepts API 13–16.

## Resources / limits

- [Example source](examples/hello_egg/hello_egg.c)
- [Host API header](sdk/host.h): includes older content APIs for reference;
  their presence doesn't mean 0.0.1 prepares the assets/IDs they need.
- [Build script](sdk/build.py) and [module linker script](sdk/module.ld)
- [ELF → YOLK packer](sdk/module_image.py)
- [Our eggs and prototypes](EGGS.md)

For 0.0.1, stick to code-only logging/event experiments. Assets must remain empty.
The ZIP is limited to 64 KiB, a YOLK to 24 KiB, and the shared module arena is
small. At most seven external eggs; memory can run out before that. Compile
ARM-mode freestanding C, not Thumb/C++/threads or direct game-address patches.
The build tool rejects unsupported relocations; it doesn't sandbox native code.
Only run eggs from people you trust, and don't test on valued saves.
