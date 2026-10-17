# Qt Desktop Viewer — Code Guide

[Back to the main guide](../README.md)

The viewer creates an **Overview** tab for explanatory text, saved metrics, and Python console output, plus a scrollable **Plots** tab with five images. Its **Run Python analysis** button launches `scripts/generate_plots.py` as a separate process and refreshes the files after a successful exit.

Despite the directory name `qt4_app`, the build configurations target Qt Widgets through Qt5 or Qt6. The application does not run the notebook or perform model training in C++.

## Source files

### `main.cpp`

`main(argc, argv)` constructs `QApplication`, creates a `MainWindow`, calls `window.show()`, and returns `app.exec()`. The event loop handles button clicks, window events, and asynchronous process signals until the application exits.

### `mainwindow.h`

The include guard prevents duplicate inclusion. `MainWindow` derives from `QMainWindow`, and `Q_OBJECT` enables Qt's meta-object support for its slots.

The private slots handle starting Python, receiving its output, finishing, and errors. Other private methods construct widgets, calculate paths, read files, and refresh displayed content.

`PlotEntry` pairs an image filename with its `QLabel*`. `m_plotEntries` stores these pairs so a refresh can update every existing image label. Other members hold the tab container, summary and console text widgets, status label, run button, and active `QProcess*`. A null `m_process` means no process is currently registered.

Most widget classes are forward-declared to keep header dependencies small. The `QProcess` forward declaration is insufficient for its nested enum types used in the slot declarations; see the issues below.

### `mainwindow.cpp`: window construction

| Function | What it does |
| --- | --- |
| `MainWindow()` | Initializes widget/process pointers, sets the title and initial size to 1100 × 760, creates the Overview and Plots tabs, installs them as the central widget, and refreshes saved content. |
| `createOverviewTab()` | Creates static descriptive labels, two explanatory group boxes, the read-only summary, and a Python integration panel. Connects the run button's `clicked()` signal to `runPythonGenerator()`. |
| `createPlotsTab()` | Creates five plot cards in a vertical layout, wraps them in a resizable `QScrollArea`, and returns the tab page. |
| `createSinglePlotCard(...)` | Combines a title, wrapped description, and registered image label inside a `QGroupBox`. |
| `createImageLabel(imageFileName, registerLabel)` | Creates a centered label with a 280-pixel minimum height. Optionally records it in `m_plotEntries`; loads its PNG as a `QPixmap`, or displays a missing-image message. |

Images are scaled to fit 980 × 500 while keeping their aspect ratio and using smooth interpolation. This is a fixed loading/refresh size, not a custom resize-event implementation. Parent widgets and layouts establish ownership of the UI components.

### `mainwindow.cpp`: paths and file reading

| Function | What it does |
| --- | --- |
| `generatedDirPath()` | Starts from the executable directory, moves up once if it is named exactly `debug` or `release`, and appends `generated`. |
| `projectRootPath()` | Applies the same debug/release adjustment, then moves up one directory to derive the repository root. |
| `scriptsPath()` | Appends `scripts/generate_plots.py` to the derived root. |
| `detectPythonCommand()` | Returns the literal string `python`. It does not probe interpreters or virtual environments. |
| `readSummaryText()` | Reads `generated/summary.txt` as UTF-8 text; returns a helpful placeholder if opening the file fails. |
| `refreshGeneratedContent()` | Reloads the summary and every registered plot. Clears old image labels before loading fresh images or missing-file messages. |

The runtime layout expected by these methods is:

```text
repository/
├── logatta.csv
├── scripts/generate_plots.py
└── qt4_app/
    ├── interview_predictor_viewer[.exe]
    └── generated/
        ├── summary.txt
        └── *.png
```

An executable under `qt4_app/debug/` or `qt4_app/release/` also matches the special handling. Arbitrary build folders and differently capitalized directory names do not.

### `mainwindow.cpp`: Python process lifecycle

1. `runPythonGenerator()` returns immediately if `m_process` is already set. Otherwise, it creates a child `QProcess`, assigns the repository working directory, and connects output, completion, and error signals.
2. It clears the console, sets the running status, disables the button, and calls `start("python", arguments)` with the script path as an argument. No shell command string is used.
3. `readProcessOutput()` reads both standard output and standard error, converts their bytes with `QString::fromLocal8Bit`, and passes them to `appendProcessText()`.
4. `appendProcessText()` appends plain text at the end of the console and moves the cursor to keep the latest text in view.
5. `handleProcessFinished(exitCode, exitStatus)` drains remaining output, updates the status, refreshes files only for a normal exit with code zero, re-enables the button, schedules process deletion with `deleteLater()`, and clears the pointer.
6. `handleProcessError(error)` selects a message for failure to start, crash, or another process error, updates the console/status, re-enables the button, and deletes/clears the process if present.

Python runs asynchronously, allowing the Qt event loop to continue handling the interface. Output delivery depends on Python buffering; the launcher does not pass `-u`.

## Build configuration

A C++ compiler, Qt Widgets development installation, and either CMake or qmake are needed. The source issues in the next section must be addressed before treating this as a working build.

### `CMakeLists.txt`

| Setting | Effect |
| --- | --- |
| `cmake_minimum_required(VERSION 3.16)` | Requires at least CMake 3.16. |
| `project(interview_predictor_viewer LANGUAGES CXX)` | Names the C++ project and executable target. |
| `CMAKE_AUTOMOC ON` | Generates Qt meta-object code for `Q_OBJECT`. |
| `CMAKE_CXX_STANDARD 11` and `...REQUIRED ON` | Requests C++11; the selected Qt version may impose a higher requirement. |
| Two `find_package` calls | Select Qt6 or Qt5, then locate that version's Widgets component. |
| `APP_SOURCES` / `APP_HEADERS` | List `main.cpp`, `mainwindow.cpp`, and `mainwindow.h`. |
| `add_executable(...)` | Builds the named viewer from those files. |
| `target_link_libraries(...)` | Links the selected Qt Widgets target. |

From the repository root, use a fresh build directory instead of the existing machine-specific cache:

```bash
cmake -S qt4_app -B qt4_app/build-local
cmake --build qt4_app/build-local
```

If Qt is not discovered automatically, configure CMake with the installation's appropriate prefix path. The current CMake file does not copy generated assets or set a runtime output directory. After compilation, place the executable in `qt4_app/` to satisfy the path logic, or adjust that logic before launching from a build folder. Platform-specific Qt runtime libraries must also be available.

### `qt4_app.pro`

`QT += widgets` enables Widgets, `TARGET` sets `interview_predictor_viewer`, and `TEMPLATE = app` selects an executable. `SOURCES` lists the two C++ files, and `HEADERS` lists `mainwindow.h`.

For an in-source qmake build, from `qt4_app/`:

```bash
qmake qt4_app.pro
make
```

Use the build tool appropriate to the installed compiler kit; `make` is a Unix-style example. Qt Creator can also open the `.pro` file, but its shadow-build output directory may not match the runtime path assumptions.

To use the repository's virtual environment, activate it and launch the viewer from that terminal so the child process inherits a `PATH` where `python` resolves to the intended interpreter.

## Existing implementation issues

These are observations from the source, not changes made by this documentation update.

- **Header dependency:** `mainwindow.h` names `QProcess::ExitStatus` and `QProcess::ProcessError` while only forward-declaring `QProcess`. Include `<QProcess>` in the header before using its nested types. Both C++ files include this header before any complete `QProcess` definition.
- **Error signal connection:** the code uses the old string-based `SIGNAL(error(QProcess::ProcessError))` connection. This needs review for the selected Qt version; in Qt6 the corresponding signal is `errorOccurred`. String-based connection failures are not checked by the current code.
- **Build-directory paths:** running from `qt4_app/build-local/` makes the viewer look for `build-local/generated/` and derive `qt4_app/` as the repository root. Neither matches the exporter layout. Moving the executable as described above resolves that layout mismatch without changing the Python script.
- **Error/finish cleanup:** the error callback can clear `m_process`, while the finish callback dereferences it at the end without a null check. Error and completion handling should be made consistent before relying on crash recovery.
- **Interpreter lookup:** the app always starts `python`; it does not automatically select `.venv`, `python3`, or a Windows `py` launcher.

## Generated results and build artifacts

The five PNGs and `generated/summary.txt` are described individually in the [exporter guide](../scripts/README.md#generated-files). The app can display existing files on startup; they are not proof that analysis ran during the current session.

The existing `build/` directory contains configuration artifacts rather than application source:

| File relative to `build/` | Role |
| --- | --- |
| `CMakeCache.txt` | Cached configuration values and absolute paths from a previous machine. It records `CMAKE_CXX_COMPILER-NOTFOUND`. |
| `CMakeFiles/CMakeConfigureLog.yaml` | Configuration diagnostics, including failed compiler-identification attempts. |
| `CMakeFiles/3.28.3/CMakeSystem.cmake` | CMake-generated description of the configured host/target system. |
| `CMakeFiles/3.28.3/CompilerIdCXX/CMakeCXXCompilerId.cpp` | Generated compiler-identification probe; not part of the viewer's behavior. |
| `CMakeFiles/cmake.check_cache` | Internal CMake cache-check marker. |

These files are generated by CMake and should not be manually edited to configure a new machine. The supplied cache reflects an unsuccessful compiler configuration, not a completed build.
