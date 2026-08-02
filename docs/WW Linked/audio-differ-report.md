# WW audio differ — per-track opcode/bank coverage vs the port parser (§264)

## 1. DISPATCH DIFF — port Cmd_Process vs donor sCmdPList (the systemic findings)

| op | donor handler | port handler | verdict |
|---|---|---|---|
| 0xC2 | OpenTrackBros | (default NOP) | NOP'd in port (donor `OpenTrackBros`) |
| 0xCB | ReadPort | (default NOP) | NOP'd in port (donor `ReadPort`) |
| 0xCC | WritePort | (default NOP) | NOP'd in port (donor `WritePort`) |
| 0xCD | CheckPortImport | (default NOP) | NOP'd in port (donor `CheckPortImport`) |
| 0xCE | CheckPortExport | (default NOP) | NOP'd in port (donor `CheckPortExport`) |
| 0xD1 | ParentWritePort | (default NOP) | NOP'd in port (donor `ParentWritePort`) |
| 0xD2 | ChildWritePort | (default NOP) | NOP'd in port (donor `ChildWritePort`) |
| 0xD4 | SetLastNote | (default NOP) | NOP'd in port (donor `SetLastNote`) |
| 0xD5 | TimeRelate | (default NOP) | NOP'd in port (donor `TimeRelate`) |
| 0xD6 | SimpleOsc | (default NOP) | NOP'd in port (donor `SimpleOsc`) |
| 0xD7 | SimpleEnv | (default NOP) | NOP'd in port (donor `SimpleEnv`) |
| 0xDB | OutSwitch | (default NOP) | NOP'd in port (donor `OutSwitch`) |
| 0xDC | UpdateSync | (default NOP) | NOP'd in port (donor `UpdateSync`) |
| 0xDD | BusConnect | (default NOP) | NOP'd in port (donor `BusConnect`) |
| 0xDE | PauseStatus | (default NOP) | NOP'd in port (donor `PauseStatus`) |
| 0xDF | SetInterrupt | (default NOP) | NOP'd in port (donor `SetInterrupt`) |
| 0xE0 | DisInterrupt | (default NOP) | NOP'd in port (donor `DisInterrupt`) |
| 0xE1 | ClrI | (default NOP) | NOP'd in port (donor `ClrI`) |
| 0xE2 | SetI | (default NOP) | NOP'd in port (donor `SetI`) |
| 0xE3 | RetI | (default NOP) | NOP'd in port (donor `RetI`) |
| 0xE4 | IntTimer | (default NOP) | NOP'd in port (donor `IntTimer`) |
| 0xE5 | VibDepth | (default NOP) | NOP'd in port (donor `VibDepth`) |
| 0xE6 | VibDepthMidi | (default NOP) | NOP'd in port (donor `VibDepthMidi`) |
| 0xE7 | SyncCPU | (default NOP) | NOP'd in port (donor `SyncCPU`) |
| 0xE8 | FlushAll | (default NOP) | NOP'd in port (donor `FlushAll`) |
| 0xE9 | FlushRelease | (default NOP) | NOP'd in port (donor `FlushRelease`) |
| 0xEA | Wait2 | (default NOP) | NOP'd in port (donor `Wait2`) |
| 0xEB | PanPowSet | (default NOP) | NOP'd in port (donor `PanPowSet`) |
| 0xEC | IIRSet | (default NOP) | NOP'd in port (donor `IIRSet`) |
| 0xED | FIRSet | (default NOP) | NOP'd in port (donor `FIRSet`) |
| 0xEE | EXTSet | (default NOP) | NOP'd in port (donor `EXTSet`) |
| 0xEF | PanSwSet | (default NOP) | NOP'd in port (donor `PanSwSet`) |
| 0xF0 | OscRoute | (default NOP) | NOP'd in port (donor `OscRoute`) |
| 0xF1 | IIRCutOff | (default NOP) | NOP'd in port (donor `IIRCutOff`) |
| 0xF2 | OscFull | (default NOP) | NOP'd in port (donor `OscFull`) |
| 0xF3 | VolumeMode | (default NOP) | NOP'd in port (donor `VolumeMode`) |
| 0xF4 | VibPitch | (default NOP) | NOP'd in port (donor `VibPitch`) |
| 0xFA | CheckWave | (default NOP) | NOP'd in port (donor `CheckWave`) |
| 0xFB | Printf | (default NOP) | NOP'd in port (donor `Printf`) |
| 0xFC | Nop | (default NOP) | NOP'd in port (donor `Nop`) |

## 2. Per-track coverage (ALL donor BMS; counts are SITES, statically reachable)

| track | sites | NOP'd sites | NOP % | top NOP'd ops | bank/prog writes | truncated paths |
|---|---|---|---|---|---|---|
| `baachan.bms` | 5329 | 27 | 1% | `E6_VibDepthMidi`x21, `E7_SyncCPU`x6 | r32=0, r33=0, r33=1, r33=4, r33=5, r33=7 | 1 |
| `bigpow.bms` | 4415 | 14 | 0% | `E7_SyncCPU`x14 | r32=1, r32=76, r33=0, r33=1, r33=2, r33=3 | 1 |
| `bird.bms` | 9678 | 15 | 0% | `E7_SyncCPU`x15 | r32=1, r32=68, r33=0, r33=1, r33=2, r33=3 | 1 |
| `boco.bms` | 3873 | 16 | 0% | `E7_SyncCPU`x16 | r32=1, r32=65, r33=0, r33=1, r33=2, r33=3 | 1 |
| `death_vl.bms` | 443 | 6 | 1% | `E7_SyncCPU`x3, `E6_VibDepthMidi`x3 | r7=2, r32=0, r32=12, r33=0, r33=8, r33=9 | 0 |
| `defaultse.bms` | 512 | 153 | 30% | `CC_WritePort`x82, `CB_ReadPort`x34, `EF_PanSwSet`x10, `DF_SetInterrupt`x8 | r0=0, r0=1, r1=0, r8=0, r9=0, r10=1 | 8 |
| `dekpinch.bms` | 1039 | 10 | 1% | `E7_SyncCPU`x10 | r32=0, r32=7, r33=2, r33=7, r33=21, r33=60 | 0 |
| `die_link.bms` | 738 | 6 | 1% | `E7_SyncCPU`x3, `E6_VibDepthMidi`x3 | r7=2, r32=0, r33=2, r33=4, r33=21 | 0 |
| `drug.bms` | 3651 | 12 | 0% | `E7_SyncCPU`x6, `E6_VibDepthMidi`x6 | r7=2, r32=9, r33=0, r33=1, r33=2, r33=3 | 0 |
| `d_amosu.bms` | 8093 | 14 | 0% | `E7_SyncCPU`x14 | r32=1, r32=71, r33=0, r33=1, r33=2, r33=3 | 1 |
| `d_earth.bms` | 14887 | 10 | 0% | `E7_SyncCPU`x10 | r32=62, r33=0, r33=1, r33=2, r33=3, r33=4 | 0 |
| `d_forest.bms` | 4542 | 20 | 0% | `E7_SyncCPU`x14, `E6_VibDepthMidi`x6 | r32=0, r32=64, r33=0, r33=1, r33=2, r33=4 | 0 |
| `d_ganon1.bms` | 13151 | 21 | 0% | `E7_SyncCPU`x16, `E6_VibDepthMidi`x5 | r7=2, r7=5, r32=0, r32=59, r33=0, r33=1 | 0 |
| `d_ganon2.bms` | 7082 | 4 | 0% | `E7_SyncCPU`x4 | r32=58, r33=0, r33=1, r33=2, r33=3 | 0 |
| `d_ryumt.bms` | 4501 | 14 | 0% | `E7_SyncCPU`x7, `E6_VibDepthMidi`x7 | r7=2, r7=5, r32=4, r33=0, r33=1, r33=2 | 0 |
| `d_wind.bms` | 9187 | 11 | 0% | `E7_SyncCPU`x11 | r32=61, r33=0, r33=1, r33=2, r33=3, r33=4 | 0 |
| `elf.bms` | 2521 | 217 | 9% | `E6_VibDepthMidi`x205, `E7_SyncCPU`x8, `F4_VibPitch`x4 | r7=2, r32=0, r32=8, r33=0, r33=2, r33=3 | 0 |
| `enemy2.bms` | 8097 | 58 | 1% | `CB_ReadPort`x21, `CC_WritePort`x19, `D2_ChildWritePort`x9, `E7_SyncCPU`x9 | r32=1, r33=0, r33=1, r33=2, r33=3, r33=5 | 2 |
| `enemy_comes.bms` | 542 | 8 | 1% | `E7_SyncCPU`x8 | r32=1, r32=67, r33=0, r33=3, r33=4, r33=5 | 0 |
| `e_dioct2.bms` | 5269 | 12 | 0% | `E7_SyncCPU`x12 | r32=0, r32=63, r33=0, r33=2, r33=3, r33=6 | 0 |
| `e_diocta.bms` | 5749 | 12 | 0% | `E7_SyncCPU`x12 | r32=0, r32=63, r33=0, r33=2, r33=3, r33=6 | 0 |
| `e_ganon.bms` | 19034 | 269 | 1% | `CB_ReadPort`x239, `D2_ChildWritePort`x14, `E7_SyncCPU`x14, `CC_WritePort`x2 | r32=0, r32=1, r32=63, r33=0, r33=2, r33=3 | 0 |
| `find_ttr.bms` | 628 | 12 | 2% | `E7_SyncCPU`x6, `E6_VibDepthMidi`x6 | r7=2, r32=0, r33=1, r33=2, r33=4, r33=6 | 0 |
| `get.bms` | 1171 | 12 | 1% | `E7_SyncCPU`x6, `E6_VibDepthMidi`x6 | r7=12, r32=0, r33=10, r33=11, r33=12, r33=13 | 0 |
| `get_h.bms` | 961 | 22 | 2% | `E7_SyncCPU`x11, `E6_VibDepthMidi`x11 | r7=2, r32=0, r33=0, r33=1, r33=3, r33=9 | 0 |
| `get_perl.bms` | 847 | 7 | 1% | `E7_SyncCPU`x7 | r32=0, r33=2, r33=6, r33=10, r33=11, r33=14 | 0 |
| `get_s.bms` | 766 | 16 | 2% | `E7_SyncCPU`x8, `E6_VibDepthMidi`x8 | r7=12, r32=0, r33=10, r33=11, r33=12, r33=13 | 0 |
| `get_song.bms` | 567 | 20 | 4% | `E7_SyncCPU`x10, `E6_VibDepthMidi`x10 | r7=12, r32=0, r33=0, r33=2, r33=4, r33=6 | 0 |
| `goma_a.bms` | 4178 | 16 | 0% | `E7_SyncCPU`x8, `F1_IIRCutOff`x8 | r32=1, r32=67, r33=0, r33=2, r33=3, r33=4 | 1 |
| `goma_b.bms` | 5321 | 31 | 1% | `F1_IIRCutOff`x20, `E7_SyncCPU`x11 | r32=1, r32=67, r33=0, r33=2, r33=3, r33=4 | 1 |
| `house.bms` | 1978 | 40 | 2% | `E6_VibDepthMidi`x32, `E7_SyncCPU`x6, `F4_VibPitch`x2 | r7=2, r32=0, r33=2, r33=5, r33=6, r33=7 | 0 |
| `house_g.bms` | 1586 | 16 | 1% | `E7_SyncCPU`x8, `E6_VibDepthMidi`x8 | r7=2, r32=0, r32=2, r33=2, r33=5, r33=6 | 0 |
| `house_t.bms` | 765 | 21 | 3% | `E7_SyncCPU`x9, `E6_VibDepthMidi`x9, `CC_WritePort`x2, `CB_ReadPort`x1 | r7=2, r32=0, r32=2, r33=2, r33=3, r33=6 | 1 |
| `house_t2.bms` | 1018 | 12 | 1% | `E7_SyncCPU`x6, `E6_VibDepthMidi`x6 | r7=2, r32=0, r32=2, r33=3, r33=6, r33=9 | 0 |
| `hyrul_of.bms` | 105 | 5 | 5% | `E7_SyncCPU`x3, `E6_VibDepthMidi`x2 | r32=60, r33=10, r33=11, r33=12 | 0 |
| `hyrul_on.bms` | 3318 | 9 | 0% | `E7_SyncCPU`x9 | r32=60, r33=0, r33=1, r33=2, r33=3, r33=4 | 0 |
| `i_link.bms` | 5215 | 41 | 1% | `E6_VibDepthMidi`x26, `E7_SyncCPU`x12, `F4_VibPitch`x3 | r7=2, r7=12, r32=0, r32=2, r33=0, r33=1 | 0 |
| `i_link2.bms` | 5215 | 41 | 1% | `E6_VibDepthMidi`x26, `E7_SyncCPU`x12, `F4_VibPitch`x3 | r7=2, r7=12, r32=0, r32=2, r33=0, r33=1 | 0 |
| `i_link3.bms` | 4177 | 20 | 0% | `E7_SyncCPU`x10, `E6_VibDepthMidi`x10 | r7=2, r7=12, r32=0, r32=2, r33=0, r33=1 | 0 |
| `i_linkf.bms` | 443 | 6 | 1% | `E7_SyncCPU`x3, `E6_VibDepthMidi`x3 | r7=2, r32=0, r32=2, r33=1, r33=8, r33=9 | 0 |
| `i_linkin.bms` | 7467 | 62 | 1% | `E6_VibDepthMidi`x45, `E7_SyncCPU`x14, `F4_VibPitch`x3 | r7=2, r7=12, r32=0, r32=2, r33=0, r33=1 | 0 |
| `i_maju.bms` | 8588 | 16 | 0% | `E7_SyncCPU`x8, `E6_VibDepthMidi`x8 | r7=2, r32=0, r32=3, r33=0, r33=1, r33=2 | 0 |
| `i_maju_j.bms` | 756 | 16 | 2% | `E7_SyncCPU`x8, `E6_VibDepthMidi`x8 | r7=2, r7=9, r32=0, r33=5, r33=6, r33=9 | 0 |
| `i_mori.bms` | 6940 | 20 | 0% | `E7_SyncCPU`x10, `E6_VibDepthMidi`x10 | r7=2, r7=12, r32=0, r32=7, r33=0, r33=2 | 1 |
| `i_moridk.bms` | 4216 | 18 | 0% | `E7_SyncCPU`x9, `E6_VibDepthMidi`x9 | r7=0, r7=2, r32=0, r32=7, r33=2, r33=4 | 0 |
| `i_ryu.bms` | 14731 | 16 | 0% | `E7_SyncCPU`x8, `E6_VibDepthMidi`x8 | r7=2, r32=10, r33=0, r33=1, r33=2, r33=3 | 0 |
| `i_taura.bms` | 4069 | 18 | 0% | `E7_SyncCPU`x9, `E6_VibDepthMidi`x9 | r7=2, r32=0, r32=11, r33=0, r33=1, r33=2 | 0 |
| `jaboo.bms` | 417 | 6 | 1% | `E7_SyncCPU`x3, `E6_VibDepthMidi`x3 | r7=2, r32=12, r33=1, r33=2, r33=3 | 0 |
| `kaminoto.bms` | 4445 | 12 | 0% | `E7_SyncCPU`x12 | r32=1, r32=73, r33=0, r33=1, r33=2, r33=3 | 0 |
| `kugutu1.bms` | 4746 | 14 | 0% | `E7_SyncCPU`x14 | r7=2, r32=1, r32=77, r33=0, r33=1, r33=2 | 1 |
| `kugutu2.bms` | 4945 | 9 | 0% | `E7_SyncCPU`x9 | r32=1, r32=77, r33=0, r33=2, r33=3, r33=4 | 0 |
| `kugutu3.bms` | 5572 | 11 | 0% | `E7_SyncCPU`x11 | r32=1, r32=77, r33=0, r33=3, r33=4, r33=5 | 1 |
| `mastersword.bms` | 863 | 14 | 2% | `E6_VibDepthMidi`x10, `E7_SyncCPU`x4 | r32=0, r33=2, r33=21, r33=22, r33=25 | 0 |
| `mboss.bms` | 11271 | 47 | 0% | `CC_WritePort`x31, `E7_SyncCPU`x14, `CB_ReadPort`x2 | r1=0, r1=1, r7=2, r32=1, r32=70, r33=0 | 1 |
| `mboss_s.bms` | 10005 | 47 | 0% | `CC_WritePort`x31, `E7_SyncCPU`x14, `CB_ReadPort`x2 | r1=0, r1=1, r7=2, r32=1, r32=70, r33=0 | 1 |
| `next_dry.bms` | 2922 | 693 | 24% | `E6_VibDepthMidi`x688, `EF_PanSwSet`x2, `E7_SyncCPU`x1, `DD_BusConnect`x1 | r7=2, r8=0, r9=0, r10=1, r32=7, r33=0 | 0 |
| `next_wet.bms` | 5829 | 1385 | 24% | `E6_VibDepthMidi`x1376, `EF_PanSwSet`x3, `E7_SyncCPU`x2, `DD_BusConnect`x2 | r7=2, r8=0, r9=0, r10=1, r32=7, r33=0 | 0 |
| `okmacole.bms` | 167 | 2 | 1% | `E7_SyncCPU`x1, `E6_VibDepthMidi`x1 | r7=2, r32=0, r33=10 | 0 |
| `okmedori.bms` | 167 | 2 | 1% | `E7_SyncCPU`x1, `E6_VibDepthMidi`x1 | r7=2, r32=0, r33=10 | 0 |
| `open_box.bms` | 1554 | 22 | 1% | `E7_SyncCPU`x11, `E6_VibDepthMidi`x11 | r7=2, r32=0, r33=0, r33=1, r33=2, r33=4 | 0 |
| `pirate.bms` | 7753 | 26 | 0% | `E7_SyncCPU`x13, `E6_VibDepthMidi`x13 | r7=2, r32=0, r32=6, r33=0, r33=1, r33=8 | 0 |
| `pirate_5.bms` | 7753 | 26 | 0% | `E7_SyncCPU`x13, `E6_VibDepthMidi`x13 | r7=2, r32=0, r32=2, r33=4, r33=5, r33=8 | 0 |
| `pirate_c.bms` | 5843 | 14 | 0% | `E7_SyncCPU`x7, `E6_VibDepthMidi`x7 | r7=2, r32=0, r32=6, r33=0, r33=1, r33=2 | 0 |
| `p_bigpow.bms` | 3944 | 20 | 1% | `E7_SyncCPU`x15, `E6_VibDepthMidi`x5 | r32=1, r32=75, r33=0, r33=1, r33=2, r33=3 | 0 |
| `p_boco.bms` | 3061 | 14 | 0% | `E7_SyncCPU`x12, `E6_VibDepthMidi`x2 | r32=1, r32=65, r32=75, r33=0, r33=1, r33=2 | 1 |
| `p_ganon1.bms` | 5304 | 83 | 2% | `F1_IIRCutOff`x64, `E7_SyncCPU`x15, `CC_WritePort`x2, `CB_ReadPort`x1 | r7=1, r7=12, r32=1, r32=74, r33=0, r33=1 | 2 |
| `p_ganon2.bms` | 11929 | 83 | 1% | `F1_IIRCutOff`x64, `E7_SyncCPU`x15, `CC_WritePort`x2, `CB_ReadPort`x1 | r7=1, r7=12, r32=1, r32=74, r33=0, r33=1 | 1 |
| `p_goma_a.bms` | 3470 | 8 | 0% | `E7_SyncCPU`x7, `E6_VibDepthMidi`x1 | r32=1, r32=75, r33=0, r33=1, r33=2, r33=3 | 0 |
| `p_goma_b.bms` | 5074 | 28 | 1% | `F1_IIRCutOff`x16, `E7_SyncCPU`x10, `E6_VibDepthMidi`x2 | r32=1, r32=75, r33=1, r33=2, r33=3, r33=4 | 1 |
| `p_rane.bms` | 4469 | 20 | 0% | `E7_SyncCPU`x15, `E6_VibDepthMidi`x5 | r32=1, r32=75, r33=0, r33=1, r33=2, r33=4 | 1 |
| `rane.bms` | 4111 | 16 | 0% | `E7_SyncCPU`x16 | r32=1, r32=72, r33=0, r33=1, r33=2, r33=3 | 0 |
| `sea.bms` | 17188 | 32 | 0% | `E7_SyncCPU`x16, `E6_VibDepthMidi`x16 | r7=2, r32=0, r33=0, r33=1, r33=2, r33=3 | 1 |
| `sea_dawn.bms` | 6329 | 20 | 0% | `E7_SyncCPU`x10, `E6_VibDepthMidi`x10 | r7=2, r32=0, r33=0, r33=1, r33=2, r33=4 | 0 |
| `sea_enemy.bms` | 7576 | 51 | 1% | `CB_ReadPort`x25, `D2_ChildWritePort`x12, `E7_SyncCPU`x12, `CC_WritePort`x2 | r32=1, r33=0, r33=1, r33=3, r33=5, r33=6 | 1 |
| `sea_fail.bms` | 887 | 14 | 2% | `E7_SyncCPU`x7, `E6_VibDepthMidi`x7 | r7=2, r7=9, r32=0, r32=8, r33=0, r33=1 | 0 |
| `sea_game.bms` | 7991 | 20 | 0% | `E7_SyncCPU`x10, `E6_VibDepthMidi`x10 | r7=2, r32=0, r32=8, r33=1, r33=2, r33=9 | 0 |
| `sea_goal.bms` | 3496 | 22 | 1% | `E7_SyncCPU`x11, `E6_VibDepthMidi`x11 | r7=2, r32=0, r33=2, r33=4, r33=11, r33=13 | 0 |
| `sea_strm.bms` | 10968 | 28 | 0% | `E7_SyncCPU`x14, `E6_VibDepthMidi`x14 | r7=2, r32=0, r33=0, r33=1, r33=2, r33=9 | 0 |
| `select.bms` | 8101 | 215 | 3% | `E6_VibDepthMidi`x205, `E7_SyncCPU`x7, `F4_VibPitch`x3 | r7=2, r32=0, r33=1, r33=3, r33=4, r33=16 | 0 |
| `store.bms` | 6921 | 158 | 2% | `E6_VibDepthMidi`x150, `E7_SyncCPU`x7, `F4_VibPitch`x1 | r7=2, r32=5, r33=0, r33=1, r33=2, r33=3 | 0 |
| `subdun.bms` | 475 | 6 | 1% | `E7_SyncCPU`x3, `E6_VibDepthMidi`x3 | r7=2, r32=12, r33=1, r33=2 | 0 |
| `takt_dn.bms` | 1315 | 14 | 1% | `E7_SyncCPU`x7, `E6_VibDepthMidi`x7 | r7=2, r32=0, r33=0, r33=2, r33=4, r33=6 | 0 |
| `takt_mcl.bms` | 954 | 2 | 0% | `E7_SyncCPU`x2 | r32=61, r33=12, r33=13 | 0 |
| `takt_mdr.bms` | 976 | 6 | 1% | `E7_SyncCPU`x3, `E6_VibDepthMidi`x3 | r7=2, r32=62, r33=11, r33=12, r33=13 | 0 |
| `takt_wnd.bms` | 1609 | 14 | 1% | `E7_SyncCPU`x7, `E6_VibDepthMidi`x7 | r7=2, r32=0, r33=0, r33=2, r33=4, r33=6 | 0 |
| `takt_wrp.bms` | 4353 | 14 | 0% | `E7_SyncCPU`x7, `E6_VibDepthMidi`x7 | r7=2, r32=0, r33=0, r33=2, r33=4, r33=6 | 0 |
| `taraba_intro.bms` | 2960 | 10 | 0% | `E7_SyncCPU`x10 | r32=1, r32=77, r33=0, r33=3, r33=5, r33=7 | 0 |
| `tetra_meet.bms` | 858 | 9 | 1% | `E7_SyncCPU`x9 | r7=2, r32=1, r32=67, r33=0, r33=1, r33=2 | 0 |
| `tetra_meet_b.bms` | 2226 | 6 | 0% | `E7_SyncCPU`x6 | r7=2, r32=1, r33=1, r33=3, r33=4, r33=7 | 0 |
| `tower.bms` | 5940 | 10 | 0% | `E7_SyncCPU`x10 | r32=1, r32=68, r33=0, r33=3, r33=4, r33=5 | 0 |
| `get_box.bms` | 466 | 12 | 3% | `E7_SyncCPU`x6, `E6_VibDepthMidi`x6 | r7=2, r7=12, r32=0, r33=10, r33=11, r33=12 | 0 |
| `tak8_mcl.bms` | 1863 | 2 | 0% | `E7_SyncCPU`x2 | r32=61, r33=12, r33=13 | 0 |
| `tak8_mdr.bms` | 1411 | 6 | 0% | `E7_SyncCPU`x3, `E6_VibDepthMidi`x3 | r7=2, r32=62, r33=11, r33=12, r33=13 | 0 |

**TOTALS: 427836 sites; 4715 (1%) hit NOP'd/misdispatched opcodes.** Top offenders across all tracks:

- `E6_VibDepthMidi` × 3115
- `E7_SyncCPU` × 835
- `CB_ReadPort` × 326
- `CC_WritePort` × 173
- `F1_IIRCutOff` × 172
- `D2_ChildWritePort` × 35
- `F4_VibPitch` × 22
- `EF_PanSwSet` × 15
- `DF_SetInterrupt` × 8
- `F3_VolumeMode` × 7
- `DD_BusConnect` × 4
- `F0_OscRoute` × 3

## 3. Staged banks (mod ww_jaudio1/banks): n2i_link_0.aw, n_zelda_0.aw

> Donor Audiores carries ~50 .aw banks; a track whose bank/prog writes resolve outside the staged set plays with wrong/missing instruments. (Bank-id -> .aw mapping via bank_waves.csv / the AAF ws directory — [INFERENCE-NEEDED until the id->aw join is receipted].)
