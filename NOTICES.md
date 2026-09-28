# Credits and dependency notices

EggLoader is a homebrew project, not an official Minecraft or Nintendo product.
This release does not include a game dump, NuMC IPS/native files, texture packs,
research captures, or the private repository history.

- [libctru](https://github.com/devkitPro/libctru): 3DS system APIs linked into the plugin.
  Upstream README, including its license, is in `notices/libctru-README.md`.
- [devkitPro/newlib](https://github.com/devkitPro/newlib): C runtime routines.
  Upstream collected notices are in `notices/COPYING.NEWLIB`.
- [GCC](https://gcc.gnu.org/): compiler/runtime support, through devkitARM.
  License and runtime exception are in `notices/COPYING3` and
  `notices/COPYING.RUNTIME`.
- [3gxtool](https://github.com/Nanquitas/3gxtool), by Nanquitas: plugin container
  conversion. The tool executable/source is not bundled.
- [Luma3DS](https://github.com/LumaTeam/Luma3DS): required custom firmware;
  firmware is not bundled. Startup checks recognize the supported firmware's
  changes using hashes, not an embedded copy of its redirector.

The existing ModMenu frontend is integrated into EggLoader, not installed as a
second plugin. Its egg graphic is based on Minecraft's icon; Minecraft names
and imagery belong to their respective owners. No third-party endorsement is
implied. No blanket license for EggLoader or third-party art is granted here.

Dependency notice copies were retrieved from their upstream projects while
preparing this release. Their licenses apply to those components, not to every
file in this repository. This is a downloads/documentation repository, not a
complete source distribution.
