#!/usr/bin/env python3
# ============================================================
# extseq_note_differ.py  --  P2 note-fidelity differ (Foundry)
#
# Verdict on whether the port's ExtSeq player performs the same
# NOTES as Wind Waker, note-for-note.
#
#   donor stream = a DuskTap capture at TTrack::noteOn @80281258
#                  (JASSeqParser.cpp:915 track->noteOn(noteid,note,r25,time,r22))
#                  e.g. docs/WW Linked/dolphin-captures-bgm-notes-20260728.txt
#                  lines:  <ts> note=<voice> p=(<key>,<vel>,<gate>)
#
#   port  stream = the receiver mirror emitted by Ja1Track::noteOn
#                  behind DUSK_EXTSEQ_NOTE_TAP=1, tag "[ExtSeq] §P2 noteTap"
#                  same shape:  note=<voice> p=(<key>,<vel>,<gate>)
#
# Both parse to an ordered sequence of (voice, key, vel, gate) tuples.
# Absolute timestamps do NOT align across runs, so the comparison is
# SEQUENCE alignment (difflib), not timestamp matching. Timing fidelity
# is a separate axis (ext-seq-audio-findings.md §B).
#
# Outputs a note-by-note verdict:
#   - strict match %  : full (voice,key,vel,gate) identical
#   - pitch  match %  : (voice,key) identical (right notes, dynamics aside)
#   - mismatch classes: dynamics-drift / wrong-note / donor-only / port-only
#   - first N concrete diffs
#
# Donor capture is multi-song (~11 min); window it with --donor-start /
# --donor-end (capture seconds) to isolate the song the port played.
# ============================================================
import argparse
import difflib
import re
import sys
from collections import Counter

# Note payload, present in BOTH streams:
#   donor: ` 2453.191 note=  0 p=(0018,005f,0d08)`
#   port : `... [ExtSeq] §P2 noteTap note=  0 p=(0018,005f,0d08)`
NOTE_RE = re.compile(
    r'note=\s*(?P<voice>[0-9a-fA-F]+)\s+'
    r'p=\(\s*(?P<key>[0-9a-fA-F]+)\s*,\s*(?P<vel>[0-9a-fA-F]+)\s*,\s*(?P<gate>[0-9a-fA-F]+)\s*\)'
)
# Donor capture timestamp = leading `<sec>.<ms>` (port lines carry DuskLog's
# own prefix instead; ts is only used to window the donor stream).
TS_RE = re.compile(r'^\s*(\d+\.\d+)\b')
# The port line is only the tap tag; ignore any other "note=" text.
PORT_GUARD = 'noteTap'


def parse(path, want_tag=None, ts_start=None, ts_end=None):
    """Return (tuples, timestamps). tuple = (voice,key,vel,gate) ints."""
    tuples, times = [], []
    with open(path, 'r', encoding='utf-8', errors='replace') as fh:
        for line in fh:
            if want_tag is not None and want_tag not in line:
                continue
            m = NOTE_RE.search(line)
            if not m:
                continue
            tsm = TS_RE.match(line)
            ts = float(tsm.group(1)) if tsm else None
            if ts is not None and ts_start is not None and ts < ts_start:
                continue
            if ts is not None and ts_end is not None and ts > ts_end:
                continue
            v = int(m.group('voice'), 16)
            k = int(m.group('key'), 16)
            vel = int(m.group('vel'), 16)
            g = int(m.group('gate'), 16)
            tuples.append((v, k, vel, g))
            times.append(ts)
    return tuples, times


def histo_line(counter, n=8, fmt=lambda x: str(x)):
    top = counter.most_common(n)
    return ', '.join(f'{fmt(k)}:{c}' for k, c in top)


def summarize(name, tuples):
    voices = Counter(t[0] for t in tuples)
    keys = Counter(t[1] for t in tuples)
    vels = Counter(t[2] for t in tuples)
    gates = Counter(t[3] for t in tuples)
    print(f'  {name}: {len(tuples)} notes')
    print(f'    voices  : {histo_line(voices, 6)}')
    print(f'    keys    : {histo_line(keys, 8, lambda k: f"0x{k:02x}")}')
    print(f'    vels    : {histo_line(vels, 8, lambda k: f"0x{k:02x}")}')
    print(f'    gates   : {histo_line(gates, 6, lambda k: f"0x{k:04x}")}')


def diff(donor, port, max_examples=40):
    # Full-tuple alignment (strict): right note AND right dynamics.
    sm = difflib.SequenceMatcher(a=donor, b=port, autojunk=False)
    equal = donor_only = port_only = replace_n = 0
    dyn_drift = wrong_note = 0
    examples = []
    for tag, i1, i2, j1, j2 in sm.get_opcodes():
        if tag == 'equal':
            equal += (i2 - i1)
        elif tag == 'delete':
            donor_only += (i2 - i1)
            for t in donor[i1:i2]:
                if len(examples) < max_examples:
                    examples.append(('donor-only', t, None))
        elif tag == 'insert':
            port_only += (j2 - j1)
            for t in port[j1:j2]:
                if len(examples) < max_examples:
                    examples.append(('port-only', None, t))
        elif tag == 'replace':
            replace_n += max(i2 - i1, j2 - j1)
            # sub-classify pairwise where the blocks line up
            span = min(i2 - i1, j2 - j1)
            for off in range(span):
                dt, pt = donor[i1 + off], port[j1 + off]
                if dt[0] == pt[0] and dt[1] == pt[1]:
                    dyn_drift += 1  # same voice+key, vel/gate differ
                    cls = 'dyn-drift'
                else:
                    wrong_note += 1
                    cls = 'wrong-note'
                if len(examples) < max_examples:
                    examples.append((cls, dt, pt))
            # leftover unpaired rows in the longer side
            if (i2 - i1) > span:
                donor_only += (i2 - i1) - span
            if (j2 - j1) > span:
                port_only += (j2 - j1) - span

    # Pitch-only alignment (voice,key): right notes, dynamics aside.
    dp = [(t[0], t[1]) for t in donor]
    pp = [(t[0], t[1]) for t in port]
    sm2 = difflib.SequenceMatcher(a=dp, b=pp, autojunk=False)
    pitch_equal = sum(b.size for b in sm2.get_matching_blocks())

    return dict(equal=equal, donor_only=donor_only, port_only=port_only,
                replace=replace_n, dyn_drift=dyn_drift, wrong_note=wrong_note,
                pitch_equal=pitch_equal, examples=examples)


def main():
    ap = argparse.ArgumentParser(description='P2 note-by-note fidelity differ (donor vs port)')
    ap.add_argument('donor', help='donor DuskTap capture (TTrack::noteOn)')
    ap.add_argument('port', help='port log containing §P2 noteTap lines')
    ap.add_argument('--donor-start', type=float, default=None, help='window donor by capture seconds (>=)')
    ap.add_argument('--donor-end', type=float, default=None, help='window donor by capture seconds (<=)')
    ap.add_argument('--examples', type=int, default=40, help='max concrete diffs to print')
    args = ap.parse_args()

    donor, _ = parse(args.donor, want_tag=None,
                     ts_start=args.donor_start, ts_end=args.donor_end)
    port, _ = parse(args.port, want_tag=PORT_GUARD)

    if not donor:
        print(f'ERROR: no notes parsed from donor {args.donor}', file=sys.stderr)
        return 2
    if not port:
        print(f'ERROR: no §P2 noteTap notes parsed from port {args.port}\n'
              f'       (run the port with DUSK_EXTSEQ_NOTE_TAP=1 and capture the log)',
              file=sys.stderr)
        return 2

    print('=== INPUT ===')
    win = ''
    if args.donor_start is not None or args.donor_end is not None:
        win = f'  [windowed {args.donor_start}..{args.donor_end}s]'
    summarize('donor' + win, donor)
    summarize('port ', port)

    r = diff(donor, port, args.examples)
    dn = len(donor)
    strict_pct = 100.0 * r['equal'] / dn if dn else 0.0
    pitch_pct = 100.0 * r['pitch_equal'] / dn if dn else 0.0

    print('\n=== VERDICT ===')
    print(f'  strict match (voice,key,vel,gate) : {r["equal"]}/{dn}  = {strict_pct:5.1f}% of donor notes')
    print(f'  pitch  match (voice,key)          : {r["pitch_equal"]}/{dn}  = {pitch_pct:5.1f}% of donor notes')
    print(f'  --- mismatch classes ---')
    print(f'  dynamics drift (same note, vel/gate differ) : {r["dyn_drift"]}')
    print(f'  wrong note     (voice/key differ)           : {r["wrong_note"]}')
    print(f'  donor-only     (port never played)          : {r["donor_only"]}')
    print(f'  port-only      (port played extra)          : {r["port_only"]}')

    if r['examples']:
        print(f'\n=== FIRST {len(r["examples"])} DIFFS (cls | donor | port) ===')
        def fmt(t):
            if t is None:
                return '        --        '
            return f'v{t[0]} k0x{t[1]:02x} vel0x{t[2]:02x} g0x{t[3]:04x}'
        for cls, dt, pt in r['examples']:
            print(f'  {cls:11s} | {fmt(dt)} | {fmt(pt)}')

    # exit non-zero if the port is materially unfaithful (tunable gate)
    return 0 if strict_pct >= 99.0 else 1


if __name__ == '__main__':
    sys.exit(main())
