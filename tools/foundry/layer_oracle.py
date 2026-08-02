"""Foundry P10: the donor layer oracle.

Given a donor .gci save, computes the EXPECTED story layer per stage/room —
a pure-Python, line-cited, VERBATIM transcription of the donor's own logic
(covenant: donor-verbatim law; vague items are [INFERENCE-NEEDED], never invented).

Donor sources (decomp @ D:\\Decomps\\WW DP, USA/retail branch):
  layer law   src/d/d_com_inf_game.cpp:185-269  dComIfG_play_c::getLayerNo(int)
  bit test    src/d/d_save.cpp:1199-1201        dSv_event_c::isEventBit(u16)
  flag values include/d/d_save_event_flag.inc   (names literally encode value)
  card layout include/m_Do/m_Do_MemCardRWmng.h  card_savedata/card_gamedata
  slot csum   src/m_Do/m_Do_MemCardRWmng.cpp:335-344 (u64: sum<<32 | ~sum)
  save sector src/m_Do/m_Do_MemCardRWmng.cpp:58-68 (sector 1, mirror sector 2)
  packed sizes include/d/d_save.h STATIC_ASSERTs (summed below, asserted)

Usage:
  layer_oracle.py <save.gci> [--slot 0|1|2] [--hour 12] [--nightstop]
                  [--triforce N] [--room 44]
"""
import sys, argparse

# ---- packed card_gamedata layout (each size cited from d_save.h asserts) ----
PACKED = [
    ("player_status_a", 0x18), ("player_status_b", 0x18),
    ("player_return_place", 0xC), ("player_item", 0x15),
    ("player_get_item", 0x15), ("player_item_record", 0x8),
    ("player_item_max", 0x8), ("player_bag_item", 0x18),
    ("player_get_bag_item", 0xC), ("player_bag_item_record", 0x18),
    ("player_collect", 0xD), ("player_map", 0x84), ("player_info", 0x5C),
    ("player_config", 0x5), ("player_priest", 0x10),
    ("player_status_c", 0x70 * 4),          # PLAYER_STATUS_C_COUNT = 4 (d_save.h:586)
    ("memory", 0x24 * 16),                  # dSv_memory_c * STAGE_MAX(=0x10)
    ("ocean", 0x64),
    ("event", 0x100),                       # dSv_event_c
    ("reserve", 0x50),
]
SLOT_DATA_SIZE = 0x768                       # dSv_save_c::PACKED_STRUCT_SIZE
SLOT_SIZE = 0x770                            # + u64 csum
assert sum(s for _, s in PACKED) == SLOT_DATA_SIZE
EVENT_OFF = sum(s for n, s in PACKED[:PACKED.index(("event", 0x100))])
assert EVENT_OFF == 0x618

GCI_HEADER = 0x40
SECTOR = 0x2000
GAMEDATA_OFF = 0x8                           # card_savedata: count,version,gamedata[3]

def slot_csum(data):
    """m_Do_MemCardRWmng.cpp:335-344, verbatim. C promotes the u8 before ~, so
    c1 accumulates the FULL 32-bit complement of each byte (transcription trap
    caught by the real-save validation run — see bus §212)."""
    c0 = c1 = 0
    for b in data:
        c0 = (c0 + b) & 0xFFFFFFFF
        c1 = (c1 + (~b & 0xFFFFFFFF)) & 0xFFFFFFFF
    return (c0 << 32) | c1

def read_event_flags(gci_path, slot):
    raw = open(gci_path, "rb").read()
    for sector_idx in (1, 2):                # primary, then mirror (RWmng.cpp:58-68)
        base = GCI_HEADER + sector_idx * SECTOR + GAMEDATA_OFF + slot * SLOT_SIZE
        data = raw[base:base + SLOT_DATA_SIZE]
        stored = int.from_bytes(raw[base + SLOT_DATA_SIZE:base + SLOT_SIZE], "big")
        if len(data) == SLOT_DATA_SIZE and slot_csum(data) == stored:
            return data[EVENT_OFF:EVENT_OFF + 0x100], sector_idx
    return None, None

def is_event_bit(flags, no):
    """d_save.cpp:1199-1201, verbatim: mFlags[no>>8] & (no & 0xFF)."""
    return bool(flags[no >> 8] & (no & 0xFF))

# Flag constants (d_save_event_flag.inc — names literally encode values)
F = {"UNK_0520": 0x0520, "UNK_0E20": 0x0E20, "UNK_0101": 0x0101,
     "UNK_2D01": 0x2D01, "UNK_1820": 0x1820, "MET_KORL": 0x0F80,
     "UNK_2C01": 0x2C01, "UNK_3280": 0x3280, "UNK_3B40": 0x3B40,
     "UNK_3B02": 0x3B02, "UNK_4002": 0x4002, "COLORS_IN_HYRULE": 0x3802}

ROOM_OUTSET, ROOM_WINDFALL, ROOM_FF = 44, 11, 1   # d_save.h dIsleRoom enum
# [INFERENCE-NEEDED] dIsleRoom_WindfallIsland_e / _ForsakenFortress_e values not
# read from the decomp this pass — Outset(44) is cited (d_save.h:350); verify the
# other two before trusting their rows.

def get_layer_no(stage, room_no, flags, hour, night_stop, start_stage_layer,
                 triforce_num):
    """d_com_inf_game.cpp:185-269, verbatim (USA retail branch of VERSION_SELECTs)."""
    if start_stage_layer >= 0:
        return start_stage_layer, "start-stage override"
    layer = 1 if night_stop else (0 if 6 <= hour < 18 else 1)
    eb = lambda name: is_event_bit(flags, F[name])
    if stage == "sea":
        if room_no == ROOM_OUTSET:
            if eb("UNK_0520"):  return layer | 4, "sea/Outset UNK_0520"
            if eb("UNK_0E20"):  return layer | 2, "sea/Outset UNK_0E20"
            if eb("UNK_0101"):  return 9, "sea/Outset UNK_0101"
        elif room_no == ROOM_WINDFALL:
            if eb("UNK_2D01"):  return layer | 4, "sea/Windfall UNK_2D01"
            if night_stop:      return layer | 2, "sea/Windfall nightstop"
        elif room_no == ROOM_FF:
            return (3 if eb("UNK_1820") else 1), "sea/FF UNK_1820"
    elif stage == "A_mori":
        if eb("MET_KORL"):      return layer | 2, "A_mori MET_KORL"
    elif stage == "Asoko":
        if eb("UNK_0520"):      return layer | 2, "Asoko UNK_0520"
    elif stage == "Hyrule":
        if triforce_num == 8:   return layer | 4, "Hyrule triforce==8"
        if eb("UNK_3280"):      return layer | 2, "Hyrule UNK_3280"
    elif stage == "Hyroom":
        if triforce_num == 8 and not eb("UNK_2C01"):
            return layer | 4, "Hyroom triforce&&!2C01"
        if eb("UNK_3280"):      return layer | 2, "Hyroom UNK_3280"
        if eb("UNK_3B40"):      return layer | 6, "Hyroom UNK_3B40"
    elif stage == "kenroom":
        if eb("UNK_2C01") or (eb("COLORS_IN_HYRULE") and not eb("UNK_3280")):
            return layer | 6, "kenroom USA branch"
        if triforce_num == 8:   return layer | 4, "kenroom triforce==8"
        if eb("COLORS_IN_HYRULE"): return layer | 2, "kenroom COLORS(USA)"
    elif stage == "M2tower":
        if eb("UNK_2D01"):      return layer | 2, "M2tower UNK_2D01"
    elif stage == "GanonK":
        if not eb("UNK_3B02"):  return 8, "GanonK !UNK_3B02"
    elif stage == "GTower":
        if not eb("UNK_4002"):  return 8, "GTower !UNK_4002"
    return layer, "base day/night"

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("gci")
    ap.add_argument("--slot", type=int, default=0)
    ap.add_argument("--hour", type=int, default=12)
    ap.add_argument("--nightstop", action="store_true")
    ap.add_argument("--triforce", type=int, default=0,
                    help="[INFERENCE-NEEDED to parse from save; pass manually]")
    args = ap.parse_args()
    flags, sector = read_event_flags(args.gci, args.slot)
    if flags is None:
        print("VERDICT: UNKNOWN — no slot passed the donor checksum "
              "(wrong slot index, empty slot, or layout mismatch).")
        sys.exit(3)
    print(f"slot {args.slot} validated by donor u64 checksum (sector {sector})\n")
    print("relevant event bits:")
    for name, val in sorted(F.items(), key=lambda kv: kv[1]):
        print(f"  {name} (0x{val:04X}) = {int(is_event_bit(flags, val))}")
    print(f"\nexpected layers (hour={args.hour} nightstop={args.nightstop} "
          f"triforce={args.triforce}):")
    cases = [("sea", ROOM_OUTSET, "Outset"), ("sea", ROOM_WINDFALL, "Windfall [INFERENCE: room id unverified]"),
             ("sea", ROOM_FF, "Forsaken Fortress [INFERENCE: room id unverified]"),
             ("A_mori", -1, ""), ("Asoko", -1, ""), ("Hyrule", -1, ""),
             ("Hyroom", -1, ""), ("kenroom", -1, ""), ("M2tower", -1, ""),
             ("GanonK", -1, ""), ("GTower", -1, "")]
    for stage, room, note in cases:
        n, why = get_layer_no(stage, room, flags, args.hour, args.nightstop,
                              -1, args.triforce)
        label = f"{stage}" + (f"/room{room}" if room >= 0 else "")
        print(f"  {label:<14} -> layer {n}  ({why}) {note}")

if __name__ == "__main__":
    main()
