Generic SQL Formatter Plugin for Geany
======================================

Generic SQL Formatter is a plugin used to format and prettify ugly,
not formatted SQL files.

This repository represents an independent project whose results could
be manually integrated with Geany.

* Basic, configurable SQL formatting (don't expect miracles)
* Formatting of whole files
* Support for partial formatting limited to the text that is currently
selected

* Supported platforms: Linux
* License: LGPL-3.0

Dependencies:

* geany
* geany-devel or geany-common  (depending on the distro)
* gtk+3.0-dev(el) or gtk+2.0-dev(el)  (depending on the distro)
* fsqlf - version(1.0.0-prealpha-02) - **integrated in this repository**
* flex
* make
* cmake
* pkg-config

Compilation
-----------

To compile run: `make`

To install (root privileges needed) run: `sudo make install`

To uninstall (root privileges needed) run: `sudo make uninstall`

Local to the current account installation 
-----------------------------------------

This is an alternative to globally install the plugin for all users. No root privileges needed.

To install for the current account run: `make localinstall`

To uninstall for the current account run: `make localuninstall`

Other notes
-----------

Attention MacOS users - this plugin will work with the manually
installed and compiled Geany editor from source code. It will not work
with the version installed from dmg files.

Other Useful Plugins
--------------------
* [Geany JSON Prettifier](https://github.com/zhgzhg/Geany-JSON-Prettifier)
* [Geany Unix Timestamp Converter](https://github.com/zhgzhg/Geany-Unix-Timestamp-Converter)
* [Geany Base64 Converter](https://github.com/zhgzhg/Geany-Base64-Converter)
