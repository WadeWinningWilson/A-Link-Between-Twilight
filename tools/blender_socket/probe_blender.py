#!/usr/bin/env python3
"""Probe the local Blender MCP socket (default localhost:9876)."""
import json
import socket
import sys


def blender_call(msg_type: str, params: dict | None = None, host: str = "127.0.0.1", port: int = 9876, timeout: float = 30.0) -> dict:
    payload: dict = {"type": msg_type}
    if params:
        payload["params"] = params
    data = json.dumps(payload).encode("utf-8")

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
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
                if len(buf) > 2_000_000:
                    raise


def blender_execute_null(code: str, host: str = "127.0.0.1", port: int = 9876, timeout: float = 30.0) -> dict:
    req = (json.dumps({"type": "execute", "code": code, "strict_json": False}) + "\0").encode("utf-8")
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.settimeout(timeout)
        s.connect((host, port))
        s.sendall(req)
        buf = bytearray()
        while b"\0" not in buf:
            buf.extend(s.recv(65536))
    return json.loads(buf.split(b"\0")[0])


def main() -> int:
    print("=== get_scene_info ===")
    print(json.dumps(blender_call("get_scene_info"), indent=2))

    code = (
        "import bpy\n"
        "result = {\n"
        "  'blend': bpy.data.filepath or '(unsaved)',\n"
        "  'version': bpy.app.version_string,\n"
        "  'objects': [f'{o.name}:{o.type}' for o in bpy.data.objects],\n"
        "  'materials': [m.name for m in bpy.data.materials],\n"
        "}\n"
    )
    print("=== execute_code (params.code) ===")
    try:
        print(json.dumps(blender_call("execute_code", {"code": code}), indent=2))
    except Exception as exc:
        print("execute_code failed:", exc)

    print("=== execute (null-delimited) ===")
    try:
        print(json.dumps(blender_execute_null(code), indent=2))
    except Exception as exc:
        print("execute null failed:", exc)

    return 0


if __name__ == "__main__":
    sys.exit(main())
