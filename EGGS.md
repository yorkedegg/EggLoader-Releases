# Eggs in 0.0.2

These six files are included in `pack/eggs/`. Install them through the preparer,
not by copying the archives alone. This release uses a fixed six-egg set.

| Egg | What it does |
| --- | --- |
| `cube_module.egg` | Loader Cube: a small custom-block example. |
| `portal_module.egg` | Our Portal Gun experiment. Not a complete PortalGunClassic port. |
| `dimension_module.egg` | Our Aether port experiment: islands, blocks, portals and Flying Cow. Not the full Java mod. |
| `hello_egg.egg` | Minimal module-entry/logging example. |
| `plugin_smoke.egg` | Startup and event smoke test. |
| `render_probe.egg` | Checks the rendering-service API. The backend is unavailable in this build; it doesn't enable 3D. |

Catify is retired and not included. The older woods/decorative batch and HUD
stereo prototype still need conversion before they can ship as supported eggs.
Phyg assets and a reserved ID remain in the prepared set, but this tested native
module does not register Phyg. The newer working-tree implementation is not shipped.

Aether source: `Aether-0.0.2-source.zip`, available alongside the binary download.
The binary uses adapted LGPL-3.0 Aether code; see [NOTICES.md](NOTICES.md).
