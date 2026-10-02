# KenobiCmd

A simple communicator using the QMK Kenobi Firmware.

## QMK Firmware

This project is based on [QMK Firmware](https://github.com/qmk/qmk_firmware/). [QMK Kenobi Firmware](https://github.com/Blizzardo1/qmk_firmware) is forked from [Keychron QMK Firmware](https://github.com/Keychron/qmk_firmware/), with my personal touches (WIP).

I own two Keychron K10 Pro ANSI RGB Keyboard with hot-swappable key switches. There's also some information I can grab from these keyboards, so why not. 
If I had other keyboards with LED Screens, knobs, speakers, and so forth; I should be able to get information from them too.

# Requirements

In order for this program to work properly, your keyboard needs to be flashed with `QMK Kenobi`.

## Install

Clone this repo:

```bash
git clone https://github.com/Blizzardo1/KenobiCmd && cd KenobiCmd
cmake -S . -B build
# Then run to get the [Help File]
sudo build/kenobicmd --help

```

## Help File

```
Usage: kenobicmd [OPTION...]
Kenobi Command -- a program that will communicate with QMK Kenobi Firmware

  -b, --battery              Get battery information
  -f, --factory-test         Run factory test
  -k, --keyboard-layout      Get keyboard layout
  -l, --lock-status          Get keyboard lock status
  -n, --no-banner            Don't print the banner
  -s, --strip                Strip text, leave only numbers
  -t, --bluetooth            Enable bluetooth
  -w, --wpm                  Get keyboard words per minute
  -?, --help                 Give this help list
      --usage                Give a short usage message
  -V, --version              Print program version
```

# Changes

You are more than welcome to introduce changes to fit your keyboard needs. If you have a different set of commands you use, I encourage you to fork this repo and make your changes.


# Pull Requests

If you would like to see your changes in this repo, please make a pull request, and your changes will be added to a branch. 

# Contributions

I welcome any contributions to this code, just make a pull request. State your changes, and why.
