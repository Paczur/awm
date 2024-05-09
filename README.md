#Array Window Manager (awm)

Work in progress automatically tiling X11 window manager using array structure
instead of tree for managing windows.

Focuses on working well with multi-monitor environments and aims to provide
complete experience.

##Disclaimer
As it is pre-alpha software, functionality is not finnished and bugs are to be
expected.
Whole project configuration and installation is made for my personal machine
don't expect it to just work. It will probably be changed many times.

##Features
- status bar with builtin launcher
- simple configuration using user_config.h and config.h
- extensibility by editing source code
- performance and low memmory footprint
- partial support for ICCCM and EWMH standards

##Alpha Roadmap
- Finish logging
- Fullscreen hints (_NET_WM_STATE_FULLSCREEN)
- Window order hint (_NET_CLIENT_LIST_STACKING)
- Support for advanced focus (WM_TAKE_FOCUS)
- Monitor configuration in user_config.h or config.h
- More stability improvements

##Beta Roadmap
- Floating mode
- Clickable status bar
- Probably some config changes
- Even more stability improvements
- More universal default settings and install location
