# FINDING - encoding sweep over the C++ population (2026-08-18)

era: era-independent
<!-- era rationale: instrument-coverage finding | Housing/Engine, 2026-08-18 -->

**Extends the Librarian's estate-wide C0 sweep, which covered `tools/foundry/*.py` (133)
and `docs/**.md` (162) = 295 files and reported CLEAN. That population EXCLUDED every
C++ source in the tree. This sweep covers the remainder.**

## DENOMINATOR FIRST (corrected 2026-08-18, on the Librarian's rule)

**2,740 of 27,801 files in the tree = 9.8%.** My original row called this "the
remainder" of the Librarian's 295, which **overclaimed exactly as their row did** -
filed in the row that corrected them for it. Scope actually swept:
`src/**` + `include/**` + `mods-src/**`, extensions `.cpp/.h/.hpp` only.

**Re-measured against the WHOLE tree** (excluding `.git`, `local_dev_backup`,
`build`, `__pycache__`): **UTF-16 = 1, and it IS inside this scope, so that number
is complete for the tree.** UTF-8-BOM = **59 total, 51 in live C++ source**; the
other 8 are outside and every one is correctly excluded:
`build_ls1.log` - `scratch_tale_report.txt` -
`companion_mods/MM-SkullKid-Reskin/_work/object_stk/object_stk.xml` -
`docs/state/fps-bisect-restore/d_s_play.cpp.wip` -
**`tools/foundry/controls/c0_injected.txt` (a DELIBERATE test fixture for `c0_audit`)** -
`tools/procdump/Eula.txt` - `tools/vendor/ZAPDTR/ExporterTest/ExporterTest.vcxproj.filters` -
`tools/vendor/ZAPDTR/ZAPD/packages.config` (**MSBuild files are SUPPOSED to carry a BOM**).

**So the finding stands and the framing did not.** And the 8 argue for the
refinement below rather than for a wider net.

## Result: 2,740 files scanned (of 27,801)

- **UTF-16LE with BOM: 1** - invisible to every byte-oriented regex in this estate.
  - `src/dusk/imgui/ImGuiActorSpawner.cpp` - **NOT IN THE BUILD** (CMakeLists uses an explicit source list, not a glob).
    Dead weight, but a landmine: a text tool sees binary noise, MSVC would compile it fine.
- **UTF-8 with BOM: 51** - a BOM defeats leading-anchor patterns (`^//`, `^#include`)
  on **line 1 only**. This is the estate's already-catalogued "BOM / `^//`" trap.

### UTF-8-BOM files, enumerated in full (never truncated)

- `src/d/d_bg_parts.cpp`
- `src/d/d_com_static.cpp`
- `src/d/d_debug_camera.cpp`
- `src/d/d_event.cpp`
- `src/d/d_event_manager.cpp`
- `src/d/d_ev_camera.cpp`
- `src/d/d_file_select.cpp`
- `src/d/d_file_sel_info.cpp`
- `src/d/d_menu_calibration.cpp`
- `src/d/d_menu_dmap.cpp`
- `src/d/d_menu_fishing.cpp`
- `src/d/d_menu_fmap.cpp`
- `src/d/d_menu_fmap2D.cpp`
- `src/d/d_menu_insect.cpp`
- `src/d/d_menu_letter.cpp`
- `src/d/d_menu_option.cpp`
- `src/d/d_menu_save.cpp`
- `src/d/d_menu_skill.cpp`
- `src/d/d_meter_button.cpp`
- `src/d/d_meter_string.cpp`
- `src/d/d_msg_scrn_3select.cpp`
- `src/d/d_msg_scrn_base.cpp`
- `src/d/d_msg_scrn_explain.cpp`
- `src/d/d_name.cpp`
- `src/d/d_stage.cpp`
- `src/d/d_timer.cpp`
- `src/d/ww_jpa_bind.cpp`
- `src/d/actor/d_a_bg_obj.cpp`
- `src/d/actor/d_a_door_dbdoor00.cpp`
- `src/d/actor/d_a_door_knob00.cpp`
- `src/d/actor/d_a_door_shutter.cpp`
- `src/d/actor/d_a_door_spiral.cpp`
- `src/d/actor/d_a_movie_player.cpp`
- `src/d/actor/d_a_npc_hanjo.cpp`
- `src/d/actor/d_a_npc_saru.cpp`
- `src/d/actor/d_a_npc_shop0.cpp`
- `src/d/actor/d_a_npc_yelia.cpp`
- `src/d/actor/d_a_npc_ykw.cpp`
- `src/d/actor/d_a_obj_flag.cpp`
- `src/d/actor/d_a_obj_flag2.cpp`
- `src/d/actor/d_a_obj_flag3.cpp`
- `src/d/actor/d_a_obj_gra2.cpp`
- `src/d/actor/d_a_obj_sekizoa.cpp`
- `src/d/actor/d_a_set_bgobj.cpp`
- `src/d/actor/d_a_tag_evt.cpp`
- `src/d/actor/d_a_tag_msg.cpp`
- `src/dusk/imgui/ImGuiProcessOverlay.cpp`
- `src/dusk/imgui/ImGuiSaveEditor.cpp`
- `src/f_op/f_op_actor.cpp`
- `src/f_op/f_op_actor_mng.cpp`
- `src/Z2AudioLib/Z2SoundMgr.cpp`

## Verified separately
- `tools/foundry/engine_watch.py`: **0 C0 bytes, 0 non-ASCII** - checked directly rather
  than inferred from the sweep, since it was created after the sweep's population was fixed.

