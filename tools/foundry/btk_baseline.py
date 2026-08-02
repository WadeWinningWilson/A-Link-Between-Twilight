"""Foundry §220: BTK UV-motion baseline verdict — receiver tap vs donor CSVs.

Parses [DuskLog] §P2 btk lines (scene1) and Bridge's model1_btk_tracks.csv
(per-component key lists), evaluates the donor curve at every logged frame
(piecewise-linear between keys; Hermite mid-tangents absorbed by tolerance),
and reports per-track max deviation + verdict. Rotation compares in s16 units
(deg * 0x10000/360). Empty/missing streams => UNKNOWN.

Usage: btk_baseline.py <dusklight.log> <model1_btk_motion.csv> <out.md>
       btk_baseline.py <dusklight.log> --btk <Outset.arc> <out.md>
The --btk mode reads the REAL key tangents from the arc's model1.btk (gclib) and
evaluates with the donor's own Hermite (JMAHermiteInterpolation, JMath.cpp:82-94,
transcribed verbatim) — retiring the piecewise-linear evaluator artifacts.
"""
import sys, re, csv, io
from collections import defaultdict

def jma_hermite(frame, time0, value0, tangent0, time1, value1, tangent1):
    """JMAHermiteInterpolation — D:\\Decomps\\WW DP\\src\\JSystem\\JMath\\JMath.cpp:82-94, verbatim."""
    length = time1 - time0
    f9 = frame - time0
    f1 = 1.0 / length
    f2 = f9 * f9 * f1
    f10 = f2 * f1
    f11 = f9 * f10
    f12 = f11 * f1
    return (value0 * (1.0 + ((2.0 * f12) - (3.0 * f10)))) + \
           (value1 * ((-2.0 * f12) + (3.0 * f10))) + \
           (tangent0 * (f9 + (f11 - (2.0 * f2)))) + \
           (tangent1 * (f11 - f2))

def load_tracks_btk(arc_path):
    """Track tables from the real BTK: keys with tangents, donor axis mapping
    (Bridge §128: SX=scale_s SY=scale_t RZ=rotation_q TX=translation_s
    TY=translation_t). Rotation keys scale by 2**rotation_frac into s16 units."""
    sys.path.insert(0, r"D:\XXXXXXX\wwrando")
    from gclib.rarc import RARC
    from gclib.j3d import BTK
    data = open(arc_path, "rb").read()
    if data[:4] == b"Yaz0":
        from gclib.yaz0_yay0 import Yaz0
        data = Yaz0.decompress(io.BytesIO(data)).read()
    rarc = RARC(io.BytesIO(data))
    fe = next(f for f in rarc.file_entries if getattr(f, "name", "") == "model1.btk")
    btk = BTK(fe)
    ttk = btk.ttk1
    rot_scale = float(1 << getattr(ttk, "rotation_frac", 0))
    tracks = defaultdict(dict)
    axes = [("SX", "scale_s", 1.0), ("SY", "scale_t", 1.0),
            ("RZ_s16", "rotation_q", rot_scale),
            ("TX", "translation_s", 1.0), ("TY", "translation_t", 1.0)]
    flat = [a for anims in ttk.mat_name_to_anims.values() for a in anims]
    for ti, anim in enumerate(flat):
        for comp, attr, scale in axes:
            tr = getattr(anim, attr, None)
            if tr is None:
                continue
            keys = [(k.time, k.value * scale, k.tangent_in * scale,
                     k.tangent_out * scale) for k in tr.keyframes]
            tracks[ti][comp] = keys[0][1] if len(keys) == 1 else keys
    return tracks, float(ttk.duration if hasattr(ttk, "duration") else 100.0)

def ev_hermite(curve, f, length):
    if isinstance(curve, float):
        return curve
    ks = curve
    f = f % length if f > length else f
    if f <= ks[0][0]:
        return ks[0][1]
    for k0, k1 in zip(ks, ks[1:]):
        if k0[0] <= f <= k1[0]:
            if k1[0] == k0[0]:
                return k0[1]
            return jma_hermite(f, k0[0], k0[1], k0[3], k1[0], k1[1], k1[2])
    return ks[-1][1]

TAP = re.compile(r"§P2 btk t=\d+ tag=scene1 f=([\d.]+) mat=(\d+) mtx=(\d+) "
                 r"s=\(([-\d.]+),([-\d.]+)\) r=(-?\d+) tr=\(([-\d.]+),([-\d.]+)\)")

def load_tracks(csv_path):
    tracks = defaultdict(dict)  # track# -> comp -> [(f,v)...] or const v
    for row in csv.DictReader(open(csv_path, encoding="utf-8-sig")):
        t = int(row["track"])
        comp = row["component"]
        if row["kind"] == "const":
            tracks[t][comp] = float(row["value0"])
        else:
            keys = []
            for part in (row["keys"] or "").split("|"):
                part = part.strip()
                if ":" in part:
                    f, v = part.split(":")
                    keys.append((float(f), float(v)))
            tracks[t][comp] = keys or float(row["value0"])
    return tracks

def ev(curve, f, length=100.0):
    if isinstance(curve, float):
        return curve
    f = f % length if f > length else f
    ks = curve
    if f <= ks[0][0]:
        return ks[0][1]
    for (f0, v0), (f1, v1) in zip(ks, ks[1:]):
        if f0 <= f <= f1:
            return v0 if f1 == f0 else v0 + (v1 - v0) * (f - f0) / (f1 - f0)
    return ks[-1][1]

def main():
    if sys.argv[2] == "--btk":
        log_path, _, src_path, out_md = sys.argv[1:5]
        tracks, length = load_tracks_btk(src_path)
        hermite = True
    else:
        log_path, src_path, out_md = sys.argv[1:4]
        tracks = load_tracks(src_path)
        length, hermite = 100.0, False
    # tap (mat,mtx) order within a frame group == CSV track order
    samples = []
    for line in open(log_path, encoding="utf-8", errors="replace"):
        m = TAP.search(line)
        if m:
            samples.append(tuple(float(x) for x in m.groups()))
    if not samples:
        print("VERDICT: UNKNOWN — no scene1 tap lines.")
        sys.exit(3)
    # establish binding order from the first frame group
    order, seen = [], set()
    first_f = samples[0][0]
    for s in samples:
        if s[0] != first_f and len(order) > 2:
            break
        key = (int(s[1]), int(s[2]))
        if key not in seen:
            seen.add(key)
            order.append(key)
    t_of = {key: i for i, key in enumerate(order)}
    dev = defaultdict(lambda: defaultdict(float))
    n_cmp = defaultdict(int)
    for f, mat, mtx, sx, sy, r, tx, ty in samples:
        tr = t_of.get((int(mat), int(mtx)))
        if tr is None or tr not in tracks:
            continue
        comp_vals = {"SX": sx, "SY": sy, "TX": tx, "TY": ty}
        for comp, got in comp_vals.items():
            if comp in tracks[tr]:
                exp = (ev_hermite if hermite else ev)(tracks[tr][comp], f, length) \
                    if hermite else ev(tracks[tr][comp], f)
                dev[tr][comp] = max(dev[tr][comp], abs(got - exp))
        rz_key = "RZ_s16" if hermite else "RZ"
        if rz_key in tracks[tr]:
            if hermite:
                exp_r = ev_hermite(tracks[tr]["RZ_s16"], f, length)
            else:
                exp_r = ev(tracks[tr]["RZ"], f) * 0x10000 / 360.0
            got_r = r if r >= 0 else r + 0x10000
            exp_r = exp_r % 0x10000
            d = min(abs(got_r - exp_r), 0x10000 - abs(got_r - exp_r))
            dev[tr]["RZ_s16"] = max(dev[tr]["RZ_s16"], d)
        n_cmp[tr] += 1
    if hermite:
        TOL = {"SX": 0.005, "SY": 0.005, "TX": 0.005, "TY": 0.005, "RZ_s16": 8.0}
    else:
        TOL = {"SX": 0.05, "SY": 0.05, "TX": 0.03, "TY": 0.03, "RZ_s16": 96.0}
    lines = [f"# BTK UV-motion baseline — scene1 vs donor model1 law",
             f"", f"{len(samples)} samples, {len(order)} bindings, "
             f"tap↔track mapping by first-frame order.", "",
             "| track | (mat,mtx) | samples | comp | max dev | tol | verdict |",
             "|---|---|---|---|---|---|---|"]
    fails = 0
    for (key, tr) in sorted(t_of.items(), key=lambda kv: kv[1]):
        for comp in sorted(dev[tr]):
            d = dev[tr][comp]
            tol = TOL.get(comp, 0.05)
            ok = d <= tol
            fails += (not ok)
            lines.append(f"| {tr} | {key} | {n_cmp[tr]} | {comp} | {d:.4f} | "
                         f"{tol} | {'MATCH' if ok else 'DRIFT'} |")
    verdict = "MATCH" if fails == 0 else f"DRIFT ({fails} components)"
    lines.append(f"\n**VERDICT: {verdict}** — piecewise-linear eval; Hermite "
                 "mid-key curvature absorbed in tolerance; 3-key pulse tracks "
                 "carry the loosest bands.")
    open(out_md, "w", encoding="utf-8").write("\n".join(lines) + "\n")
    print(f"VERDICT: {verdict}; wrote {out_md}")
    sys.exit(0 if fails == 0 else 1)

if __name__ == "__main__":
    main()
