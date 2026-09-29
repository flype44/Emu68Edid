# **Emu68EDID** version 1.1

## **Description**

The `Emu68EDID` program is an AmigaOS command line tool, for [PiStorm](https://github.com/captain-amygdala/pistorm)/[Emu68](https://github.com/michalsc/Emu68/releases), to gather information from the connected display (monitor or TV), through the Raspberry Pi > Mailbox interface > [Get EDID block](https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface#get-edid-block).

This program retrieves the [EDID](https://en.wikipedia.org/wiki/Extended_Display_Identification_Data) data (Extended Display Identification Data) from your monitor, parse it, and outputs the decoded data.

This allows identifies, for example, the best native preferred resolution supported by a display device (old display may not support this feature).

Written by `Philippe CARPENTIER`, 2025-2026.

Compiled with SAS/C 6.59 for AmigaOS/M68K.

Requires AmigaOS 3.x and PiStorm/Emu68 1.1 or later (uses `mailbox.resource`).

Freely distributed for non-commercial purposes.

## **Arguments**

```
DISPLAY/N: Selects the display number to query (0 = primary, 1 = secondary).
           If omitted, the primary display is used.
DUMP/S:    Outputs the EDID binary data in standard hexadecimal representation.
TO/K:      Saves the EDID binary data to file (for example: TO=EDID.bin).
PARSE/S:   Outputs the EDID decoded data, in human readable format.
FULL/S:    Outputs the EDID decoded data, in a more exhaustive way.
FROM/K:    Outputs the EDID decoded data, from a previously saved EDID binary data file.
```

## **Syntax**

```
Emu68EDID DUMP [DISPLAY=<num>] [TO=<file>]
Emu68EDID PARSE [DISPLAY=<num>] [FROM=<file>] [FULL]
```

## **Example**

```
Emu68EDID ?
Emu68EDID DUMP
Emu68EDID DUMP >EDID.txt
Emu68EDID DUMP TO=EDID.bin
Emu68EDID DUMP DISPLAY=0
Emu68EDID DUMP DISPLAY=1
Emu68EDID PARSE
Emu68EDID PARSE FULL
Emu68EDID PARSE FULL DISPLAY=0
Emu68EDID PARSE FULL DISPLAY=1
Emu68EDID PARSE FROM=EDID.bin
Emu68EDID PARSE FROM=EDID.bin FULL
```

## **Remarks**

The `EDID` data is composed of blocks of 128 bytes. The most important data are in the first block 0.
Then, optionally, there can be more, those are blocks of `EDID` extensions data.
Most of the times we get 2 blocks of 128 bytes, so a dumped file is usually 256 bytes.

The `DISPLAY` argument is only meaningful on Raspberry Pi models that provide two HDMI outputs, such as the Raspberry Pi 4B (two micro-HDMI ports) or a Compute Module 4 on a carrier board that exposes both.
Models with a single HDMI output (for example the Raspberry Pi 3, Zero 2 or 400) only offer display 0. Querying `DISPLAY=1` there, or with no second display connected, returns no EDID data.

The internal `EDID` decoder used in this program is "borrowed" from the `SDL2` project [here](https://github.com/libsdl-org/SDL/blob/main/src/video/x11/edid-parse.c).

The byte data obtain with the `DUMP` option can be copy/pasted alternatively into any valid Online EDID parsers, such as:

> https://www.edidreader.com/

> https://people.freedesktop.org/~imirkin/edid-decode/

> https://hverkuil.home.xs4all.nl/edid-decode/edid-decode.html

Type `EDID reader` or `EDID decoder` in any web search engine.

Real `EDID` dump examples, captured from various monitors and TVs, are provided in the archive (`bin` directory, `EDID_*.dump` files).
They are raw binary files, as produced by the `TO` option, so they can be decoded without any display attached, for example:

```
Emu68EDID PARSE FROM=EDID_SAM735A.dump FULL
```

## **Output example**

```
RAM:> Emu68Edid PARSE FULL
Checksum: 0 (correct)
Manufacturer Code: BNQ
Product Code: 0x8013
Serial Number: 21573
Production Week: 33
Production Year: 2017
Model Year: unspecified
EDID revision: 1.3
Display is digital
Bits Per Primary: 8
Interface: undefined
RGB 4:4:4: yes
YCrCb 4:4:4: no
YCrCb 4:2:2: no
Width: 340 mm
Height: 270 mm
Aspect Ratio: undefined
Gamma: 2.200000
Standby: no
Suspend: no
Active Off: yes
SRGB is Standard: yes
Preferred Timing Includes Native: yes
Continuous Frequency: no
Red   X: 0.644531
Red   Y: 0.339844
Green X: 0.288086
Green Y: 0.606445
Blue  X: 0.146484
Blue  Y: 0.066406
White X: 0.313477
White Y: 0.329102
Established Timings:
  800 x 600 @ 60 Hz
  640 x 480 @ 75 Hz
  640 x 480 @ 60 Hz
  720 x 400 @ 70 Hz
  1280 x 1024 @ 75 Hz
  1024 x 768 @ 75 Hz
  1024 x 768 @ 60 Hz
  832 x 624 @ 75 Hz
  800 x 600 @ 75 Hz
  1152 x 870 @ 75 Hz
Standard Timings:
  1280 x 720 @ 60 Hz
  1280 x 800 @ 60 Hz
  1280 x 960 @ 60 Hz
  1280 x 1024 @ 60 Hz
Timing (Preferred): 
  Pixel Clock: 108000000
  H Addressable: 1280
  H Blank: 408
  H Front Porch: 48
  H Sync: 112
  V Addressable: 1024
  V Blank: 42
  V Front Porch: 1
  V Sync: 3
  Width: 338 mm
  Height: 270 mm
  Right Border: 0
  Top Border: 0
  Stereo: No Stereo
  Digital Sync:
    composite: no
    serrations: no
    negative vsync: no
    negative hsync: yes
Detailed Product information:
  Product Name: BenQ BL702
  Serial Number: T8H00835SL0
  Unspecified String:
```

## **Screenshot**

<img width="687" height="1080" alt="image" src="https://github.com/user-attachments/assets/5b24f568-e9d8-43a7-84af-43371601083d" />

.
