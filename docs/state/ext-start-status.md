# Ext Status — Collect sibling (Tools / Quest / Atlas)

**Status:** Phase 1–3 shell + registry + mod claims loader shipped (2026-07-21)  
**Next:** Playtest Collect L/R → tabs; real WW labels/icons via mod `claims.ini`  
**Do not:** put WW place/item string literals in core; replace Fmap/Dmap; compile any WW layout/asset into the exe
**RULING (user, 2026-07-22):** the socket GETS the WW menu layouts — as MOD DATA (adapted BLO, mod-folder-loaded, dialogue-BLO host pattern). TP stays receiver: socket + ALL pane-driving logic remain ours; WW screens are skins. The former "don't port WW BLO" do-not is superseded by this ruling; the covenant boundary is now "data-hosted, never compiled-in".

---

## Product

| Piece | Behavior |
|-------|----------|
| Entry | START → Collect unchanged; **L/R** → Ext Status |
| Tabs | Tools → Quest → Atlas (L/R); wrap L from Tools / R from Atlas → Collect |
| Close | B / START → close pause (via Collect close) |
| Tools A | Deep-link `dExtStatus_tryDeepLinkZ` → `dQe_deepLinkAssignZ` |
| Window status | `11` (Collect stays `3`) |

---

## Registry

**Header:** [`include/d/d_ext_status.h`](../../include/d/d_ext_status.h)  
**Menu:** [`include/d/d_menu_ext_status.h`](../../include/d/d_menu_ext_status.h)  
**Impl:** [`src/d/d_ext_mod_flags.cpp`](../../src/d/d_ext_mod_flags.cpp) + window machine in [`src/d/d_menu_window.cpp`](../../src/d/d_menu_window.cpp)

Mod data: `model_replacements/<mod>/ext_inv/claims.ini` — see skeleton example under `tools/ww_crew_restoration_skeleton/ext_inv/`.

---

## №234 — cautioned WW pause arcs vs these surfaces (interaction pass)

The mod carries `pause/UNWIRED_DONOR/*.arc` (itemres/itemicon/menures/clctres/clothres/
fmapres+Fmap/saveres/optres) under explicit CAUTION. Interaction verdict: **no collision —
different layers.** Our surfaces are data-driven registries (claims.ini); the arcs are WW's
finished screens. The CAUTION's "don't point dQe_/Ext Status at these" and this doc's "don't
port WW BLO" are the SAME rule from both sides.

- **Only live interaction: `itemicon.arc` as an icon QUARRY** — extract individual members
  (first: `clothes.bti` for the wardrobe row), convert via the custom-icon pipeline, ship as mod
  assets. Never mount the arc. **VERIFY-POINT: WW BTI formats vs the pipeline's RGB5A3 TIMG
  expectation** — one test icon settles it (paletted formats carry the №216 lesson's shadow).
- `itemres`/`clctres` BLO screens: **PROMOTED by the user ruling** — they become the socket's visual layer (adapt → host → our socket logic drives their panes). Per-screen pane-driving map = the new work item.
- `fmapres`+`Fmap`: foot-gun adjacency with the Atlas tab — "replace Fmap/Dmap" stays a do-not;
  Atlas remains claims-driven.
- `clothres`: likely permanently inert (wear-design = TP clothes; menu home = wardrobe row).
- `menures`/`saveres`/`optres`: TP owns pause/save/options — inert cargo.
- Future fork (currently forbidden): the NPC-dialogue BLO precedent proves adapted WW BLO CAN
  host on TP — a user decision could someday open WW-authentic pause visuals; until then the
  do-nots stand.
- Wardrobe/clothes row: lands in **Quest tab** via claims.ini (WW filed clothes NOWHERE — №233 —
  so menu presence is additive; wheel stays clothes-free per fidelity).

## Related

| Doc | Role |
|-----|------|
| [quick-equip-wheel.md](quick-equip-wheel.md) | Paged inventory sockets |
| cut-actors №103 | Two-inventory forced |
| Cookbook | TP owns menus |
