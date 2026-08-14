"""Add one member file into an existing RARC actor arc, in place.

The adaptation pipeline (adapt_bdl_arcs.py) edits members inside their existing
slots and can never ADD one; the mount's attach system resolves attachment
models strictly from the mount's own arc (addAttachment -> dComIfG_getObjectRes
on mManifest.arc). Prop work therefore needs a way to insert a new model into
an actor arc without rebuilding it from scratch.

Strategy: parse the RARC, splice one file entry into the node that already
holds a named sibling (so the new member sits beside the models it belongs
with), append its name to the string table and its payload to the data blob,
and fix every offset the splice moved. Entry data offsets are relative to the
data-section start, so appending the payload at the end leaves every existing
offset untouched — only the section starts shift.

.bdl members are run through adapt_bdl first, matching what the batch
adaptation did to every other model in the mod (donor TEV regs etc.).

Usage:
#   Order    : AFTER adapt_bdl_arcs.py, and this is a CAPABILITY edge, not a convention:
#              the adaptation pass edits members inside their EXISTING slots and can never
#              ADD one (its own header says so). Any member that does not yet exist must be
#              added here, afterwards. Independent of every stage/event step.
#   R1 note  : takes an explicit arc + member + payload + sibling; four required args and no
#              defaults. A recipe step must name all four — a defaulted target would write a
#              member into whichever arc happened to be first, which no test would catch.
  python arc_add_member.py <arc> <member-name> <payload-file> <sibling-name>

The arc is modified in place; a .pre-<member>-bak copy is written first.
Refuses to run if the member name already exists in the arc.
"""
# ============================================================================
# TIER-3 QUARANTINE (kit audit + OUTPUT LAW wiring). This tool writes donor-
# format bytes and belongs to the era the project is leaving. It must NOT run
# in the disc-native porting wave. Kept RE-RUNNABLE per the No116 ceremony:
# pass --tier3-override; the run is then LEDGERED so no donor-format write
# happens silently. IMPORT-SAFE: gates only under __main__, so Tier-1 kits
# that import helpers from this file are unaffected.
# ============================================================================
import sys as _q_sys
if __name__ == "__main__":
    if "--tier3-override" not in _q_sys.argv:
        _q_sys.stderr.write(
            "TIER-3 QUARANTINED (kit audit): retired from the disc-native wave. "
            "Deliberate rerun: --tier3-override (ledgered).\n")
        raise SystemExit(3)
    _q_sys.argv.remove("--tier3-override")
    import io as _q_io
    from pathlib import Path as _q_P
    _q_led = _q_P(__file__).resolve().parents[2] / "docs" / "state" / "ww-staging" / "OUTPUT-LEDGER.md"
    with _q_io.open(_q_led, "a", encoding="utf-8", newline="\r\n") as _q_f:
        _q_f.write("| TIER3-OVERRIDE RUN: %s | deliberate rerun | quarantine stands |\n"
                   % _q_P(__file__).name)
import os
import shutil
import struct
import sys

from adapt_bdl_arcs import adapt_bdl, be32, yaz0_dec


def jkr_hash(name: str) -> int:
    h = 0
    for ch in name:
        h = (h * 3 + ord(ch)) & 0xFFFF
    return h


def align32(v: int) -> int:
    return (v + 31) & ~31


def main() -> None:
    if len(sys.argv) != 5:
        sys.exit(__doc__)
    arc_path, member_name, payload_path, sibling = sys.argv[1:5]

    payload = bytearray(open(payload_path, "rb").read())
    if member_name.lower().endswith(".bdl"):
        new_size = adapt_bdl(payload)
        if new_size is not None:
            payload = payload[:new_size]
            print(f"{member_name}: adapted ({new_size} bytes)")
        else:
            print(f"{member_name}: not bdl4 — inserted unmodified")

    d = bytearray(open(arc_path, "rb").read())
    if bytes(d[:4]) == b"Yaz0":
        d = yaz0_dec(d)
    if bytes(d[:4]) != b"RARC":
        sys.exit(f"{arc_path}: not a RARC")

    info = 0x20
    data_abs = 0x20 + be32(d, 0x0C)
    data_size = be32(d, 0x10)
    num_nodes = be32(d, info + 0x00)
    node_off = info + be32(d, info + 0x04)
    num_entries = be32(d, info + 0x08)
    ent_off = info + be32(d, info + 0x0C)
    str_size = be32(d, info + 0x10)
    str_off = info + be32(d, info + 0x14)

    def entry_name(idx: int) -> str:
        e = ent_off + idx * 0x14
        no = struct.unpack_from(">H", d, e + 6)[0]
        end = d.index(b"\0", str_off + no)
        return d[str_off + no:end].decode("ascii", "replace")

    # locate the sibling's node and refuse duplicates
    target_node = -1
    max_file_id = -1
    for i in range(num_entries):
        e = ent_off + i * 0x14
        fid = struct.unpack_from(">H", d, e)[0]
        if fid != 0xFFFF and fid > max_file_id:
            max_file_id = fid
        if entry_name(i).lower() == member_name.lower():
            sys.exit(f"{arc_path}: '{member_name}' already present — refusing")
    for n in range(num_nodes):
        no = node_off + n * 0x10
        cnt = struct.unpack_from(">H", d, no + 0x0A)[0]
        first = be32(d, no + 0x0C)
        for k in range(cnt):
            if entry_name(first + k).lower() == sibling.lower():
                target_node = n
                break
        if target_node >= 0:
            break
    if target_node < 0:
        sys.exit(f"{arc_path}: sibling '{sibling}' not found in any node")

    no = node_off + target_node * 0x10
    t_cnt = struct.unpack_from(">H", d, no + 0x0A)[0]
    t_first = be32(d, no + 0x0C)
    insert_idx = t_first + t_cnt  # end of the target node's contiguous range

    # build the new entry
    new_data_off = align32(data_size)
    new_entry = struct.pack(
        ">HHHHIII",
        (max_file_id + 1) & 0xFFFF,
        jkr_hash(member_name),
        0x1100,          # plain stored file
        str_size,        # name goes at the current end of the string table
        new_data_off,
        len(payload),
        0,
    )

    # splice: entries table gains one record at insert_idx
    entries = bytearray(d[ent_off:ent_off + num_entries * 0x14])
    entries[insert_idx * 0x14:insert_idx * 0x14] = new_entry

    strings = bytearray(d[str_off:str_off + str_size])
    strings += member_name.encode("ascii") + b"\0"

    # fix nodes: target grows; any node starting at/after the splice shifts
    nodes = bytearray(d[node_off:node_off + num_nodes * 0x10])
    for n in range(num_nodes):
        cnt = struct.unpack_from(">H", nodes, n * 0x10 + 0x0A)[0]
        first = struct.unpack_from(">I", nodes, n * 0x10 + 0x0C)[0]
        if n == target_node:
            struct.pack_into(">H", nodes, n * 0x10 + 0x0A, cnt + 1)
        elif first >= insert_idx:
            struct.pack_into(">I", nodes, n * 0x10 + 0x0C, first + 1)

    # reassemble: header + info + nodes + entries + strings + pad + data + pad + payload
    old_data = d[data_abs:data_abs + data_size]
    ent_rel = (node_off - info) + len(nodes)
    str_rel = ent_rel + len(entries)
    data_rel = align32(0x20 + str_rel + len(strings)) - 0x20

    out = bytearray(d[:0x20])
    struct.pack_into(">I", out, 0x0C, data_rel)
    struct.pack_into(">I", out, 0x10, new_data_off + len(payload))
    info_blk = bytearray(d[info:info + 0x20])
    struct.pack_into(">I", info_blk, 0x08, num_entries + 1)
    struct.pack_into(">I", info_blk, 0x0C, ent_rel)
    struct.pack_into(">I", info_blk, 0x10, len(strings))
    struct.pack_into(">I", info_blk, 0x14, str_rel)
    # info+0x18 low u16 mirrors the file-entry id watermark in shipped arcs
    wm = struct.unpack_from(">H", info_blk, 0x18)[0]
    if wm in (max_file_id + 1, num_entries):
        struct.pack_into(">H", info_blk, 0x18, wm + 1)
    out += info_blk + nodes + entries + strings
    out += b"\0" * (0x20 + data_rel - len(out))
    out += old_data
    out += b"\0" * (new_data_off - len(old_data))
    out += payload
    struct.pack_into(">I", out, 0x04, len(out))

    shutil.copyfile(arc_path, arc_path + f".pre-{os.path.splitext(member_name)[0]}-bak")
    open(arc_path, "wb").write(out)
    print(f"{os.path.basename(arc_path)}: +{member_name} ({len(payload)} b) "
          f"into node {target_node} beside {sibling} — entries {num_entries}->{num_entries + 1}")


if __name__ == "__main__":
    main()
