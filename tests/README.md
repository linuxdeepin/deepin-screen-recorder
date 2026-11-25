# Unit tests

Install the build dependencies in `debian/control`, plus `libgtest-dev` and
`libgmock-dev`. To build the test binaries from the repository root:

```sh
cmake -S . -B build-tests -DDSR_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build-tests --target dsr_unit_tests --parallel 4
```

Run the tests only in an isolated CI environment or disposable VM. Some
tests can activate installed desktop services over D-Bus, even when Qt uses
the offscreen platform.

```sh
dbus-run-session -- ctest --test-dir build-tests --output-on-failure
```

The CTest configuration includes the recorder, screenshot session, pinned
screenshots, three Dock plugins, and startup regressions. The GUI unit tests
use the Qt offscreen platform.

Each GoogleTest case runs in its own process. The recorder suite runs cases
one at a time because its GUI tests share global state. A few pre-existing
recorder cases (modal file dialogs that block headless, plus order-sensitive
assertions) are excluded from the default run; the old qmake harness tolerated
the same cases via per-case timeouts.

For the existing per-test XML and merged coverage reports, install `lcov`
and run `bash tests/test-prj-running.sh` in the same isolated environment.
The script runs the recorder cases in separate processes, enables
`DSR_TEST_COVERAGE`, and writes reports to `build-ut/`. Set `DSR_TEST_JOBS`
to change the build parallelism (default: 4).
