"""Foundry §264: the AUDIO DIFFER — why WW music sounds wrong, measured.

Static instrument (no capture needed): walks every donor BMS sequence with the
EXACT byte-consumption rules of the port's Ja1Parser (which mirror the donor
Arglist), then diffs three things per track:

  1. OPCODE COVERAGE — which command sites hit ops the port parser NOPs for
     playback (consumed-per-Arglist, no effect). Labeled with the donor's own
     handler names (JASSeqParser::sCmdPList, src verbatim).
  2. DISPATCH DIFF — port dispatch vs donor sCmdPList index-by-index (the
     §264 finding: 0xD8/0xD9/0xDA are SHIFTED — donor ADSR plays as transpose,
     donor transpose CLOSES a track, donor close NOPs).
  3. BANK COVERAGE — bank/prog register writes seen per track vs the .aw banks
     actually staged in the mod (audio/ww_jaudio1/banks/).

Walk is static reachability (work-list over open_track/call/jmp targets,
visited-set loop guard); loops make execution counts unknowable statically, so
counts are SITES not executions. Dynamic register-driven ops (0xB8-0xBF nested,
0xF9 reg note-off) stop that path and are counted as truncations — honest tier.

Usage: audio_differ.py [--seq name.bms ...] [--out report.md]
Run with D:\\Decomps\\foundry-py312\\Scripts\\python.exe (gclib venv).
"""
import io, os, re, sys
from collections import Counter, defaultdict

AAF = r"D:\XXXXXXX\Ex WW\files\Audiores\JaiInit.aaf"
SEQ_ARC = r"D:\XXXXXXX\Ex WW\files\Audiores\Seqs\JaiSeqs.arc"
MOD_BANKS = (r"C:\Users\xxxxx\AppData\Roaming\TwilitRealm\Dusklight"
             r"\model_replacements\WW-Crew-Restoration\audio\ww_jaudio1\banks")

# Donor sCmdPList (JASSeqParser.cpp:15, verbatim order) — op = 0xC0 + index.
DONOR_CMDS = [
    None, "OpenTrack", "OpenTrackBros", None, "Call", None, "Ret", None,
    "Jmp", "LoopS", "LoopE", "ReadPort", "WritePort", "CheckPortImport",
    "CheckPortExport", "Wait", None, "ParentWritePort", "ChildWritePort",
    None, "SetLastNote", "TimeRelate", "SimpleOsc", "SimpleEnv", "SimpleADSR",
    "Transpose", "CloseTrack", "OutSwitch", "UpdateSync", "BusConnect",
    "PauseStatus", "SetInterrupt", "DisInterrupt", "ClrI", "SetI", "RetI",
    "IntTimer", "VibDepth", "VibDepthMidi", "SyncCPU", "FlushAll",
    "FlushRelease", "Wait2", "PanPowSet", "IIRSet", "FIRSet", "EXTSet",
    "PanSwSet", "OscRoute", "IIRCutOff", "OscFull", "VolumeMode", "VibPitch",
    None, None, None, None, None, "CheckWave", "Printf", "Nop", "Tempo",
    "TimeBase", "Finish",
]
# Port Ja1Parser::Cmd_Process dispatch — PARSED LIVE from ja1_parser.cpp
# (§266: hardcoding it hid landings; now every parser fix shows up on rerun).
PORT_PARSER = (r"C:\Users\xxxxx\Documents\dusklight\src\d\ext_seq"
               r"\ja1_parser.cpp")

def port_dispatch():
    t = open(PORT_PARSER, encoding="utf-8", errors="replace").read()
    m = re.search(r"Cmd_Process\([^)]*\)\s*\{(.*?)\n\}", t, re.S)
    body = m.group(1) if m else t
    out = {}
    for cm in re.finditer(r"case (0x[0-9A-Fa-f]{2}):\s*(?://[^\n]*\n\s*)?"
                          r"return cmd(\w+)\(", body):
        out[int(cm.group(1), 16)] = cm.group(2)
    return out

PORT_CMDS = port_dispatch()
# Arglist consumption (ja1_parser.cpp:410, == donor Arglist).
ARG_COUNT = [0, 2, 2, 1, 0, 0, 1, 1, 0, 1, 0, 2, 2, 1, 1, 1, 0, 2, 2, 0, 1, 1,
             1, 2, 5, 1, 1, 1, 1, 2, 1, 2, 1, 0, 0, 0, 2, 1, 1, 1, 0, 0, 1, 5,
             4, 1, 1, 3, 1, 1, 3, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0]
ARG_FMT = [0x0000, 0x0008, 0x0008, 0x0002, 0x0000, 0x0000, 0x0000, 0x0002,
           0x0000, 0x0001, 0x0000, 0x0000, 0x000C, 0x0000, 0x0000, 0x0003,
           0x0000, 0x000C, 0x000C, 0x0000, 0x0000, 0x0000, 0x0000, 0x0008,
           0x0155, 0x0000, 0x0000, 0x0000, 0x0001, 0x0004, 0x0000, 0x0008,
           0x0000, 0x0000, 0x0000, 0x0000, 0x0004, 0x0000, 0x0001, 0x0001,
           0x0000, 0x0000, 0x0002, 0x0000, 0x0055, 0x0002, 0x0002, 0x0000,
           0x0000, 0x0000, 0x0028, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
           0x0000, 0x0000, 0x0001, 0x0000, 0x0000, 0x0001, 0x0001, 0x0000]
FMT_LEN = {0: 1, 1: 2, 2: 3, 3: 1}  # u8, u16, u24, reg(1 byte)


class Walker:
    def __init__(self, data):
        self.d = data
        self.sites = Counter()          # opcode -> distinct sites
        self.bankprog = []              # (dest_reg_or_param, value)
        self.truncations = 0
        self.visited = set()

    class _End(Exception):
        pass

    def u8(self):
        if self.p >= len(self.d):
            raise Walker._End()
        v = self.d[self.p]; self.p += 1; return v

    def u16(self):
        v = int.from_bytes(self.d[self.p:self.p+2], "big"); self.p += 2; return v

    def u24(self):
        v = int.from_bytes(self.d[self.p:self.p+3], "big"); self.p += 3; return v

    def walk(self, start):
        work = [start]
        while work:
            self.p = work.pop()
            try:
                self._run(work)
            except Walker._End:
                self.truncations += 1
        return self

    def _run(self, work):
            while True:
                if self.p >= len(self.d) or self.p in self.visited:
                    break
                self.visited.add(self.p)
                op_at = self.p
                flag = self.u8()
                if not (flag & 0x80):                      # note on
                    self.sites["note_on"] += 1
                    self.u8(); self.u8()
                    continue
                if (flag & 0xF0) == 0x80 and not (flag & 0x07):   # wait
                    self.sites["wait"] += 1
                    self.u8() if flag == 0x80 else self.u16()
                    continue
                if (flag & 0xF0) == 0x80 or flag == 0xF9:  # note off
                    self.sites["note_off"] += 1
                    if flag == 0xF9:
                        self.truncations += 1              # reg-driven form
                        break
                    if flag & 0x08:
                        self.u8()
                    continue
                hi = flag & 0xF0
                if hi == 0x90:                             # setParam
                    self.sites["set_param"] += 1
                    n = flag & 0xF
                    self.u8()
                    {0: self.u8, 4: self.u8, 8: self.u8,
                     12: self.u16}[n & 0xC]()
                    [lambda: None, self.u8, self.u8, self.u16][n & 0x3]()
                    continue
                if hi == 0xA0:                             # writeRegParam
                    n = flag & 0xF
                    self.sites[f"reg_A{n:X}"] += 1
                    if n in (9, 10, 11):
                        if n in (9, 10):
                            self.u8()
                        self.u8()
                        {0: self.u8, 4: self.u8, 8: self.u8,
                         12: self.u16}[n & 0xC]()
                        continue
                    dest = self.u8()
                    val = {0: self.u8, 4: self.u8, 8: self.u8,
                           12: self.u16}[n & 0xC]()
                    if n == 4:                             # bank/prog path
                        self.bankprog.append((dest, val))
                    continue
                if hi == 0xB0:                             # nested reg cmd
                    if (flag >> 3) & 1:
                        self.sites["reg_cmd_dynamic"] += 1
                        self.truncations += 1
                        break
                    nested = self.u8()
                    if not self.cmd(nested, work, op_at):
                        break
                    continue
                if not self.cmd(flag, work, op_at):
                    break
            # end linear path

    def cmd(self, op, work, site):
        """0xC0+ command. Returns False to stop the linear path."""
        if op < 0xC0:
            return False
        idx = op - 0xC0
        name = DONOR_CMDS[idx] or f"cmd_{op:02X}"
        self.sites[f"{op:02X}_{name}"] += 1
        if op == 0xC4 or op == 0xC8:                      # call / jmp
            flag = self.u8()
            if flag & 0x80:
                self.u8()
                if flag & 0x40:
                    self.u8() if flag & 0x20 else self.u24()
                # register-indexed target: statically unknowable
                self.truncations += 1
                return op != 0xC8
            tgt = self.u24()
            work.append(tgt)
            return op != 0xC8 or (flag & 0x07) != 0       # uncond jmp ends path
        n, fmt = ARG_COUNT[idx], ARG_FMT[idx]
        args = []
        for _ in range(n):
            k = fmt & 3
            args.append({0: self.u8, 1: self.u16, 2: self.u24}[k]()
                        if k != 3 else self.u8())
            fmt >>= 2
        if op == 0xC1 and len(args) >= 2:                 # open track
            work.append(args[1])
        if op == 0xFF:                                     # finish
            return False
        return True


def load_seqs():
    from gclib.rarc import RARC
    rarc = RARC(io.BytesIO(open(SEQ_ARC, "rb").read()))
    out = []
    for fe in rarc.file_entries:
        nm = getattr(fe, "name", "")
        if nm.lower().endswith(".bms"):
            raw = fe.data
            raw = raw.read() if hasattr(raw, "read") else bytes(raw)
            if hasattr(fe.data, "seek"):
                fe.data.seek(0)
            out.append((nm, raw))
    return out


def dispatch_diff():
    rows = []
    for idx, donor in enumerate(DONOR_CMDS):
        op = 0xC0 + idx
        port = PORT_CMDS.get(op)
        if donor is None and port is None:
            continue
        if donor != port:
            state = (f"**MISDISPATCH: donor `{donor}` -> port `{port}`**"
                     if port else
                     f"NOP'd in port (donor `{donor}`)")
            rows.append((op, donor or "(NULL)", port or "(default NOP)",
                         state))
    return rows


def main():
    argv = sys.argv[1:]
    out = None
    if "--out" in argv:
        out = argv[argv.index("--out") + 1]
    only = [argv[i + 1] for i, a in enumerate(argv) if a == "--seq"] or None
    staged = sorted(os.listdir(MOD_BANKS)) if os.path.isdir(MOD_BANKS) else []
    implemented = {"note_on", "wait", "note_off", "set_param"} | \
        {f"{op:02X}_{n}" for op, n in PORT_CMDS.items()} | \
        {f"reg_A{n:X}" for n in range(9)} | {"reg_AC", "reg_AD", "reg_AE",
                                             "reg_AF"}
    lines = ["# WW audio differ — per-track opcode/bank coverage vs the port "
             "parser (§264)", "",
             "## 1. DISPATCH DIFF — port Cmd_Process vs donor sCmdPList "
             "(the systemic findings)", "",
             "| op | donor handler | port handler | verdict |",
             "|---|---|---|---|"]
    for op, dn, pn, state in dispatch_diff():
        lines.append(f"| 0x{op:02X} | {dn} | {pn} | {state} |")
    lines += ["", f"## 2. Per-track coverage ({'chosen' if only else 'ALL'} "
              "donor BMS; counts are SITES, statically reachable)", "",
              "| track | sites | NOP'd sites | NOP % | top NOP'd ops | "
              "bank/prog writes | truncated paths |",
              "|---|---|---|---|---|---|---|"]
    totals = Counter()
    for nm, raw in load_seqs():
        if only and nm not in only:
            continue
        w = Walker(raw).walk(0)
        total = sum(w.sites.values())
        nop = {k: c for k, c in w.sites.items()
               if k not in implemented and not k.startswith("reg_cmd")}
        n_nop = sum(nop.values())
        top = ", ".join(f"`{k}`x{c}" for k, c in
                        Counter(nop).most_common(4))
        banks = ", ".join(f"r{d}={v}" for d, v in
                          sorted(set(w.bankprog))[:6]) or "—"
        pct = 100.0 * n_nop / total if total else 0.0
        lines.append(f"| `{nm}` | {total} | {n_nop} | {pct:.0f}% | {top} | "
                     f"{banks} | {w.truncations} |")
        totals["sites"] += total
        totals["nop"] += n_nop
        for k, c in nop.items():
            totals["op:" + k] += c
    pct = 100.0 * totals["nop"] / max(1, totals["sites"])
    top_all = Counter({k[3:]: c for k, c in totals.items()
                       if k.startswith("op:")}).most_common(12)
    lines += ["", f"**TOTALS: {totals['sites']} sites; {totals['nop']} "
              f"({pct:.0f}%) hit NOP'd/misdispatched opcodes.** Top offenders "
              "across all tracks:", ""]
    lines += [f"- `{k}` × {c}" for k, c in top_all]
    lines += ["", f"## 3. Staged banks (mod ww_jaudio1/banks): "
              f"{', '.join(staged) or 'NONE'}",
              "", "> Donor Audiores carries ~50 .aw banks; a track whose "
              "bank/prog writes resolve outside the staged set plays with "
              "wrong/missing instruments. (Bank-id -> .aw mapping via "
              "bank_waves.csv / the AAF ws directory — [INFERENCE-NEEDED "
              "until the id->aw join is receipted].)"]
    body = "\n".join(lines) + "\n"
    if out:
        os.makedirs(os.path.dirname(out) or ".", exist_ok=True)
        open(out, "w", encoding="utf-8").write(body)
        print(f"wrote {out}")
    else:
        print(body)


if __name__ == "__main__":
    main()
