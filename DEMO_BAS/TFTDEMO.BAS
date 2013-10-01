cls 1 : font #2 : color white,blue
option usb off

print @(100,6) "CMs TFT Maximite Demo"
settick 500, ledblink

Touch Size 80,25	'set default button size (power on)
' Touch Create <refrum>, x, y, "Caption",<colour>, <type> (B)utton or (S)witch or (R)adio or (C)heck
Touch Create 0,50,40,"OFF",yellow,B 	'create and draw some buttons
Touch Create 1,150,40,"LED",red,B
Touch Create 3,250,40,"ignore",purple,S 	'create and draw a ON/OFF switch, caption ignored

TouchVal(3) = 1 ' set switch state manually to "on" by RefNum (3)

Touch Size 120,40	'set new bigger button size
Touch Create 2,350,220,"EXIT",green,B

'create some radio buttons
Touch Size 80,25	'set default button size (power on)
Touch Create 4,360,120,"KickMe",white,R	'create and draw some radio buttons
Touch Create 5,360,150,"Opt 2",white,R
Touch Create 6,360,180,"Opt 3",white,R
TouchVal(4) = 1

Touch Size 80,20
Touch Create 7,50,230,"CheckMe",cyan,C	'create and draw a checkbox
Touch Create 8,200,230,"or me",white,C	'create and draw a checkbox

Touch Size 25,100	'set vertical slider size - drawed item is somewhat longer due to knob 
Touch Create 9,300,80,"",cyan,V,B	  ' Vertical Slider, Bottom fill below knob 

Touch Size 80,25
Touch Create 10,360,50,"LED 1",red,L 'create a small LED
Touch Create 11,360,80,"LED 2",green,L

toggle_onoff = 0
Line(5,80)-(140,210), black, BF
do
	if Touched(#S) then	' Screen touched?
		x = Touched(#X)		' needed for sketch drawing only
		y = Touched(#Y)
		if x >= 5 and x < 140 then
			if y >= 80 and y < 210 then
				Circle(x,y) ,2,white,F
				font #2
				print @(150,90) x "  " @(190,90)" tX"
				print @(150,120) y "  " @(190,120)" tY"
				Touch Release	' wait for finger releasing touch screen
			endif
		endif
	endif
	if TouchVal(0) then	' automatically resets button state
		TouchVal(10)=0	'LED 1 off
	endif
	If TouchVal(1) then	' automatically resets button state
		TouchVal(10)=1	'LED 1 on
	endif

	if TouchVal(2) then
		Touch Remove All
		Cls black
		font #2
		colour yellow, black
		print: print "Good Bye"
		pause 500
		run "AUTORUN.BAS"
		end
  endif

loop


ledblink:
	if TouchVal(3) then
		blink_on = not blink_on
		TouchVal(11) = blink_on ' blink LED
	endif
ireturn

