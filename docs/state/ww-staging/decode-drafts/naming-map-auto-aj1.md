# Aj1 auto naming map (template_name.py §258 — the ba1 §257 rules, mechanized)

| offset | name | tier | receipt (rule) |
|---|---|---|---|
| 0x7BB | `mSttNum` | [RECEIPT] | setStt arg store |
| 0x7BA | `mOrderType` | [RECEIPT] | eventOrder selector |
| 0x746 | `mEventIdx` | [RECEIPT] | event index |
| 0x744 | `mEventIdTable` | [RECEIPT] | event id table |
| — | `mBtpNum` | [RECEIPT] | setAnm_tex store: NO MATCH (shape differs — name by hand, ba1 §257 method) |
| — | `mBlinkTimer` | [RECEIPT] | blink timer: NO MATCH (shape differs — name by hand, ba1 §257 method) |
| — | `mBtpFrame` | [RECEIPT] | btp frame: NO MATCH (shape differs — name by hand, ba1 §257 method) |
| — | `mpBtpRes` | [RECEIPT] | btp resource: NO MATCH (shape differs — name by hand, ba1 §257 method) |
| — | `mEvtActionNo` | [RECEIPT] | event action no: NO MATCH (shape differs — name by hand, ba1 §257 method) |
| 0x7B6 | `mAnmAtr` | [INFERENCE] | anm attribute |
| 0x7BC | `mSttNumOld` | [RECEIPT] | stt old |
| 0x7BA | `mSttTimer` | [INFERENCE] | stt timer: COLLIDES with `mOrderType` — resolve by hand |

49 renames applied → `d_a_npc_aj1.draft.cpp`. Rules that NO-MATCHed or COLLIDED need the manual ba1-method pass.
