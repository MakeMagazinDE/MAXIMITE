Mode 3
Cls
Locate 0,0

Touch Size 80,25
Touch Create 1,50,50,"LED1",red,L,d	'LED, Disabled touch (-)
Touch Size 120,40	'set new bigger button size
Touch Create 2,50,120,"LEDBTN2",green,L	'LED
Touch Create 3,50,180,"LED3",red,L,d	'LED disabled
Touch Create 9,350,220,"EXIT",green,B
TouchVal(2)=1	'switch LED on manually
TouchVal(3)=1	'switch LED on manually

do
	TouchVal(1)=0
	pause 500
	TouchVal(1)=1
	pause 500
	if TouchVal(9) then
		Touch Remove All
		Cls black
		run "AUTORUN.BAS"
  endif

loop
