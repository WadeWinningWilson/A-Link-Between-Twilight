# Cut Actors / Demo Restore — live state

**Run:** [Cut-Actors-Demo-Restore-Cursor-History.md](../Interconnected%20Chats/Cut-Actors-Demo-Restore-Cursor-History.md)  
**Control:** [run-control.md](run-control.md)

---

| Field | Value |
|-------|--------|
| **status** | executing — I1-REVISED shipped (Link tevStr + MAJI + setList like leftover viewer); awaiting №13 screenshot |
| **next** | User retest NPC_MK Ivan. If still black → SharedDL bake / setLightTevColorType (non-MAJI) isolation. Slice II after green light |
| **do-not** | Commit WW arcs; WW files in game tree |
| **updated** | 2026-07-18 |

---

## Tip

**№12:** still black; shadow appeared. History MAT3 dump = model healthy → mount light path.

**I1-REVISED shipped:** draw matches leftover viewer — `MAJI(model, &Link->tevStr)` →
`dComIfGd_setList()` → `entryDL` (+ material `change()` before MAJI). Retest Ivan.

**Identity:** Mk=Ivan, Cb=Makar, Md=Medli, P2=pirates. Slice II after lighting green.

**HISTORY RESEARCH (№12c, user's Iron-Boots lead RAN TO GROUND — data-side root cause
found + FIXED):** per-material dump maps the screenshot exactly — `SC_eyeMAT` unlit
(renders → glowing eyes); `fukuMAT`+`skinMAT` lit with **litMask=0x03 (light slots
0+1)** → black. Comparisons: TP Sera body = mask 0xFF; **the WORKING WW-boots overlay
(`Kmdl_13.bmd`) = mask 0x01** — TP's path reliably populates slot 0 only; slot-1
garbage blacks the channel. **Fix shipped in adapt_bdl_arcs.py (`normalize_litmask`):
enabled channels clamped to mask 0x01 (boots-proven); all four arcs REGENERATED in the
mod folder** (Ivan body chan now 0x01, verified). Combined with Cursor's MAJI/tevStr
revision, №13 should be a colored Ivan. Note: the Wind Waker Skins mod = per-member
overlays (`<arc>_<index>.bmd`) drawn lit by their HOST actors — second proven recipe
documented.
