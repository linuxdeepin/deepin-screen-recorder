# Screenshot startup tests

These focused tests need Qt6 Core/Gui development files, Xlib, CMake and a C++17 compiler. They do not require recording or camera dependencies and do not open a window.

```sh
cmake -S tests/startup -B build/startup-tests -DCMAKE_BUILD_TYPE=Release
cmake --build build/startup-tests -j4
ctest --test-dir build/startup-tests --output-on-failure
build/startup-tests/benchmark_startup
```

The pixel tests cover row padding, alpha, source ownership, byte order, 24-bit storage, source alignment, nonstandard depth/masks and invalid input. CPU tests use in-memory metadata, including empty, unreadable and misleading input.

`benchmark_startup` alternates the old and new conversion order for 31 samples and compares every pixel. Add `--desktop` to also compare a real X11 desktop image in memory; it does not save the image. CPU timing compares `lscpu` with the procfs detector.

The image regression can be run against the original conversion, where it must fail because native RGB32 still calls XGetPixel per pixel:

```sh
cmake -S tests/startup -B build/startup-legacy -DSTARTUP_TEST_LEGACY=ON
cmake --build build/startup-legacy -j4
ctest --test-dir build/startup-legacy --output-on-failure
```

Compare two full app builds through the screenshot D-Bus method:

```sh
python3 tests/startup/benchmark_launch.py /path/to/before /path/to/after --samples 11
```

This temporarily displays and closes each overlay, refuses to interfere with an existing screenshot/recording service, and only terminates its own subprocesses. It reports a new-process measurement with warm filesystem cache. The endpoint is showFullScreen return, not the first frame presented by the compositor. Results and per-run logs default to `/tmp/dsr-launch-benchmark`.

## Local measurements

Measured on X11 with Qt6, a 2880x1800 physical screen and DPR 1.75. Both variants used the same source baseline and Release build options. Eleven runs per variant were interleaved, each using a new process with warm filesystem cache:

| Boundary | Before median | After median |
| --- | ---: | ---: |
| Process launch to showFullScreen return | 240.30 ms | 184.63 ms |
| StartScreenshot method entry to showFullScreen return | 152.15 ms | 95.57 ms |
| Process launch to service ready | 83.29 ms | 83.43 ms |

For 31 alternating samples, conversion of a real desktop XImage fell from 80.94 ms to 4.11 ms with identical pixels; CPU detection fell from 11.39 ms to 0.71 ms. These component timings exclude capture, QPixmap creation and display. Application measurements use log receipt times and include logging/reader overhead; they do not measure keyboard dispatch, cold D-Bus activation, cold filesystem cache or compositor presentation.

Both before/after builds covered only the screenshot main program (`src/src.pro`). The top-level subdirectory project, Dock plugins, pin-screenshots program and Debian package were not built. In particular, the missing `libdtk6tablerecognizer-dev` dependency still prevents validating the pin-screenshots build. Qt5, multiscreen, Wayland/Treeland and the full existing unit-test suite were not exercised. The optimized main program also saved a valid opaque 2880x1800 PNG in the local fullscreen-save smoke test.
