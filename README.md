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
- Floating and tiling as separate workspace modes
- All layout modes provide same interface
- Layout module chooses appropriate layout
- Stack of minimized windows
- Monitor workspaces bound together (same or separate workspace?)

Tiling Choice:
- Manual tiling with automatic placement
or:
- Manual tiling (no automatic placement)
- Placeholder window inserted on split
- Splits in floating mode problem

### General
- One shortcut per key + modifiers + mode + type
- Bar as a separate process
- Opaque pointers where it makes sense
- Multiple entry points
- Opaque structs
- Separate protocol for additional information
- Use X atoms as long term storage

### XCB Wrapper
- Sole global X interface
- Caching
- Unknown state on startup, lookup when needed
- Every information in atoms
