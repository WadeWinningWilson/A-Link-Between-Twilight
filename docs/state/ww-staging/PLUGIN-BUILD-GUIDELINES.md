# WW Plugin Build Guidelines — STANDING (user order 2026-08-23)

**Status: IN FORCE until the user revokes it. No instance may self-approve an
exception.**

## 1. The fork builds plugin-free

`DUSK_ENABLE_WW_PLUGIN` defaults **OFF** in `CMakeLists.txt`, and
`DUSK_WW_PLUGIN_DIR` defaults **empty**. A sibling checkout of
`A Link Between Twilight WW` must **not** auto-`add_subdirectory`.
The ninja graph is generated from that file: **`ninja dusklight`** is the
fork build. Do not name `ww_donor_disc_package` on the ninja command line —
if CMake did not emit the target, ninja fails; if it did, the plugin is
already a defect against this order.

The gitignored `cmake/local_paths.cmake` is the **reconnect recipe**, not the
off-switch — public clones never have it, so the in-tree default has to be
OFF or every machine with the sibling folder pulls the plugin into the fork
link (FEATURES game → `dusklight_imports.lib` → unresolved symbols take the
whole ninja graph down).

- Do **not** turn `DUSK_ENABLE_WW_PLUGIN` on.
- Do **not** re-point `DUSK_WW_PLUGIN_DIR`.
- Do **not** restore the in-tree hookup.
- Do **not** copy any `.dusk` into the build's `mods/` or AppData `mods/`.

A fork build that produces or loads `ww_donor_disc.dusk` is a **defect against
this order**, not a convenience. History: the in-tree hookup silently
regenerated the plugin on every lane's build — three unplug-by-hand rounds
(the last after Integrator's 2026-08-23 11:57 build) before the mechanism was
removed. A later default-to-sibling CACHE path undid that for any instance
whose tree sat next to the plugin repo.

## 2. The plugin's only build path is standalone

All WW plugin iteration happens in the plugin's own repo via `standalone/`
(`FEATURES game webgpu`, `build_install.bat`). That path is unaffected by this
order. The both-trees compile gate (fork + dusklight-main) continues to run
against the standalone build.

## 3. Reconnection is deferred, and it is the user's call

The user's exact framing: *"Eventually we'll reconnect the plugin to the fork,
but that will come way later."* Reconnection is a **user decision** — no
instance may self-approve it. When that day comes, the restore recipe is
written inside `cmake/local_paths.cmake` (`DUSK_ENABLE_WW_PLUGIN=ON` + the
sibling path, both FORCE, then reconfigure).

## 4. If the plugin reappears

A `ww_donor_disc` target in `build.ninja` or a `.dusk` in `mods/` means
`DUSK_ENABLE_WW_PLUGIN` was turned on or the path was re-set. **Stop, do not
build further, flag it on CALLS.**

## Reference state (verified 2026-08-23; CMake default hardened 2026-08-24)

| Check | Value |
|---|---|
| `CMakeLists.txt` | `DUSK_ENABLE_WW_PLUGIN` option **OFF**; `DUSK_WW_PLUGIN_DIR` default empty |
| fork ninja command | `ninja dusklight` (matches CMakeLists; no plugin target) |
| `CMakeCache.txt` | `DUSK_ENABLE_WW_PLUGIN=OFF`; `DUSK_WW_PLUGIN_DIR` empty STRING |
| `build.ninja` `ww_donor_disc` mentions | 0 |
| `build/windows-msvc-relwithdebinfo/mods/` | empty |
| Last artifact stash | `build/windows-msvc-relwithdebinfo/mods-unplugged-2026-08-23/ww_donor_disc.dusk` |
