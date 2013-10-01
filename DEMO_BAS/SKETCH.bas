Mode 3
Cls
Line(0,0)-(479,271), White, B
Touch Remove All
Colour white, black
Touch Size 120,40	'set new bigger button size
Touch Create 9,350,220,"EXIT",green,B
Touch Create 1,200,220,"CLEAR",blue,B

Touch Size 80,25	'set normal button size
Touch Create 4,360,20,"Red",red,R	'create and draw some radio buttons
Touch Create 5,360,50,"Green",green,R
Touch Create 6,360,80,"Cyan",cyan,R
Touch Create 7,360,110,"Yellow",yellow,R
Touch Create 8,360,140,"White",white,R
TouchVal(4) = 1

do

 	if TouchVal(1) then
		run
  endif

 	if TouchVal(4) then
		my_colour = red
  endif
  
 	if TouchVal(5) then
		my_colour = green
  endif

 	if TouchVal(6) then
		my_colour = cyan
  endif
  
 	if TouchVal(7) then
		my_colour = yellow
  endif

 	if TouchVal(8) then
		my_colour = white
  endif

	if Touched(#S) and Touched(#X) < 360 then
		Circle(Touched(#X),Touched(#Y)),2,my_colour,F
	endif
	
 	if TouchVal(9) then
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

run "autorun.bas"
