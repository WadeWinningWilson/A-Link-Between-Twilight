# TP Link wielding WW sword & shield — scoping (№240)

**Questline position:** the beats AFTER the clothes — Grandma's room already seeds them (`get_shield`
/ `LOOK_SHIELD` co-located with `Ba1_Get_Itm`; ledger №231). The Hero's Sword follows on Aryll's
Lookout path. This scopes how TP Link *holds and uses* them.

**The key framing:** clothes were a TEXTURE swap (whole-body, no function). A weapon is a **held
MODEL that must FUNCTION in combat.** Those are two independent axes — visual and functional — and
the good news is the receiver already separates them for us.

---

## Donor facts (decomp-verified)

- **WW sword** = `dItemNo_SWORD_e` (0x38): arc `VSWON`, model `vswon.bdl`, get-message 0x195.
- **WW shield** = `dItemNo_SHIELD_e` (0x3B): arc `VSHIN`, model `vshin.bdl`, get-message 0x196.
- WW Link holds them as real J3D models (`mpSwordModel1` etc. in `d_a_player_main.cpp`), animated
  with the body — exactly the itemmdl "held model" class.

## Receiver facts (decomp-verified)

- **TP Link already swaps its held sword model** between a fixed set — `mWoodSwordModel` (Ordon),
  `mpSwMModel` (Master), `mpSwAModel` (Ordon-tier) + matching sheaths — chosen by equip checks
  (`checkWoodSwordEquip` / `checkMasterSwordEquip`) in `setSelectEquipItem` (`d_a_alink.cpp:4823`).
  Adding a WW-sword model slot gated by a new check is the SAME pattern, one more branch.
- **BASIC melee is model-independent** — the attack shapes are Link's, not the sword's
  (`setSwordAtCollision` builds `mAtCyl`/`mAtCps` from Link's swing; the §61 grass path). So the
  plain swing/guard rides TP unchanged. **But WW weapons ALSO carry behavior TP lacks** (Mirror
  Shield reflect) — that part is a ported layer, NOT covered by the held-skin. See axis-4 below.
- **The wardrobe ALREADY models weapons as first-class entities**: `dAlbwWardrobe_isActiveSword` /
  `isActiveShield`, per-item ownership, store/retrieve, the D-pad cycle, even the recovery-penalty
  math (`countActiveSwords`). Ownership + storage + swap infrastructure for weapons EXISTS.

---

## The architecture — CORRECTED (user, 2026-07-22)

My first draft said "pure skin over TP combat, no behavior port." **Wrong — that was parking the
donor's state machine as optional, the telescope mistake ([[feedback_port_full_state_machines]]).**
The user's correction, verified in the decomp:

| axis | mechanism | prior art / note |
|---|---|---|
| **Home / own / swap** | the **2nd inventory screen's sword & shield SOCKETS** (`dQe_seedTpBuiltin` page 1, `dQeKind_SwordEquip`/`ShieldEquip`) — WW weapons get NEW entries here. **The TP-native seeding of these sockets is erroneous and gets UNDONE.** | sockets confirmed at `d_ext_mod_flags.cpp:543/567`; currently seed `WOOD_STICK/SWORD/MASTER_SWORD/LIGHT_SWORD` |
| **Held visual** | itemmdl **held-skin** — WW model on Link's held slot | bow Track B (proven) |
| **Get-item** | itemmdl **Track A** — VSWON/VSHIN in the get-item pose | bow / VFUKU precedent |
| **Combat function** | **HYBRID — some WW behavior MUST be ported, not skinned** | see below |

**The combat axis is the correction.** Basic melee (swing, directional guard) rides TP Link's
native AT/guard as a skin — true for the plain Hero's Sword + Hero's Shield. But WW weapons carry
**WW-specific behaviors TP cannot perform**, and those are REQUIRED, not optional:

- **Mirror Shield light reflection** (`d_a_player_main.cpp:9640`, `checkMirrorShieldEquip`): a whole
  subsystem — `mLightCyl` light-TG cylinder, `daPyRFlg0_LIGHT_REFLECT`, a reflected light-vector
  from the shield's matrix, `mFanLightCps` fan-light, hit particles. **TP shields have none of it.**
  It also depends on WW light-beam actors (`d_a_obj_light`, `d_a_arrow_lighteff`, `d_a_tag_light`)
  existing in-world to reflect — a content dependency that only matters where WW light puzzles are.
- Any WW-sword-specific behavior (parry/spin specifics) gets the same treatment when identified.

**So the honest split:**
1. **Outset-questline weapons (Hero's Sword + Hero's Shield, BASIC):** held-skin + TP melee/guard —
   the immediate, cheap need. Works now via the composed systems.
2. **WW-specific weapon BEHAVIOR (Mirror Shield reflect, etc.):** a ported behavior LAYER on TP
   Link (sumo-outfit-style state), gated to when that weapon AND its puzzle content arrive. The
   donor's `checkMirrorShieldEquip` block is the spec; port it whole, don't skin over it.

---

## Design decisions — ANSWERED (user, 2026-07-22)

1. **No replace.** WW weapons do NOT replace TP's native sword/shield. They occupy NEW slots in the
   **2nd inventory screen's sword/shield sockets** (currently mis-seeded with TP natives — undo
   that seeding). WW weapons coexist as their own equippable entries there.
2. **WW behavior REQUIRED, not skin.** The shield must do what WW shields do — Mirror Shield light
   reflection is the flagship TP cannot replicate. Behavior is ported, per axis-4 above.
3. **Master Sword follows (1)** — a socket entry, not a tier replacement, when/if that beat arrives.

## Covenant

- VSWON/VSHIN sourced from adapted mod arcs (like Vfuku), never compiled in; no WW names in exe.
- Held-skin + get-item both ride the itemmdl pipeline — **Cursor's lane** (Wind Clau review-only).
  This scoping is the design; the impl is a coordination item when the beat is reached.
- Basic melee stays TP's; WW-specific WEAPON behavior (Mirror Shield reflect) is a ported layer on
  TP Link (sumo-outfit-style state), distinct from the cc AT/TG ENEMY-combat channel (that's
  [[interactions/README]], for when the first WW enemy lands). Two different behavior ports.

## Sequence when the beat activates

1. Adapt VSWON / VSHIN into the mod arcs (Vfuku precedent).
2. Cursor: held-skin slot for sword & shield (bow Track B generalization) + get-item (Track A).
3. History: wardrobe grant + a `grant_weapon:` dialogue action mirroring `grant_outfit:` (№238),
   wired to the `get_shield` / sword event beats.
4. Confirm BASIC combat unchanged (skin doesn't touch AT shapes).
5. For a WW weapon with distinct behavior (Mirror Shield): port `checkMirrorShieldEquip`'s block
   whole as an equipped-state layer, WITH its WW light-actor dependencies — gated to that content.
