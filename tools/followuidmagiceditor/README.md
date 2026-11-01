# followuidmagiceditor

Sets the head and tail points of follow-UID (跟随) magic APNGs. The GUI uses
PySide6, and it reads the magic records through the `followuidmagiceditor`
python module, a small nanobind module built from the header-only DBCOM in
`common/src`. It runs on Linux and is a developer tool; it is installed to
`tools/followuidmagiceditor`, not with the client.

Head and tail are stored in the APNG file names (see
[magic](../gfxres2apng/src/README.md#magic)). Edits stay in memory until
**File > Save**, which renames the edited files in the magic directory.
Nothing else is written.

## Build

Install PySide6, Pillow and nanobind in a Python environment, then build the
`followuidmagiceditor` target with that Python:

```
python3 -m venv ~/.venv
~/.venv/bin/pip install PySide6 Pillow nanobind

./build.py ... --python ~/.venv/bin/python3 --target followuidmagiceditor
```

The module is
`<build-dir>/build/tools/followuidmagiceditor/src/followuidmagiceditor.cpython-*.so`.
If the build Python has no nanobind, CMake skips the target with a status
message.

Install puts the module, the Python files and a copy of
`tools/gfxres2apng/src/gfxres2apng.py` (the name codec) into
`<install-dir>/tools/followuidmagiceditor`. The module only loads in the
Python version it was built for.

## Run

```
~/.venv/bin/python3 <install-dir>/tools/followuidmagiceditor/main.py
```

- The `followuidmagiceditor` module is always imported from the directory of
  `main.py`, so run the installed copy; the source tree has no module next
  to `main.py`.
- Nothing is shown until **File > Open Magic Dir** opens the APNG directory
  (`texture/magic`), or a `gfxres2apng.py` output root that has
  `texture/magic`. Nothing is remembered between runs.

## Use

- **Magic** menu: every magic whose 运行 stage is a 跟随 gfx, with its
  `gfxID` and direction count. `→ name` means the entry is a `.ref` to that
  magic. Magics sharing a `gfxID` share the same APNGs, so editing one
  edits all of them; the info pane lists them.
- The canvas shows direction `i` of `N` at `i * 360 / N` degrees clockwise
  from up. Each frame is drawn at its anchor (white cross) plus its offset.
  The head is red, the tail is cyan, and the yellow line is `tail -> head`.
  A red X is a missing APNG.
- **Left click** sets the head and **right click** sets the tail of the
  direction under the mouse (green border), relative to its anchor. Hold
  the button and drag to move the point; the drag stays on the direction it
  starts on, and the point is clamped to that frame. An edited direction has
  a yellow `*` label, and the title shows the number of unsaved APNGs.
- **Edit > Tail on Head** (T) puts every direction's tail on its head,
  so the magic is never rotated; then left and right click/drag both move
  head and tail together. Turning it off restores the tails from before it
  was turned on, and right click sets the tail again. It's per `gfxID`, so
  magics sharing the APNGs share it, and it's on at load when every
  direction already has head == tail.
- **File > Save** (Ctrl+S) renames the edited APNGs, **File > Revert**
  discards the edits. Edits of all magics are kept until then; Open, Reload
  and Quit ask to save or discard them.
- **Drag** outside every frame moves the view, **Ctrl + drag** changes the
  circle radius, the **wheel** zooms around the mouse, **Space** plays and
  pauses, **View > Reset View** restores the view.
- **File > Reload** rereads the directory after it is changed outside.
- The right **info pane** shows the magic dir (APNG and unsaved counts),
  the magic record, its gfx entry, the hovered direction (mouse, current
  and saved head/tail, file and edited name), and a table of all
  directions. Clicking a table row selects that direction on the canvas
  (orange border). The status bar repeats the hover info in one line.

After saving, repack `magic.zsdb` from the magic directory. Don't
regenerate the directory with `gfxres2apng.py`: it resets every head and tail.
