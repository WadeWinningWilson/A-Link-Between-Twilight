# Ym1 auto naming map (template_name.py §258 — the ba1 §257 rules, mechanized)

| offset | name | tier | receipt (rule) |
|---|---|---|---|
| 0x8AD | `mSttNum` | [RECEIPT] | setStt arg store |
| 0x8AC | `mOrderType` | [RECEIPT] | eventOrder selector |
| — | `mEventIdx` | [RECEIPT] | event index: NO MATCH (shape differs — name by hand, ba1 §257 method) |
| — | `mEventIdTable` | [RECEIPT] | event id table: NO MATCH (shape differs — name by hand, ba1 §257 method) |
| — | `mBtpNum` | [RECEIPT] | setAnm_tex store: NO MATCH (shape differs — name by hand, ba1 §257 method) |
| — | `mBlinkTimer` | [RECEIPT] | blink timer: NO MATCH (shape differs — name by hand, ba1 §257 method) |
| — | `mBtpFrame` | [RECEIPT] | btp frame: NO MATCH (shape differs — name by hand, ba1 §257 method) |
| — | `mpBtpRes` | [RECEIPT] | btp resource: NO MATCH (shape differs — name by hand, ba1 §257 method) |
| — | `mEvtActionNo` | [RECEIPT] | event action no: NO MATCH (shape differs — name by hand, ba1 §257 method) |
| 0x8A8 | `mAnmAtr` | [INFERENCE] | anm attribute |
| 0x8AE | `mSttNumOld` | [RECEIPT] | stt old |
| 0x8AC | `mSttTimer` | [INFERENCE] | stt timer: COLLIDES with `mOrderType` — resolve by hand |

32 renames applied → `d_a_npc_ym1.draft.cpp`. Rules that NO-MATCHed or COLLIDED need the manual ba1-method pass.
