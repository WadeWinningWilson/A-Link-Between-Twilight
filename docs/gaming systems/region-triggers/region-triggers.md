# Region triggers (TagEv 0x0A) — and the geometry saga

How a WW region volume fires an event (the tale cutscene's "walk to the loft" trigger), and the
mis-scaling that cost the most tuning time of any single value.

## ① What the donor does

WW's `d_a_tag_event` type **0x0A** fires a named event when Link enters a box, while a story flag is
set, then sets a done-flag so it can't re-fire. Geometry (`d_a_tag_event.cpp:242-263`, `actionHunt`):
```
in-box == (Link - center).abs2XZ() < xz_radius^2   AND   |Link.y - center.y| <= y_halfband
```
Both radii come from the TagEv SCOB **scale**, and the **SCOB scale byte convention is `scale/10`**
(byte 10 → float 1.0):
```
xz_radius  = scale.x*100 = (scaleByte.x/10)*100
y_halfband = scale.y*100 = (scaleByte.y/10)*100
```

## ② Traps

- **TRAP — the SCOB scale byte mis-scale (the saga).** I first claimed the donor was `1000/400` and
  "tuned" to `300/80` at a moved centre. **Both wrong.** Parsing `LinkRM/Room0 room.dzr` directly, the
  tale `TagEv` (param `0x02FF110A`) is pos `(-413.6,375,314.7)`, scaleBytes `(10,4,10)` →
  **xz_radius=100, y_halfband=40**. My `1000/400` was a `byte×100` mis-scale (should be `byte/10×100`).
  The whole tuning had been compensating for a bad number. **Lesson:** parse the DZR/SCOB directly and
  apply the byte convention; do not eyeball-tune geometry.
- **TRAP — a wrong number reads as a different bug.** The `1000/400` box fired on the ground floor, which
  looked like an *arming* problem. It wasn't — the donor y-band 40 already excludes the ground floor.
  The entry-fire bug was purely the wrong radius.

## ③ Correct method

Apply the **donor bytes verbatim**. The tale loft trigger (`population/region_triggers.ini`
`[tale_loft]`) is donor byte-exact: center `(-413.6,375,314.7)`, `xz_radius=100`, `y_halfband=40`,
`arm_if ba.tale_window`, `arm_unless ba.clothes_given`, `done ba.clothes_given`. Gates are **folder-side
flag strings** (`dExtModFlags`), never raw WW `UNK_` bits in code/data.

## ④ Imperfect — the arming chain (L-1 reconciled, L-3 open)

- **L-1 (geometry): RECONCILED** — donor byte-exact, no port value remains.
- **L-3 (arming): OPEN.** The donor arms the tale (type-0x0A, gated on `UNK_0E20`) via a **3-stage TagEv
  chain**: Beat B (`A_mori` forest, type 0x2 → `UNK_0101`) → Beat C (`sea`/Outset room 44, type 0x3 →
  `UNK_0E20`) → gates the tale. **`A_mori` is not yet restored**, so `UNK_0101` has no faithful setter.
  Our port **auto-arms `ba.tale_window` on R_DL01 entry** as a stand-in. Faithful-in-effect for now
  (small loft box; in normal play the Outset beat precedes house entry), but the full chain must be
  authored when A_mori is restored, deleting the auto-arm. Tracked as **L-3** in
  [../../WW Linked/port-liberties.md](../../WW%20Linked/port-liberties.md).
