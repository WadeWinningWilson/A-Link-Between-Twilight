# Blender Socket Bridge — drive Blender from Cursor / Claude (port 9876)

**Purpose:** Single reference for talking to a **running Blender** with the **Blender MCP addon** listening on **`localhost:9876`**. Use this when an AI session does **not** have the Blender MCP hammer wired in (Cursor often doesn't) but **can** run Python in the terminal.

**Related asset docs:** [`Blender-WW-Items.md`](Blender-WW-Items.md) (WW item re-rig), [`HANDOFF-Gohma-Reveal-BMD-Reexport.md`](HANDOFF-Gohma-Reveal-BMD-Reexport.md) (orientation fix example), [`BMD-Reskin-Tool.md`](BMD-Reskin-Tool.md) (animation-safe rigged exports).

---

## Prerequisites (user)

1. Blender open with the target `.blend` loaded.
2. **Blender MCP** addon enabled (several forks exist; Dusklight has used **newline JSON** and **null-delimited `execute`** variants).
3. Server **started** (many builds: viewport **N** panel → **Blender MCP** → **Start Server** / **Connect**). Addon auto-start on launch is also common.
4. Confirm port open:
   ```powershell
   Test-NetConnection -ComputerName 127.0.0.1 -Port 9876
   ```

---

## Protocol (what actually works here)

### Variant A — `get_scene_info` / `execute_code` (newline JSON)

Send **one UTF-8 JSON object** (no length prefix). Response is a single JSON object (read until `json.loads` succeeds).

**Request shapes:**

```json
{"type": "get_scene_info"}
```

```json
{"type": "execute_code", "params": {"code": "import bpy\nprint(len(bpy.data.objects))"}}
```

```json
{"type": "get_viewport_screenshot", "params": {"max_size": 800}}
```

**Response shape (typical):**

```json
{"status": "success", "result": { ... }}
```

```json
{"status": "error", "message": "..."}
```

**Python client (minimal):**

```python
import json, socket

def blender_call(msg_type, params=None, host="127.0.0.1", port=9876, timeout=30):
    payload = {"type": msg_type}
    if params:
        payload["params"] = params
    data = json.dumps(payload).encode("utf-8")
    with socket.socket() as s:
        s.settimeout(timeout)
        s.connect((host, port))
        s.sendall(data)
        buf = b""
        while True:
            chunk = s.recv(65536)
            if not chunk:
                break
            buf += chunk
            try:
                return json.loads(buf.decode("utf-8"))
            except json.JSONDecodeError:
                pass
```

**Repo helper:** `tools/blender_socket/probe_blender.py` (scene probe + protocol smoke test).

### Variant B — `execute` (null-byte delimited)

Some addon builds (e.g. [Oli97430/blender-mcp-addon](https://github.com/Oli97430/blender-mcp-addon)) use:

```json
{"type": "execute", "code": "result = {'n': len(bpy.data.objects)}", "strict_json": false}\0
```

Response also ends with `\0`. Assign return value to a variable named **`result`** in the executed code.

If `execute_code` returns empty `result` but the addon docs mention `execute`, try this variant.

---

## What each side can do

| Capability | Cursor (terminal socket) | Claude w/ Blender MCP |
|------------|--------------------------|------------------------|
| `get_scene_info` | Yes | Yes |
| Run `bpy` scripts | Yes (`execute_code` / `execute`) | Yes |
| Viewport screenshot | Yes (returns in JSON; save path depends on addon) | Yes |
| Interactive weight paint | Weak — prefer scripted weights | Same |

**Limits:** AI is strong at deterministic steps (import DAE, list bones, apply transforms, export). Weak at artistic weight paint — use **region-match / rigid weights** for boots and similar (see `Blender-WW-Items.md`).

---

## Standard workflow loop

1. **`get_scene_info`** — confirm correct file (object names, counts).
2. **`execute_code`** — inspect armatures, bounds, `matrix_world`, materials.
3. **Edit** — transforms, split meshes, weight scripts, delete loose geometry.
4. **Export DAE** — Collada, triangulated, armature included; armature named **`skeleton_root`** for SuperBMD.
5. **SuperBMD** → `.bmd` / `.bdl` → drop in `%AppData%\TwilitRealm\Dusklight\model_replacements\...`
6. **In-game test** — Item Viewer or fight; wipe GPU caches after game rebuilds (`dawn_cache.db*`, `pipeline_cache.db*`).

---

## SuperBMD gotchas (always)

- Armature object name **`skeleton_root`** or SuperBMD errors *"No Skeleton found."*
- **`transform_apply`** before export — avoid double-rotation with `--rotate` (Gohma handoff).
- Materials: cel shader keys on **`SC_` prefix at start** of material name (not `m0SC_...`).
- Rigged BMD edited in Blender → use **[BMD reskin tool](BMD-Reskin-Tool.md)** if vanilla animations shatter limbs.
- Layer-B custom models: build BMD **with** material JSON (`-m` / `-x`) or engine NULL-crashes on load.

---

## Troubleshooting

| Symptom | Fix |
|---------|-----|
| Connection timeout | Server not started in Blender; wrong port; firewall |
| Empty `execute_code` result | Try `execute` + `\0` protocol; or use `print()` and read `stdout` field if present |
| Wrong scene | User loaded different `.blend`; always `get_scene_info` first |
| `get_scene_info` shows 0 objects | Empty scene — import DAE/BMD or open correct file |

---

## Handoff block for a new Blender chat

Copy to Claude / a fresh Cursor session:

> Blender MCP server is on **localhost:9876**. Read **`docs/Blender-Socket-Bridge.md`** for the socket protocol and **`tools/blender_socket/probe_blender.py`** to verify connectivity. Asset context for WW boots: **`docs/Blender-WW-Items.md`**. Drive Blender via `get_scene_info` + `execute_code` (or terminal `blender_call`). User is Blender-inexperienced — prefer paste-ready `bpy` scripts and explain panel locations. Do not modify game code unless the task explicitly requires it.

---

## Session log (2026-07-16)

- Port **9876** listening on `127.0.0.1` (process verified via `Get-NetTCPConnection`).
- **Variant A** works: `get_scene_info` returned success with **0 objects**, **2 materials** (fresh/empty scene).
- Cursor has **no native Blender MCP tool** in the tool list; terminal socket client is the working path from this environment.
- **MM Skull Kid (2026-07-16):** body DAE imported to `MM_SkullKid_v1.blend` via `tools/blender_socket/mm_skullkid_import.py`. See **`docs/Blender-MM-SkullKid.md`**.
