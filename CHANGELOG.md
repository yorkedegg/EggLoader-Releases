# Releases

## 0.0.2 — experimental content pack

- Loader Cube, Portal Gun and Aether/Flying Cow, plus three example/test eggs.
- A local asset preparer using your own game dump and supported NuMC files.
- Fixed saved-ID mapping, exact file checks and the existing Mods menu.
- Actual eggs included, with Aether source/build files in a separate ZIP.
- Catify removed; its stock animated textures restored.

The combined pack was tested in-game by the owner. The later Catify cleanup
retains the native modules and was installed/read-back checked. No extra gameplay
pass, long-run benchmark or general multiplayer compatibility claim.
The plugin executable is unchanged from that cleanup candidate; release metadata
now correctly describes the supported NuMC content setup.

Source rebuild reproduces Aether's egg and the loader executable exactly.
A modified Aether build also passes 32 offline ARM startup checks.
Phyg registration, stereo and the older decorative batch are not shipped features.

## Docs and starter update

Shorter README, a list of our eggs/prototypes, and a buildable hello-egg example.
The example was checked against the compiled loader offline. The released
plugin hasn't changed.

## 0.0.1 — experimental

First public download of EggLoader's code-only native egg loader.

- One EggLoader plugin, with a drop-in `eggloader/eggs/` folder.
- Exact game/NuMC checks and checked startup hook installation.
- Integrated title-screen Mods list backed by real loader status.
- Included smoke-test egg to verify loading and event delivery.
- Console-tested title boot, NuMC settings, Mods open, B/Done exit and reopen.

This is a title/menu test release for the pinned USA/NuMC/Luma combination in
the README. It is not a world/gameplay release, a working stereo egg, or general
compatibility for all custom blocks/entities/assets. Back up first.
