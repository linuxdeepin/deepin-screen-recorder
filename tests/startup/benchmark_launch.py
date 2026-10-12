#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
# SPDX-License-Identifier: GPL-3.0-or-later
"""Compare two built screenshot apps through their normal D-Bus entry point.

Temporarily displays each screenshot overlay and terminates only the process
started by this script. Does not select, save, install or activate system apps.
The display boundary is showFullScreen return, not compositor presentation.
"""
import argparse
import json
import math
import os
from pathlib import Path
import queue
import statistics
import subprocess
import threading
import time

BUS = ["gdbus", "call", "--session", "--dest", "org.freedesktop.DBus",
       "--object-path", "/org/freedesktop/DBus"]
SHOT = ["gdbus", "call", "--session", "--dest", "com.deepin.Screenshot",
        "--object-path", "/com/deepin/Screenshot", "--method",
        "com.deepin.Screenshot.StartScreenshot"]


def owner(name):
    result = subprocess.run(BUS + ["--method", "org.freedesktop.DBus.NameHasOwner", name],
                            capture_output=True, text=True, check=True, timeout=5)
    return "true" in result.stdout


def sample(binary, log_path):
    for name in ("com.deepin.Screenshot", "com.deepin.ScreenRecorder"):
        if owner(name):
            raise RuntimeError(f"{name} is already running; refusing to interfere")
    env = dict(os.environ, QT_LOGGING_RULES="dsr.app.debug=true;dsr.app.info=true")
    started = time.monotonic_ns()
    process = subprocess.Popen([str(binary), "--dbus"], env=env, stdout=subprocess.PIPE,
                               stderr=subprocess.STDOUT, text=True, bufsize=1)
    events = queue.Queue()
    lines = []

    def read_output():
        for line in process.stdout:
            received = time.monotonic_ns()
            lines.append(line)
            events.put((received, line))
        events.put((time.monotonic_ns(), None))

    reader = threading.Thread(target=read_output, daemon=True)
    reader.start()
    call = None
    invoked = entered = shown = ready = None
    try:
        deadline = time.monotonic() + 10
        while time.monotonic() < deadline:
            received, line = events.get(timeout=max(0.01, deadline - time.monotonic()))
            if line is None:
                raise RuntimeError(f"App exited before showing overlay: {process.poll()}")
            if "DBus option set, waiting" in line and call is None:
                ready = received
                invoked = time.monotonic_ns()
                call = subprocess.Popen(SHOT, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
            if "StartScreenshot method called." in line:
                entered = received
            if "Showing window in full screen mode" in line:
                shown = received
                break
        if not all((ready, invoked, entered, shown)):
            raise RuntimeError("Missing startup timing boundaries")
        stdout, stderr = call.communicate(timeout=5)
        if call.returncode:
            raise RuntimeError(f"D-Bus screenshot call failed: {stderr}")
        # Allow the normal event loop to paint before closing our own process.
        time.sleep(0.15)
        return {"process_to_ready_ms": (ready - started) / 1e6,
                "dbus_to_show_ms": (shown - entered) / 1e6,
                "invoke_to_show_ms": (shown - invoked) / 1e6,
                "process_to_show_ms": (shown - started) / 1e6}
    finally:
        if call and call.poll() is None:
            call.terminate()
            call.communicate(timeout=3)
        if process.poll() is None:
            process.terminate()
            try:
                process.wait(timeout=3)
            except subprocess.TimeoutExpired:
                process.kill()
                process.wait(timeout=3)
        reader.join(timeout=1)
        process.stdout.close()
        log_path.write_text("".join(lines))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("before", type=Path)
    parser.add_argument("after", type=Path)
    parser.add_argument("--samples", type=int, default=11)
    parser.add_argument("--output", type=Path, default=Path("/tmp/dsr-launch-benchmark"))
    args = parser.parse_args()
    if args.samples < 1:
        parser.error("samples must be positive")
    args.output.mkdir(parents=True, exist_ok=True)
    samples = {"before": [], "after": []}
    for index in range(args.samples):
        names = ("before", "after") if index % 2 == 0 else ("after", "before")
        for name in names:
            samples[name].append(sample(getattr(args, name).resolve(),
                                        args.output / f"{name}-{index:02}.log"))
    summary = {}
    for name, values in samples.items():
        summary[name] = {}
        for metric in values[0]:
            numbers = sorted(value[metric] for value in values)
            summary[name][metric] = {"median": statistics.median(numbers),
                                     "p95": numbers[math.ceil(len(numbers) * 0.95) - 1],
                                     "min": min(numbers), "max": max(numbers)}
    report = {"samples_per_variant": args.samples,
              "boundary": "showFullScreen return; page cache warm, new process per sample",
              "summary": summary, "samples": samples}
    (args.output / "results.json").write_text(json.dumps(report, indent=2) + "\n")
    print(json.dumps(summary, indent=2))


if __name__ == "__main__":
    main()
