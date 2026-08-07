# WW-layer build exclusion — GENERATED, do not hand-edit.
# regenerate: python tools/ww_crew_restoration_skeleton/ww_layer_manifest.py --emit-cmake
# tool_sha256:      9f91b5ecd05cf9054fefad24c30bee25f79de1f5aa3fd15e9e4407e56763153a
# files_cmake_sha256: de34752bb301fc7ee7aa021638b7986acf270d8950b7243446b538fee4cfd5f1
# count: 48
#
# BASIS: filename convention. This is a FLOOR, not the WW layer.
# It cannot see a donor port correctly named after the receiver file
# it replaces, and it cannot see a LEG (donor lines inside a
# receiver-owned TU) at all -- those are not excludable by dropping a
# file. Excluding these does NOT make the build WW-free; it removes
# the separable stacks. Partial by definition until roadmap step 19.
set(WW_LAYER_FILES
    src/d/actor/d_a_ext_ep.cpp
    src/d/actor/d_a_ext_plank_span.cpp
    src/d/actor/d_a_ext_vegetation.cpp
    src/d/actor/d_a_ww_demo00.cpp
    src/d/d_albw_dialogue.cpp
    src/d/d_ext_dmesg.cpp
    src/d/d_ext_mod_flags.cpp
    src/d/d_ext_npc_doors.cpp
    src/d/d_ext_npc_mount.cpp
    src/d/d_ext_npc_population.cpp
    src/d/d_ext_room_verify.cpp
    src/d/d_ext_save_flags.cpp
    src/d/d_ext_seq_space.cpp
    src/d/d_ext_tree.cpp
    src/d/d_ext_ww_actor_shims.cpp
    src/d/d_kankyo_ww.cpp
    src/d/d_kankyo_ww_sky.cpp
    src/d/d_ww_itemmdl_pc.cpp
    src/d/d_ww_itemmdl_test.cpp
    src/d/ext_evt/evt1_boundary.cpp
    src/d/ext_evt/evt1_event_data.cpp
    src/d/ext_evt/evt1_event_manager.cpp
    src/d/ext_line/mdoext1_3dline.cpp
    src/d/ext_plugin/ww_import_gate.cpp
    src/d/ext_seq/ja1_bank.cpp
    src/d/ext_seq/ja1_event_dump.cpp
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
    src/d/ext_seq/ja1_native.cpp
    src/d/ext_seq/ja1_oscillator.cpp
    src/d/ext_seq/ja1_parser.cpp
    src/d/ext_seq/ja1_seq_ctrl.cpp
    src/d/ext_seq/ja1_track.cpp
    src/d/ww_jpa.cpp
    src/d/ww_jpa_bind.cpp
)
