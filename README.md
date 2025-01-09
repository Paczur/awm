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

## Notes
### Layout
- Grid and floating layout provide same interface
- Layout module chooses appropriate layout
- Floating and tiling as separate workspace modes
- Manual tiling (automatic as option?)
- Emacs/Vim style splits with placeholder window
- Monitors bound together (same or separate workspace?)
- Stack of minimized windows

### General
- One shortcut per key + modifiers + mode + type
- Bar as a separate executable
- Opaque pointers where it makes sense
- Multiple entry points: event module, default config, ipc?, lua?

### XCB Wrapper
- Sole global X interface
- Caching
- Unknown state on startup, lookup when needed
- Every struct is opaque
