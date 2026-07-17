#!/usr/bin/env python3
import json
import urllib.request

BASE = "https://api.github.com/repos/GhostlyDark/MM-Reloaded/contents/"


def list_dir(path: str) -> list[dict]:
    req = urllib.request.Request(BASE + path, headers={"User-Agent": "dusklight"})
    return json.loads(urllib.request.urlopen(req, timeout=60).read())


def walk(path: str, depth: int = 0, max_depth: int = 3) -> None:
    if depth > max_depth:
        return
    try:
        entries = list_dir(path)
    except Exception as exc:
        print("ERR", path, exc)
        return
    for x in sorted(entries, key=lambda i: i["name"].lower()):
        indent = "  " * depth
        if x["type"] == "dir":
            name_lower = x["name"].lower()
            marker = " ***" if "flute" in name_lower or "ocarina" in name_lower else ""
            print(f"{indent}[dir] {x['name']}{marker}")
            if depth < max_depth:
                walk(x["path"].replace(" ", "%20") if "%" not in x["path"] else urllib.parse.quote(x["path"], safe="/"), depth + 1, max_depth)
        else:
            nl = x["name"].lower()
            if "flute" in nl or "ocarina" in nl:
                print(f"{indent}[file] {x['name']}")


if __name__ == "__main__":
    import urllib.parse

    print("=== Nerrel/Characters ===")
    chars = list_dir("ZELDA%20MAJORA'S%20MASK/Nerrel/Characters")
    for x in sorted(chars, key=lambda i: i["name"].lower()):
        print(f"  [{x['type']}] {x['name']}")

    print("\n=== Flute/ocarina hits under Nerrel (depth 4) ===")
    walk("ZELDA%20MAJORA'S%20MASK/Nerrel", max_depth=4)
