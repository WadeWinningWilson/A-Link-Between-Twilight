#!/usr/bin/env python3
# ============================================================================
# dusktap_world.py — §371 LIVE WORLD CENSUS on donor WW (user-ordered).
#
# WHY THIS EXISTS (the honest reason): the offline dzr/arc census route is
# INCOMPLETE. It reads what the room's DATA authors and therefore cannot see
# anything the donor produces another way — geometry-resident props (the
# cauldron, the small table, the planters), actors spawned by CODE rather than
# by a dzr row, VFX/SFX, or props that live in a sibling model/arc. The user
# validated missing content from real WW gameplay that no dzr row explains.
# This probe answers the only question that settles it: WHAT ACTUALLY EXISTS,
# LIVE, IN THE DONOR, ROOM BY ROOM.
#
# Method: attach to a running mainline Dolphin (donor GZLE01) and walk the
# donor's OWN process tree — every live process, with its proc name resolved
# to a symbol, its position, params and room. Layout receipts (donor headers):
#   l_fpcLy_LayerList  .data:0x803726DC  node_list_class (12B: first/last/count)
#   layer_class        mNode 0x00 (node_class 12B: next/prev/list),
#                      mLayerID 0x0C, mNodeListTree 0x10 {lists*, numLists},
#                      mpPcNode 0x18, mCancelList 0x1C, counts 0x28
#   base_process_class mProcName s16 @0x08, mParameters u32 @0xB0,
#                      mLyTg @0x18 -> mCreateTag -> node @0x00
#                      => PROCESS = node_ptr - 0x18
#   fopAc_ac_c         current @0x1F8 (pos f32x3, angle @+0x0C, roomNo @+0x12)
#   proc-name symbols  parsed from donor include/f_pc/f_pc_name.h X-macro
#   OBJNAME rows       parsed from donor src/d/d_stage.cpp (proc -> dzr names)
#
# Usage (foundry python), with WW running in Dolphin:
#   dusktap_world.py            — one census now, to stdout + CSV
#   dusktap_world.py --watch    — re-census on every room change (walk the
#                                 house, get one block per room, no guessing)
# Read-only. Never writes game memory.
# ============================================================================
import csv
import re
import struct
import sys
import time
from pathlib import Path

import dolphin_memory_engine as dme

DONOR = Path("<decomp-root>/WW DP")
OUT = Path("%USERPROFILE%/Documents/ww-arc-staging")

LY_LIST = 0x803726DC          # l_fpcLy_LayerList (node_list_class)
LN_QUEUE = 0x803F6180         # g_fpcLn_Queue (node_lists_tree_class)
GAMEINFO = 0x803C4C08
PLAY = GAMEINFO + 0x12A0
PROC_FROM_NODE = 0x18         # base_process_class.mLyTg.mCreateTag.mNode
VALID = range(0x80000000, 0x81800000)


def u8(a):
    return dme.read_bytes(a, 1)[0]


def u16(a):
    return struct.unpack(">H", dme.read_bytes(a, 2))[0]


def s16(a):
    return struct.unpack(">h", dme.read_bytes(a, 2))[0]


def u32(a):
    return struct.unpack(">I", dme.read_bytes(a, 4))[0]


def f32(a):
    return struct.unpack(">f", dme.read_bytes(a, 4))[0]


def proc_names():
    """fpcNm_* index -> symbol, from the donor X-macro list."""
    txt = (DONOR / "include/f_pc/f_pc_name.h").read_text(encoding="utf-8",
                                                         errors="replace")
    out = {}
    # donor form is a plain enum with offset comments:
    #   /* 0x00BA */ fpcNm_EP_e,
    for m in re.finditer(r"/\*\s*0x([0-9A-Fa-f]+)\s*\*/\s*(fpcNm_\w+)", txt):
        out[int(m.group(1), 16)] = m.group(2)
    return out


def objnames():
    """proc symbol -> [dzr OBJNAME rows] (what a proc is CALLED in room data)."""
    txt = (DONOR / "src/d/d_stage.cpp").read_text(encoding="utf-8",
                                                  errors="replace")
    out = {}
    for m in re.finditer(r'OBJNAME\("([^"]+)",\s*(fpcNm_\w+)', txt):
        out.setdefault(m.group(2), []).append(m.group(1))
    return out


def walk_processes(limit=4096):
    """Every live process, walked the donor's OWN way.

    LAYOUT RECEIPTS (donor SSystem/SComponent, read after the first attempt
    walked `prev` and died at one node — DECOMP-FIRST, applied late):
      node_class      { mpPrevNode 0x00, mpData 0x04, mpNextNode 0x08 }
      node_list_class { mpHead 0x00, mpTail 0x04, mSize 0x08 }
      node_lists_tree_class { mpLists 0x00, mNumLists 0x04 }
    So: next = +0x08, and **mpData (+0x04) IS the owning process** — no
    node-to-process offset arithmetic needed at all.

    Root = g_fpcLn_Queue (the LINE queue: every process in execution order),
    with the layer list as a cross-check. Guarded: pointer ranges, caps, and
    None (=UNKNOWN) rather than garbage.
    """
    procs = []
    seen = set()

    def walk_list(lst):
        head, size = u32(lst), u32(lst + 8)
        if head not in VALID or size < 0 or size > limit:
            return
        node, g = head, 0
        while node in VALID and g < limit:
            g += 1
            if node in seen:
                break
            seen.add(node)
            data = u32(node + 4)          # mpData -> owning process
            if data in VALID:
                try:
                    procs.append((s16(data + 0x08), data))
                except RuntimeError:
                    pass
            node = u32(node + 8)          # mpNextNode

    # (a) the line queue — all processes
    lists_ptr, num = u32(LN_QUEUE), u32(LN_QUEUE + 4)
    if lists_ptr in VALID and 0 < num <= 64:
        for li in range(num):
            walk_list(lists_ptr + li * 0x0C)

    # (b) layer list -> each layer's node-list tree (catches anything the
    #     line queue does not carry)
    lhead = u32(LY_LIST)
    layer, g = lhead, 0
    while layer in VALID and g < 64:
        g += 1
        lay = u32(layer + 4) if u32(layer + 4) in VALID else layer
        tree_lists, tree_num = u32(lay + 0x10), u32(lay + 0x14)
        if tree_lists in VALID and 0 < tree_num <= 32:
            for li in range(tree_num):
                walk_list(tree_lists + li * 0x0C)
        layer = u32(layer + 8)

    if not procs:
        return None
    # de-dup by process address, keep first name seen
    uniq = {}
    for pname, addr in procs:
        uniq.setdefault(addr, pname)
    return [(pname, addr) for addr, pname in uniq.items()]


def actor_detail(addr):
    """Position/params/room if this process is actor-shaped (fopAc_ac_c)."""
    try:
        params = u32(addr + 0xB0)
        pos = tuple(round(f32(addr + 0x1F8 + 4 * i), 1) for i in range(3))
        room = u8(addr + 0x1F8 + 0x12)
        plausible = all(abs(v) < 1e7 for v in pos)
        return (params, pos if plausible else None, room if room < 64 else None)
    except RuntimeError:
        return (None, None, None)


def census(names, objs, tag=""):
    procs = walk_processes()
    if procs is None:
        print("UNKNOWN — process walk implausible (scene loading?); retry")
        return None
    rows = []
    for pname_id, addr in procs:
        sym = names.get(pname_id & 0xFFFF, f"proc_{pname_id:#x}")
        dzr = ",".join(objs.get(sym, []))
        params, pos, room = actor_detail(addr)
        rows.append({
            "proc": sym, "dzr_names": dzr, "addr": f"{addr:08x}",
            "params": f"{params:08x}" if params is not None else "",
            "x": pos[0] if pos else "", "y": pos[1] if pos else "",
            "z": pos[2] if pos else "", "room": room if room is not None else "",
        })
    print(f"\n===== LIVE CENSUS {tag} — {len(rows)} live processes =====")
    from collections import Counter
    c = Counter(r["proc"] for r in rows)
    for sym, n in c.most_common():
        dzr = objs.get(sym, [])
        print(f"  {sym:28s} x{n:<3d} {('dzr: ' + ','.join(dzr)) if dzr else ''}")
    return rows


def main():
    watch = "--watch" in sys.argv
    names, objs = proc_names(), objnames()
    print(f"symbol tables: {len(names)} proc names, {len(objs)} OBJNAME procs")
    print("waiting for Dolphin…")
    while True:
        dme.hook()
        if dme.is_hooked():
            break
        time.sleep(2)
    print("hooked.")

    out = OUT / "dusktap_world_census.csv"
    fields = ["stamp", "proc", "dzr_names", "addr", "params", "x", "y", "z", "room"]
    f = open(out, "w", newline="", encoding="utf-8")
    w = csv.DictWriter(f, fieldnames=fields)
    w.writeheader()

    last_key = None
    try:
        while True:
            try:
                rows = census(names, objs, tag=time.strftime("%H:%M:%S"))
            except RuntimeError:
                time.sleep(1)
                continue
            if rows:
                key = tuple(sorted((r["proc"], r["addr"]) for r in rows))
                if key != last_key:
                    last_key = key
                    stamp = time.strftime("%H:%M:%S")
                    for r in rows:
                        r["stamp"] = stamp
                        w.writerow(r)
                    f.flush()
                    print(f"  -> {len(rows)} rows appended ({out.name})")
            if not watch:
                break
            time.sleep(3)
    except KeyboardInterrupt:
        pass
    finally:
        f.close()
        print(f"\ncensus CSV: {out}")


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    main()
