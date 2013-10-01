'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
' Demonstration of Colour MMBasic
' Geoff Graham,  June 2012
' TFT Touchscreen by Carsten Meyer
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

Mode 3
Cls
cx = MM.HRes/2 : cy = MM.VRes/2
Touch Size 120,40	'set new bigger button size
Touch Create 0,180,116,"EXIT",red,B

' draw random circles with smaller circles near to the centre
i = 0
Do
	Do
	x = Rnd * MM.HRes                            ' horiz center of the circle
	y = Rnd * MM.VRes                            ' vert center of the circle
	c = Int(Rnd * 6) + 1                       ' the colour
	Loop Until Pixel(x, y) <> c                  ' must be different
	d = Sqr(Abs(x-cx)^2 + Abs(y-cy)^2)           ' distance from the centre
	r = Rnd * d/8 + 2 + d/14                     ' radius
	Circle (x,y), r, c, f                        ' draw the sphere
	If r > Rnd*8 + 13 Then Circle (x,y), r+1, 0  ' draw the edge in black
	i= i+1
	If i>99 Then
		' redraw overwritten button regularily
		Touch Item Create 0,180,116,"EXIT",c,B
		i=0
	endif
	'Touch Check	' handle all buttons/switch events
	If TouchVal(0) then
		Touch Remove all	' make touch items invalid, don't redraw
		cls black
		colour white, black
		font #2
		print: print "Good Bye"
		font #1
		Pause 500
		run "autorun.bas"
	endif
Loop
