# Credits and licenses

EggLoader is unofficial homebrew. No Minecraft or NuMC game files are bundled.
The asset preparer reads your own inputs and makes a private SD installation.

## Aether

`dimension_module.egg` contains a C adaptation of code from
[The Aether (Aether Legacy), 1.12.2](https://github.com/The-Aether-Team/The-Aether-Archived/tree/1.12.2),
by The Aether Team and its contributors, licensed under **LGPL-3.0**.
The port was modified for EggLoader's callbacks, ARM/VFP execution, block families,
entity registration, world generation and dimension travel in September 2026.
It is not an official Aether release and does not contain the full mod.

The corresponding modified source and build/relink files are available at no
charge in **Aether-0.0.2-source.zip**, alongside this binary on the
[0.0.2 release page](https://github.com/yorkedegg/EggLoader-Releases/releases/tag/v0.0.2).
Copies of LGPLv3 and GPLv3 are in `notices/COPYING.LESSER` and `notices/COPYING3`.
The LGPL applies to the adapted Aether code; it is not a blanket license for
EggLoader's unrelated code, game inputs or artwork.

The original Aether assets are separately restricted and are **not included**.
Our tiles were made for this port. Entity definitions and base geometry are
derived locally from the user's game files rather than distributed as game data.

The source kit includes enough EggLoader application code/object files to rebuild
the pack binding with a modified Aether module. You may use and modify those
support files to recombine/relink this release with modified Aether code, and
reverse-engineer it to debug those modifications. No restriction in these notes
limits rights granted by the LGPL. The private repo/history is not included.

## Dependencies

- libctru, by devkitPro and contributors: `notices/libctru-README.md`.
- Newlib: `notices/COPYING.NEWLIB`.
- GCC/devkitARM runtime: `notices/COPYING3` and `notices/COPYING.RUNTIME`.
- 3gxtool, by Nanquitas: used to make the plugin container; not bundled.
- Luma3DS and NuMC3DS: required, obtained separately, not bundled.

The Mods frontend's egg graphic is based on Minecraft's icon. Minecraft names
and imagery belong to their owners. No endorsement by Mojang, Microsoft,
Nintendo, The Aether Team or other upstream projects is implied.
