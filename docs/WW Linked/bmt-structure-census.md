# BMT structure census (§236 — the pig-shade gap class)

| arc | bmt | TEX1? | MAT3? | bytes | exposure |
|---|---|---|---|---|---|
| `Bk.arc` | `bk_boko.bmt` | NO | YES | 1888 | **MATERIAL-ONLY — any unconditional texture-table swap AVs (§236 law: guard replaceTextures on TEX1)** |
| `Bk.arc` | `bk_ken.bmt` | NO | YES | 1888 | **MATERIAL-ONLY — any unconditional texture-table swap AVs (§236 law: guard replaceTextures on TEX1)** |
| `Bk.arc` | `green.bmt` | YES | YES | 12928 |  |
| `Bk.arc` | `pink.bmt` | YES | YES | 12864 |  |
| `Bm.arc` | `bm02.bmt` | YES | YES | 5504 |  |
| `Bm.arc` | `bmarm02.bmt` | YES | YES | 2560 |  |
| `Kb.arc` | `pg_big_buti.bmt` | YES | YES | 10784 |  |
| `Kb.arc` | `pg_big_kuro.bmt` | YES | YES | 10784 |  |
| `Kb.arc` | `pg_big_pink.bmt` | YES | YES | 10784 |  |
| `Kb.arc` | `pg_buti.bmt` | NO | YES | 1984 | **MATERIAL-ONLY — any unconditional texture-table swap AVs (§236 law: guard replaceTextures on TEX1)** |
| `Kb.arc` | `pg_kuro.bmt` | NO | YES | 1984 | **MATERIAL-ONLY — any unconditional texture-table swap AVs (§236 law: guard replaceTextures on TEX1)** |
| `Kb.arc` | `pg_pink.bmt` | NO | YES | 1984 | **MATERIAL-ONLY — any unconditional texture-table swap AVs (§236 law: guard replaceTextures on TEX1)** |
| `Ko.arc` | `ko02.bmt` | YES | NO | 1248 |  |
| `Mo2.arc` | `mo_blur.bmt` | NO | YES | 1888 | **MATERIAL-ONLY — any unconditional texture-table swap AVs (§236 law: guard replaceTextures on TEX1)** |
| `Mo2.arc` | `mo_green.bmt` | YES | YES | 23776 |  |
| `P1.arc` | `p1b_body.bmt` | YES | YES | 9632 |  |
| `P1.arc` | `p1c_body.bmt` | YES | YES | 9632 |  |
| `P2.arc` | `p2b.bmt` | YES | YES | 10176 |  |
| `P2.arc` | `p2c.bmt` | YES | YES | 10240 |  |
| `WwAlways.arc` | `ice.bmt` | YES | YES | 7776 |  |
| `Ym.arc` | `ym2.bmt` | YES | NO | 8384 |  |

**6 of 21 BMTs are MATERIAL-ONLY.** Donor semantics for these = J3DMatCopyFlag_Material (copy materials, keep the model's own textures).
