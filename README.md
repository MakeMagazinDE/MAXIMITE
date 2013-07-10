MAXIMITE
========

MAXIMITE single board computer with TFT

Here you will find my modded Color Maximite firmware, working with a simple Sharp LQ043
4.3" TFT display (spare part for classic Playstation Portable 1000, on Ebay for around 20$), 
or more recent TFT displays from INNOLUX, AUO or similar.

Board has two display connectors: one for Sharp LQ043T3DX02 display,
other for newer ones with similar (but not identical) 40 pin connector which includes LED
supply and optional touchscreen pins (types TF43014A, PJ43002A etc, not tested yet).

Screen resolution is 480 x 272 px which yields 80 chars and 22 lines of text in MMBASIC.
Mode 4 (half resolution graphics) not supported.

For those who want to compile their own firmware: Use my video.c source file instead of original and add MMCUSTOM.c to your project. Disk size for A: should be reduced to fit in flash memory.

Touchscreen additions:

<value> = TouchX 	' get manual touch coordinates for drawing etc.
<value> = TouchY

TouchItemSize <x_size>,<y_size>		'set button/switch size in pixels for all subsequent TouchItemCreates

TouchItemCreate(x,y), "Caption", <refnum>, <colour>, <type> [B][S][R][C] ' make (B)utton or (S)witch or (R)adio or (C)heckbox

TouchItem(<refnum>) = value			' set touch item value manually

TouchCheck     ' handle all buttons/switch events and set items accordingly, to be used within loop

<value> = TouchItem(<refnum>)	' retrieve value of touch item (and reset button state to unpressed)

TouchItemRemove(<refnum>)		' remove touch item from list

See BTNDEMO.BAS in DEMO.zip for example use.

MMBASIC (c) by Geoff Graham, 2011-2013. Touch Additions and graphic widgets (c) by Carsten Meyer 2013.

-cm