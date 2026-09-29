# Installing 0.0.2

Back up your SD card and export any worlds you care about first. Keep those worlds
closed while testing. Removing mods doesn't remove their blocks or entities from
a save, and this version doesn't protect you from opening incompatible worlds.

## What you need

- Minecraft: New Nintendo 3DS Edition, USA `00040000001B8700`, update v9.12.0.
- Luma **13.3.3**, with game patching and its plugin loader enabled.
- Your own extracted **stock update ROMFS**. Not a NuMC overlay or modded dump.
- The matching, unmodified NuMC title folder, obtained separately.
- Python **3.9 or newer** on your computer.

NuMC's build names alone aren't enough. This release accepts these SHA-256s:

| File | SHA-256 |
| --- | --- |
| `code.ips` | `1f238db722433d4cb51a36294193179b4a024fc45e128553e9c0f9861fe911b4` |
| `romfs/numc3ds/native.bin` | `e316f4404b73bb1235fb005375aa41e876ce171f25ae32f340b581094b906ebf` |

The preparer also checks the other NuMC files. A newer build is not automatically
compatible. The download includes neither NuMC nor Minecraft assets.

## Prepare on your computer

Extract `EggLoader-0.0.2-experimental.zip`, open a terminal in its folder, then:

```sh
python3 prepare.py --romfs "/path/to/stock/romfs" --numc "/path/to/00040000001B8700" --out "/path/to/new-eggloader-output" --fresh-worlds
```

On Windows, use `py -3` instead of `python3` if needed. Point `--numc` at the
folder directly containing `code.ips` and `romfs`. The output folder must not
exist and must be separate from the inputs. Nothing is installed automatically.

`--fresh-worlds` means you'll use this release's fixed ID mapping for **new
throwaway worlds**. If you already have EggLoader content saves, replace that
flag with `--existing-ids "/path/to/your/eggloader/ids.bin"`. A different registry
is refused. Don't bypass that refusal by choosing fresh-worlds for old saves.
Matching IDs alone still don't prove every old save is compatible.

Success prints a verified file count and creates `sd/` plus `VERIFIED.json`.
The generated folder contains your game files: **don't upload or redistribute it**.

## Copy to SD

Fully close Minecraft. Use a card reader or FTPD with the game closed.

1. Back up these existing folders somewhere outside their active locations:
   `luma/titles/00040000001B8700`, `luma/plugins/00040000001B8700`, and `eggloader`.
2. Move the original folders out of those locations. Don't merge over an unknown
   mod set: stale eggs or texture overrides can break the pack. Don't discard backups.
3. Copy the generated `sd/` contents to the SD root. That creates the supported
   NuMC title tree, one `EggLoader.3gx`, six eggs, prepared modules and `ids.bin`.
4. Launch Minecraft. Check NuMC's settings and the Mods screen, then use a new
   throwaway world. Do not run a second game plugin alongside EggLoader.

Keep the whole prepared set together. Do not rename eggs, delete test eggs,
replace the registry, or copy only the `.3gx`. The startup checks will refuse
changed or incomplete sets. `render_probe` reporting unavailable is expected;
it is not the stereo mod.

## If it refuses or hangs

Close the game and read `eggloader/plugin.log`. Wrong NuMC versions, missing
files and changed eggs are common causes. A refusal can leave the launch screen
waiting; it doesn't mean you should keep opening worlds with a partial setup.

For rollback, close Minecraft, move the new three folders aside, and restore
your three original backup folders together. Keep the new files for diagnosis.
Don't open saves containing custom content with the content removed.

Modified Aether builds: use the instructions in the separate source ZIP. Its
relink tool updates the plugin's file checks; it doesn't disable them globally.
