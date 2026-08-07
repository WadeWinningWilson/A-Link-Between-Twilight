#!/usr/bin/env python3
# ============================================================================
# input_trace.py — hash the files a generator ACTUALLY OPENED.
#
# WHY THIS REPLACES REMEMBERED INPUT LISTS
# Three stamps were added in sequence, each closing the omission the last one
# missed: tool_sha256 (§527, by analogy), input_sha256 on the .def (§531),
# derivation_sha256 over the roster (§535). Each was a judgment about which
# inputs mattered, and each judgment was incomplete. §536 demonstrated the next
# one: a line into a roster TU moves derivation_sha256, a line into
# `sdk/include/mods/api.h` does not -- yet `sdk_service_symbols()` reads those
# headers and produces the (a) SERVICE bucket, of which (c) is the complement.
# Publishing a new SDK service changes the count with every stamp green.
#
# A fourth stamp would have the same shape as the first three. So this stops
# enumerating inputs and RECORDS them instead: patch the read paths for the
# duration of a generation, note every file opened and its digest, and hash the
# result. "Did we remember every input?" becomes a mechanism rather than a
# question, which is the only form that does not need a successor next week.
#
# LIMIT, STATED BECAUSE IT IS THE SAME CLASS OF GAP
# This traces `pathlib.Path.read_text/read_bytes` and `builtins.open`. A
# generator reading through something else -- os.read, mmap, a C extension --
# is NOT traced, and would reproduce exactly the defect this closes. `count`
# is reported alongside the digest so an implausibly small number is visible
# rather than silent.
# ============================================================================
import builtins
import hashlib
import io
import pathlib
from contextlib import contextmanager


@contextmanager
def trace():
    """Record {path: sha256} for every file read inside the block."""
    seen = {}
    o_text = pathlib.Path.read_text
    o_bytes = pathlib.Path.read_bytes
    o_open = builtins.open

    def _note(path):
        try:
            p = pathlib.Path(path)
            if p.is_file():
                seen[str(p.resolve())] = hashlib.sha256(
                    o_bytes(p)).hexdigest()
        except (OSError, ValueError, TypeError):
            pass

    def rt(self, *a, **k):
        _note(self)
        return o_text(self, *a, **k)

    def rb(self, *a, **k):
        _note(self)
        return o_bytes(self, *a, **k)

    def op(file, mode="r", *a, **k):
        if "r" in mode and "w" not in mode and "a" not in mode:
            _note(file)
        return o_open(file, mode, *a, **k)

    pathlib.Path.read_text = rt
    pathlib.Path.read_bytes = rb
    builtins.open = op
    try:
        yield seen
    finally:
        pathlib.Path.read_text = o_text
        pathlib.Path.read_bytes = o_bytes
        builtins.open = o_open


def digest(seen):
    """One digest over (path, content-hash) pairs, order-independent."""
    h = hashlib.sha256()
    for p in sorted(seen):
        h.update(p.encode("utf-8", "replace"))
        h.update(b"\0")
        h.update(seen[p].encode())
        h.update(b"\n")
    return h.hexdigest()
