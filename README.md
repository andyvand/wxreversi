# wxReversi

A portable **wxWidgets** port of the classic Windows *Reversi* game by Chris
Peters. The original Win16 sample has been split into a GUI-free C++ engine
and a thin wxWidgets front-end, so it now builds and runs natively on
Windows, macOS, and Linux/GTK.

## Screenshots

- macOS:

![](/screenshots/wxreversi_macOS.png)

- Windows:

![](/screenshots/wxreversi_win.png)

- Linux:

![](/screenshots/wxreversi_linux.png)

## Features

- Faithful reproduction of the original Microsoft Reversi gameplay
- Four skill levels (alpha-beta search depths 1, 2, 4, 6):
  **Beginner**, **Novice**, **Expert**, **Master**
- Hint (flashing marker) and Pass commands
- Color / monochrome board modes
- Legal-move cursor feedback (cross on legal squares, arrow otherwise)
- Resizable, flicker-free buffered rendering
- XRC resources (`reversi.xrc` + pattern bitmaps) compiled with `wxrc` and
  embedded into the executable via `bin2c` — no runtime data files required
- Platform integration:
  - Windows: GUI subsystem, icon + manifest (`reversi.rc`, `reversi.ico`)
  - macOS: `.app` bundle with `reversi.icns`
  - Linux: `.desktop` entry, AppStream metainfo, hicolor icon set

## Project layout

| File / dir                | Purpose                                          |
|---------------------------|--------------------------------------------------|
| `reversi_engine.{h,cpp}`  | Portable engine: board state, move generation, alpha-beta search |
| `reversi_wx.cpp`          | wxWidgets front-end (`wxApp`, `wxFrame`, `BoardPanel`) |
| `reversi.xrc`             | XRC resource file referencing the pattern bitmaps |
| `REVPAT1.bmp`, `REVPAT8.bmp` | Monochrome / 8-bit board pattern bitmaps      |
| `bin2c.c`                 | Host tool that embeds the compiled `.xrs` archive as a C byte array |
| `cmake/HostCompile.cmake` | Helper to build `bin2c` with the host compiler even when cross-compiling |
| `reversi.rc`, `reversi.manifest`, `reversi.ico` | Windows resources |
| `reversi.icns`, `reversi.iconset/` | macOS bundle icon                   |
| `sizes/`                  | Linux hicolor icon theme (16x16 … 256x256, scalable) |
| `wxmine.desktop`, `wxmine.metainfo.xml` | Linux desktop / AppStream files |

## Requirements

- **CMake** 3.16 or newer
- **wxWidgets** 3.0+ (3.2+ recommended), built with the `core`, `base`,
  `xml`, and `xrc` components, and including the `wxrc` resource compiler
- A C++17 compiler (MSVC, Clang, or GCC)

### Installing wxWidgets

- **Linux (Debian/Ubuntu):** `sudo apt install libwxgtk3.2-dev`
- **macOS (Homebrew):**      `brew install wxwidgets`
- **Windows:** build wxWidgets from source and set `wxWidgets_ROOT_DIR`
  (or use the official pre-built binaries)

## Building

```sh
cmake -S . -B build
cmake --build build --config Release
```

The build does the following extra steps automatically:

1. Compiles the host tool `bin2c` with `HostCompile.cmake`.
2. Runs `wxrc` to pack `reversi.xrc` + the two BMPs into `reversi.xrs`.
3. Runs `bin2c` to turn `reversi.xrs` into `reversi_xrs.h`, which is
   compiled into the executable.

### Running

- **Linux:**   `./build/wxreversi`
- **macOS:**   `open build/wxreversi.app`
- **Windows:** `build\Release\wxreversi.exe`

### Installing (Linux)

```sh
cmake --install build --prefix /usr/local
```

Installs the binary to `bin/`, the `.desktop` file to
`share/applications/`, the AppStream metainfo to `share/metainfo/`, and the
icon set to `share/icons/hicolor/`.

## How to play

You (red / white) play against the computer (blue / black). Click any
square where the cursor becomes a cross to place a piece; all bracketed
enemy pieces are flipped to your colour. The computer replies
automatically.

| Menu                | Action                                           |
|---------------------|--------------------------------------------------|
| **Game → New**      | Start a new game                                 |
| **Game → Hint**     | Flash the engine's recommended move (marks the session as a "Practice Game") |
| **Game → Pass**     | Pass when no legal move is available             |
| **Game → Change Color Mode** | Toggle between color and black-and-white boards |
| **Skill**           | Select search depth: Beginner / Novice / Expert / Master |
| **Help → About**    | About dialog                                     |

Keyboard: <kbd>Ctrl</kbd>+<kbd>H</kbd> hints, <kbd>Return</kbd> /
<kbd>Space</kbd> plays the first legal move in preference order.

The game ends when neither side has a legal move; the final score is
displayed along with the margin of victory.

## Engine notes

- Board is a 10x10 sentinel-padded `uint8_t[100]` (squares 11..88 are
  playable), which lets the flipping loops safely walk off the edge.
- Search: alpha-beta with a fixed move-ordering table (corners first,
  edges next, interior last). Depths 1 / 2 / 4 / 6 match the original
  skill levels.
- The engine preallocates `MaxDepth + 2` board copies so the recursive
  search performs no heap allocation.
- `reversi_engine.{h,cpp}` is entirely GUI-free — it can be reused with
  any C++ front-end (Qt, SDL, a CLI, etc.).

## Credits

- Original game: **Microsoft / Chris Peters** (Windows Reversi sample)
- wxWidgets port: 2026
