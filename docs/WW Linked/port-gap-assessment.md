# Port gap assessment — props · ship-wave · collision · shadows (Foundry §223)

> User-ordered assessment while the kankyo tap is queued. Every claim cited;
> designs align with the DO-NOT registry (DN-1 collision room-stamping, DN-3
> consume-time BDL law). Ferries at the end.

## 1. Ship-wave particle system — GAP CONFIRMED, and it closes the §209 ambience mystery

- **Donor law (cited):** `d_a_obj_ikada.cpp:327/334` emits
  `ID_AK_JN_SHIPWAVE00 (0x0037)` at the ship's LEFT/RIGHT wave-calc points
  (`mWavePos`/`mWaveRot` + `mWaveLCal`/`mWaveRCal`) every update while moving;
  family: `SHIPIMPACT00 0x34 · SHIPSPLASH00 0x35 · SHIPTAIL00 0x36 ·
  SHIPWAVE00 0x37` (+ `setShipTail` smoke for the bomb variant).
- **All four ids VERIFIED present in `common.jpc`** (gclib check, this session) —
  the same already-staged supplemental bank as windline 0x31.
- **The connection:** §209's donor-only ambient rows included **0x37 and 0x24** —
  the "missing ambience" was largely the ships' wakes. The port's §218 ships are
  socketed MODELS with no wake logic.
- **Ferry → ENGINE:** port the ikada wave-emit block onto the socketed ship
  (per-frame L/R emit while in motion; positions from the model's wave-cal
  points); extend the WW-common bank resolve set `{0x31, 0x3DA, 0x3DB}` →
  `+{0x24, 0x34-0x37}` (the §192 mechanism, same slot). **Verdict instrument
  already live:** `DUSK_EMITTER_TAP` — donor law = wake-rate while a ship moves;
  re-run the emitter census after the change.

## 2. Shadows on models — MOSTLY PRESENT; quality tier is the open question

- **Receiver state (cited):** the ext-mount NPC/prop draw path casts a simple
  ground shadow — `d_ext_npc_mount.cpp` "I3: simple ground shadow (no Acch —
  use up-normal)" → `dComIfGd_setSimpleShadow(pos, y, 50*scale, upNrm, …
  getSimpleTex())`. So every mpMorf-drawn socketed model (NPCs, furniture,
  props, ship) already shadows. Scene/BG models (`mpBgModels`) cast none —
  correct for world geometry.
- **Gaps/tiers:** (a) radius is a flat `50*scale` — donor actors author per-actor
  radii ([liberty], fine until a size mismatch is visible); (b) TP's REAL
  projected shadows (`dDlst_shadowControl_c` real-shadow path, 48+ native users
  per vexp) are not used for socketed models — donor WW used real shadows on
  characters; **[liberty-ledger candidate, not a bug]**: simple-shadow stand-in
  is faithful-in-effect at current fidelity; revisit per-character on visual
  acceptance.
- **No ferry needed now** — ledger the liberty (History), revisit on demand.

## 3. Missing collision — instrument delivered; registration law already exists

- **Donor collision is AUTHORED as static `dCcD_Src*` blocks** in each actor
  (e.g. `d_a_kanban.cpp:1050 dCcD_SrcCyl`), mechanically extractable verbatim →
  NEW `tools/foundry/cc_map.py`; first specs shipped:
  `donor-ccmap-kanban-kb-ikada.md` (sign, pig, ships).
- **The law split that keeps this DN-1-safe:** `cc_` colliders (touch/hit/attack
  cylinders — what cc_map extracts) are NOT DN-1 territory; **standable BG**
  registration IS — and the sanctioned path already exists
  (`resolveIdentityBgHostRoom`, DN-1). Any prop that must be STOOD ON goes
  through that resolver; everything else is a cc_ port from the extracted spec.
- **Ferry → ENGINE/HISTORY:** per restored prop, run `cc_map.py` on its donor
  source → port the block 1:1 onto the socketed actor (cc_ path); flag
  standable cases explicitly for the DN-1 resolver. **Assessment of CURRENT
  gaps:** socketed props today are visual-only unless a case-by-case cc was
  added — the P9 verdict tables (MATCH rows) are the worklist to sweep with
  cc_map.

## 4. Prop restoration — status consolidated (no new gap class)

- **Visual axis:** §218 auto-BTK covers every socketed model; §221 verdict
  proves motion fidelity. Flowers' numbered-btk case = History's open §218 note.
- **Placement axis:** the P9×P10 join is the authoritative status — 16 confirmed
  MATCH placements; MISSING clusters = Tag* (RegionTrig liberty — ledger),
  Salvage family (ocean props — History's restoration call), 4 cast gaps;
  PORT-ONLY rows await the ba.*↔UNK_* mapping (History, in progress).
- **New axes from this assessment:** wake FX (§1) and cc colliders (§3) are the
  two systematic prop gaps; both now have donor law + instruments.

## Ferry summary

| To | Item |
|---|---|
| Engine | ikada wave-emit port + bank-resolve extension {0x24, 0x34-0x37} |
| Engine/History | cc_map sweep over restored props (cc_ 1:1; standables → DN-1 resolver) |
| History | shadow-tier liberty ledger entry; Salvage/Tag* calls (standing) |
| Foundry | emitter-tap re-census after wake lands; kankyo tap → palette differ (standing) |
