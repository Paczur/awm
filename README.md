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
- X Core fonts only
- Limit of 4 windows per workspace

## Non portable settings
As it is AWM is developed for my usage, which means there will
be a lot of functionality only working with my specific setup.
Notable ones are:
- volume block in status bar (external script)
- colorscheme toggle (external script)
- screenshot function (hardcoded path)
- terminal and browser keybind (hardcoded name)
- hardcoded font

## Configuration
Most settings (keybinds, colorscheme) are customizable through `config.h` file.
Options more specific to window layout or status bar are to be found in
`layout/layout_config.h` and `bar/bar_config.h` files respectively.

## Screenshots
![empty](https://github.com/user-attachments/assets/4bc23708-2048-4c90-932a-75e25e95c462)
![tiled](https://github.com/user-attachments/assets/2719d272-c37d-4dd7-8a5d-109a4240e72a)
![floating](https://github.com/user-attachments/assets/bf06e33c-573e-4daf-ba62-76f2eb6215c4)

## Installation
awm uses Makefiles for compilation so simple `make && make install` should work.
Scripts used are copied to /etc/awm/scripts.
