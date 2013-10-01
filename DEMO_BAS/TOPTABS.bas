Mode 3:	Cls: 	Font #2: Color white, black
' advanced touch functions demo by Carsten Meyer, cm@ct.de
option usb off
Touch remove all

TabFrame(0)
tab_active = 0
draw_new = 1

do
  for i = 0 to 2
  	if Touched(i) then
			TabFrame(i)	' drawn new tab frame
			tab_active = i
			draw_new = 1
  	endif
  next i
	if draw_new then	'executed only once after tab change
		Font #2: Color white, black
		if tab_active=0 then
			print @(12,40) "first tab page"
			Touch Size 30,150	'set vertical slider size - drawed item is somewhat longer due to knob
			Touch Create 10,40,80,"",blue,V,B	  ' Vertical slider, fill Btm side
			Touch Size 50,150	'set vertical bar size 
			Touch Create 11,100,80,"",purple,V,BND	' Vertical slider, fill Bottom side, No knob, disable automatic touch (-)
			Touch Size 120,40	'set new bigger button size
			Touch Create 12,220,80, "",red,s
			Touch Create 13,220,150, "FIX IT",256*cyan+green,p
		elseif tab_active=1 then
			print @(12,40) "this is my second page"
			Touch Size 80,25	'set default button size
			Touch Create 10,260,120,"Slow",white,R	'create and draw some radio buttons, re-use IDs
			Touch Create 11,260,150,"Medium",white,R
			Touch Create 12,260,180,"Fast",white,R
			TouchVal(10) = 1			
			Touch Size 120,30	'set new bigger button size
			Touch Create 13,50,120,"Testing",red,L,d	'LED disabled
			Touch Create 14,50,170,"Overload",green,L,d	'strange string behaviour!!!
			Font #1: Color white, black
			print @(50,215) "I know this bug - strange text behaviour!"
		elseif tab_active=2 then
			print @(12,40) "and my third page"
		else
		endif
	endif
	
	if tab_active=0 then	'some sliders and switches
		if Touched(10) then	' don't waste time if nothing happened
			TouchVal(11) = TouchVal(10) \ 2 + 50
			Touch Create 12,220,80, "",TouchVal(10)/21,s	' overwrite existing switch		
		endif
		
		if Touched(13) then	' update only if needed
		  old_val=TouchVal(12)
			if TouchVal(13) then
				Touch Create 12,220,80, "",cyan,s	' overwrite existing switch
			else
				Touch Create 12,220,80, "",red,s
			endif
		  TouchVal(12)=old_val
		endif
		
	elseif tab_active=1 then	'blinking LED plus radio buttons
		if TouchVal(10) then ledspeed=1
		if TouchVal(11) then ledspeed=2
		if TouchVal(12) then ledspeed=5
		ledcount = ledcount+ledspeed
		if ledcount	> 500 then
			TouchVal(13) = not TouchVal(13)
			ledcount=0
		endif
		
	elseif tab_active=2 then 'display date and time
		Font #2: Color green, black
		print @(12,80) time$ "  " date$
		
	else
	endif

	if TouchVal(30) then
		Touch Remove All
		Cls black
		run "AUTORUN.BAS"
	endif
  draw_new = 0
loop

sub TabFrame(tab_on)
	Touch remove all
	Touch Beep (50)
	Touch Size 80,25
	'cls black
	Touch Create 30,390,0,"EXIT",green,B
	
	Touch Size 120,30	'set new bigger button size
	Touch Create 0,0,0, "Sliders",white,p,d 'Pushbutton, disabled auto
	Touch Create 1,122,0, "Options",white,p,d
	Touch Create 2,244,0, "Time",white,p,d
	if tab_on = 0 then
		TouchVal(0)=1
		TouchVal(1)=0
		TouchVal(2)=0
	elseif tab_on = 1 then
		TouchVal(0)=0
		TouchVal(1)=1
		TouchVal(2)=0
	elseif tab_on = 2 then
		TouchVal(0)=0
		TouchVal(1)=0
		TouchVal(2)=1
	endif
	Line(0,30)-(479,270), White, B

	'draw void "Button" and erase rect, somewhat faster than 	Line(1,36)-(478,269), black, BF
	Touch Size 477,233	'set new bigger button size for dummy
	Touch Create 9,1,31, "",8,n,d	'colour 8 means black with item redraw
	Touch Remove 9	'no longer needed

	Touch Release
end sub
	