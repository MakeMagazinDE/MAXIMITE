c't TFT Maximite
================

<b>BASIC panel computer with TFT touchscreen</b> based on PIC32MX695 microcontroller

MMBASIC and Colour Maximite design (c) by Geoff Graham<br>
TFT Maximite schematics, board layout artwork and TFT touch functions (c) by Carsten Meyer and c't 
magazine (www.ct-hacks.de)

### Updates

<b>TFT MM Schaltplan 1_4.pdf</b> Schematics for hardware revision #1.4<br>
<b>TFT MM Silk 1_4.pdf</b> Component placement silk screen print for hardware revision #1.4<br>
<b>TFT MM Dimensions 1_4.pdf</b> Board and connector dimensions drawing for hardware revision #1.4<br>

* Correct pin assignment for all MMBASIC Pins on PL6 (Pin 20 bug fixed)
* Additional I/O pins available on PL9: D4 to D13, A4, A5 and unfiltered PWM signals
* Modification of vertical sync circuit for flicker-free display (R23, D3)
* Higher voltage for D5 zener, does not limit display brightness anymore
* Relocated PL12 for optional connector/SD slot break-out board
* Current firmware version 4.4b1 will run on TFT Maximite #1.4 without change

### Firmware History

* <b>4.4B1</b> Added touch commands:
   * TOUCH DISABLE(item) and TOUCH ENABLE(item) to temporarily enable or disable a touch item
   * TOUCH REMOVE(item) changed, will now also erase item from screen to current background color
* <b>4.4B</b> Officially released by Geoff Graham
* <b>4.4A</b> initial TFT firmware version on Segor kit, hardware #1.3

### Files

<b>TFT_Maximite.hex</b> Firmware HEX file with MMBASIC 4.4A, to be used with bootloader<br>
<b>TFT MM PartsList.txt</b> Parts list<br>
<b>TFT MM Dimensions.pdf</b> Board and connector dimensions drawing<br>
<b>TFT MM Silk.pdf</b>	Component placement silk screen print<br>
<b>TFT MM Schaltplan.pdf</b> Schematic<br>
<b>TFT Maximite Manual.pdf</b> Documentation to MMBASIC's new TOUCH functions and commands<br>
<b>TFT Maximite Manual.pdf</b> Documentation to MMBASIC's new TOUCH functions and commands<br>
<b>wordfile_mm.txt</b> MMBASIC Wordfile for UltraEdit syntax highlighting

<b>TFT_Maximite_xxx.hex</b> latest firmware build (current: 4.4B1), to be uploaded with Maximite bootloader.
<b>TFT_Maximite_plus_bootloader_44B.hex</b> initial TFT firmware version, includes Maximite bootloader, to be flashed with PicKit3 or similar programmer to virgin or corrupted PIC32

### Folders

<b>DEMO_BAS</b> contains several MMBASIC programs for demonstration of touch functions and commands. Copy to SD-card and run from BASIC prompt (AUTORUN with file select utility will start automatically)<br>
<b>SOURCE</b> Contains source files for those who want to modify the BASIC interpreter or add own BASIC commands. You also need the sources from Geoff Graham (see below). Not needed for the rest of us.

### Useful Links

http://www.segor.de/#Q%3DTFTMaximite4.3%252522Touch%26M%3D1 TFT Maximite pre-assembled kit from Segor, 2 versions available with 4.3" and 5" TFT<br>
http://www.mmbasic.com	MMBASIC Homepage with documentation<br>
http://geoffg.net/maximite.html	Original Maximite homepage, links to serial USB driver, 
utilities and application software<br>
http://geoffg.net/tft-maximite.html TFT Maximirte support page from Geoff Graham, with most recent firmware updates<br>
http://www.c-com.com.au/MMedit.htm	MMedit, Uploader and editor for MMBASIC sources, 
also applicable for TFT Maximite<br>
http://www.circuitgizmos.com/products/cgcolormax2/cgcolormax2.shtml	GC Color Maximite 2 with MMIDE utility, 
also applicable for TFT Maximite<br>

### Remarks

Touch calibration may not work if real time clock is not set or clock battery 
removed. Before using TOUCH CALIBRATE, please insert 3V CR2032 battery and set 
time and date with commands TIME$="xx:xx" and DATE$="xx-xx-xxxx".

Due to layout quirk, MMBASIC Pin 20 is not usable (should be connected to PIC32 Pin 17 instead of Pin 11). Fixed with hardware revision 1.4

