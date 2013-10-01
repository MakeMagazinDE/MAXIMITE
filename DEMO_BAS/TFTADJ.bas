Mode 3
Cls
Line(0,0)-(479,271), White, B
Line(0,0)-(479,271)
Locate 0,0

Touch Size 120,40	'set new bigger button size
Touch Create 0,350,120,"CLEAR",blue,B
Touch Create 1,350,170,"CALIBR",cyan,B
Touch Create 9,350,220,"EXIT",green,B

Touch Size 80,25	'set normal button size
Touch Create 2,250,170,"PSP",cyan,B
Touch Create 3,250,220,"HANN",cyan,B

do
	x=100
	if Touched(#S) then
		Circle(Touched(#X),Touched(#Y)),2,white,F
	endif

 	if TouchVal(0) then
		run
  endif
  
 	if TouchVal(1) then
		Touch Calibrate
		run
  endif

 	if TouchVal(2) then
		CONFIG TFT PSP
  endif
 	if TouchVal(3) then
		CONFIG TFT HANN
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
