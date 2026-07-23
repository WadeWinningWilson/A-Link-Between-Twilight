# Mass-interaction recipe — the proven TP↔WW point-query pattern

**Proven by:** grass cutting (ledger №220–№229). Grass is the reference; this recipe is the
reusable pattern any "did an attack/contact reach this point" interaction copies.

**When to use:** a WW-restored actor needs to know an attack shape (sword, bomb, arrow) or a body
(walk-through) reached a point — WITHOUT full damage routing. Cuttable grass, breakable pots,
dig-spots, contact triggers. For actual HP damage, that is the cc AT/TG channel, not this.

---

## The mechanism (both engines share it — verified WW `d_grass.cpp` vs TP)

The collision system carries a small **mass list** each frame. Attackers REGISTER their attack/body
shapes into it (`SetMass`); pollers ASK "is any registered shape at this point?" (`ChkMass`). The
engine clears the list at frame end. It is a spatial point-query, nothing more.

```
Attacker (Link, bomb, arrow):  dComIfG_Ccsp()->SetMass(&atShape, 1)      // per frame, from execute
Poller  (our WW actor):        dComIfG_Ccsp()->PrepareMass()             // build the spatial index
                               dComIfG_Ccsp()->SetMassAttr(r, h, 11, 0)  // the TEST volume size
                               ret = ChkMass(&point, &hitActor, &hitInf) // bit0=AT, bit2=CO(body)
                               // ...react...
                               // (DO NOT MassClear — see pitfall 3)
```

`ChkMass` return bits: **bit 0 = AT** (an attack reached the point), **bit 1 = CO** (a body/contact
overlap — the walk-through feed). Filter carried objects (`fopAcM_GetName != Carry`) so a thrown pot
doesn't mow the field.

---

## THE FOUR PITFALLS (each cost a debug round; each is a receiver-vs-donor lesson)

Every one of these came from importing the DONOR's numbers/placement into the RECEIVER's engine.
The rule they share: **the donor is the spec for BEHAVIOR; the receiver is the spec for FRAME
PLUMBING.** (Mirror of №173's flags lesson, inverted.)

**1. Poll at the RECEIVER's frame slot, not the donor's (№224).**
WW's grass manager runs at process List ID 7; TP's own grass poller runs at **List ID 11** — after
every mass feeder in the frame. Poll at 7 and the list is still empty (247/247 polls read massN=0);
poll at 11 and Link's shapes are there. → Set the actor profile's List ID to the receiver's
equivalent poller, not the donor's. Draw priority is a separate field — leave it.

**2. Size the TEST VOLUME to the receiver's attack shapes, not the donor's (№225).**
WW sized its grass test cylinder 40×**80**; TP sized its own 40×**120** because TP Link's swing
planes ride higher than WW's. At 80, only a low wide spin sphere connected; slashes passed over. →
Use the receiver's own value for the same content class (`d_grass.inc:1226` = 40×120 for grass).

**3. Do NOT `MassClear` per actor (№226).**
`MassClear` wipes the GLOBAL list. The native manager may clear because it is the SOLE poller of its
stage; a restoration has MANY actors of the same kind. The first in execute order sees the shapes,
reacts, then wipes the list — starving every actor behind it, forever (symptom: exactly ONE patch
ever cut). → Let the engine's own frame-end clear do it; never clear from a per-actor poll.

**4. "No line at all" must be a DIAGNOSTIC outcome, not silence (probe-design law).**
An early-return before the probe makes "gate closed" indistinguishable from "player not near." Log
the gate. (This is the [[feedback_multi_hypothesis_logging]] LAW applied to interaction probes.)

---

## The consequence layer (grass specifics, but the shape generalizes)

Once a hit is detected, the donor's `WorkAt` does three things — port each with the RECEIVER's API:
1. **Particle** — a SCENE particle (`0x89D7`), which the host stage's own bank may lack. The mod's
   supplemental-archive path (`ensureTearSceneRes` → Pscene011, general fallback) already resolves
   any missing scene particle (№229). Kick it at actor create.
2. **State change / visual** — grass draws a STUB DL after the cut (`mAnimIdx>=0 ? full : cut`),
   persisting until reload (№227). Cut data is NOT deleted. Analog for any reactable prop: a
   post-interaction visual variant, not a vanish.
3. **Drops** — COVENANT-GATED OFF by default (№225): a TP-model reward in WW space breaks №31.
   Re-enable only through the mod's own WW-visual reward path.

---

## Checklist for the NEXT mass interaction

- [ ] Actor profile List ID = the receiver's poller slot for that content class (not the donor's)
- [ ] `SetMassAttr` = the receiver's test volume for that class
- [ ] No per-actor `MassClear`
- [ ] Filter carried objects on AT hits
- [ ] Consequence uses receiver APIs; scene particles via the supplemental path
- [ ] Any reward is WW-visual or gated off (covenant)
- [ ] Probe logs its own gate (silence must never be ambiguous)

---

## What grass did NOT yet port (tracked, not parked — [[feedback_port_full_state_machines]])

`WorkCo` walk-through bend (16 anim slots + recovery + rustle SE), `WorkAt_NoCutAnim`
(wind/arrow/hookshot push without cut), `setBatta` (grasshopper spawn). Detection for all of these
already works (the CO feed registers); the animation/spawn layers are the remaining donor state.
