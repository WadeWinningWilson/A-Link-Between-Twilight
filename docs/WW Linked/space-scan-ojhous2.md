# Space scan — donor 'Ojhous2' (§299 prototype, read-only)

**56 findings** (0 critical, 55 major).

| sev | check | finding |
|---|---|---|
| MAJOR | R3-evnt-missing | donor stage-event 'StartCamera' has no host REVT row — its spawn-triggered entry can never fire |
| MAJOR | R3-evnt-missing | donor stage-event 'KNOB_S_BR' has no host REVT row — its spawn-triggered entry can never fire |
| MAJOR | R4-event-missing | donor event 'StartCamera' not merged into host event_list — any actor ordering it gets idx -1 / silent no-op |
| MAJOR | R4-event-missing | donor event 'Ji1_linkmove' not merged into host event_list — any actor ordering it gets idx -1 / silent no-op |
| MAJOR | R4-event-missing | donor event 'Ji1_Speak' not merged into host event_list — any actor ordering it gets idx -1 / silent no-op |
| MAJOR | R4-event-missing | donor event 'Ji1_StartSpeak' not merged into host event_list — any actor ordering it gets idx -1 / silent no-op |
| MAJOR | R4-event-missing | donor event 'Ji1_BadSpeak' not merged into host event_list — any actor ordering it gets idx -1 / silent no-op |
| MAJOR | R4-event-missing | donor event 'Ji1_EndSpeak' not merged into host event_list — any actor ordering it gets idx -1 / silent no-op |
| MAJOR | R4-event-missing | donor event 'Ji1_kaiten' not merged into host event_list — any actor ordering it gets idx -1 / silent no-op |
| MAJOR | R4-event-missing | donor event 'Ji1_kaiten_exp' not merged into host event_list — any actor ordering it gets idx -1 / silent no-op |
| MAJOR | R4-event-missing | donor event 'Ji1_ItemGetEnd' not merged into host event_list — any actor ordering it gets idx -1 / silent no-op |
| MAJOR | R4-event-missing | donor event 'Ji1_ItemGetTalkEnd' not merged into host event_list — any actor ordering it gets idx -1 / silent no-op |
| MAJOR | R4-event-missing | donor event 'Ji1_SwordGetTalkEnd' not merged into host event_list — any actor ordering it gets idx -1 / silent no-op |
| MAJOR | R4-event-missing | donor event 'Ji1_pl_backroom' not merged into host event_list — any actor ordering it gets idx -1 / silent no-op |
| MAJOR | R4-event-missing | donor event 'Ji1_SpRollFail' not merged into host event_list — any actor ordering it gets idx -1 / silent no-op |
| MAJOR | R4-event-missing | donor event 'Ji1_TeachSpRollCut' not merged into host event_list — any actor ordering it gets idx -1 / silent no-op |
| MAJOR | R4-event-missing | donor event 'Ji1_NormAng' not merged into host event_list — any actor ordering it gets idx -1 / silent no-op |
| MAJOR | R4-event-missing | donor event 'Ji1_NormAng2' not merged into host event_list — any actor ordering it gets idx -1 / silent no-op |
| MAJOR | R4-event-missing | donor event 'Ji1_NormAngFin' not merged into host event_list — any actor ordering it gets idx -1 / silent no-op |
| MAJOR | R4-event-missing | donor event 'Ji1_KmonTalk' not merged into host event_list — any actor ordering it gets idx -1 / silent no-op |
| MAJOR | R4-event-missing | donor event 'Ji1_EquipTalk' not merged into host event_list — any actor ordering it gets idx -1 / silent no-op |
| MAJOR | R4-event-missing | donor event 'angry' not merged into host event_list — any actor ordering it gets idx -1 / silent no-op |
| MAJOR | R4-event-missing | donor event 'NORMAL_COMEBACK' not merged into host event_list — any actor ordering it gets idx -1 / silent no-op |
| MAJOR | R4-event-missing | donor event 'SHORT_COMEBACK' not merged into host event_list — any actor ordering it gets idx -1 / silent no-op |
| MAJOR | R4-event-missing | donor event 'MAGMA_COMEBACK' not merged into host event_list — any actor ordering it gets idx -1 / silent no-op |
| MAJOR | R4-event-missing | donor event 'fire_off_cam' not merged into host event_list — any actor ordering it gets idx -1 / silent no-op |
| MAJOR | R4-event-missing | donor event 'fire_off_cam_ws' not merged into host event_list — any actor ordering it gets idx -1 / silent no-op |
| MAJOR | R4-event-missing | donor event 'fire_on_cam' not merged into host event_list — any actor ordering it gets idx -1 / silent no-op |
| MAJOR | R4-event-missing | donor event 'zenfire_cam' not merged into host event_list — any actor ordering it gets idx -1 / silent no-op |
| MAJOR | R4-event-missing | donor event 'zenfire_cam_ws' not merged into host event_list — any actor ordering it gets idx -1 / silent no-op |
| MAJOR | R4-event-missing | donor event 'FMASTER_START' not merged into host event_list — any actor ordering it gets idx -1 / silent no-op |
| MAJOR | R4-event-missing | donor event 'KNOB_START' not merged into host event_list — any actor ordering it gets idx -1 / silent no-op |
| MAJOR | R4-event-missing | donor event 'KNOB_START_B' not merged into host event_list — any actor ordering it gets idx -1 / silent no-op |
| MAJOR | R4-event-missing | donor event 'KNOB_S_BR' not merged into host event_list — any actor ordering it gets idx -1 / silent no-op |
| MAJOR | R4-event-missing | donor event 'KNOB_S_BL' not merged into host event_list — any actor ordering it gets idx -1 / silent no-op |
| MAJOR | R4-event-missing | donor event 'KNOB_S_FR' not merged into host event_list — any actor ordering it gets idx -1 / silent no-op |
| MAJOR | R4-event-missing | donor event 'KNOB_S_FL' not merged into host event_list — any actor ordering it gets idx -1 / silent no-op |
| MAJOR | R4-event-missing | donor event 'SHUTTER_START' not merged into host event_list — any actor ordering it gets idx -1 / silent no-op |
| MAJOR | R4-event-missing | donor event 'SHUTTER_START_STOP' not merged into host event_list — any actor ordering it gets idx -1 / silent no-op |
| MAJOR | R4-event-missing | donor event 'BS_SHUTTER_START' not merged into host event_list — any actor ordering it gets idx -1 / silent no-op |
| MAJOR | R4-event-missing | donor event 'SHUTTER_DROP_CARRY' not merged into host event_list — any actor ordering it gets idx -1 / silent no-op |
| MAJOR | R4-event-missing | donor event 'TACT_WINDOW' not merged into host event_list — any actor ordering it gets idx -1 / silent no-op |
| MAJOR | R4-event-missing | donor event 'TACT_WINDOW2' not merged into host event_list — any actor ordering it gets idx -1 / silent no-op |
| MAJOR | R4-event-missing | donor event 'TACT_WINDOW2_SHIP' not merged into host event_list — any actor ordering it gets idx -1 / silent no-op |
| MAJOR | R4-event-missing | donor event 'WARPT_OPEN' not merged into host event_list — any actor ordering it gets idx -1 / silent no-op |
| MAJOR | R4-event-missing | donor event 'DUMMY2' not merged into host event_list — any actor ordering it gets idx -1 / silent no-op |
| MAJOR | R4-event-missing | donor event 'DUNGEON_WARP' not merged into host event_list — any actor ordering it gets idx -1 / silent no-op |
| MAJOR | R4-event-missing | donor event 'FALL_START' not merged into host event_list — any actor ordering it gets idx -1 / silent no-op |
| MAJOR | R4-event-missing | donor event 'OPTION_CHAR_END' not merged into host event_list — any actor ordering it gets idx -1 / silent no-op |
| MAJOR | R4-event-missing | donor event 'OPTION_CHAR_START' not merged into host event_list — any actor ordering it gets idx -1 / silent no-op |
| MAJOR | R4-event-missing | donor event 'RiddleSound0' not merged into host event_list — any actor ordering it gets idx -1 / silent no-op |
| MAJOR | R4-event-missing | donor event 'RiddleSound30' not merged into host event_list — any actor ordering it gets idx -1 / silent no-op |
| MAJOR | R4-event-missing | donor event 'TIMEWARP' not merged into host event_list — any actor ordering it gets idx -1 / silent no-op |
| MAJOR | R4-event-missing | donor event 'TIMEWARP_COMEBACK' not merged into host event_list — any actor ordering it gets idx -1 / silent no-op |
| MAJOR | R4-event-missing | donor event 'MapToolCamera' not merged into host event_list — any actor ordering it gets idx -1 / silent no-op |
| MINOR | R9-unregistered-actors | 15 placed actor names lack OBJNAME registration (defer/CSV-side): ['Aj1', 'AttTag', 'Ji1', 'KNOB00', 'Lamp', 'Ostool', 'Otana', 'Otble', 'Paper', 'Plant', 'SPitem', 'Yw1'] ... |
