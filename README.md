# Array Window Manager (awm)

Work in progress automatically tiling X11 window manager using array structure
instead of tree for managing windows.

Focuses on working well with multi-monitor environments and aims to provide
complete experience.

## Disclaimer
As it is pre-alpha software, functionality is not finnished and bugs are to be
expected.

## Features
- Status bar with builtin launcher
- Minimization of windows
- Configuration using config.h and lua?
- Performance and low memmory footprint
- Partial support for ICCCM and EWMH standards
- Universal controls for floating and tiling windows

## Installation
awm uses Makefiles for compilation so simple `make && make install` should work.
Scripts used are copied to /etc/awm/scripts.

## Implementation notes
- Bar as a separate executable
- Grid and floating layout provide same interface
- Layout module chooses appropriate layout
- Layout uses opaque pointer in interface
- Multiple entry points: event module, default config, ipc?, lua?
- Wrapper over xcb
- Hints managaed by most general function designed to handle specific functionality
- One shortcut per key + modifiers + mode + type
