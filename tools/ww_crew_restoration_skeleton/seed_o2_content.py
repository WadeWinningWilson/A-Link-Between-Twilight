#!/usr/bin/env python3
"""Seed Phase O2 / №27 folder content into AppData + this skeleton (no WW arcs)."""
from __future__ import annotations

import os
from pathlib import Path

SKEL = Path(__file__).resolve().parent
APP = (
    Path(os.environ["APPDATA"])
    / "TwilitRealm/Dusklight/model_replacements/WW-Crew-Restoration"
)

# (proc, arg, arc, label, host, spawn, btk, populate_interior, warp_menu)
# №32 B2: Omori = Forest Haven — no Outset door / no day-1 warp row (shelved for W6).
# №53-E: hosts parked far offshore (z=-240000), spaced ~30k — WITH exit-warp, never alone.
INTERIORS = [
    ("EXT_BG1", "1", "LinkRM", "Links House (interior)", "-60000,2000,-240000", "140,50,537", False, True, True),
    ("EXT_BG2", "2", "LinkUG", "Links Basement (interior)", "-30000,2000,-240000", "70,-100,306", False, False, True),
    ("EXT_BG3", "3", "Ojhous", "Orcas House (interior)", "0,2000,-240000", "-53,40,-227", False, True, True),
    ("EXT_BG4", "4", "Ojhous2", "Sturgeons House (interior)", "30000,2000,-240000", "-53,40,-227", False, True, True),
    ("EXT_BG5", "5", "Omasao", "Mesas House (interior)", "60000,2000,-240000", "-113,20,195", False, True, True),
    ("EXT_BG6", "6", "Onobuta", "Roses House (interior)", "90000,2000,-240000", "534,40,196", False, True, True),
    ("EXT_BG7", "7", "Omori", "Forest Haven (shelved W6)", "120000,3000,-240000", "225,140,-226", True, False, False),
    ("EXT_BG8", "8", "Pjavdou", "Jabuns Cave (interior)", "150000,2500,-240000", "200,100,250", False, False, True),
]

INI_TMPL = """proc={proc}
socket=NPC_KDK
socket_arg={arg}
type=bg
model_space=local
arc={arc}
model=model.bdl
model2=model1.bdl
model3=model3.bdl
{btk}collision=room.dzb
display_name={label}
{warp}host_stage=F_SP115
host_room=0
host_layer=-1
host_pos={host}
anchor=0,0,0
spawn_rel={spawn}
amb=5a5a5a
{pop}"""

# №27 N4: NO invented character names — census key + unverified until user locks identity.ini.
# Fields: proc, socket_arg, arc, model, idle, talk1, display, dialogue, extra
FOLK_NPCS = [
    # №36 A: Aryll hands = joint_slave (no hand BCK — tww d_a_npc_ls1).
    ("NPC_LS", 5, "Ls", "ls.bdl", "ls_wait01.bck", "ls_talk01.bck", "Ls1 ? (unverified)", "folk.ls",
     "companion_model=lshand.bdl\ncompanion_mode=joint_slave\n"),
    # №35 H3: Zl = Tetra — cloth companion; stays quest-gated via actor_map.
    ("NPC_ZL", 6, "Zl", "zl.bdl", "wait.bck", "talk01.bck", "Zl1 ? (unverified)", "folk.zl",
     "companion_model=cloth.bdl\n"),
    ("NPC_OB", 7, "Ob", "ob.bdl", "wait.bck", "talk.bck", "Ob1 ? (unverified)", "folk.ob", ""),
    ("NPC_KO", 8, "Ko", "ko.bdl", "ko_wait01.bck", "ko_talk01.bck", "Ko1 ? (unverified)", "folk.ko", ""),
    ("NPC_KO2", 23, "Ko", "ko.bdl", "ko_wait01.bck", "ko_talk01.bck", "Ko2 ? (unverified)", "folk.ko2", ""),
    ("NPC_YM", 9, "Ym", "ym.bdl", "wait01.bck", "ym_talk01.bck", "Ym1 ? (unverified)", "folk.ym", ""),
    ("NPC_YM2", 24, "Ym", "ym.bdl", "wait01.bck", "ym_talk01.bck", "Ym2 ? (unverified)", "folk.ym2", ""),
    ("NPC_YW", 10, "Yw", "yw.bdl", "wait01.bck", "yw_talk01.bck", "Yw1 ? (unverified)", "folk.yw", ""),
    ("NPC_AH", 11, "Ah", "ah.bdl", "ah_wait01.bck", "ah_wait02.bck", "Ah ? (unverified)", "folk.ah", ""),
    ("NPC_AJ", 12, "Aj", "aj.bdl", "wait01.bck", "look.bck", "Aj1 ? (unverified)", "folk.aj", ""),
    ("NPC_BM", 13, "Bm", "bm.bdl", "bm02_wait01.bck", "bm02_talk01.bck", "Bm1 ? (unverified)", "folk.bm", ""),
    ("NPC_DK", 14, "Dk", "dk.bdl", "fly1.bck", "fly1.bck", "Dk ? (unverified)", "folk.dk", ""),
    ("NPC_KB", 15, "Kb", "pg.bdl", "wait1.bck", "", "Pig ? (unverified)", "", "carryable=1\n"),
    ("NPC_KN", 16, "Kn", "kn.bdl", "wait01.bck", "wait02.bck", "kani ? (unverified)", "folk.crab", ""),
    ("NPC_PT", 17, "Pt", "pt.bdl", "wait.bck", "happy.bck", "Pt ? (unverified)", "folk.pt", ""),
    ("NPC_BB", 18, "Bb", "bb.bdl", "fly01.bck", "fly02.bck", "Bb ? (unverified)", "folk.bb", ""),
    ("NPC_KAMOME", 19, "Kamome", "ka.bdl", "ka_fly1.bck", "ka_sing1.bck", "Kamome ? (unverified)", "folk.gull", ""),
    ("NPC_CC", 20, "Cc", "cc_beta.bdl", "start.bck", "atack01.bck", "Cc ? (unverified)", "folk.cc", ""),
    # №37: Knob frame = door.bdl + dooropenadoor.bck (hinge at actor origin).
    ("NPC_KNOB", 25, "Knob", "door.bdl", "", "", "Door ? (unverified)", "",
     "static=1\ndoor=1\ndoor_open_bck=dooropenadoor.bck\n"),
    ("NPC_BA", 26, "Ba", "ba.bdl", "wait01.bck", "talk01.bck", "Ba1 ? (unverified)", "folk.ba",
     "companion_model=ba_cloth.bdl\n"),
    ("NPC_JI", 27, "Ji", "ji.bdl", "ji_wait01.bck", "ji_talk01.bck", "Ji1 ? (unverified)", "folk.ji", ""),
    # №34 P1: Kusa/long.bmd RETIRED (not field grass). №34 P2 rocks/chests + props.
    ("NPC_YAFLW", 30, "Yaflw00", "yaflw00.bdl", "yaflw00.bck", "", "Flower ? (unverified)", "", ""),
    ("NPC_PTUBO", 31, "Ptubo", "ptubo.bdl", "", "", "Pot ? (unverified)", "", "static=1\n"),
    ("NPC_TPOST", 32, "Toripost", "vpost.bdl", "post_wait.bck", "", "Mailbox ? (unverified)", "", ""),
    ("NPC_OYASHI", 33, "Oyashi", "oyashi.bdl", "", "", "Palm ? (unverified)", "", "static=1\n"),
    ("NPC_KANBAN", 34, "Kanban", "kanban.bdl", "", "", "Sign ? (unverified)", "", "static=1\n"),
    ("NPC_PIWA", 35, "Piwa", "piwa.bdl", "", "", "Rock ? (unverified)", "", "static=1\n"),
    # WwAlways/WwDalways — NEVER arc=Always/Dalways (would clobber TP Object overlays).
    ("NPC_KROCK", 36, "WwAlways", "krock_00.bdl", "", "", "Rock ? (unverified)", "", "static=1\n"),
    ("NPC_KOISI", 37, "WwAlways", "obm_koisi1.bdl", "", "", "Pebble ? (unverified)", "", "static=1\n"),
    ("NPC_BOXA", 38, "WwDalways", "boxa.bdl", "", "", "Chest ? (unverified)", "", "static=1\n"),
    ("NPC_BOXB", 39, "WwDalways", "boxb.bdl", "", "", "Chest ? (unverified)", "", "static=1\n"),
    ("NPC_BOXC", 40, "WwDalways", "boxc.bdl", "", "", "Chest ? (unverified)", "", "static=1\n"),
    # №35 H3: P1 large pirate trio (heads via actor_map).
    ("NPC_P1", 41, "P1", "p1.bdl", "wait.bck", "talk.bck", "P1 ? (unverified)", "folk.p1", ""),
    # №36 C: WW rupee visual (Vlupy) — params low byte = WW item id; grant → TP wallet.
    ("NPC_VLUPY", 42, "Vlupy", "vlupy.bdl", "", "", "Rupee ? (unverified)", "",
     "static=1\npickup_rupee=1\nbrk=vlupy.brk\nbtk=vlupy.btk\ncyl_radius=25\ncyl_height=40\n"),
]

ACTOR_MAP = """# WW placement name -> ExtNpc proc (folder-side only).
# №27 N1: [layers] gates story chunks. ACTR/SCO* always; ACT0 = day-1; later need flags.
# №27 N2: head_from_params / head_model. №27 N4: labels stay census/? until identity.ini lock.

[layers]
ACT0=
# №35 H5: story layers → quest flags (island population shifts with beats).
# ACT2 intentionally OMITTED (not blank): blank = always-on and was spawning
# extra day-1 bleed + FPS cost. Re-add only after tww confirms same-day variant.
ACT4=qs.ah_state
ACT5=qs.ah_state
ACT6=qs.ah_state
ACT7=qs.ah_state
ACT8=qs.pirates_ashore
ACT9=qs.aryll_taken
ACTa=qs.depart
ACTb=qs.depart

[Pirates]
proc=NPC_P2
arg=0
unique=1

[P1a]
proc=NPC_P1
arg=41
head_model=p1a_head.bdl
head_joint=head
unique=1

[P1b]
proc=NPC_P1
arg=41
head_model=p1b_head.bdl
head_joint=head
unique=1

[P2b]
proc=NPC_P2
arg=1
unique=1

[Mk]
proc=NPC_MK
unique=1

[Zl1]
proc=NPC_ZL
arg=6
unique=1
spawn_if_flag=beat.tetra

[Ls1]
proc=NPC_LS
arg=5
unique=1

[Ob1]
proc=NPC_OB
arg=7
head_model=oba_head.bdl
head_joint=head
unique=1

[Ko1]
proc=NPC_KO
arg=8
head_model=kohead01.bdl
head_joint=head
unique=1
# №37: Joel event alternate ACT0 row (params 00ff0002).
disable_pos=-203870,495,317220

[Ko2]
proc=NPC_KO2
arg=23
head_model=kohead02.bdl
head_joint=head
unique=1

[Ym1]
proc=NPC_YM
arg=9
head_model=ymhead01.bdl
head_joint=head
unique=1

[Ym2]
proc=NPC_YM2
arg=24
head_model=ymhead02.bdl
head_joint=head
unique=1

[Yw1]
proc=NPC_YW
arg=10
head_model=ywhead01.bdl
head_joint=head
unique=1
# №37: Sue-Belle event alternate ACT0 row (params 00ff0001).
disable_pos=-203752,481,317449

[Ah]
proc=NPC_AH
arg=11
unique=1

[Aj1]
proc=NPC_AJ
arg=12
unique=1

[Bm1]
proc=NPC_BM
arg=13
head_from_params=bmhead
head_joint=head
unique=1

[Dk]
proc=NPC_DK
arg=14
head_model=dk_kamen.bdl
head_joint=head
unique=1
spawn_if_flag=beat.helmaroc

[Pig]
proc=NPC_KB
arg=15
unique=0

[kani]
proc=NPC_KN
arg=16
unique=0

[Bb]
proc=NPC_BB
arg=18
unique=0

[Kamome]
proc=NPC_KAMOME
arg=19
unique=0

[Ba1]
proc=NPC_BA
arg=26
unique=1

[Ji1]
proc=NPC_JI
arg=27
unique=1

# №34 P1: kusax*/flower/flwr* = HOLES (no map). pflower kept as WW Yaflw00.
[pflower]
proc=NPC_YAFLW
arg=30
unique=0

[pflwrx7]
proc=NPC_YAFLW
arg=30
unique=0

[kotubo]
proc=NPC_PTUBO
arg=31
unique=0

[ootubo1]
proc=NPC_PTUBO
arg=31
unique=0

[Tpost]
proc=NPC_TPOST
arg=32
unique=0

[Oyashi]
proc=NPC_OYASHI
arg=33
unique=0

[Kanban]
proc=NPC_KANBAN
arg=34
unique=0

[KNOB00]
proc=NPC_KNOB
arg=25
unique=0

[KNOB00D]
proc=NPC_KNOB
arg=25
unique=0

# №34 P2 rocks / pebbles / chests
[Throck]
proc=NPC_PIWA
arg=35
unique=0

[koisi1]
proc=NPC_KOISI
arg=37
unique=0

[agbTBOX]
proc=NPC_BOXA
arg=38
unique=0

# №36 C / №31: WW item rupees → Vlupy WW visual; grant credits TP wallet on pickup.
[item]
proc=NPC_VLUPY
unique=0
"""

# №27 N5: authentic WW lines via ww_ref= (population/ww_dialogue_full.txt indices).
QUESTS = r"""# Outset day-1 quest spine — authentic WW dialogue (ww_ref) + thin quest wrappers.

[q1.entry]
unless_flag=q1.telescope
else=q1.done
set_flag=q1.met
next=q1.offer
ww_ref=858

[q1.offer]
set_flag=q1.telescope
action=grant:0x3E
ww_ref=851

[q1.done]
if_flag=q1.telescope
ww_ref=855

[q2.entry]
unless_flag=q2.sword
else=q2.done
set_flag=q2.met
next=q2.train
ww_ref=627

[q2.train]
set_flag=q2.sword
action=grant:0x28
ww_ref=643

[q2.done]
if_flag=q2.sword
ww_ref=669

[q3.entry]
unless_flag=q3.clothes
else=q3.done
set_flag=q3.met
next=q3.gift
ww_ref=568

[q3.gift]
set_flag=q3.clothes
action=grant:0x2F
ww_ref=585

[q3.done]
if_flag=q3.clothes
ww_ref=577

[q4.entry]
unless_flag=q4.pigs_done
else=q4.done
ww_ref=736

[q4.done]
if_flag=q4.pigs_done
ww_ref=742

[q5.entry]
unless_flag=q5.rescue
else=q5.done
set_flag=q5.seen
ww_ref=849

[q5.done]
if_flag=q5.rescue
ww_ref=710

[q6.mesa]
ww_ref=720

[q6.sturgeon]
ww_ref=641

[q6.joel]
ww_ref=792

[q6.zill]
ww_ref=713

[q6.sue]
ww_ref=584

[folk.ls]
ww_ref=858

[folk.zl]
ww_ref=893

[folk.ob]
ww_ref=570

[folk.ko]
ww_ref=792

[folk.ko2]
ww_ref=713

[folk.ym]
ww_ref=736

[folk.ym2]
ww_ref=751

[folk.yw]
ww_ref=584

[folk.ah]
ww_ref=720

[folk.aj]
ww_ref=641

[folk.bm]
ww_ref=613

[folk.dk]
ww_ref=80

[folk.pig]
ww_ref=780

[folk.ba]
ww_ref=568

[folk.ji]
ww_ref=627

[folk.p1]
ww_ref=80

[folk.crab]
ww_ref=736

[folk.pt]
ww_ref=80

[folk.bb]
ww_ref=80

[folk.gull]
ww_ref=399

[folk.cc]
ww_ref=80

[depart.offer]
unless_flag=depart.asked
else=depart.done
set_flag=depart.asked
next=depart.confirm
ww_ref=893

[depart.confirm]
set_flag=depart.offered
ww_ref=913

[depart.done]
if_flag=depart.offered
ww_ref=916
"""

NPC_Q1 = """proc=NPC_Q1
socket=NPC_HENNA0
socket_arg=2
arc=Mk
model=mk.bdl
idle=mk_wait.bck
talk1=mk_talk01.bck
talk2=mk_talk02.bck
display_name=Q1 ? (unverified)
neck_joint=head
dialogue=q1.entry
cyl_radius=40
cyl_height=100
"""

NPC_Q2 = """proc=NPC_Q2
socket=NPC_HENNA0
socket_arg=3
arc=Mk
model=mk.bdl
idle=mk_wait.bck
talk1=mk_talk01.bck
display_name=Q2 ? (unverified)
neck_joint=head
dialogue=q2.entry
cyl_radius=45
cyl_height=160
"""

NPC_Q3 = """proc=NPC_Q3
socket=NPC_HENNA0
socket_arg=4
arc=Mk
model=mk.bdl
idle=mk_wait.bck
talk1=mk_talk01.bck
display_name=Q3 ? (unverified)
neck_joint=head
dialogue=q3.entry
cyl_radius=40
cyl_height=120
"""

# №32 B1/B2: A-press ENTER + real Knob.arc props. Omori/Forest Haven door REMOVED.
DOORS_INI = """# Outdoor door triggers from Outset.arc room.dzr TGDR (WW world space).
# Interaction = near + A (walk-through retired). Omori/KNOB03D absent (W6 Forest Haven).
# №34 P3: ww_ry from TGDR (entry stride 0x24).

[linkrm]
ww_pos=-201478.1,1275.0,320761.6
ww_ry=19109
radius=240
enter_proc=EXT_BG1
exit_radius=220
knob=1

[ojhous]
ww_pos=-203060.4,675.0,318175.0
ww_ry=32708
radius=240
enter_proc=EXT_BG3
exit_radius=220
knob=1

[ojhous2]
ww_pos=-205170.5,720.0,316800.7
ww_ry=20158
radius=240
enter_proc=EXT_BG4
exit_radius=220
knob=1

[onobuta]
ww_pos=-202637.1,425.0,317825.0
ww_ry=-32768
radius=240
enter_proc=EXT_BG6
exit_radius=220
knob=1
"""

IDENTITY_INI = """# №27 N4 — user identity locks (filled by Cut Actors → Lock identity name).
# Example:
# [NPC_LS]
# display_name=Aryll
"""


def folk_ini(proc, arg, arc, model, idle, talk, display, dialogue, extra) -> str:
    lines = [
        f"proc={proc}",
        "socket=NPC_HENNA0",
        f"socket_arg={arg}",
        f"arc={arc}",
        f"model={model}",
    ]
    if idle:
        lines.append(f"idle={idle}")
    if talk:
        lines.append(f"talk1={talk}")
    lines.append(f"display_name={display}")
    lines.append("neck_joint=head")
    if dialogue:
        lines.append(f"dialogue={dialogue}")
    lines.append("cyl_radius=40")
    lines.append("cyl_height=120")
    if extra:
        for el in extra.strip().splitlines():
            if el:
                lines.append(el)
    lines.append("")
    return "\n".join(lines)


def ensure_population_field(ini_path: Path) -> None:
    if not ini_path.exists():
        return
    text = ini_path.read_text(encoding="utf-8")
    if "population=" in text:
        return
    if not text.endswith("\n"):
        text += "\n"
    text += "population=outset_placements.csv\n"
    ini_path.write_text(text, encoding="utf-8")


def ensure_socket_arg_ext_bg0(ini_path: Path) -> None:
    if not ini_path.exists():
        return
    text = ini_path.read_text(encoding="utf-8")
    if "socket_arg=" in text:
        return
    out = []
    for line in text.splitlines(keepends=True):
        out.append(line)
        if line.startswith("socket=") and "socket_arg=" not in text:
            out.append("socket_arg=0\n")
    ini_path.write_text("".join(out), encoding="utf-8")


def write_root(root: Path) -> None:
    (root / "npc").mkdir(parents=True, exist_ok=True)
    (root / "population").mkdir(parents=True, exist_ok=True)
    (root / "dialogue").mkdir(parents=True, exist_ok=True)

    ensure_population_field(root / "npc" / "ext_bg0.ini")
    ensure_socket_arg_ext_bg0(root / "npc" / "ext_bg0.ini")

    for proc, arg, arc, label, host, spawn, btk, pop_interior, warp_menu in INTERIORS:
        warp = f"warp_label={label}\n" if warp_menu else ""
        pop = (
            "population=interior_placements.csv\n"
            f"population_stage={arc}\n"
            if pop_interior
            else ""
        )
        body = INI_TMPL.format(
            proc=proc,
            arg=arg,
            arc=arc,
            label=label,
            host=host,
            spawn=spawn,
            btk=("model2_btk=model1.btk\n" if btk else ""),
            warp=warp,
            pop=pop,
        )
        (root / "npc" / f"{proc.lower()}.ini").write_text(body, encoding="utf-8")

    (root / "population" / "actor_map.ini").write_text(ACTOR_MAP, encoding="utf-8")
    (root / "population" / "doors.ini").write_text(DOORS_INI, encoding="utf-8")
    id_path = root / "population" / "identity.ini"
    if not id_path.exists():
        id_path.write_text(IDENTITY_INI, encoding="utf-8")
    (root / "dialogue" / "outset_quests.txt").write_text(QUESTS, encoding="utf-8")
    (root / "npc" / "npc_q1.ini").write_text(NPC_Q1, encoding="utf-8")
    (root / "npc" / "npc_q2.ini").write_text(NPC_Q2, encoding="utf-8")
    (root / "npc" / "npc_q3.ini").write_text(NPC_Q3, encoding="utf-8")

    for proc, arg, arc, model, idle, talk, display, dialogue, extra in FOLK_NPCS:
        body = folk_ini(proc, arg, arc, model, idle, talk, display, dialogue, extra)
        (root / "npc" / f"{proc.lower()}.ini").write_text(body, encoding="utf-8")

    # N6: departure offer on pirates (overwrite p2 dialogue key if present).
    p2 = root / "npc" / "p2.ini"
    if p2.exists():
        text = p2.read_text(encoding="utf-8")
        if "dialogue=" in text:
            lines = []
            for line in text.splitlines():
                if line.startswith("dialogue="):
                    lines.append("dialogue=depart.offer")
                else:
                    lines.append(line)
            p2.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> None:
    write_root(SKEL)
    write_root(APP)
    print(f"seeded skeleton -> {SKEL}")
    print(f"seeded AppData  -> {APP}")


if __name__ == "__main__":
    main()
