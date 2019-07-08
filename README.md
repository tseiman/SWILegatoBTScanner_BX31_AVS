# SWILegatoBTScanner_BX31_AVS
Bluetooth scanner for BT advertisements on MangOH red Board with WP7607, using BX310x, and AirVantage


## Starting Development

First setup the Leaf environment for the project:

    leaf setup -p swi-wp76_4.1.0 wp76

This installs the Legato toolchain for the WP76 and creates a build profile named 'wp76'.

To build from the command line, first make sure you are in a Leaf shell.  You can tell if you see
the following pre-pended to your prompt:

    (lsh:wp76)

Then, run the following command.

    mksys -t wp76xx BX31_DemoSystem.sdef

To build from VS Code, open this project directory, make sure that BX31_DemoSystem.sdef is current.
Then hit Ctrl+B and select either:

    Legato: Build
    Legato: Build and install
