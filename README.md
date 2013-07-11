TFT MAXIMITE
============

MAXIMITE single board computer with 4.3" TFT touchsceen

Here you will find my modded Color Maximite firmware TFT_MM.HEX, working with a simple Sharp LQ043
4.3" TFT display (spare part for classic Playstation Portable 1000, on Ebay for around 20$), 
or more recent TFT displays from INNOLUX, AUO or similar.

Board has two display connectors: one for Sharp LQ043T3DX02 PSP display,
other for newer ones with similar (but not identical) 40 pin connector which includes LED
supply and optional touchscreen pins (types HannStar HSD043I9W1-A, EastRising ER-TFT043-3, TF43014A, PJ43002A etc). 

Screen resolution is 480 x 272 px which yields 80 chars and 22 lines of text in MMBASIC.
Mode 4 (half resolution graphics) not supported.

For those who want to compile their own firmware: Use my video.c source file instead of original and add MMCUSTOM.c to your project. Disk size for A: should be reduced to fit in flash memory. Please note #define TFT_* in video.c to select either PSP/Sharp LQ043 compatible or generic TFT (TFT_HANN), have slihghtly different timing.


Touchscreen additions:

Touch Item Size *x_size*,*y_size*		'set button/switch size in pixels for all subsequent TouchItemCreates

Touch Item Create *refnum*, x, y, "Caption", *colour*, *type* [B][S][R][C] ' make (B)utton or (S)witch or (R)adio or (C)heckbox

Touch Value(*refnum*) = value			' set touch item value manually

Touch Check     ' handle all buttons/switch events and set items accordingly, to be used within loop

*value* = Touch Value(*refnum*)	' retrieve value of touch item (and reset button state to unpressed)
*xcoord* = Touch Value(#X)	' retrieve X coordinate of current touch or -1 if none
*ycoord* = Touch Value(#Y)	' retrieve Y coordinate of current touch or -1 if none
*refnum* = Touch Value(#I)	' retrieve item Refnum of last item hit

Touch Release	' Wait for any touch released

Touch Wait	' Wait until any touch happened

Touch Item Remove [*refnum*][,*refnum*,*refnum*,..]	' remove one or more touch item from list
Touch Item Remove ALL	' remove all touch item from list

See BTNDEMO.BAS in DEMO.zip for example use.

MMBASIC (c) by Geoff Graham, 2011-2013. Touch Additions and graphic widgets (c) by Carsten Meyer 2013.

-cm