Mode 3:	Cls: 	Font #2: Color white, black
option usb off
Touch remove all
print @(100,6) "CMs TFT Button Demo"
Touch Size 110,45	'set new bigger button size

Touch Create 1, 5,30, "VIBR",256*red+yellow,s
Touch Create 2, 245,30, "UPPER",256*yellow+green,p
Touch Create 3, 365,30, "LOWER",256*yellow+green,p

Touch Create 4, 5,100, "PercON",256*red+yellow,p
Touch Create 5, 125,100, "FAST",yellow,p,d          ' Disable automatic touch handling
Touch Create 6, 245,100, "SOFT",256*cyan+green,p,d	' for all button pairs
Touch Create 7, 365,100, "3rd",256*cyan+blue,b,d

Touch Create 8, 125,150, "SLOW",yellow,p,d
Touch Create 9, 245,150, "NORMAL",256*cyan+green,p,d
Touch Create 10, 365,150, "2nd",256*cyan+blue,b,d
Touch Create 0, 365,220, "EXIT",red,b

TouchVal(5) = 1	' default button pair state
TouchVal(6) = 1
TouchVal(7) = 1

do
	if Touched(4) then	'registers any touch of item
		Touch Beep 50
		if TouchVal(4) then
			Pause 50
			Touch Beep 50
		endif 
	endif


' handle button pairs manually
	if Touched(5) then
		TouchVal(5) = 1
		TouchVal(8) = 0
		Touch Beep (50)
		Touch Release
	endif
	if Touched(8) then
		TouchVal(8) = 1
		TouchVal(5) = 0
		Touch Beep (50)
		Touch Release
	endif

	if Touched(6) then
		TouchVal(6) = 1
		TouchVal(9) = 0
		Touch Beep (50)
		Touch Release
	endif
	if Touched(9) then
		TouchVal(9) = 1
		TouchVal(6) = 0
		Touch Beep (50)
		Touch Release
	endif

	if Touched(7) then
		TouchVal(7) = 1
		TouchVal(10) = 0
		Touch Beep (50)
		Touch Release
	endif
	if Touched(10) then
		TouchVal(10) = 1
		TouchVal(7) = 0
		Touch Beep (50)
		Touch Release
	endif

	if TouchVal(0) then
		Touch Remove All
		Cls black
		run "AUTORUN.BAS"
	endif
	pause 100
loop
