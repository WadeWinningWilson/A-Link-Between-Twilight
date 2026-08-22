# Outfit / sumo / quick-swap — live state

**Status:** Linkle/Beta overlay+audio fixes shipped (united FST push + per-wave audio). Visual toggle/swap **works**. Two open items: disabled-mod stale pool + boss-cutscene quick-swap crash.  
**Next:** (1) Implement disabled-mod non-live clothes eviction on FST winner change. (2) Triage Midna `J3DModel::calc` crash during event + quick-swap (see crash note below).  
**Do not:** Bake receiver-only guards without donor port proof (DN-10). Same-arc in-place rebuild without DVD remount for post-disable winner (zombie path). Skip `dropCachedNonLiveClothesArcs` silently when `donorBusy` without understanding hold.

**Detail:** [Interconnected Chats/Quick-Sumo Work.md](../Interconnected%20Chats/Quick-Sumo%20Work.md) · [Custom-Model-API-Work.md](../Custom-Model-API-Work.md) §4/§5/§9

---

## Open — disabled Linkle still in quick-switch pool (research plan, not implemented)

**Symptom (user-confirmed, distinct from §9 “live outfit stale until different-arc switch”):** With Linkle **disabled**, repeated quick-swaps still surface her body (often Hero's arc) as if disabled clothes RARCs remain in the ring pool.

**Likely mechanism:**

1. FST flip on disable does not rewrite in-memory `mObjectInfo` clothes arcs; name-cached non-live `Bmdl`/`Kmdl`/… retain old overlay bytes until purged + DVD remount (`dropCachedNonLiveClothesArcs` in `d_albw_sumo_test.cpp`).
2. Quick-switch may **`incCount`** stale archives (global **Kmdl donor** held for Ordon→Hero's) instead of remounting from new FST.
3. `dropCachedNonLiveClothesArcs(liveArc)` evicts non-live only; skipped when `donorBusy` (clothes timer / swap / sumo).
4. Same-arc path in `d_a_alink_swindow.inc` rebuilds in place **without DVD remount** (§9 zombie avoidance).
5. `s_albwForceClothesRemount` latched but **not read** to force remount path.

**Fix direction (when approved):** Post-disable winner change must always evict non-live clothes caches and remount on cross-arc swap; audit `donorBusy` skip + global Kmdl donor holding old bytes; wire `s_albwForceClothesRemount` or equivalent native remount trigger.

**Key files:** `src/d/d_albw_sumo_test.cpp` (~530–563, ~761–787), `src/d/actor/d_a_alink_swindow.inc` (~182–191), `src/dusk/custom_assets.cpp`, `src/dusk/mods/svc/overlay.cpp`.

---

## Open — boss-cutscene quick-swap crash (2026-08-19)

**Log:** `%AppData%\TwilitRealm\Dusklight\logs\dusklight-20260819-125330.log` (crash ~13:07; session restarted as `dusklight-20260819-130922.log`).

| Field | Value |
|-------|--------|
| Build | `v1.4.1-474-dirty` · `8e42fdda` · RelWithDebInfo |
| Reason | `EXCEPTION_ACCESS_VIOLATION` |
| Fault addr | `0x22f00010021` (garbage / UAF-class, not null) |
| Stage | `F_SP102` (King Bulblin chase / title-adjacent) |
| Event | `mEventStatus=1` repeating; `[ExtWw] §50 demo START` at gFrm≈23560; `demoMode=0` at crash window |
| Swap context | `outfit_swap_debug.txt` tail: rapid Ordon(46)/Hero's(47)/Goron(49)/Sumo cycle; `leave force-reload cloth=46`; sumo cap on |
| Last loads before crash | Ordon `bl.bmd` size **165088** (vanilla, not Linkle overlay) |

**Symbolized stack (current exe SYMGEN manifest — RVAs match crash log):**

```
#00 J3DMtxCalcCalcTransformBasic::calcTransform  (rva 0x132840a)
#01 J3DJoint::recursiveCalc                      (0x1328b2e)
#02 J3DJointTree::calc                           (0x1330ed4)
#03 J3DModel::calc                               (0x132ad7f)
#04 daMidna_c::setBodyPartMatrix                 (0x586e43)
#05 daMidna_c::execute                           (0x583ecc)
→ fopAc_Execute → main loop
```

**Read:** Crash is in **Midna's** skeletal calc during `execute`, not Link `modelCalc` directly — but triggered while a **quick-swap clothes reload** was in flight (Ordon BMD load lines immediately precede crash). Related class to Quick-Sumo Open **#B** (demo anim bound across rebuild) but fault addr and top frame differ from documented Link `0x10` / `J3DMtxCalcAnimation::calc` case — may be same lifecycle family affecting Midna, or concurrent swap tearing shared demo/anim state during an active event.

**Fix direction (research):** Reproduce on current build with Midna present; check whether outfit rebuild during `mEventStatus=1` should be blocked vs demo-anim rebind on swap; compare Midna body-part matrix source anim to Link swap teardown order.

**Updated:** 2026-08-19
