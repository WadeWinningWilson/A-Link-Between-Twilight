# WW `d_a_item` native port — plan (History/Engine, 2026-08-11)

> The §725 ruling's exit for the `item` no-op. DECOMP-FIRST scoping done; donor
> measured, dependencies dispositioned, seams named. Full-state-machine law
> applies: the donor's 12 states ARE the spec — nothing parked as "optional".

## Donor inventory (measured)

| piece | size | what it is |
|---|---|---|
| `d_a_item.cpp` | 1,538 lines | the field-item actor: 12 states, 6 per-class actions (rupee/heart/key/emono/sword/arrow), modes WAIT/WATER, get-demo paths, boss-item paths |
| `d_a_itembase.cpp` | ~200 lines | model/anim base (`daItemBase_c`): loads per-item model via `d_item_data`'s `item_resource[256]`, brk/btk anim wiring |
| `d_item.cpp` (consumption side) | partial | `execItemGet` etc. — WW inventory effects on pickup |
| header `d_a_item.h` | 147 lines | full class read; param space: itemNo=param&0xFF, itemBitNo=param>>8, action=param>>0x1A, type=param>>0x18, switches in home.angle.z + param>>0x10 |

## The five seams (each carries its R5/registry citation)

1. **Item-id CONSUMPTION boundary** — the port's ONLY translation point. On pickup,
   WW itemNo → TP inventory effect via R5 `item.id_space` (WW 0x2F=HOOKSHOT vs TP
   0x2F=WEAR_KOKIRI is the canonical collision). Donor data unchanged (zero-bake);
   `execItemGet`'s WW effects re-expressed as TP ops (rupee += , heart +=, arrows...)
   keyed by the DONOR id. Trap #3 discipline: always the donor's number.
2. **Models** — donor field-item models (rupee/heart/etc.) come from the DONOR disc
   (item resource table arc names); the disc reader already serves Object arcs.
   `d_item_data`'s `item_resource[256]` table ports as DATA (name/arc/model id
   columns), not re-typed by hand — trap #1/#2 apply to its layout read.
3. **Sea coupling** (`mode_water`, ripple callback) — donor queries `daSea` for
   water Y. Disposition: seam to the host stage's water query (the port's sea is
   room content); if no queryable sea exists yet, the WATER mode ports COMPLETE but
   its entry condition can't fire until the sea query lands — recorded as a gate,
   not a parked state (state machine intact, entry blocked by missing donor
   subsystem, same class as L-5's waveRot).
4. **Particles** — dPa ripple/follow/smoke callbacks → the port's WW JPA support
   (`ww_jpa` — JPA lineage traps #1/#2 already handled there). Effect ids go
   through the R5 particle_id lineage rows (§396 precedent — donor numbers only).
5. **Get-demo states** (INIT/WAIT/MAIN_GET_DEMO) — port complete; they bind to the
   demo-item machinery the ba1/clothes work already exercises. Field items
   (rupees/hearts) never enter them; important items do. No shortcut.

## Registration (the §725 exit executed)

- New port profile `fpcNm_WW_ITEM_e` at the next free port index (0x330+ block,
  ww_profile_register row + selftest, per the P5 Ruling-3 pattern).
- `l_objectName` routing: `item` is TP-placed too (unlike §696's donor-only veg
  names), so the route must be RUNTIME host-scoped (the №93/§679 skip-class site
  where the no-op lives today flips from skip → route). itemFLY/itemDek: donor-only
  names, data-keyed rows fine.
- The §725 no-op strips in the SAME change (guard-ledger discipline: exit executed
  = strip trigger fired).

## Scratch rule (trap #6, minted tonight)

Any per-instance table keyed by donor indices sizes from donor data. The item
actor is per-instance (fopAc), so no global scratch expected — flagged so the
review checks it.

## Order of work (each step buildable + gate-runnable)

1. `d_item_data` resource-table port (data TU, no behavior) + model load via disc.
2. `daItemBase_c` + `daItem_c` create/draw/delete + MODE_WAIT + rupee/heart
   actions + consumption boundary. **First visible win: real WW rupees at WW
   positions.**
3. Remaining actions (key/emono/sword/arrow) + boss/nezumi paths + get-demo states.
4. MODE_WATER + ripple (gated on sea query availability — gate stated in-file).
5. Registration flip (profile + route + no-op strip) — LAST, after the actor
   passes its own battery, so the no-op keeps shielding until the port is real.

## Battery (acceptance)

Outset field items: rupee pickup increments TP wallet by the DONOR amount
(green=1, blue=5... from WW's tables, translated at consumption); heart heals
quarter-hearts (R5 health.unit: no scaling); items despawn/timer per donor;
`item` census rows show WW_ITEM profile, zero TP-ITEM tags.
