# WW itemmdl — live state

| Field | Value |
|-------|--------|
| **status** | Track A get-item bow: **4E committed**; color near sign-off. Track B held bow working (template) |
| **owner_impl** | Cursor (Wind Curs) |
| **owner_review** | Claude (Wind Clau) — review only, no source edits |
| **next** | Replay vs WW ref; if tips still bloom → SC A/B toggle; after color sign-off → BTK then generalize |
| **do_not** | MAJI; material struct 14; per-shape double `modelUpdateDL`; per-shape dump at load; re-bake every spawn; reopen locked Plan A pipeline |
| **pipeline_locked** | 2N′ + 2B‴ + struct-0 + ambient-only (4E); no MAJI / no struct 14 |
| **updated** | 2026-07-01 (from interconnected RESUME; refresh when next session lands) |
| **canonical** | [wind-waker-item-work.md](../wind-waker-item-work.md) |
| **archive** | [Interconnected Chats/Wind Curs-Wind Clau.md](../Interconnected%20Chats/Wind%20Curs-Wind%20Clau.md) — archaeology only |

---

## Roles

| Role | Does |
|------|------|
| **Cursor** | Implement, build (`build_run.bat`), update this file’s header when status changes |
| **Wind Clau** | Review screenshots / plan next knob; do not edit source |
| **User** | Playtest, screenshots, commit calls |

## Fresh openers

**Impl:**

> Read `docs/AGENT_INDEX.md` + `docs/state/ww-itemmdl.md`. Continue WW `itemmdl` Track A. Pipeline locked (see do_not / pipeline_locked). Update this state file when done — do not append to the interconnected archive.

**Review:**

> Read `docs/state/ww-itemmdl.md` + `docs/wind-waker-item-work.md`. Review Cursor’s screenshots vs WW ref; pick next knob. No source edits. Trust this state + git over chat memory.

## Acceptance (Track A color)

| Check | State |
|-------|--------|
| Whole mesh + nocked arrow | locked |
| Orange body (not lemon) | near sign-off — may still be hot vs ref |
| SC detail | restored (4a TEV replay) |
| Silver caps (not white bloom) | open — SC A/B / 4b if needed |

## Hard-won (do not re-derive)

- CRT `0xC0000409` from unbound texmap → Aurora `bitset::set` — fixed A+A′; do not revert.
- Colorless bow = same unbound path; Fix B = per-draw `GXSetTevOrder` from valid material struct.
- Aurora GX is deferred fifo — trust visuals + apply logs over pre-drain snapshots.
- Non-MAJI `setLightTevColorType` is empty stub on PC.

Full narrative / rejected experiments → archive doc only.
