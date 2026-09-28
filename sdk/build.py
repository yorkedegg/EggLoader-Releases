"""Build a code-only egg for EggLoader 0.0.1. Does not install anything."""
import argparse
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import zipfile
import module_image

SDK = Path(__file__).resolve().parent

def compiler():
    roots = []
    if os.environ.get('DEVKITARM'): roots.append(Path(os.environ['DEVKITARM']))
    if os.environ.get('DEVKITPRO'): roots.append(Path(os.environ['DEVKITPRO'])/'devkitARM')
    roots.append(Path('/opt/devkitpro/devkitARM'))
    for root in roots:
        for name in ('arm-none-eabi-gcc', 'arm-none-eabi-gcc.exe'):
            candidate = root/'bin'/name
            if candidate.is_file(): return str(candidate)
    found = shutil.which('arm-none-eabi-gcc')
    if not found: raise ValueError('devkitARM not found; set DEVKITARM to its installation folder')
    return found

def build(source, out):
    manifest = json.loads((source/'egg.json').read_text())
    name = manifest.get('name', '')
    if not re.fullmatch('[a-z][a-z0-9_]{0,30}', name) or name == 'egg_core':
        raise ValueError('choose a lowercase mod name (not egg_core), at most 31 characters')
    required = {'format','name','version','api','module','assets'}
    if set(manifest) != required or manifest['format'] != 'egg/1':
        raise ValueError('use the example egg.json fields')
    if type(manifest['version']) is not int or not 1 <= manifest['version'] <= 0xffffffff:
        raise ValueError('version must be a positive 32-bit integer')
    if type(manifest['api']) is not int or not 13 <= manifest['api'] <= 16:
        raise ValueError('0.0.1 supports package API 13 through 16')
    if manifest['module'] != name+'.yolk' or manifest['assets'] != name+'.assets.json':
        raise ValueError('module and assets filenames must match the mod name')
    if out.name != name+'.egg': raise ValueError('output filename must be '+name+'.egg')
    if out.exists(): raise ValueError('output exists; choose a fresh build directory')
    assets = json.loads((source/manifest['assets']).read_text())
    if assets != {'module':name,'blocks':[]}:
        raise ValueError('this starter only supports empty code-only assets')
    gcc = compiler()
    flags = ['-std=c99','-Wall','-Wextra','-Werror','-pedantic','-Os','-march=armv6k','-marm',
             '-mno-unaligned-access','-ffreestanding','-fno-builtin','-fno-stack-protector',
             '-fno-unwind-tables','-fno-asynchronous-unwind-tables','-ffunction-sections',
             '-fdata-sections','-fno-pic']
    with tempfile.TemporaryDirectory(prefix='egg-build-') as temp:
        stage = Path(temp)
        subprocess.run([gcc,*flags,'-I',str(SDK),'-c',str(source/(name+'.c')),
                        '-o',str(stage/'module.o')],check=True)
        subprocess.run([gcc,'-march=armv6k','-marm','-nostdlib',str(stage/'module.o'),
                        '-Wl,--emit-relocs,--build-id=none,-T,'+str(SDK/'module.ld'),
                        '-o',str(stage/'module.elf')],check=True)
        yolk = module_image.from_elf((stage/'module.elf').read_bytes())
    if len(yolk) > 24576: raise ValueError('YOLK exceeds the 24 KiB limit')
    files = {'egg.json':json.dumps(manifest,separators=(',',':')).encode(),
             manifest['assets']:json.dumps(assets,separators=(',',':')).encode(),
             manifest['module']:yolk}
    out.parent.mkdir(parents=True,exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='egg-pack-') as temp:
        package = Path(temp)/(name+'.egg')
        with zipfile.ZipFile(package,'x') as z:
            for filename,data in files.items():
                info=zipfile.ZipInfo(filename,(2026,9,28,0,0,0))
                info.compress_type=zipfile.ZIP_DEFLATED;info.external_attr=0o100644<<16
                z.writestr(info,data)
        if package.stat().st_size > 65536: raise ValueError('egg exceeds 64 KiB')
        with zipfile.ZipFile(package) as z:
            if z.testzip() or set(z.namelist()) != set(files): raise ValueError('archive check failed')
        with out.open('xb') as file: file.write(package.read_bytes())
    print('Built '+str(out))

if __name__ == '__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('source',type=Path);parser.add_argument('--out',type=Path,required=True)
    args=parser.parse_args()
    try: build(args.source,args.out)
    except (ValueError,OSError,subprocess.CalledProcessError) as error: parser.exit(1,str(error)+'\n')
