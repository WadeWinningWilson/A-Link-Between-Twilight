# Yw1 auto naming map (template_name.py §258 — the ba1 §257 rules, mechanized)

| offset | name | tier | receipt (rule) |
|---|---|---|---|
| 0x7C2 | `mSttNum` | [RECEIPT] | setStt arg store |
| 0x7C1 | `mOrderType` | [RECEIPT] | eventOrder selector |
| — | `mEventIdx` | [RECEIPT] | event index: NO MATCH (shape differs — name by hand, ba1 §257 method) |
| — | `mEventIdTable` | [RECEIPT] | event id table: NO MATCH (shape differs — name by hand, ba1 §257 method) |
| — | `mBtpNum` | [RECEIPT] | setAnm_tex store: NO MATCH (shape differs — name by hand, ba1 §257 method) |
| — | `mBlinkTimer` | [RECEIPT] | blink timer: NO MATCH (shape differs — name by hand, ba1 §257 method) |
| — | `mBtpFrame` | [RECEIPT] | btp frame: NO MATCH (shape differs — name by hand, ba1 §257 method) |
| — | `mpBtpRes` | [RECEIPT] | btp resource: NO MATCH (shape differs — name by hand, ba1 §257 method) |
| — | `mEvtActionNo` | [RECEIPT] | event action no: NO MATCH (shape differs — name by hand, ba1 §257 method) |
| 0x7BD | `mAnmAtr` | [INFERENCE] | anm attribute |
| — | `mSttNumOld` | [RECEIPT] | stt old: NO MATCH (shape differs — name by hand, ba1 §257 method) |
| 0x7C8 | `mSttTimer` | [INFERENCE] | stt timer |

30 renames applied → `d_a_npc_yw1.draft.cpp`. Rules that NO-MATCHed or COLLIDED need the manual ba1-method pass.
