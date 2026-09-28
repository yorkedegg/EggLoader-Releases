# Our eggs & experiments

Stuff we've made while building EggLoader. Some of it predates the drop-in
loader, so this isn't a folder of mods you can throw into 0.0.1 yet.

| Name | What it does | Where it's at |
| --- | --- | --- |
| `plugin_smoke` | Logs when an egg loads and receives events. | Included in 0.0.1; tested on console. |
| `render_probe` | Checks whether the loader exposes rendering services. | Built as an egg; tested offline. 0.0.1 reports the backend unavailable. It doesn't add 3D. |
| Catify | Puts a cat on block textures. We also tried a sharper 192×192 version. | Asset-only egg / texture-overlay experiment. Worked on console with the older setup; not a 0.0.1 drop-in. Shelved for now. |
| Loader Cube | Our first custom block. | Older content module; needs prepared textures and IDs. |
| Portal Gun | Portal blocks and a portal-gun item. | Older prototype; needs its matching assets/setup. |
| Aether / Flying Cow | Aether-style blocks/worldgen and a custom flying cow. | Older content prototype. The cow survived saving and reopening the world after we fixed an ID mix-up. |
| Wood & decorative blocks | Extra wood types, decorative blocks and recipes from the early block tests. | Earlier block-mod builds, not standalone eggs for this release. |
| HUD Stereo | Gives the hearts/hunger HUD depth. | Worked as a separate plugin on an older build. Conversion to a stereo egg is unfinished. |

`egg_core` is part of the loader, not another mod you need to download.

The content mods above need the older prepared setup. Dropping them into 0.0.1
won't install their textures or IDs. Only the smoke egg ships here for now.

Want to make one? [Start here.](MAKING-EGGS.md)
