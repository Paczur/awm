# Array Window Manager (awm)

Work in progress automatically tiling X11 window manager using array structure
instead of tree for managing windows.

Focuses on working well with multi-monitor environments and aims to provide
complete experience.

## Disclaimer
As it is pre-alpha software, functionality is not finnished and bugs are to be
expected.
Whole project configuration and installation is made for my personal machine
don't expect it to just work. It will probably be changed many times.

## Features
- Status bar with builtin launcher
- Minimize windows
- Simple configuration using user_config.h and config.h
- Extensibility by editing source code
- Performance and low memmory footprint
- Partial support for ICCCM and EWMH standards

## Alpha Roadmap
- Finish logging
- Fullscreen hints (_NET_WM_STATE_FULLSCREEN)
- Window order hint (_NET_CLIENT_LIST_STACKING)
- Support for advanced focus (WM_TAKE_FOCUS)
- Monitor configuration in user_config.h or config.h
- More stability improvements

## Beta Roadmap
- Floating mode
- Clickable status bar
- Probably some config changes
- Even more stability improvements
- More universal default settings

## Build && Installation
awm uses Makefiles for compilation so simple `make && make install` should work.
Keep in mind user_config.h has some setup specific settings so they require changing
to work with your configuration.
Scripts used are copied to /etc/awm/scripts.

## Source structure
Made of a lot of modules which are all orchestrated by controller.
### Module Structure ('module' used as placeholder for module name)
- only files from inside the module can be included unless there's exception specified below
- module.{c,h} - file gathering all of the functionality of specific module (header has to include module_types.h if present)
- module_types.h (not required) - types to be shared outside the module (has to include types.h)
- files used to split functionality as needed
### Big modules:
- layout - managing windows
- bar - displaying status bar
- hint - managing EWMH and ICCCM hints
### Smaller modules:
- config - functions used in user_config.h
- event - main event function and map from event type to function
- shortcut - shortcut addition and handling
- system - commands for getting system information and launching programs
