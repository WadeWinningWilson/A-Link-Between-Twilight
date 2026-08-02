# The get-item box (Hero's Clothes)

The clothes-get in Grandma's tale cutscene: an **item box with the clothes icon** showing "You got the
Hero's Clothes!". Getting it there took untangling several message-id drifts and three fukiKind bugs.

## ① What the donor does

The get-item box is **not special** — it is the same message machine with a different screen class,
chosen per-message from BMG **`mTextboxType==9` → `dMesg_screenDataItem_c`** (`d_mesg.cpp:1958-1964`),
same suspend/wait/unsuspend contract as any talk box (see
[../cutscene-work/demo-message-step-in-step.md](../cutscene-work/demo-message-step-in-step.md)). The
item **icon** is BMG-driven (`mItemImage`), independent of the text. The get text is a normal BMG
message; in our restoration it is **catalog[3095]** = "You got the Hero's Clothes!\nThey look like they
might be a little warm for this weather..." (and `[4410]` = the already-have variant).

## ② Traps (message-id drifts — all real, all cost builds)

- **TRAP — `setDemoMessage(151)` showed the FAIRY line.** 151 is a BMG index, and in our BMG that row is
  "You caught a fairy in your bottle!" — the WW clothes get-text is **not in the TP BMG at all**. The
  correct text is **catalog[3095]**, fed via code-text.
- **TRAP — `WEAR_KOKIRI` (0x2F) ≠ donor FUKU id (0x32).** Computing the message as `WEAR_KOKIRI + 0x65`
  gave `0x94` (148) = the magic-armor slot ("You got the magic armor / Malo Mart"). Our item enum is not
  the donor's FUKU id; never derive the donor message from our enum.
- **TRAP — native item box can't read catalog text (the old L-4 debt).** The native box reads BMG by
  code; our text is in the catalog. Early builds therefore rendered the get as a **talk box** (no icon).
  Resolved by code-text injection + forcing the item box (below).
- **TRAP — the three fukiKind bugs (§202):**
  1. a vestigial clear undid the presentation arm before the box opened;
  2. the `mFukiKind=9` override was trapped in the change-block (fukiKind 0→0 never rebuilt);
  3. the idle-poll could clear the flag pre-open.
  See [../dialogue-boxes/traps.md](../dialogue-boxes/traps.md) for the fixes.

## ③ Correct method (§201/§202)

The get is just a demo message with the item box forced:
```cpp
// dExtWw_handleDemoMessage, for donorMsgId 3095/4410:
//   1. do the wear-change (Link dons the clothes)
dWwItemmdl_beginClothesGetPresentation();     // arm: forces mFukiKind → 9 (item box + icon)
//   2. fall through to the native path with catalog[3095] text:
dMsgObject_setWord(catalog[3095]);
fopMsgM_messageSetDemo(4900);                 // native demo box (wait + unsuspend)
```
`talkStartInit` forces `mFukiKind=9` while `dWwItemmdl_isClothesGetPresentation()` is armed
(`d_msg_object.cpp:1450-1452`, §202 moved the check *before* the change gate). The icon is the direct
RGB5A3 48×48 path (`d_msg_scrn_item.cpp`). Text comes from the code-text tag (`mWord`). Wait +
unsuspend are native. **This retired the L-4 item-icon debt** — the get now renders through the real
native item box, not a talk box.

Verify in the log: a `[MsgObject] §65 scrn-create: fukiKind=9` at the 3095 message (there were **zero**
fukiKind=9 before §202).

## ④ Imperfect

- The item box is **forced** by our presentation flag (a port mechanism) because our text is code-text
  (fukiKind 0), not a real `mTextboxType==9` BMG entry. Faithful in outcome, not in data.
- The **item-get jingle / SE and any get-item camera** were not separately verified against the donor.
- Only the clothes-get is wired this way; other WW get-items would each need their catalog text + arm.
