# Cel-shade lighting (WW models in TP's engine)

Why WW cel-shade models render **black** in the port's demo path, and the recipe that fixes it.

## ① What the donor does

WW cel-shade models carry a `ZAtoon`/`ZB*` toon texture and are lit by WW's own cel-shade path — which
does **not** run TP's `setLightTevColorType_MAJI`. The donor `demo00` draw has **no MAJI at all**.

## ② Traps

- **TRAP — the port ADDED MAJI to demo00's draw.** `setLightTevColorType_MAJI` on a WW model that has a
  `ZAtoon` texture renders it **black**. This is a port addition the donor never had. (An earlier false
  memory blamed the model; the real cause was the added MAJI — corrected after the user pointed out the
  demo prop `d_act0` was still black while a *different* textured prop, old work, looked fine.)
- **TRAP — brightness multipliers are the wrong lever.** The player reads brighter than the cast not
  because of a multiplier but because he renders through the **MAJI path the cast never runs**. Pinning
  ambient without also skipping MAJI only gets halfway.

## ③ Correct method — the WW-item recipe (L-2)

For a WW cel-shade demo model (texture name starts `ZA`/`ZB`): **skip MAJI** and apply the port's
clothes-bundle recipe — `settingTevStruct(0)` + neutral ambient `(90,90,90,255)` +
`dWwItemmdl_applyBowMaterialAmbientOnly` (the same recipe `d_a_demo_item.cpp` uses for bow/clothes).
Implemented in `src/d/actor/d_a_demo00.cpp` draw via a `demoModelUsesZAtoon()` gate.

- **Skip-MAJI is FAITHFUL** (removing a port addition the donor lacks).
- The **neutral-ambient values are a PORT VALUE** (the port's shared WW-item lighting), not WW's exact
  demo lighting — tracked as **L-2** in [../../WW Linked/port-liberties.md](../../WW%20Linked/port-liberties.md).

## ④ Imperfect

- **L-2 is OPEN.** The `90/90/90` ambient is internally consistent with every other WW item in the port
  but is **not** a donor byte. Reconcile by finding WW's exact demo-model lighting, or ratify as a
  permanent port choice.
- **Confirm visual fixes by screenshot, never by metrics** — this area burned a build declaring "fixed"
  off a log count; the user requires a current screenshot + confirmation for any visual claim.
