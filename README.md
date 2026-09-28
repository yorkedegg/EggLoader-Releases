# EggLoader 0.0.1

An early code-only mod loader for Minecraft: New Nintendo 3DS Edition.
One `EggLoader.3gx` plugin loads trusted `.egg` packages from your SD card.

[Download 0.0.1](https://github.com/yorkedegg/EggLoader-Releases/releases/tag/v0.0.1)

Choose `EggLoader-0.0.1-experimental.zip` under Assets. GitHub's automatic
"Source code" downloads contain this release documentation, not the plugin.

**Experimental prerelease. Title/menu testing only. Don't open a world with
this build yet.** This isn't the full block/entity framework release, and it
doesn't make every existing `.egg` compatible.

## What this build does

- Loads and relocates code-only native eggs and dispatches game lifecycle events.
- Checks the game/NuMC build before installing hooks.
- Includes a title-screen Mods list showing actual loader status.
- Includes `plugin_smoke.egg`, a small loading/event test, not a content mod.

No mod toggles, automatic dependency resolution, custom asset preparation,
world-save compatibility guarantee, or working stereo backend in this release.
Native eggs aren't sandboxed. Only install code you trust.

## Exact requirements

- New Nintendo 3DS family; old-model support is not established here.
- USA title `00040000001B8700`, update v9.12.0.
- Luma 13.3.3 with plugin loading enabled.
- The supported NuMC trace build (September 26, 2026), with these SHA-256 hashes:

| File | SHA-256 |
| --- | --- |
| `code.ips` | `340738abd774ffa0b6bec50e7546d69dfde5128785ef15be6e7527f07f5c220c` |
| `romfs/numc3ds/native.bin` | `f7faed8db91d41d5d339f8c634344c39b32013e5c0040bc0531f30b814e1a27d` |

A version label alone isn't enough: other NuMC builds may differ. Don't install
this on an unmatched build or bypass its guards. NuMC and the game are not
included; obtain them separately from their respective owners/distributors.

## Install and test

1. Fully close Minecraft. Back up your SD files and export valued worlds.
2. Back up any existing EggLoader plugin. Keep standalone ModMenu, HudStereo,
   Tonic and other 3GX plugins disabled for this test; don't delete them.
3. Copy the contents of `sd/` to your SD card:

   ```text
   luma/plugins/00040000001B8700/EggLoader.3gx
   eggloader/eggs/plugin_smoke.egg
   ```

4. For this first test, keep only the supplied smoke egg in `eggloader/eggs/`.
   Move other eggs to a backup folder rather than deleting them.
5. Disable Rosalina's debugger and launch Minecraft. Stay in the menus.
6. Confirm NuMC's extra settings are present. Open Mods, exit with B, reopen,
   and exit with Done. Expected rows: NuMC Confirmed; `egg_core` and
   `plugin_smoke` Active. **Do not open a world or join a server yet.**

The log is `sdmc:/eggloader/plugin.log`. Save it before another launch if
something fails. Logs and crash dumps can contain private data; share them
privately after reviewing them, not in a public release attachment.

## Back out

Close Minecraft; reboot if it is unresponsive. Rename the candidate plugin to
`EggLoader.3gx.disabled`, then restore your backed-up working plugin if you had
one. Don't enable two copies. Restore your previous egg selection as needed.
This package doesn't replace NuMC, game assets, skins, or saves.

## Evidence

The underlying menu candidate passed 62 startup ARM checks, 19 integrated menu
ARM checks, 8 status ARM checks and 70 focused regression tests. The ARM tests
model engine UI/OS behavior; they aren't a console gameplay test. The preceding
loader build booted on hardware with NuMC and delivered a smoke-egg event.
On September 28, the tester confirmed the integrated candidate booted with
NuMC settings and Mods could open, exit with B/Done, and reopen. This is a
title/menu pass, not a world, multiplayer or long-session stability test.
Release version metadata does not change its executable code; packaging checks
that separately. This public repo contains releases/docs, not the private
research history or the full framework source.

## Credits

Built using devkitPro's devkitARM/libctru toolchain and Nanquitas's 3gxtool,
running under Luma3DS. NuMC3DS is a separate project and a required input, not
bundled here. The Mods frontend comes from this project's existing ModMenu work.
See `NOTICES.md` and `notices/` for dependency notices. No blanket open-source
license is assigned to EggLoader by this release.

Not affiliated with Mojang, Microsoft or Nintendo.
