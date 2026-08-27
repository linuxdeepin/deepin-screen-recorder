#!/usr/bin/env python3

import configparser
import ctypes
import os
import pathlib
import re
import shutil
import subprocess
import sys
import tempfile
import time
from ctypes import POINTER, byref, c_char, c_char_p, c_int, c_long, c_ubyte, c_uint, c_ulong, c_void_p


CONF_PATH = pathlib.Path.home() / ".config/deepin/deepin-screen-recorder/deepin-screen-recorder.conf"


def root_dir():
    base = pathlib.Path(os.environ.get("XDG_RUNTIME_DIR") or tempfile.gettempdir()).resolve()
    return base / f"youqu_dsr_basic_{os.getuid()}"


def ensure_root(clean=False):
    root = root_dir()
    base = root.parent.resolve()
    if root.exists() or root.is_symlink():
        resolved = root.resolve()
        if root.is_symlink() or base not in resolved.parents:
            raise RuntimeError(f"refuse unsafe runtime path: {root}")
        if clean:
            shutil.rmtree(root)
    root.mkdir(mode=0o700, parents=True, exist_ok=True)
    return root


def marker_path(name):
    return ensure_root() / f"{name}.ok"


def clear_marker(name):
    marker_path(name).unlink(missing_ok=True)


def mark(name, value="ok"):
    marker_path(name).write_text(value, encoding="utf-8")


def clean_child(name):
    root = ensure_root()
    path = (root / name).resolve()
    if root.resolve() not in path.parents:
        raise RuntimeError(f"refuse unsafe child path: {path}")
    if path.exists():
        if path.is_symlink():
            raise RuntimeError(f"refuse symlink child path: {path}")
        shutil.rmtree(path)
    path.mkdir(parents=True, exist_ok=True)
    print(path)


def assert_marker(name):
    path = marker_path(name)
    if not path.exists():
        raise AssertionError(f"marker not found: {path}")


def backup_config(root):
    backup = root / "deepin-screen-recorder.conf.bak"
    missing = root / "deepin-screen-recorder.conf.missing"
    if backup.exists() or missing.exists():
        return
    if CONF_PATH.exists():
        shutil.copy2(CONF_PATH, backup)
    else:
        missing.write_text("missing", encoding="utf-8")


def write_config(config):
    CONF_PATH.parent.mkdir(parents=True, exist_ok=True)
    fd, tmp_name = tempfile.mkstemp(prefix="deepin-screen-recorder.", suffix=".conf", dir=str(CONF_PATH.parent))
    with os.fdopen(fd, "w", encoding="utf-8") as tmp:
        config.write(tmp)
    os.replace(tmp_name, CONF_PATH)


def update_config(section, pairs):
    root = ensure_root()
    backup_config(root)
    config = configparser.ConfigParser()
    config.read(CONF_PATH)
    if not config.has_section(section):
        config.add_section(section)
    for key, value in pairs.items():
        config.set(section, key, str(value))
    write_config(config)


def restore_config():
    root = ensure_root()
    backup = root / "deepin-screen-recorder.conf.bak"
    missing = root / "deepin-screen-recorder.conf.missing"
    if backup.exists():
        CONF_PATH.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(backup, CONF_PATH)
    elif missing.exists():
        CONF_PATH.unlink(missing_ok=True)


def dpi_scale():
    try:
        out = subprocess.check_output(["xrdb", "-query"], text=True)
        match = re.search(r"Xft\.dpi:\s*([0-9.]+)", out)
        return float(match.group(1)) / 96.0 if match else 1.0
    except Exception:
        return 1.0


def find_at_name(target_name):
    import gi

    gi.require_version("Atspi", "2.0")
    from gi.repository import Atspi

    desktop = Atspi.get_desktop(0)
    stack = [desktop]
    while stack:
        obj = stack.pop()
        try:
            if (obj.get_name() or "") == target_name:
                return obj
            for index in range(obj.get_child_count()):
                stack.append(obj.get_child_at_index(index))
        except Exception:
            pass
    return None


def wait_at_name(name, timeout=6):
    deadline = time.time() + timeout
    while time.time() < deadline:
        obj = find_at_name(name)
        if obj:
            return obj
        time.sleep(0.2)
    raise AssertionError(f"AT element did not appear: {name}")


def click_at_name(name):
    import gi

    gi.require_version("Atspi", "2.0")
    from gi.repository import Atspi

    target = wait_at_name(name)
    extents = target.get_extents(Atspi.CoordType.SCREEN)
    scale = dpi_scale()
    x = int(round((extents.x + extents.width / 2) * scale))
    y = int(round((extents.y + extents.height / 2) * scale))
    subprocess.check_call(["xdotool", "mousemove", str(x), str(y), "click", "1"])


def iter_images(directory, suffixes):
    root = pathlib.Path(directory)
    if not root.exists():
        return []
    return sorted(path for path in root.iterdir() if path.suffix.lower() in suffixes and path.is_file())


def assert_image(path, expected_format=None):
    from PIL import Image

    image = Image.open(path)
    if image.width <= 10 or image.height <= 10:
        raise AssertionError(f"image is too small: {image.width}x{image.height}")
    if expected_format and image.format != expected_format:
        raise AssertionError(f"expected {expected_format}, got {image.format}")


def wait_image(directory, suffixes, marker, expected_format=None, timeout=25):
    deadline = time.time() + timeout
    while time.time() < deadline:
        for path in iter_images(directory, suffixes):
            if path.stat().st_size > 0:
                assert_image(path, expected_format)
                mark(marker, str(path))
                return
        time.sleep(0.25)
    raise AssertionError(f"no image generated in {directory}")


def verify_autoname(directory, marker):
    files = iter_images(directory, {".png", ".jpg", ".jpeg"})
    if len(files) < 2 or len({path.name for path in files}) < 2:
        raise AssertionError(f"expected at least two unique images: {files}")
    for path in files[:2]:
        assert_image(path)
    mark(marker, "\n".join(str(path) for path in files[:2]))


def check_clipboard_image(marker):
    lib = ctypes.cdll.LoadLibrary("libX11.so.6")
    display_p = c_void_p
    window = c_ulong
    atom = c_ulong

    class XSelectionEvent(ctypes.Structure):
        _fields_ = [
            ("type", c_int), ("serial", c_ulong), ("send_event", c_int),
            ("display", display_p), ("requestor", window), ("selection", atom),
            ("target", atom), ("property", atom), ("time", c_ulong),
        ]

    class XEvent(ctypes.Union):
        _fields_ = [("type", c_int), ("xselection", XSelectionEvent), ("pad", c_char * 256)]

    lib.XOpenDisplay.argtypes = [c_char_p]
    lib.XOpenDisplay.restype = display_p
    lib.XDefaultRootWindow.argtypes = [display_p]
    lib.XDefaultRootWindow.restype = window
    lib.XCreateSimpleWindow.argtypes = [display_p, window, c_int, c_int, c_uint, c_uint, c_uint, c_ulong, c_ulong]
    lib.XCreateSimpleWindow.restype = window
    lib.XInternAtom.argtypes = [display_p, c_char_p, c_int]
    lib.XInternAtom.restype = atom
    lib.XConvertSelection.argtypes = [display_p, atom, atom, atom, window, c_ulong]
    lib.XFlush.argtypes = [display_p]
    lib.XPending.argtypes = [display_p]
    lib.XPending.restype = c_int
    lib.XNextEvent.argtypes = [display_p, POINTER(XEvent)]
    lib.XGetWindowProperty.argtypes = [
        display_p, window, atom, c_long, c_long, c_int, atom,
        POINTER(atom), POINTER(c_int), POINTER(c_ulong), POINTER(c_ulong), POINTER(POINTER(c_ubyte)),
    ]
    lib.XGetWindowProperty.restype = c_int
    lib.XGetAtomName.argtypes = [display_p, atom]
    lib.XGetAtomName.restype = c_void_p
    lib.XFree.argtypes = [c_void_p]

    display = lib.XOpenDisplay(os.environ.get("DISPLAY", ":0").encode())
    if not display:
        raise AssertionError("cannot open X display")
    win = lib.XCreateSimpleWindow(display, lib.XDefaultRootWindow(display), 0, 0, 1, 1, 0, 0, 0)
    clipboard = lib.XInternAtom(display, b"CLIPBOARD", 0)
    targets = lib.XInternAtom(display, b"TARGETS", 0)
    prop = lib.XInternAtom(display, b"YOUQU_CLIPBOARD_TARGETS", 0)
    lib.XConvertSelection(display, clipboard, targets, prop, win, 0)
    lib.XFlush(display)

    event = XEvent()
    deadline = time.time() + 5
    while time.time() < deadline:
        if lib.XPending(display):
            lib.XNextEvent(display, byref(event))
            if event.type == 31:
                break
        time.sleep(0.05)
    else:
        raise AssertionError("no SelectionNotify from system clipboard")
    if event.xselection.property == 0:
        raise AssertionError("clipboard refused TARGETS conversion")

    actual_type = atom()
    actual_format = c_int()
    nitems = c_ulong()
    bytes_after = c_ulong()
    data = POINTER(c_ubyte)()
    rc = lib.XGetWindowProperty(
        display, win, prop, 0, 4096, 0, 0,
        byref(actual_type), byref(actual_format), byref(nitems), byref(bytes_after), byref(data),
    )
    if rc != 0 or nitems.value == 0 or actual_format.value != 32:
        raise AssertionError("cannot read clipboard TARGETS")
    atoms = (atom * nitems.value).from_address(ctypes.addressof(data.contents))
    names = []
    for item in atoms:
        name_ptr = lib.XGetAtomName(display, item)
        if name_ptr:
            try:
                names.append(ctypes.string_at(name_ptr).decode(errors="replace"))
            finally:
                lib.XFree(name_ptr)
    lib.XFree(data)
    wanted = {"image/png", "image/bmp", "image/jpeg", "application/x-qt-image", "application/octet-stream"}
    if not any(name in wanted or name.startswith("image/") for name in names):
        raise AssertionError("clipboard has no image target: " + ",".join(names))
    mark(marker, "\n".join(names))


def wait_mp4(directory, marker):
    deadline = time.time() + 30
    while time.time() < deadline:
        ffmpeg_running = subprocess.call(
            ["pgrep", "-f", "ffmpeg.*x11grab|ffmpeg.*Screen Recordings"],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        ) == 0
        for path in pathlib.Path(directory).glob("*.mp4") if pathlib.Path(directory).exists() else []:
            if path.stat().st_size > 0 and not ffmpeg_running:
                mark(marker, str(path))
                return
        time.sleep(0.5)
    raise AssertionError(f"no mp4 generated in {directory}")


def set_opts(section, pairs):
    update_config(section, pairs)


def parse_key_vals(args):
    pairs = {}
    for item in args:
        key, _, value = item.partition("=")
        try:
            value = int(value)
        except ValueError:
            pass
        pairs[key] = value
    return pairs


def main():
    command = sys.argv[1]
    root = ensure_root(clean=command == "init")
    if command == "init":
        (root / "out").mkdir(exist_ok=True)
        return
    if command == "root":
        print(root)
    elif command == "set-shot-opts":
        set_opts("shot", parse_key_vals(sys.argv[2:]))
    elif command == "set-recorder-opts":
        set_opts("recorder", parse_key_vals(sys.argv[2:]))
    elif command == "clear-marker":
        clear_marker(sys.argv[2])
    elif command == "mark":
        mark(sys.argv[2], sys.argv[3] if len(sys.argv) > 3 else "ok")
    elif command == "clean-child":
        clean_child(sys.argv[2])
    elif command == "assert-marker":
        assert_marker(sys.argv[2])
    elif command == "assert-at-name":
        wait_at_name(sys.argv[2])
    elif command == "click-at-name":
        click_at_name(sys.argv[2])
    elif command == "set-shot-file":
        update_config("shot", {"save_ways": 1, "save_op": 3, "save_dir": sys.argv[2], "save_dir_change": "false", "format": sys.argv[3]})
    elif command == "set-shot-clipboard":
        update_config("shot", {"save_ways": 1, "save_op": 0, "save_dir_change": "false", "format": 0})
    elif command == "set-recorder":
        update_config("recorder", {"format": 1, "frame_rate": 5, "save_op": 0, "save_dir": sys.argv[2], "audio": 0, "cursor": 0})
    elif command == "wait-image":
        wait_image(sys.argv[2], set(sys.argv[3].split(",")), sys.argv[4], sys.argv[5] if sys.argv[5] != "any" else None)
    elif command == "verify-autoname":
        verify_autoname(sys.argv[2], sys.argv[3])
    elif command == "check-clipboard":
        check_clipboard_image(sys.argv[2])
    elif command == "wait-mp4":
        wait_mp4(sys.argv[2], sys.argv[3])
    elif command == "restore":
        restore_config()
    else:
        raise SystemExit(f"unknown command: {command}")


if __name__ == "__main__":
    main()
