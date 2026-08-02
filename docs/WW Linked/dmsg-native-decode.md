# Native dMesg — asset census + structure decode (§307 deliverable)

**Lane: Foundry.** Everything History needs to wire the native WW demo-message subsystem
(§307: retire the §201 reconstruction). All receipts are donor source lines
(`D:\XXXXXXX\WW DP\src`) or byte-verified donor data. Full row TSV lives OUTSIDE the repo
(staging, content-neutrality wall): `ww-arc-staging/zel_00_rows.tsv`.

## 1. Staged archives (donor-verbatim, md5-verified, both staging + live mod dir)

| arc | bytes | md5 | contents |
|---|---|---|---|
| `dmsgres.arc` | 13,184 | `0969d17e…` | hukidashi_d00/_d09.blo + 5 BTIs (box, arrow, dot, ring, rays) |
| `bmgres.arc` | 640,672 | `9e55b52e…` | `zel_00.bmg` (ALL message text, 4,411 entries) + `color.bmc` |
| `bmgresh.arc` | 2,848 | `079527b7…` | `zel_01.bmg` (Hylian-language variants) + `color.bmc` |
| `fontres.arc` | 74,560 | `2f352827…` | `rock_24_20_4i_usa.bfn` — THE main text font |
| `rubyres.arc` | 41,824 | `a8d798d4…` | `hyrule.bfn` — US "ruby" font (Hylian glyphs) |
| `menures.arc` | 110,752 | `af25e8ea…` | holds `font_00.bti`/`font_07_02.bti` (outfont A-prompt) + `key.bti` |

Live copies: `model_replacements/WW-Crew-Restoration/arcs/`. №106-clean (donor names, no "ww").

## 2. Donor boot mounts (the d_s_logo pattern History cited)

`d_s_logo.cpp`: `l_dmsgCommand = aramMount("/res/Msg/dmsgres.arc")` (:876);
`l_msgDtCommand = onMemMount("/res/Msg/bmgres.arc")` (:885 — PAL uses
`/res/Msg/data%d/bmgres.arc` keyed by `dComIfGs_getPalLanguage()`, :882);
`l_msgDtCommand2 = onMemMount("/res/Msg/bmgresh.arc")` (:889).

## 3. Text resolution — THE INDEX-BASE ANSWER (§311 CORRECTED)

> **CORRECTION (§311):** the first delivery of this section (§308) said "STB codes are
> BMG message IDS, not row indices" — that was INVERTED (a TSV column-label swap during
> verification, caught on raw-byte re-derivation). The truth, from the raw INF1 bytes:

Text is NOT in dmsgres.arc — it is `zel_00.bmg` inside **bmgres.arc**, fetched once at
`dMesg_parse()`: `JKRGetResource('ROOT', "zel_00.bmg", dComIfGp_getMsgDtArchive())`
(d_mesg.cpp:1805; zel_01 from the h-archive :1808).

**The STB `setMessageCode` value is the INF1 ROW INDEX — a direct index into the
24-byte entry table.** Each entry is a `JMSMesgEntry_c` (f_op_msg_mng.h:22):
`+0 mDataOffs(u32, into DAT1) | +4 mMsgNo(u16) | +6 mItemPrice | +8 mNextMsgNo |
+0x0C mTextboxType | +0x0D mDrawType | +0x0E mTextboxPosition | +0x0F mItemImage |
+0x10 mTextAlignment | +0x11 mInitialSound | +0x12 mInitialCamera | +0x13 mInitialAnimation`.
Raw receipts for the §304 timeline codes (as indices):

| INF1 idx (STB code) | mMsgNo | tboxType | beat |
|---|---|---|---|
| 539 | 2001 | 0 | greeting + try-these-on ({player} insert) |
| 540 | 2002 | 0 | time-flies, old enough for these clothes |
| 3095 | **151** | **9** | **clothes ITEM-GET box** (d09 screen) |
| 541 | 2003 | 0 | "don't look so disappointed" |
| 542 | 2004 | 0 | olden days, boys became men |
| 547 | 2009 | 0 | ours are ways of peace now |
| 543 | 2005 | 0 | family shield on the wall |
| 544 | 2006 | 0 | does all this make sense? |
| 545 | 2007 | 0 | suit you perfectly + tonight |

**4410 stands as History had it**: INF1 index 4410 = the SECOND-QUEST "Hero's New
Clothes" item-get (mMsgNo=156, tboxType=9). The 3095/4410 pairing in the §307 ask was
correct all along; §308's "caution" about it inherited the inversion and is void.

## 3b. Item-get icon resolution (ask (c)) — closed end-to-end

Detection: `mTextboxType == 9` at entry+0x0C (byte-receipted above). Icon, both donor
routes, byte-verified to AGREE:
* `dItem_data::getTexture(mMsgNo − 101)` → 151−101 = **50** = `dItemNo_FUKU_e` (0x32)
* the entry's own `mItemImage` at +0x0F = **0x32 = 50** (second-quest row: 156−101 =
  55 = 0x37 `dItemNo_NEW_FUKU_e`, mItemImage=0x37 ✓)

`item_resource[0x32].mTexture = item_texture_tbl[TEXTURE_IDX_CLOTHES]` = **`clothes.bti`**
(d_item_data.cpp:1405, ordinal 99 in the texture table; NEW_FUKU shares the same
texture). `clothes.bti` is a member of **itemicon.arc** — NOW STAGED donor-verbatim
(176,672 bytes, md5 `be709f64…`, both staging + live mod arcs/). The native path needs
NO WW→port icon mapping: stage the WW arc, read TIMG by name via
`JKRArchive::readTypeResource(buf, 0xC00, 'TIMG', "clothes.bti", <itemicon>)`
exactly as d_mesg.cpp:1540 does.

## 4. The two box screens — pane binding contract (donor source, verbatim)

`dMesg_screenDataTalk_c::createScreen()` (d_mesg.cpp:1337) — **hukidashi_d00.blo**
(talk box; blocks: INF1, PIC1, PAN1, [PIC1, TBX1×4, PIC1×2], EXT1):

| tag | pane | role |
|---|---|---|
| `ms00` | PIC1 `hukidashi_00_d.bti` | box body — gets `setWhite(30,30,30,215)`/`setBlack(30,30,75,0)` |
| `tx02` `txt0` | TBX1 | text rows (main) |
| `tx03` `tx01` | TBX1 | shadow/ruby rows — HIDDEN when `getOptRuby()` (offsetY −4) |
| `yz00` | PIC1 `yazirushi_00_d.bti` | continue arrow |
| `dt00` | PIC1 `dot_02_d.bti` | end-of-message dot |
| (unnamed) | PIC1 `back.bti` | **texture does not exist ANYWHERE in donor files** — ships unresolved; the port's J2D must tolerate a missing named texture without asserting |

`createScreen()` for **hukidashi_d09.blo** (d_mesg.cpp:1499) — the ITEM-GET box
(fukiKind 9 ↔ `_d09`, History's clothes-get bug surface):

| tag | role |
|---|---|
| `ms90` `tx90-93` `yz90` `dt90` | same roles as d00 (body/text/arrow/dot) |
| `item` | PIC1 (placeholder `key.bti`, lives in menures.arc) — **runtime texture swap**: `dItem_data::getTexture(msgEntry.mMsgNo − 101)` read from ItemIconArchive into a 0xC00-byte TIMG buffer, `changeTexture(texBuffer, 0)` (:1540) — the INF1 entry itself carries the item number |
| `rin1` (+`rin2-4` in tree) | ring flare — `ring_loght_d.bti` |
| `lig1`…`lig8` | 8 light rays — `back_light_d.bti` |

Both screens: `scrn->set(<blo>, dComIfGp_getDmsgArchive())`, pane fetch via
`scrn->search('<4cc>')`, fonts applied `setFont(dMesg_gpFont, dMesg_gpRFont)` (:1969-71).

## 5. Fonts — what is NOT resident in the port

| font | file | arc | loader (donor) |
|---|---|---|---|
| main text | `rock_24_20_4i_usa.bfn` | fontres.arc | `mDoExt_initFont0()` m_Do_ext.cpp:2595 (US branch), via `getFontArchive()` |
| "ruby" = Hylian | `hyrule.bfn` | rubyres.arc | `mDoExt_initFont1()` US branch (JPN uses rodb_16_11_4i.bfn — do not stage that) |
| outfont icons | `font_00.bti` `font_07_02.bti` | menures.arc | `dMesg_outFont_c::_create()` d_mesg.cpp:52-58 — resolved BY NAME globally |

Neither .bfn is in the port (TP fonts are different files). `dMsg_Create` ASSERTS both
fonts non-NULL (:2219-2223) — the ruby font must load even though the tale never uses it.
Hylian path irrelevant here: `dMesg_hyrule_language_check` (d_mesg.cpp:1865) lists codes
0x3A3-0x3AE… (Deku Tree/Jabun/Valoo speeches) and clears after first playthrough —
tale ids 539-547/3095 are NOT in the list → plain font, zel_00 only.

## 6. Suspend/release contract

Already delivered in §304 (byte-exact for the tale): message sets code → control-track
`suspend(1)` → box close (`dMesg_closeProc`) → `getControl()->unsuspend(1)`
(d_mesg.cpp:2112). No data-authored releases; the box IS the release owner.

## 7. Font metrics (§310 ask (a)) — the authoritative source

**The port has NO `g_msgHIO` of any kind** — TP never shipped WW's `dMeter_msg_HIO_c`;
the only reference in the whole port is History's own §308 TU. So "read the port's
g_msgHIO" reads nothing: the values cannot come from any resident object today.

**The authoritative WW source is `dMeter_msg_HIO_c`'s CONSTRUCTOR** (d_meter.cpp:371,
class d_meter.h:286). There is no data file: HIO objects are JOR-reflectable dev-tuning
blocks whose retail values ARE the ctor initializers (no JOR host connects on retail).
The message-box fields, ctor-receipted:

| field | ctor value | consumed by |
|---|---|---|
| `field_0x70` | `DEMO_SELECT(0x19, 0x17)` → retail **23** (kiosk demo 25) — §317 CORRECTED: `DEMO_SELECT(DEMO, RETAIL)` picks the SECOND arg on retail (global.h:67); LIVE-CONFIRMED font 23 in every box of DuskTap run 1 | fontSize X=Y (setCommonData :1170-1173); also `mInitFontSize`/`mNowFontSize` seed (d_mesg.cpp:118) |
| `field_0x5a` | **0** | charSpace, main text rows (:1175/1177) |
| `field_0x5c` | **0** | charSpace, shadow/ruby rows (:1176/1178) |
| `field_0x5e` | **30** (0x1e) | lineSpace (:1180-1181); also line-position math in do_tag (:552) |
| `field_0x5` | RGBA(30,30,30,215) | box white tint — createScreen's local `white` replicates it |
| `field_0x9` | RGBA(30,30,75,0) | box black tint — ditto |

**Where WW applies them:** `dMesg_screenData_c::setCommonData()` (d_mesg.cpp:1163) —
called at the END of each `createScreen()` (:1362), NOT every frame. It overrides the
.blo's baked TBX1 values right after `scrn->set(...)` — so **the .blo pane values are
NOT authoritative**; reading them reads pre-override state. Font size is also
runtime-varying (`setNowFontSize(getInitFontSize)` per message, :197; text escapes can
change it) — a literal freezes what the donor treats as state.

**Native shape:** port the `dMeter_msg_HIO_c` ctor verbatim as the WW-side `g_msgHIO`
instance (ext namespace; №106-clean home) and run `setCommonData()` verbatim against
it. Values identical to History's constants — the mechanism (struct + consumer), not
the numbers, is what turns the bridge native.

## 8. Pagination symbols + control tags (§310 ask (b))

**arrow/dot animation is PURE CODE — nothing to stage.** `arwAnime()` (d_mesg.cpp:1231)
is procedural squash-stretch: static tables `scaleX {1.0,1.3,0.8,1.2,1.0}`,
`scaleY {1.0,0.3,1.1,0.8,1.0}`, `step {60,67,71,74,76}` + a 10-frame alpha fade-in.
`dotAnime()` (:1284) is a procedural color/alpha pulse toward (150,150,150)/(255,255,220).
Porting the TU brings the animation; there is no BCK/BRK data.

**Control-tag groups, donor dispatch** (`dMesg_tSequenceProcessor::do_tag`, :429):
group = `tag & 0xFF0000`: `0x00`=text ops (code 0 player-name; 4 wait+stop-input,
u16 arg; 7 timed-wait, u16 frames; 1/2 = `field_0x8c` mode toggle; 10-29 = out-font
icon glyphs), `0x01`=SE (`mDoAud_messageSePlay`), `0x02`=camera tag, `0x03`=anime tag,
`0xFF`=color (the table History already has).

**What the tale's nine rows ACTUALLY author** (raw 0x1A escape census from zel_00.bmg —
escape payloads contain 0x00 bytes, so any null-terminated scan truncates them):

| idx | escapes |
|---|---|
| 539 | player-name ×2, **SE 0x00A7**, **timed-wait 60** (group 0 code 7) |
| 540/541/542/544 | none |
| 543 / 547 | color on/off pairs only |
| 545 | player-name + color pair |
| 3095 | mode toggles (codes 1,2), color pair, **timed-wait 10** |

**No camera tags, no anime tags anywhere in the tale.** Minimum-correct tag surface =
group 0 (codes 0/1/2/7), group 0x01 SE, group 0xFF color — though porting do_tag
verbatim covers all groups by construction (the always-native shape).
