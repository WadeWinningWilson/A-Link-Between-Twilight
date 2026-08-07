#!/usr/bin/env python3
# ============================================================================
# bgm_wav_differ.py — §376: the BGM ratification differ (Foundry assignment
# from History's §372 JAudio1-on-PC milestone).
#
# THE GATE THIS CLOSES: "the ear passed" is not ratification. History's ported
# JAudio1 stack (donor sequence reader, tempo governor, channel mixer, banks,
# oscillators, voices) must be shown to render the SAME MUSIC as vanilla WW —
# by measurement, per track, with a stated verdict. This compares a Dusklight
# capture against a vanilla-WW (Dolphin) capture of the same track and reports
# per-dimension deltas, each chosen because it fails a DIFFERENT port bug:
#
#   TEMPO      dominant inter-onset period ratio    -> tempo-governor error
#              (the §372 governor is the exact thing under test)
#   PITCH      spectral centroid + peak-bin ratio   -> transpose / wrong sample
#              rate / oscillator table misread
#   TIMBRE     mean log-magnitude spectrum distance -> bank/voice selection,
#              ADSR envelope, mixer gain staging
#   STRUCTURE  onset-pattern cross-correlation      -> sequence reader stepping
#              (dropped/extra events, loop points)
#   LEVEL      RMS + peak                           -> mixer gain / clipping
#
# Alignment is measured, never assumed: the tool cross-correlates onset
# envelopes to find the best offset before any frame-wise comparison, so a
# recording that starts late still compares correctly.
#
# №31-C: any dimension it cannot compute reports UNKNOWN, never PASS.
# Read-only. numpy only (no heavy audio deps).
#
# Usage:
#   bgm_wav_differ.py <vanilla.wav> <dusklight.wav> [--label "Outset exterior"]
#   bgm_wav_differ.py --pair-dir <dir>   (expects *_vanilla.wav / *_dusk.wav)
# ============================================================================
import struct
import sys
import wave
from pathlib import Path

import numpy as np

SR = 22050          # common analysis rate
FRAME = 1024
HOP = 256
# verdict thresholds (deliberately generous on absolutes, tight on RATIOS:
# a port bug shows as a systematic ratio error, not a small level difference)
TEMPO_TOL = 0.03     # 3% — a tempo-governor bug is far larger than this
PITCH_TOL = 0.02     # 2% — a semitone is ~5.9%; half that is still audible
TIMBRE_TOL = 0.15    # mean |log-spectrum| difference, normalized
CENTROID_TOL = 0.08  # spectral centroid ratio — a TIMBRE feature, not pitch
STRUCT_MIN = 0.55    # onset-pattern correlation floor
ALIGN_MIN = 0.30     # below this = NOT the same passage -> UNKNOWN, not DRIFT
MIN_OVERLAP_S = 8.0  # minimum aligned overlap for any verdict


def load_wav(path):
    """Mono float32 at SR. Handles 8/16/24/32-bit PCM and float WAV."""
    with wave.open(str(path), "rb") as w:
        nch, width, rate, n = (w.getnchannels(), w.getsampwidth(),
                               w.getframerate(), w.getnframes())
        raw = w.readframes(n)
    if width == 2:
        a = np.frombuffer(raw, dtype="<i2").astype(np.float32) / 32768.0
    elif width == 1:
        a = (np.frombuffer(raw, dtype=np.uint8).astype(np.float32) - 128) / 128.0
    elif width == 4:
        a = np.frombuffer(raw, dtype="<i4").astype(np.float32) / 2147483648.0
    elif width == 3:
        b = np.frombuffer(raw, dtype=np.uint8).reshape(-1, 3)
        v = (b[:, 0].astype(np.int32) | (b[:, 1].astype(np.int32) << 8) |
             (b[:, 2].astype(np.int8).astype(np.int32) << 16))
        a = v.astype(np.float32) / 8388608.0
    else:
        raise ValueError(f"unsupported sample width {width}")
    if nch > 1:
        a = a.reshape(-1, nch).mean(axis=1)
    if rate != SR:  # linear resample (adequate for spectral comparison)
        idx = np.linspace(0, len(a) - 1, int(len(a) * SR / rate))
        a = np.interp(idx, np.arange(len(a)), a).astype(np.float32)
    return a


def spectrogram(x):
    if len(x) < FRAME:
        return None
    win = np.hanning(FRAME).astype(np.float32)
    frames = 1 + (len(x) - FRAME) // HOP
    S = np.empty((frames, FRAME // 2 + 1), dtype=np.float32)
    for i in range(frames):
        seg = x[i * HOP:i * HOP + FRAME] * win
        S[i] = np.abs(np.fft.rfft(seg))
    return S


def onset_env(S):
    """Spectral-flux onset envelope (positive differences), normalized."""
    d = np.diff(S, axis=0)
    d[d < 0] = 0
    e = d.sum(axis=1)
    if e.max() > 0:
        e = e / e.max()
    return e


def best_lag(a, b, max_lag_frames=800):
    """Cross-correlate onset envelopes; return (lag, correlation)."""
    n = min(len(a), len(b))
    if n < 32:
        return None, None
    a = a[:n] - a[:n].mean()
    b = b[:n] - b[:n].mean()
    denom = (np.linalg.norm(a) * np.linalg.norm(b)) or 1.0
    lags = range(-min(max_lag_frames, n - 16), min(max_lag_frames, n - 16))
    best, bl = -2.0, 0
    for L in lags:
        if L < 0:
            aa, bb = a[-L:], b[:len(b) + L]
        else:
            aa, bb = a[:len(a) - L], b[L:]
        m = min(len(aa), len(bb))
        if m < 16:
            continue
        c = float(np.dot(aa[:m], bb[:m]) / denom)
        if c > best:
            best, bl = c, L
    return bl, best


def tempo_period(e):
    """Dominant onset period (frames) via autocorrelation of the envelope."""
    if len(e) < 64:
        return None
    x = e - e.mean()
    ac = np.correlate(x, x, mode="full")[len(x) - 1:]
    lo, hi = 4, min(len(ac) - 1, 400)   # ~46ms..4.6s at HOP/SR
    if hi <= lo:
        return None
    k = int(np.argmax(ac[lo:hi]) + lo)
    return k


def summarize(x):
    S = spectrogram(x)
    if S is None:
        return None
    # §377: frame-wise MEDIAN, not mean — the captures carry game SFX
    # (yawn/gulls/waves/fire crackle). Transients move a mean and barely
    # move a median, so what gets compared is the music.
    mag = np.median(S, axis=0) + 1e-9
    freqs = np.fft.rfftfreq(FRAME, 1.0 / SR)
    centroid = float((mag * freqs).sum() / mag.sum())
    peak_bin = int(np.argmax(mag))
    return {
        "S": S,
        "onset": onset_env(S),
        "logspec": np.log(mag / mag.sum()),
        "centroid": centroid,
        "peak_hz": float(freqs[peak_bin]),
        "rms": float(np.sqrt((x ** 2).mean())),
        "peak": float(np.abs(x).max()),
        "seconds": len(x) / SR,
    }


def align_and_trim(xv, xd):
    """Offset both captures onto the same musical passage (onset xcorr over the
    FULL range), then trim to the aligned overlap. Every feature below is
    computed on that overlap — never on unaligned wholes."""
    sv, sd = summarize(xv), summarize(xd)
    if sv is None or sd is None:
        return None
    lag, corr = best_lag(sv["onset"], sd["onset"],
                         max_lag_frames=max(len(sv["onset"]), len(sd["onset"])))
    if lag is None:
        return None
    off = abs(lag) * HOP
    a, b = (xv, xd[off:]) if lag >= 0 else (xv[off:], xd)
    n = min(len(a), len(b))
    return {"van": a[:n], "dus": b[:n], "lag": lag, "corr": corr,
            "overlap_s": n / SR}


def verdict(label, xv, xd):
    print("")
    print(f"===== BGM RATIFICATION - {label} =====")
    al = align_and_trim(xv, xd)
    if al is None:
        print("  UNKNOWN - captures too short to align")
        return 2
    print(f"aligned overlap {al['overlap_s']:.1f}s at lag "
          f"{al['lag'] * HOP / SR:+.2f}s (alignment confidence {al['corr']:.3f})")
    # §377 CALIBRATION HONESTY: low alignment confidence has TWO causes that
    # this method cannot separate — (a) the captures are different passages,
    # (b) the port renders the passage so differently that onsets no longer
    # correlate (i.e. a REAL fault). Gating on it as UNKNOWN hid a known-broken
    # capture in testing, so it is ADVISORY: the axes are still reported, and
    # the verdict is labelled ADVISORY so a PASS can never be quoted as
    # ratification off a weak alignment.
    advisory = al["corr"] < ALIGN_MIN or al["overlap_s"] < MIN_OVERLAP_S
    if advisory:
        print(f"  [ADVISORY] weak alignment ({al['corr']:.3f}) or short overlap "
              f"({al['overlap_s']:.1f}s): either different passages OR a fault "
              f"large enough to destroy onset correlation. Axes below are "
              f"indicative, NOT ratification.")
    van, dus = summarize(al["van"]), summarize(al["dus"])
    fails = []

    lag, corr = best_lag(van["onset"], dus["onset"])
    if lag is None:
        print("  STRUCTURE : UNKNOWN (captures too short to align)")
        fails.append("STRUCTURE(UNKNOWN)")
    else:
        ok = corr is not None and corr >= STRUCT_MIN
        print(f"  STRUCTURE : onset-pattern corr={corr:.3f} at lag={lag} frames "
              f"({lag * HOP / SR:+.2f}s)  {'PASS' if ok else 'DRIFT'}")
        if not ok:
            fails.append("STRUCTURE")

    tv, td = tempo_period(van["onset"]), tempo_period(dus["onset"])
    if not tv or not td:
        print("  TEMPO     : UNKNOWN (no periodicity found)")
        fails.append("TEMPO(UNKNOWN)")
    else:
        ratio = td / tv
        ok = abs(ratio - 1.0) <= TEMPO_TOL
        print(f"  TEMPO     : period vanilla={tv} dusk={td} frames  ratio={ratio:.4f}"
              f"  {'PASS' if ok else 'DRIFT'}"
              f"{'' if ok else '   <-- tempo governor'}")
        if not ok:
            fails.append("TEMPO")

    # PITCH = FUNDAMENTAL only. Self-test lesson (§376): spectral centroid
    # moves when harmonics change, so including it made a timbre/bank fault
    # report as a transpose. Centroid is a timbre feature and lives below.
    pr = dus["peak_hz"] / (van["peak_hz"] or 1.0)
    ok = abs(pr - 1.0) <= PITCH_TOL
    print(f"  PITCH     : fundamental {van['peak_hz']:.0f}->{dus['peak_hz']:.0f} Hz "
          f"(x{pr:.4f})  {'PASS' if ok else 'DRIFT'}"
          f"{'' if ok else '   <-- transpose / sample-rate / oscillator'}")
    if not ok:
        fails.append("PITCH")

    n = min(len(van["logspec"]), len(dus["logspec"]))
    tim = float(np.abs(van["logspec"][:n] - dus["logspec"][:n]).mean())
    cr = dus["centroid"] / (van["centroid"] or 1.0)
    ok = tim <= TIMBRE_TOL and abs(cr - 1.0) <= CENTROID_TOL
    print(f"  TIMBRE    : mean |dlog-spectrum| = {tim:.3f}, centroid "
          f"{van['centroid']:.0f}->{dus['centroid']:.0f} Hz (x{cr:.4f})  "
          f"{'PASS' if ok else 'DRIFT'}"
          f"{'' if ok else '   <-- bank/voice/ADSR/gain'}")
    if not ok:
        fails.append("TIMBRE")

    lr = dus["rms"] / (van["rms"] or 1e-9)
    print(f"  LEVEL     : rms x{lr:.3f} (vanilla {van['rms']:.4f} -> "
          f"{dus['rms']:.4f}), peak {van['peak']:.3f} -> {dus['peak']:.3f}"
          f"{'   <-- clipping' if dus['peak'] >= 0.999 else ''}")

    print(f"\n  VERDICT: {'MATCH' if not fails else 'DRIFT: ' + ', '.join(fails)}")
    return 0 if not fails else 1


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    if "--pair-dir" in sys.argv:
        d = Path(sys.argv[sys.argv.index("--pair-dir") + 1])
        pairs = []
        for v in sorted(d.glob("*_vanilla.wav")):
            k = v.name[: -len("_vanilla.wav")]
            t = d / f"{k}_dusk.wav"
            if t.is_file():
                pairs.append((k, v, t))
        if not pairs:
            sys.exit(f"no *_vanilla.wav / *_dusk.wav pairs in {d}")
        rc = 0
        for label, v, t in pairs:
            rc |= verdict(label, load_wav(v), load_wav(t))
        return rc
    if len(args) < 2:
        sys.exit("usage: bgm_wav_differ.py <vanilla.wav> <dusklight.wav> "
                 "[--label X] | --pair-dir <dir>")
    label = (sys.argv[sys.argv.index("--label") + 1]
             if "--label" in sys.argv else Path(args[1]).stem)
    return verdict(label, load_wav(args[0]), load_wav(args[1]))


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    raise SystemExit(main())
