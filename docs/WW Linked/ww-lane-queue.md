# Lane queue — generated, do not hand-edit
#
# Last CLAIMED state per item, derived from the bus. A claim is a
# claim, not a verdict: the lane that raised an item still rules on
# whether it is satisfied. This exists because four consecutive
# hand-offs re-listed work closed one to three sections earlier.
# regenerate: python tools/foundry/lane_queue.py --emit
# bus_sha256: 4b4d95e7a572b35afa2fd9064fadc77c56aeec3248c9ea529d4325e3685e01ff
# tool_sha256: 5cbccd8006b5a03b2ff010dfe2708d4b29f7eb123c8dd0e73aaa614423e757f1
# items: 33   open: 7   closed: 26   ambiguous/unknown: 0
# A CLAIMED state is prose; a VERIFIED state ran an acceptance test.

| item | last claim | section | mentions |
|---|---|---|---|
| `ABI:B1` | OPEN-BEHAVIOURAL | §547 | 4 |
| `ABI:B3` | OPEN-BEHAVIOURAL | §547 | 4 |
| `C3` | OPEN-VERIFIED | §545 | 6 |
| `C5` | OPEN-VERIFIED | §560 | 8 |
| `HT-13` | OPEN-SRC | §539 | 4 |
| `HT-14` | OPEN-SRC | §539 | 3 |
| `WWB:B2` | OPEN-VERIFIED | §560 | 23 |
| `ABI:B2` | CLOSED-BEHAVIOURAL | §560 | 7 |
| `ABI:B4` | CLOSED-SRC | §517 | 3 |
| `C1` | CLOSED-SRC | §530 | 3 |
| `C2` | CLOSED-SRC | §530 | 2 |
| `C3a` | CLOSED-BEHAVIOURAL | §544 | 2 |
| `C3d` | CLOSED-BEHAVIOURAL | §544 | 2 |
| `C3e` | CLOSED-VERIFIED | §550 | 10 |
| `C4` | CLOSED-SRC | §530 | 3 |
| `C6` | CLOSED-SRC | §530 | 3 |
| `C7` | CLOSED-SRC | §560 | 2 |
| `D-1` | CLOSED-BEHAVIOURAL | §538 | 5 |
| `D-2` | CLOSED-SRC | §547 | 10 |
| `D-3` | CLOSED-BEHAVIOURAL | §538 | 5 |
| `HT-15` | CLOSED-VERIFIED | §515 | 2 |
| `HT-18` | CLOSED-VERIFIED | §515 | 2 |
| `HT-20` | CLOSED-SRC | §560 | 5 |
| `HT-21` | CLOSED-BEHAVIOURAL | §497 | 2 |
| `HT-22` | CLOSED-SRC | §540 | 3 |
| `HT-23` | CLOSED-VERIFIED | §540 | 3 |
| `HT-24` | CLOSED-BEHAVIOURAL | §538 | 3 |
| `HT-25` | CLOSED-BEHAVIOURAL | §557 | 4 |
| `HT-26` | CLOSED-BEHAVIOURAL | §538 | 4 |
| `HT-27` | CLOSED-SRC | §540 | 5 |
| `HT-8` | CLOSED-SRC | §554 | 13 |
| `WWB:B3` | CLOSED-VERIFIED | §541 | 3 |
| `WWB:B4` | CLOSED-VERIFIED | §544 | 5 |
