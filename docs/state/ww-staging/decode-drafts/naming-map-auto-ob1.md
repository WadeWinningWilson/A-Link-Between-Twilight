# Ob1 auto naming map (template_name.py §258 — the ba1 §257 rules, mechanized)

| offset | name | tier | receipt (rule) |
|---|---|---|---|
| 0x805 | `mSttNum` | [RECEIPT] | setStt arg store |
| 0x804 | `mOrderType` | [RECEIPT] | eventOrder selector |
| 0x7D2 | `mEventIdx` | [RECEIPT] | event index |
| 0x7D0 | `mEventIdTable` | [RECEIPT] | event id table |
| 0x802 | `mBtpNum` | [RECEIPT] | setAnm_tex store |
| 0x6F2 | `mBlinkTimer` | [RECEIPT] | blink timer |
| 0x6F0 | `mBtpFrame` | [RECEIPT] | btp frame |
| 0x330 | `mpBtpRes` | [RECEIPT] | btp resource |
| 0x805 | `mEvtActionNo` | [RECEIPT] | event action no: COLLIDES with `mSttNum` — resolve by hand |
| 0x800 | `mAnmAtr` | [INFERENCE] | anm attribute |
| — | `mSttNumOld` | [RECEIPT] | stt old: NO MATCH (shape differs — name by hand, ba1 §257 method) |
| 0x7D6 | `mSttTimer` | [INFERENCE] | stt timer |

106 renames applied → `d_a_npc_ob1.draft.cpp`. Rules that NO-MATCHed or COLLIDED need the manual ba1-method pass.
