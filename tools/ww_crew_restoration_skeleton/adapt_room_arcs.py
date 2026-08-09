"""Adapt a WW-derived STAGE BAKE (room arcs + STG) to the receiver's room contract.

WHY THIS IS SEPARATE FROM adapt_bdl_arcs.py
That tool adapts OBJECT arcs, which reach the game through getObjectRes and the
DN-3 consume-time resolver. A ROOM arc takes a different road entirely:
dRes_info_c::setRes walks the archive and dispatches the per-resource fixup on
the RARC DIRECTORY NODE TYPE -- a 4CC compared for exact equality -- not on the
file extension. A room model that is not under a recognised node type is never
parsed at all, and daBg_c::createHeap then receives the raw file buffer where it
expects a J3DModelData.

That is not hypothetical. R_DL02's first live entry (2026-08-09) crashed in
mDoExt_setupStageTexture reading [modelData + 0xC8] + 8. model.bdl's own bytes at
+0xC8, read little-endian, are 0x0000010004001200; +8 is 0x10004001208, which is
the reported fault address bit for bit. The buffer was never parsed.

WHAT THE RECEIVER ACTUALLY REQUIRES (measured, not assumed)
Census of the receiver's own shipped stages -- 305 room arcs, 79 STG_00:
    room models   'BMDR' 230/305      'BMDE'  27/305
    collision     'KCL ' 305/305      'DZB ' (WW-era rooms)
    placement     'DZR ' 305/305      'PLC ' 305/305
    stage data    'DZS ' 79/79        'DAT ' 79/79
Every shipped node type is UPPERCASE. A WW bake inherits WW's folder names, so
it arrives with 'BDL ', 'dat ', 'dzs ' -- which look right and compare wrong.

WHAT THIS DOES
  1. Node retype 'BDL ' -> 'BMDR'. BMDR is the plain room-model branch and the
     305-arc majority; BMDE (flags |= 0x20, setTexMtxLoadType 0x2000) is the
     27-arc special and is opt-in via --bmde.
  2. Node case normalised to the receiver's uppercase form. No branch in
     dRes_info_c reads 'DAT '/'DZS ' today, so this fixes no present bug -- it
     closes the divergence before a consumer appears.
  3. Payload bdl4 -> bmd3 (MDL3 stripped, retagged), reusing adapt_bdl_arcs so
     there is exactly one implementation of that conversion.
  4. dzb: the No21 through-cluster clear, if the bake has not had it.

WHAT THIS DELIBERATELY DOES NOT DO
  - No lighting rewrites. adapt_bdl_arcs applies normalize_litmask (0x03->0x01)
    and normalize_tevregs (->white) to actor models; DO-NOT.md's 2026-08-04
    amendment flags both as a deviation that rewrites donor lighting state.
    They are suppressed here and the suppression is printed on every run.
  - No mount-time BDL parse. Leaving a genuine bdl4 payload under a 'BMDR' node
    would route it into loaderBasicBmd's loadBinaryDisplayList fallback at MOUNT
    time, which is what DO-NOT's DN-3 forbids adding. Converting the payload to
    bmd3 first means the node mount-parses a BMD, which the same entry states
    this port does "exactly as the donor does". If you ever want the bdl4 to
    stay bdl4 under a room node, that needs an explicit user go -- do not
    self-approve it.

Usage: python adapt_room_arcs.py <stage_dir> [--check] [--bmde] [--no-dzb]
       --check reports and writes nothing. Otherwise each modified arc is backed
       up alongside as <name>.arc.pre-room-adapt.bak before being rewritten.
"""
import os
import struct
import sys

import adapt_bdl_arcs as A

# Node-type remap: WW folder name -> the receiver's room contract.
NODE_REMAP = {b"BDL ": b"BMDR"}
NODE_REMAP_BMDE = {b"BDL ": b"BMDE"}


def be32(d, o):
    return struct.unpack_from(">I", d, o)[0]


def _members(d):
    """Yield (entry_off, name, data_off, size) for every FILE entry."""
    data_abs = 0x20 + be32(d, 0x0C)
    n = be32(d, 0x28)
    ent = 0x20 + be32(d, 0x2C)
    strs = 0x20 + be32(d, 0x34)
    for i in range(n):
        e = ent + i * 0x14
        if struct.unpack_from(">H", d, e + 4)[0] & 0x1100 != 0x1100:
            continue
        no = struct.unpack_from(">H", d, e + 6)[0]
        end = d.index(b"\0", strs + no)
        name = d[strs + no:end].decode("ascii", "replace")
        yield e, name, data_abs + be32(d, e + 8), be32(d, e + 12)


def retype_nodes(d, remap):
    """Rewrite directory-node 4CCs. Returns list of 'old -> new' strings."""
    cnt = be32(d, 0x20)
    off = 0x20 + be32(d, 0x24)
    changed = []
    for i in range(cnt):
        n = off + i * 0x10
        cur = bytes(d[n:n + 4])
        if cur == b"ROOT":
            continue
        new = remap.get(cur, cur.upper())
        if new != cur:
            d[n:n + 4] = new
            changed.append(f"{cur.decode('ascii','replace')!r} -> "
                           f"{new.decode('ascii','replace')!r}")
    return changed


def adapt(path, remap, do_dzb):
    d = bytearray(open(path, "rb").read())
    if bytes(d[:4]) == b"Yaz0":
        d = bytearray(A.yaz0_dec(d))
    if bytes(d[:4]) != b"RARC":
        return None, [f"not a RARC ({bytes(d[:4])!r}) -- skipped"]

    notes = []
    notes += [f"node {c}" for c in retype_nodes(d, remap)]

    for e, name, off, size in list(_members(d)):
        low = name.lower()
        if low.endswith(".bdl"):
            member = bytearray(d[off:off + size])
            new_size = A.adapt_bdl(member, skip_tevregs=True)
            if new_size is None:
                notes.append(f"{name}: already bmd3 -- untouched")
                continue
            d[off:off + size] = member
            struct.pack_into(">I", d, e + 12, new_size)
            notes.append(f"{name}: bdl4 -> bmd3, {size} -> {new_size} "
                         f"(-{size - new_size})")
        elif low.endswith(".dzb") and do_dzb:
            member = bytearray(d[off:off + size])
            n = A.adapt_dzb(member)
            if n:
                d[off:off + size] = member
                notes.append(f"{name}: No21 through-cluster cleared on {n} "
                             f"property record(s)")
            else:
                notes.append(f"{name}: No21 clear already applied -- untouched")
    return d, notes


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    if len(args) != 1:
        sys.exit(__doc__)
    stage_dir = args[0]
    check = "--check" in sys.argv
    remap = NODE_REMAP_BMDE if "--bmde" in sys.argv else NODE_REMAP
    do_dzb = "--no-dzb" not in sys.argv

    # DO-NOT.md 2026-08-04 amendment: the actor adapter's lighting rewrites are a
    # flagged deviation. Suppress them for room arcs, loudly.
    A.normalize_litmask = lambda buf: 0
    A.normalize_tevregs = lambda buf: 0
    print("lighting rewrites SUPPRESSED (normalize_litmask / normalize_tevregs) "
          "-- DO-NOT.md 2026-08-04 amendment")
    print(f"room-model node -> {list(remap.values())[0].decode()}"
          f"{'  [--check: nothing will be written]' if check else ''}\n")

    touched = 0
    for fn in sorted(os.listdir(stage_dir)):
        if not fn.lower().endswith(".arc"):
            continue
        path = os.path.join(stage_dir, fn)
        out, notes = adapt(path, remap, do_dzb)
        print(f"{fn}")
        for n in notes:
            print(f"   {n}")
        if not notes:
            print("   (nothing to change)")
            continue
        touched += 1
        if check or out is None:
            continue
        bak = path + ".pre-room-adapt.bak"
        if not os.path.exists(bak):
            os.replace(path, bak)
        else:
            print(f"   NOTE: {os.path.basename(bak)} exists -- kept the "
                  f"original backup, not overwriting it")
        open(path, "wb").write(out)
        print(f"   written; original at {os.path.basename(bak)}")

    print(f"\n{touched} arc(s) {'would be' if check else ''} changed")


if __name__ == "__main__":
    main()
