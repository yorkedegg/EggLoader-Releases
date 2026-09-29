# EggLoader

a mod loader for Minecraft on the 3DS.

[download 0.0.2](https://github.com/yorkedegg/EggLoader-Releases/releases/tag/v0.0.2)
· [install](INSTALL.md)
· [our eggs](EGGS.md)
· [make your own](MAKING-EGGS.md)

## what's in this one

Loader Cube, Portal Gun, our Aether/Flying Cow experiment, and three test eggs.
All six eggs are in the download, with a Mods button in the title menu.
Catify is retired. Stereo isn't included yet.

This is still experimental. The combined pack worked in-game on our console;
the later Catify cleanup was installed and read-back checked, not another
gameplay pass. Phyg's newer code isn't in the tested binary.

**Read the install guide first.** Content eggs currently need their assets
prepared on a computer using your own stock game files and matching NuMC build.
The included Python tool does that; no compiler needed for the release.
It accepts one exact USA v9.12.0 / Luma 13.3.3 / NuMC combination.
Back up your saves and use throwaway worlds.

The plugin and prepared files belong together. This isn't arbitrary
drag-and-drop content loading yet: changing the egg set needs another preparation
and plugin build. Don't just delete or add eggs to this pack.

## making mods

[Start here](MAKING-EGGS.md) for the small code-only SDK example and its limits.
Aether's corresponding source and rebuild tools are a separate
[source ZIP](https://github.com/yorkedegg/EggLoader-Releases/releases/download/v0.0.2/Aether-0.0.2-source.zip).
Normal players don't need it.

Thanks to NuMC3DS, Luma3DS, devkitPro, Nanquitas and The Aether Team.
[Credits and licenses](NOTICES.md).

Not affiliated with Mojang, Microsoft, Nintendo or The Aether Team.
