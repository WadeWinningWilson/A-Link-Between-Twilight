# Shared library arcs — `Always.arc` / `Dalways.arc`

**What these are.** The donor's two always-resident library archives, `files\res\Object\Always.arc`
and `Dalways.arc`. Housed verbatim at `arcs_lib\` and as `Ww`-prefixed copies at `arcs\WwAlways.arc`
/ `arcs\WwDalways.arc`.

**Why this doc exists.** `anim-ledger` reports both as `no_res_header` orphans. That is **expected,
not a defect** — they are shared libraries with no per-actor enum header, not actors missing one.
More importantly they are a **standing parts bin**: 137 members that several open work items already
need. Recorded so nobody re-derives the contents, and so the parts are findable when the work that
needs them comes up.

> **IVAN RULE note.** These were *not* identified by name. "Always" was initially assumed to mean a
> shared **demo/cutscene animation** archive — that was wrong, and was the same name-based inference
> that caused the §47 `IsleLink_0.aw` violation. `WwAlways`/`WwDalways` appear **nowhere in the WW
> decomp source**; those are our names. Everything below is read from actual arc members.
> **They are prop, effect, and UI libraries — not character animation banks.**

---

## Directly relevant to open work

| member(s) | bears on |
|---|---|
| `rope.bti`, `txm_rope1.bti` | **Outset suspender-rope bridge** — the 2 `bridge` rows on `ACT0` that the plank-instancing actor does not handle. Rope *textures* exist here. **Suspected relevance, not proven** — does not by itself confirm the rope-variant hypothesis. |
| `vlupl.bdl` `.brk` `.btk` | **Vlupy rupee** — the exact case №31 names (WW rupee visual, TP wallet credit). Model + register + texture anims all present. |
| `vhrtl.bdl`, `vkeyl.bdl`, `vhkal.bdl`, `vhutl.bdl`, `vbokl.bdl`, `vchul.bdl`, `vdokl.bdl`, `vesal.bdl`, `vhanl.bdl`, `vhapl.bdl` | the rest of the pickup family, same pattern as Vlupy |
| `txa_taiyo.bti`, `txa_tuki_as/bs/cs/ds.bti` | **sun + 4 moon phases** — bears on the skybox/lighting half of `grass-port-analysis.md` (grass colour comes from `dKy_tevstr_c`, so lighting and grass are one problem) |
| `txa_usonami_*.bti`, `efa_usonami_01.btk`, `b_sea_tex0and2.bti`, `umipt.bti` | sea/wave surface — Outset exterior |
| `boxa`–`boxd`, `boxsea`, `box_shadow`, `it_takara_flash*` **+ 7 `.dzb`** | **treasure chests, with collision** — full set: model, register anim, texture anim, open animation, flash effect, and per-variant DZB |

---

## `Always.arc` — 546,688 b, 108 members

**`.bck` (16)** `fa`, `obm_syougekisw`, `start`, `vbell`, `vbokl`, `vchul`, `vdokl`, `vesal`,
`vhanl`, `vhapl`, `w_pillar_a`, `yj_delete`, `yj_in`, `yj_loop`, `yj_out`, `yj_scale`

**`.bdl` (31)** `arrowitem`, `fa`, `ib`, `krock_00`, `mpa_simi`, `mpi_koishi`, `mpm_tubo`, `mpoda`,
`mpodb`, `nh`, `obm_koisi1`, `obm_koisi2`, `obm_kotubo1`, `obm_ootubo1`, `obm_syougekisw`, `vbell`,
`vbokl`, `vchul`, `vdokl`, `vesal`, `vhanl`, `vhapl`, `vhkal`, `vhrtl`, `vhutl`, `vkeyl`, `vlupl`,
`w_pillar_a`, `ya`, `yazirushi_01`, `ythdr00`

**`.bti` (30)** `ak_houshi00`, `ak_smoke01`, `b_sea_tex0and2`, `b_wyurayura_tex1`, `black_white_2`,
`camera_free`, `camera_self`, `f_shipicon`, `map_black_ci8`, `map_check16_ci4`, `map_frame1`–`4`,
`rope`, `shmref`, `treasurebox`, `txa_lens_32half`, `txa_moya_a_64`, `txa_moya_b_64`,
`txa_ring_a_32hafe`, `txa_taiyo`, `txa_tuki_as`/`bs`/`cs`/`ds`, `txa_usonami_256_64mip`,
`txa_usonami_m_256_64mip`, `txm_rope1`, `umipt`

**`.btk` (12)** `efa_usonami_01`, `ib`, `ice`, `obm_syougekisw`, `vbell`, `vhanl`, `vhkal`, `vhutl`,
`vlupl`, `w_pillar_a`, `ya`, `ythdr00`

**`.brk` (8)** `ib_01`, `ib_02`, `mpa_simi`, `tnh`, `vchul`, `vhanl`, `vlupl`, `ythdr00`

**`.btp` (5)** `mpa_simi`, `mpi_kibako_taru`, `mpi_kinohahen`, `mpi_koishi`, `mpm_tubo`

**`.bpk` (5)** `yj_delete`, `yj_in`, `yj_loop`, `yj_out`, `yj_scale`  · **`.bmt` (1)** `ice`

---

## `Dalways.arc` — 256,704 b, 29 members

**`.bck` (4)** `boxopenbox`, `boxopenshortbox`, `it_takara_flash`, `it_takara_flash2`

**`.bdl` (8)** `box_shadow`, `boxa`, `boxb`, `boxc`, `boxd`, `boxsea`, `it_takara_flash`, `ytrif00`

**`.brk` (6)** `box_shadow`, `boxa`, `boxb`, `boxc`, `it_takara_flash`, `ytrif00`

**`.btk` (4)** `boxa`, `boxb`, `boxc`, `it_takara_flash`

**`.dzb` (7)** `boxa_00`, `boxa_01`, `boxb_00`, `boxb_01`, `boxd_00`, `boxd_01`, `kinb_00`

---

## Standing caveats

- **Purposes above the "relevant to open work" table are inferred from filename patterns and member
  groupings, not from decomp confirmation.** Anything acted on must be confirmed against the decomp
  first. The groupings are a *search index*, not identity claims.
- **№31 still governs.** Presence in a shared library does not license use in a TP space. These are
  WW assets; they go to WW spaces only.
- **The `.dzb` members carry collision** and are therefore subject to the DZB word0 bits 14–23
  adaptation like any other donor collision.
