# Quick-equip wheel + sockets

**Status:** Phase 0 corrective shipped (occupied-only, page cue, tap+hold paging, exclusive L/R) + nested bag stub + mod `claims.ini`  
**Next:** Playtest pages; Link run-in-place polish (deferred)  
**Do not:** expand `mItems[24]` without a save plan; put WW names in ring/registry source

---

## Product

| Piece | Behavior |
|-------|----------|
| Setting | **Quick Equip Wheel** (`game.quickEquipWheel`, default **off**) |
| Gate | Extra Item Slot on |
| **Tap** | Paused wheel using `dQe_` pages (when feature on) |
| **Hold** | Live socket ring (0.3× sim), release → confirm by kind |
| Layout | **Occupied-only** draw (capacity still pages×24) |
| Pages | Registry `dQe_getPageCount()` (default **2**, max **4**) |
| Cue | On-screen `page / pageCount` (or `BAG n` in nested view) |
| **D-pad L/R** | Flip pages (tap + hold); exclusive vs Midna/Quick Swap while open |
| Bags | `dQeKind_Bag` → A opens nested ≤8 children; B closes |
| Charts / songs | **Ext Status** — not on this wheel |

---

## Socket registry (WW-agnostic)

**Header:** [`include/d/d_ext_quick_equip.h`](../../include/d/d_ext_quick_equip.h)  
**Impl:** folded into [`src/d/d_ext_mod_flags.cpp`](../../src/d/d_ext_mod_flags.cpp) (no new cmake TU)

| API | Role |
|-----|------|
| `dQe_setPageCount` / `getPageCount` | 1..4 pages |
| `dQe_claim` / `clear` / … | Occupy / free sockets |
| `dQe_claimBagChild` / `peekBagChild` | Nested bag |
| `dQe_deepLinkAssignZ` | Tools-tab → Z assign |
| `dQe_seedTpBuiltin` | TP **tools only** on page 0 (vanilla wheel contents). Page 1+ are mod sockets — no TP sword/shield seed |

**Kinds:** `Empty`, `InvSlot_Z`, `SwordEquip`, `ShieldEquip`, `ZSelect`, `Custom`, `Bag`.

Confirm dispatch is by kind only. `SwordEquip` / `ShieldEquip` exist for **mod** claims (e.g. WW weapons), not vanilla Collect gear. Mod claims: `ext_inv/claims.ini` via `dExtInv_rescanClaims()`.

---

## Capacity vs WW bag (decomp / №103)

| Content | Count | Host |
|---------|------:|------|
| WW XYZ bag | **21** | Track A pages |
| Nested spoils/bait/mail | 8+8+8 | Nested bag sockets |
| Charts / songs / quest | ~70+ | **Ext Status** |

---

## Related

| Doc | Role |
|-----|------|
| [ext-start-status.md](ext-start-status.md) | Collect sibling Tools/Quest/Atlas |
| [cut-actors-demo-restore.md](cut-actors-demo-restore.md) №103 | Full TP id table; two-inventory |
| [wind-waker-item-work.md](../wind-waker-item-work.md) | 21 `itemmdl` bag meshes |
| [WW-Restoration-Cookbook.md](../WW%20Linked/WW-Restoration-Cookbook.md) | TP owns inventory |
