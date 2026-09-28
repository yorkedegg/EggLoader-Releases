# Installing 0.0.1

You need a New 3DS-family console, USA Minecraft (`00040000001B8700`) update
v9.12.0, Luma 13.3.3, and the supported September 26 NuMC trace build.
Other regions, old-model consoles and other NuMC builds aren't covered.

Check your NuMC files against these SHA-256 hashes:

| File | SHA-256 |
| --- | --- |
| `code.ips` | `340738abd774ffa0b6bec50e7546d69dfde5128785ef15be6e7527f07f5c220c` |
| `romfs/numc3ds/native.bin` | `f7faed8db91d41d5d339f8c634344c39b32013e5c0040bc0531f30b814e1a27d` |

NuMC and Minecraft aren't included. A matching version name alone isn't enough;
don't bypass the checks if your files differ.

1. Close Minecraft and back up your working setup and worlds.
2. Keep other 3GX plugins disabled for this test, including standalone ModMenu,
   HudStereo and Tonic. Keep backups rather than deleting them.
3. Download the release ZIP and copy its `sd/` contents to the SD card:

   ```text
   luma/plugins/00040000001B8700/EggLoader.3gx
   eggloader/eggs/plugin_smoke.egg
   ```

4. Keep only the supplied smoke egg in the eggs folder for the first boot.
   Enable plugin loading in Luma; disable Rosalina's debugger.
5. Launch Minecraft. Check NuMC settings, then Mods → B → Mods → Done.
   The list should show NuMC Confirmed, with `egg_core` and `plugin_smoke` Active.

**Stay in the menus for this release. Don't open a world or join a server.**

If it fails, close Minecraft or reboot, disable `EggLoader.3gx`, and restore
your backed-up plugin/egg selection. Don't enable both plugin copies.
The log is `/eggloader/plugin.log`; save it before another launch.
Review logs/dumps for personal information before sharing them.
