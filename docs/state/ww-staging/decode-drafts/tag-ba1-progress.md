# d_a_tag_ba1 decode campaign — FULLY MATCHED

Opened and closed 2026-08-22 (same session; SIXTH TU, second consecutive
100). **17/17 exact, unit fuzzy 100.00** (WWDP 6de063b4).

Identity: **Grandma's fairy-revival event tag** — the HIO label decodes
to おばあちゃんタグ ("grandma tag"), the event is "Use_Fairy", and the
XyCheck callback fires when the player selects dItemNo_FAIRY_BOTTLE_e
(0x57) on an item button near the tag. Gating: event bit 0x520 set AND
0x2A20 (already-revived) clear, else the tag creates as ERROR/no-op.

## Decode facts

- eventInfo XyCheckCB/XyEventCB pair: static (void*, int) thunks
  forwarding to s16 methods (CallbackFunc = s16(void*, int) from
  dEvt_info_c); check = getSelectItem(i_itemBtn) == FAIRY_BOTTLE;
  event = returns m290[m292] after zeroing m292 (single-entry event-idx
  array indexed by a selector — donor kept the array form).
- HIO ctor: memcpy(&field_0xc, a_prm_tbl, sizeof) from a 1-byte
  function-local static {0} — a degenerate prm-table copy.
- createInit: attention_info.flags = 8, attention_info.distances[3] =
  26 (the 0x26F byte is the DISTANCES array, not a standalone field),
  getEventIdx(l_evn_tbl[0]) where l_evn_tbl is `static char*[1]`.
- _execute = the yw1 event-tag idiom: runCheck && !checkCommandTalk ->
  getMyStaffId("TagBa1"); on endCheck -> event_reset + fopAcM_delete.

## Levers confirmed by the 100

1. **Guard-return uninverted pair**: `if (!ret) { return ret; }` after
   a bool def emits `bne CONT; b RETURN` (uninverted + branch) — the
   single-condition sibling of the OR-guard lever. Same shape closed
   _create: `if (!createInit()) return cPhs_ERROR_e; return
   cPhs_COMPLEATE_e;` (a ternary emits the inverted beq instead).
2. **Single-variable live-range split**: reusing ONE bool
   (`ret = A; if (!ret) return ret; ret = !B; ... return ret;`) lets
   MWCC split the webs — first web dies in r3, second def lands
   STRAIGHT in r30. Two separate bools force a def-in-r3 + mr copy.
3. attention_info.distances[] carve: bytes at fopAc 0x26C..0x273 are
   the 8-slot distances array; 0x26F = distances[3].
