# SWILegatoBTScanner_BX31_AVS
Bluetooth scanner for BT advertisements on MangOH red Board with WP7607, using BX310x, and AirVantage


## Starting Development

First setup the Leaf environment for the project:

    leaf setup -p swi-wp76_4.1.0 wp76

This installs the Legato toolchain for the WP76 and creates a build profile named 'wp76'.

To build from the command line run:

    mksys -t wp76xx BX31_DemoSystem.sdef

To build from VS Code, open this project directory within VS Code and hit Ctrl+B.
