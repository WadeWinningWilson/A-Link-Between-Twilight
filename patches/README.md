# `patches/` — changes to code we do not own

## Why this folder exists, outside `src/` and outside the mod folder

Some fixes the WW layer needs are not ours to ship. They live in projects we
only consume:

```
dusklight   TwilitRealm/dusklight     not ours   (origin)
aurora      encounter/aurora          not ours   (extern/aurora submodule)
```

Those changes cannot live in the **mod folder** or the **plugin**, and this is a
hard constraint rather than a preference: `aurora` is compiled *into*
`dusklight.exe` (`add_subdirectory(extern/aurora)` +
`target_link_libraries(dusklight PRIVATE aurora::main ...)`). By the time a
plugin loads, aurora has already drawn the title screen. A fix below the plugin
boundary cannot be delivered by something above it.

They also should not be **commits inside `extern/aurora`**. That is a clone of a
third party's repository: such a commit can never be pushed anywhere we control,
exists only on one machine, and would leave the submodule pin referencing a
commit nobody else can fetch. A patch in a repo we DO own is the durable form.

So: patches here, applied to the submodule, tracked in ALBD. When the plugin
gets its own repository these travel alongside it, because a consumer building
the plugin needs the same host fixes we do.

## Layout

```
patches/aurora/upstream/   candidates for encounter/aurora — real fixes, clean
patches/aurora/local/      our working-tree state — scaffolding, NOT for anyone else
```

The split is load-bearing. `upstream/` is what we would submit; `local/` is a
safety net so work is not one `git submodule update` from gone. Do not send
`local/` anywhere.

## Applying

```
cd extern/aurora
git apply --unidiff-zero ../../patches/aurora/upstream/0001-load-indx-fix.patch
```

`--unidiff-zero` is required: the upstream patch was generated with `-U0` so
that adjacent debug hunks were not swept into it.

## Register

The reasoning, evidence and status for each entry lives in
[`docs/WW Linked/ww-host-side-requirements.md`](../docs/WW%20Linked/ww-host-side-requirements.md)
as HR-N. This folder holds the artifacts; the register holds the argument.

### aurora/upstream/0001-load-indx-fix.patch — HR-1

32 added lines. Two defects in the indexed-XF (`LOAD_INDX`) path of aurora's
FIFO parser: an operator-precedence error that read `g_gxState.arrays[]` 23–47
entries out of bounds (access violation in `copy_xf_data`), and a payload
misparsed as 1+4 bytes instead of the SDK writer's u8/u16/u16, which advanced
one byte too far and desynced the remainder of the FIFO.

**This does not become obsolete as more of WW is ported — the opposite.** The
opcode is emitted by core J3D (`J3DShapeMtx::loadMtxIndx_PNGP`) through the
Dolphin SDK's own `GDWriteXFIndx[ABCD]Cmd` writers, for any model using indexed
matrix loads. Every additional skinned character or multi-matrix mesh sends
MORE of them. Porting WW's own native systems would not help either: a faithful
renderer emits the same opcode, because that is what the hardware takes.

TP content never emits it, which is why the bug survived the whole life of the
port. Outset's rope bridge was simply the first thing to reach it (§449 —
*"ropes only made one reachable at Outset"*). Ropes revealed it; they are not
its scope.

It goes obsolete only if `encounter` fixes it upstream or dusklight drops
aurora. Neither is about our porting.

### aurora/local/working-tree.patch — LIVE instrumentation, retained

The full submodule diff, ~1300 lines across 13 files. Of it, **32 lines are the
fix above**; the remainder is FIFO/GX/shader/pipeline debug instrumentation.

**RETAINED UNTIL THE ROPES ARE RESOLVED (user ruling, 2026-08-07.)** I had
filed this as spent scaffolding. That was wrong: the rope rendering defect is
still open, and this is the instrument still being used to chase it. Removing a
probe because the bug it found is fixed, while the bug it was BUILT for is not,
would cost the next round of debugging its only visibility.

Strip it when ropes close, not before. **Never submit this** — it is local
tooling either way.
