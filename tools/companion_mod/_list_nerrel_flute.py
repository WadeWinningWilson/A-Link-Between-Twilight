#!/usr/bin/env python3
import json
import time
import urllib.request

BASE = "https://api.github.com/repos/GhostlyDark/MM-Reloaded/contents/"


def ls(path: str) -> list:
    req = urllib.request.Request(BASE + path, headers={"User-Agent": "dusklight"})
    return json.loads(urllib.request.urlopen(req, timeout=60).read())


inv = ls("ZELDA%20MAJORA'S%20MASK/Nerrel/Objects/Inventory")
print("=== Objects/Inventory ===")
for x in inv:
    print(x["type"], x["name"])

time.sleep(2)
fx = ls("ZELDA%20MAJORA'S%20MASK/Nerrel/Effects")
print("\n=== Effects dirs ===")
for x in sorted(fx, key=lambda i: i["name"]):
    if x["type"] == "dir":
        print(" ", x["name"])
