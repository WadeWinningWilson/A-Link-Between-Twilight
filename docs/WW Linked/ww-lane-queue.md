# Lane queue — generated, do not hand-edit
#
# Last CLAIMED state per item, derived from the bus. A claim is a
# claim, not a verdict: the lane that raised an item still rules on
# whether it is satisfied. This exists because four consecutive
# hand-offs re-listed work closed one to three sections earlier.
# regenerate: python tools/foundry/lane_queue.py --emit
# bus_sha256: 80b29233f74be7d68d0c594ae7bfa13f819f9bfcec6a1c631b39f72e30f59bbf
# tool_sha256: fceb8d1341970624c90438fd8332743f3a1b84c099e9ae6202a5a222e74f45b9
# items: 36   open: 6   closed: 29   ambiguous/unknown: 1
# A CLAIMED state is prose; a VERIFIED state ran an acceptance test.

| item | last claim | section | mentions |
|---|---|---|---|
| `ABI:B1` | OPEN-BEHAVIOURAL | §579 | 7 |
| `ABI:B3` | OPEN-BEHAVIOURAL | §547 | 4 |
| `C3` | OPEN-VERIFIED | §545 | 6 |
| `C5` | OPEN-VERIFIED | §561 | 9 |
| `HT-13` | OPEN-SRC | §539 | 4 |
| `HT-14` | OPEN-SRC | §539 | 3 |
| `19a` | CLOSED-SRC | §680 | 3 |
| `19b` | CLOSED-SRC | §571 | 4 |
| `19c` | CLAIMED-UNTESTED | §531 | 2 |
| `ABI:B2` | CLOSED-BEHAVIOURAL | §560 | 7 |
| `ABI:B4` | CLOSED-SRC | §517 | 3 |
| `C1` | CLOSED-SRC | §561 | 4 |
| `C2` | CLOSED-SRC | §561 | 3 |
| `C3a` | CLOSED-BEHAVIOURAL | §544 | 2 |
| `C3d` | CLOSED-BEHAVIOURAL | §544 | 2 |
| `C3e` | CLOSED-VERIFIED | §550 | 10 |
| `C4` | CLOSED-SRC | §561 | 4 |
| `C6` | CLOSED-SRC | §561 | 4 |
| `C7` | CLOSED-SRC | §560 | 2 |
| `D-1` | CLOSED-BEHAVIOURAL | §538 | 5 |
| `D-2` | CLOSED-SRC | §592 | 11 |
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
| `WWB:B2` | CLOSED-VERIFIED | §563 | 27 |
| `WWB:B3` | CLOSED-VERIFIED | §541 | 3 |
| `WWB:B4` | CLOSED-VERIFIED | §561 | 6 |
