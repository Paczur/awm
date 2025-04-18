# A Window Manager (awm)

Tiling X11 window manager featuring modal keybindings.

## Features
- Status bar with builtin launcher
- Minimization of windows
- Configuration by editing source code
- Performance and low memmory footprint
- Partial support for ICCCM and EWMH standards
- Modal keybidings
- Floating mode
- Non-reparenting
- Limit of 4 windows per workspace

## Non portable settings
- volume block in status bar (external script)
- colorscheme toggle (external script)
- screenshot function (hardcoded path)
- terminal and browser keybind (hardcoded name)

## Configuration
Most settings (keybinds, colorscheme) are customizable through **config.h** file.
Options more specific to window layout or status bar are to be found in
**layout/layout_config.h** and **bar/bar_config.h** files respectively.

## Screenshots

## Installation
awm uses Makefiles for compilation so simple `make && make install` should work.
Scripts used are copied to /etc/awm/scripts.
