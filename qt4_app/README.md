# Qt Desktop Window

This folder contains a small desktop viewer for the project.

## What it shows
- a project explanation
- why a desktop UI is useful
- live Python console output
- the generated model summary
- all plots exported from the notebook workflow

## Build

With Qt Creator on Windows, the easiest path is to open `qt4_app.pro` and build it there.

From a terminal with Qt installed, you can also build with `qmake`:

```bash
qmake qt4_app.pro
make
```

Or build with CMake:

```bash
cmake -S . -B build
cmake --build build
```

The project now targets modern Qt Widgets and supports Qt5 or Qt6.

## Refresh the content

The Qt app can run the Python analysis itself and then reload the results inside the window.

It starts:

```bash
python scripts/generate_plots.py
```

and then reads the updated files from `qt4_app/generated/`.
