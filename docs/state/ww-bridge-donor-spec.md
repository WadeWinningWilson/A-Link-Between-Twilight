# WW d_a_bridge donor spec — §429 implementation blueprint (extracted 2026-08-04)

Full extraction from WW DP `src/d/actor/d_a_bridge.cpp` (1546 lines) + `d_a_bridge.h` +
`m_Do_ext.cpp` (3Dline) + `d_bg_w_sv.cpp` (vertex-editable collision) + `Bridge.h` arc
indices. The receiver's `d_a_ext_plank_span.cpp` has only plank instancing + count
formula; everything below is the missing 90%. Line refs = donor files.

## Architecture in one paragraph
One actor = up to 50 planks (`br_s[50]`, 0x41C each) hung on a simulated rope: a node
array `m3CC[i]` runs a TWO-PASS constrained chain relaxation each frame (fixed segment
length **75.0**, forward pass from `home.pos`, backward pass from `mEndPos` — the
BACKWARD pass writes each plank's yaw/pitch as the heading to its next node). Sag is a
per-node target-height term, NOT a catenary formula. Collision is ONE `dBgWSv` moving
mesh (`mbrdg.dzb`, authored **4 vertices per plank**: left-low, right-low, left-top,
right-top) whose vertex table is rewritten every frame from the plank edge points, then
`Move()`. Ropes are procedural camera-facing textured strips (`mDoExt_3DlineMat1_c`),
not models (chain-variant uses `obm_chain1.bdl` models instead).

## Key structures (annotated, donor d_a_bridge.h:12-93)
br_s per-plank: mpModel; mpModelRope0/1 (chain posts); mLineMat1 (4 lines × 5 seg
hanger ropes, posts only); m0F8[3]/m11C[3] = right/left anchor sets {[0] rope top
(edge+200Y; chain +1000Y), [1] plank edge (local ±99·scale.x X), [2] edge 30 below =
collision quad lower}; mCyl[2] sword-cut cylinders (r=5 h=1000; rope h=200+shield-off);
m3A0[2] hit-wobble timers (20 cut/15 chain); ropeHP L/R init 3 (dmg>1 → 4); fire
timers 30f + emitters; m3CC sim node; mPosition rendered pos; mRotation (y=heading,
x=pitch, z=roll=m402+m404); mRotationYExtra random 180° flip; m3F0 span profile
|sin(π·i/(count−1))| (chain: 1.0); m3F4/m3F8 rider deflection target/current;
m3FC sag → −15 (cut −50); m400/m402 rider roll; m404 cut lean (±7000); m406 rider
countdown; m408 flags bit0/1 rope intact, bit2 post; mScale x 1.0–1.1 (post 1.05),
y 1.0–1.3, z **1.5**; m418 post timer (chain 0x32, rope −1).
bridge_class: mMoveProcMode 0→2 setup→3 sim (4 snap→5 cut); mLineMat (2×14 main
handrails); mTypeBits param&0xFF (bit0 chain/obm_bridge2/MBRDG2/uncuttable, bit1 aite
broken pair (draws 7/11/15 planks), bit2 model2+no ropes, bit3 TXM_ROPE1+thick);
m02D9=(param>>8)&0xFF cut-wave phase; mPathId=(param>>16)&0xFF (0xFF=error);
mBrCount = dist/((dist>1300?3:0)+47)/1.5... exactly: `delta.abs() / ((fVar1+47.0f)*1.5f)`,
**count>=50 = create ERROR**; m02E0/m02E4 rider energy (wind floor 2.0/0.6);
mpBgW dBgWSv; m02EC/m02EE phases += m02F0(0x578)/m02F2(3000); m02F4/m02F8/m02FC sway
amplitudes = energies; m0300 wobble phase (+=3000, cut +=4000); m0304 snap index;
m0308 severed-post accumulator (>100 ⇒ snap; bomb m3F4<−200 instant); m030C post
count = handrail segment index; mBr[50]; heap **0x2FB60**.
Statics: wind_vec/wy/wp from dKyw each Execute; ita_z_p[11] = {0.1,0.3,0.5,0.75,0.9,
1.0,0.9,0.75,0.5,0.3,0.1} — ±5 neighbor kernel.

## Create (donor :1446-1520) / heap CB (:1315-1444)
- resLoad "Bridge"; path via dPath_GetRoomPath(mPathId, room): point0=home.pos,
  point1=mEndPos; home.angle from delta.
- Posts every 4th plank: rope bridges i≡2 (mod 4) (iVar8=2), chain i≡0 (iVar8=0).
- Rope textures from **Always arc**: rope.bti idx **0x7E**, txm_rope1.bti **0x8D**.
  Models: OBM_BRIDGE 0x4, OBM_BRIDGE2 0x5, OBM_CHAIN1 0x6; **DZB MBRDG 0x9 / MBRDG2
  0xA** (Bridge.arc). Model create flags (data, 0x80000, **0x11020002**).
- dBgWSv: new; Set(dzb,0) (flags MOVE_BG|NO_CALC_VTX|GLOBAL|UNK40 inside);
  SetRideCallback(ride_call_back); CopyBackVtx; all verts := actor pos; Move();
  then dComIfG_Bgsp()->Regist(mpBgW, actor). CullSizeBox(−120,−30,−60,120,30,60),
  cullFar 10.

## Sim (bridge_move case 3, :657-794 + control1 :388-447 / control2 :449-482 / control3 :485-492)
Per frame: node0 := home.pos (chain: −2·m02F8·cos(m02EC) lateral pre-offset).
FORWARD (i=1..n−1): target y `tmp = m3F8*0.5 + (m3FC*m3F0*0.5 + m3CC.y)`; lateral
wave `m02F4*sin(m02EC + i*stride)*m3F0` (stride 4000 if count>10 else 8000) along
bridge-lateral unit; vertical wave `m02FC*sin(m02EE + i*(stride+1000))*m3F0`; sway
offset `m02F8*cos(m02EC)` rotated by home yaw, ×m3F0; wind offset local Z =
`windPow*5` rotated by wind yaw. Build delta → atan/atan2 → node = prev +
rotate(0,0,75) by YrotS(yaw)·XrotM(pitch).
BACKWARD: last node := mEndPos; iterate n−2..0: same target-height; **writes
pBr[1].mRotation.y = yaw, .x = pitch**; node = next + rotate(0,0,75). control3 fixes
plank 0 from node0→node1.
TAPER (case 3 :680-691): residual sp14 = home.pos − node0; per plank
`mPosition = m3CC + sp14 * ((count−i)/count * 0.75)`.
Final matrix (:902-910, 1101-1110): Trans(mPosition) · YrotM(rot.y) · XrotM(rot.x) ·
ZrotM(rot.z) [· snap shove ±30 local Z if m0304] · YrotM(mRotationYExtra).
Anchors from same matrix: local (±scale.x·99, 0, 0) → edges; local y −30 → collision
lowers; rope top = edge + 200Y (chain +1000Y with m418 raise anim).
Springs/decays: rider roll & deflection spread over ±5 neighbors via ita_z_p kernel
(addCalcAngleS2 step 4/0x800; addCalc2 1.0/10.0); cut lean target ±7000 (depth −30,
both −80, step 15); decay m402/m404 →0 (4/0x400); m3FC→−15 (1,5); m3F8→0 (1,5);
amplitudes: m02FC=m02F4=m02E0, m02F8=m02E4; wind floor: m02E0→2.0 (0.1,0.1),
m02E4→0.6·... (`t*0.3`, 0.1, 0.05) when windPow>0.1.

## Rider callback (ride_call_back :29-124), registered on the bgW
plank index = horizDist(to plank0)/**76.5** + 0.5 (clamp). Weights/depths: player
100/−31 (+5f rumble), MO2 150/−40 (speed.y=−20), BK boar 100/−25 (+chase fields),
bomb-near-explode −300 depth + m02E0=20, default 50/−10; riders speed.y=−5 glue.
Lateral offset (plank-local) → m400 = −x·weight; m3F4 += m02FC·sin(m0300)·0.03·weight.
Energy: m02E0 = max(m02E0, |Δ|·0.3·k) cap 20; m02E4 → |Δx| step 0.5 cap 50.
Stiffness k factor 0.85 for chain/no-rope types.

## Collision vertex writer (Execute :1170-1243)
CopyBackVtx; for each dzb vert i: idx=i>>2, corner=i&3 → {0:m11C[2],1:m0F8[2],
2:m11C[1],3:m0F8[1]}; end planks (0, m02DD−1) extend ±50/−40 local Z by plank
yaw/pitch; verts beyond m02DD collapse to last; snapped mode: ALL verts y=10000
(collision off); Move(). dBgWSv::CrrPos/TransPos re-express riders in deformed frame
(d_bg_w_sv.cpp:36-142) — riders track deformation.

## Ropes (Draw :144-386; 3Dline in m_Do_ext.cpp)
Main handrails: mLineMat 2 lines × 14 segs; endpoints local (∓120,350,∓40) from
home/end; interior segs [1..m030C] = post rope tops (or cut interpolation + wind +
himo_cut_control1 dangle sim: 5 nodes, seg 23, gravity −10, wind ×7, :586-618);
update(m030C+2, width **4.0** (bit3: 6.5), color {150,150,150,255}, 0, &tevStr);
dComIfGd_set3DlineMat. Hangers per post: mLineMat1 4×5, per-vertex width **3** (bit3
5), 5 interpolated points edge→top, mid bulge {0,0.7,1.0,0.7,0}×wobble(m3A0·sin(m0300·5)),
HP shrinks mid width (HP≤1→0, ==2→1). Chain: obm_chain1 models aimed edge→top with
hit wobble ±100·sin(m0300·6). 3Dline renderer internals: m_Do_ext.cpp:1880-2515
(billboard = normalize((p1−p0)×(p0−eye))·halfWidth; tex.y += 0.1·segLen; TEVREG2 =
line color, REG0/1 = tevstr C0/K0; GX_TRIANGLESTRIP; dKy_setLight_again + fog).
Submission: dComIfGd_set3DlineMat → mDoExt_3DlineMatSortPacket (xlu sorted).

## Draw order + lighting
Planks: settingTevStruct(TEV_TYPE_BG0) in Execute (:1244); per plank
setLightTevColorType → **dComIfGd_setListBG()** → modelUpdateDL → setList().
Ropes/chains on the NORMAL list. StopDraw cull: >5000 from eye AND behind camera.

## Cut/fire (Execute :944-1092; modes 4/5 :495-584, kikuzu :127-141)
Cylinders at rope edges (gone rope → center −10000Y); ChkTgHit → at_power_check dmg;
HP 3; particles ID_AK_JN_OK / _NG (chain clank); Boko-stick flame within 50 (y-weight
0.4) ignites 30-frame burn → severed; fire emitter ID_AK_SN_BRIDGEROPEFIRE00. Severed
post stood on: m0308+=2, >100 ⇒ snap: mode 4 = sawdust bursts (ID_AK_JN_ELEMENTKIKUZU00)
+ JA_SE_OBJ_SBRIDGE_BREAK + camera shock; mode 5 = split chain relaxation at m0304
(cut_control1/2 :495-584), halves hang from pinned ends, m3FC→−50, floor clamp
m3EC+30, phase +=4000.

## Port order (recommended)
0. STAGE RESOURCES: receiver needs mbrdg.dzb (+ mbrdg2), rope.bti (Always 0x7E —
   verify staged in WwAlways; txm_rope1 0x8D), obm_bridge2/obm_chain1 if variants
   wanted. The 4-verts-per-plank dzb layout is LOAD-BEARING for the vertex writer.
1. Node sim (m3CC + control1/2/3 + taper) → planks positioned/oriented donor-true.
2. dBgWSv port (receiver has dBgW? check d_bg_w_sv presence) + vertex writer + ride
   callback → walkable.
3. 3Dline ropes (receiver mDoExt_3DlineMat1_c — check exists; TP same-lineage).
4. Springs/sway/wind wiring.
5. Cut/fire (optional last; needs particles+sounds).
Receiver adaptation notes: paths — receiver spans come from bridges.ini (authored
RPAT extract), so home.pos/mEndPos = ini endpoints (already done in plank_span);
census arg carries no donor param word — typeBits=0 (rope bridge) for Outset.
