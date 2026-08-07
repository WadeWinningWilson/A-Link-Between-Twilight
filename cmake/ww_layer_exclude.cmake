# WW-layer build exclusion — GENERATED, do not hand-edit.
# regenerate: python tools/ww_crew_restoration_skeleton/ww_layer_manifest.py --emit-cmake
# tool_sha256:      e3de5f7078b31ac6911e4575b5c45860ea8571e87f10f3a619922eb490a45cac
# files_cmake_sha256: de34752bb301fc7ee7aa021638b7986acf270d8950b7243446b538fee4cfd5f1
# count: 45
#
# BASIS: DECLARED LINEAGE (KIT-LINEAGE banners), user-ruled 2026-08-07.
# Excludes native-port + bridge-owed only. The previous FILENAME basis
# was wrong in both directions (§573): it swept in 14 host-plumbing
# files carrying KIT-DONOR: none -- including a different game's port
# -- and missed 22 TUs that DO declare donor lineage.
#
# STILL PARTIAL, and for a reason no basis can fix: `mixed` TUs are
# donor lines INSIDE receiver-owned files. Dropping them would remove
# the receiver, so they are listed below as LEG-STRIP REQUIRED and
# left in the build. Excluding this set does NOT make the build
# WW-free -- it removes the separable stacks. Step 19 owns the rest.
#
# LEG-STRIP REQUIRED (11 mixed TUs, NOT excluded here):
#   src/d/actor/d_a_demo00.cpp
#   src/d/actor/d_a_swhit0.cpp
#   src/d/actor/d_a_vrbox.cpp
#   src/d/actor/d_a_vrbox2.cpp
#   src/d/d_demo.cpp
#   src/d/d_ext_dmesg.cpp
#   src/d/d_ext_save_flags.cpp
#   src/d/d_kankyo_ww.cpp
#   src/d/d_particle.cpp
#   src/d/d_stage.cpp
#   src/f_op/f_op_msg_mng.cpp
#
# Matched the OLD filename rules but declare no donor content (14),
# so they are no longer excluded. Listed so the change is auditable:
#   src/d/d_albw_dialogue.cpp
#   src/d/d_ext_mod_flags.cpp
#   src/d/d_ext_npc_doors.cpp
#   src/d/d_ext_npc_mount.cpp
#   src/d/d_ext_npc_population.cpp
#   src/d/d_ext_room_verify.cpp
#   src/d/d_ww_itemmdl_pc.cpp
#   src/d/d_ww_itemmdl_test.cpp
#   src/d/ext_evt/evt1_boundary.cpp
#   src/d/ext_plugin/ww_import_gate.cpp
#   src/d/ext_seq/ja1_bank.cpp
#   src/d/ext_seq/ja1_event_dump.cpp
#   src/d/ext_seq/ja1_native.cpp
#   src/d/ww_jpa_bind.cpp
set(WW_LAYER_FILES
    src/d/actor/d_a_esa.cpp
    src/d/actor/d_a_ext_ep.cpp
    src/d/actor/d_a_ext_plank_span.cpp
    src/d/actor/d_a_ext_vegetation.cpp
    src/d/actor/d_a_kamome.cpp
    src/d/actor/d_a_kb.cpp
    src/d/actor/d_a_knob00.cpp
    src/d/actor/d_a_lamp.cpp
    src/d/actor/d_a_npc_ba1.cpp
    src/d/actor/d_a_npc_bm1.cpp
    src/d/actor/d_a_npc_ls1.cpp
    src/d/actor/d_a_npc_zl1.cpp
    src/d/actor/d_a_obj_mshokki.cpp
    src/d/actor/d_a_obj_otble.cpp
    src/d/actor/d_a_obj_toripost.cpp
    src/d/actor/d_a_spc_item01.cpp
    src/d/actor/d_a_ww_demo00.cpp
    src/d/d_door.cpp
    src/d/d_ext_seq_space.cpp
    src/d/d_ext_tree.cpp
    src/d/d_ext_ww_actor_shims.cpp
    src/d/d_kankyo_ww_sky.cpp
    src/d/ext_evt/evt1_event_data.cpp
    src/d/ext_evt/evt1_event_manager.cpp
    src/d/ext_line/mdoext1_3dline.cpp
    src/d/ext_seq/ja1_jasbank.cpp
    src/d/ext_seq/ja1_jasbnkparser.cpp
    src/d/ext_seq/ja1_jascalc.cpp
    src/d/ext_seq/ja1_jaschannel.cpp
    src/d/ext_seq/ja1_jasdspchannel.cpp
    src/d/ext_seq/ja1_jasnotemgr.cpp
    src/d/ext_seq/ja1_jasouterparam.cpp
    src/d/ext_seq/ja1_jasplayer.cpp
    src/d/ext_seq/ja1_jasregisterparam.cpp
    src/d/ext_seq/ja1_jasseqctrl.cpp
    src/d/ext_seq/ja1_jasseqparser.cpp
    src/d/ext_seq/ja1_jastrack.cpp
    src/d/ext_seq/ja1_jastrackinterrupt.cpp
    src/d/ext_seq/ja1_jastrackport.cpp
    src/d/ext_seq/ja1_kernel.cpp
    src/d/ext_seq/ja1_oscillator.cpp
    src/d/ext_seq/ja1_parser.cpp
    src/d/ext_seq/ja1_seq_ctrl.cpp
    src/d/ext_seq/ja1_track.cpp
    src/d/ww_jpa.cpp
)
