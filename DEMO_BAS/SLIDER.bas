'SLIDER DEMO for TFT MAXIMITE
option usb off
Mode 3: Cls
Touch remove all

Touch Size 100,25	'set horizontal slider size - drawed item is somewhat longer due to knob
Touch Create 1,75,40,"ignore",cyan,H,LR	' fill both sides of knob, horizontal

Touch Size 200,30	'set horizontal slider size
Touch Create 2,20,100,"",yellow*256+red,H				' horizontal slider, do not fill sides 
Touch Create 3,20,160,"",green,H,LND		' fill left side, no knob, disable automatic touch (-)
Touch Create 4,20,220,"",yellow,H,R	' fill right side (colour + 16)

Touch Size 25,100	'set vertical slider size - drawed item is somewhat longer due to knob 
Touch Create 5,300,50,"",blue,V,T	  ' Vertical slider, fill Top side, 
Touch Create 6,370,50,"",purple,V,BND	' Vertical slider, fill Bottom side, No knob, disable automatic touch (-)
Touch Size 120,40	'set new button size
Touch Create 0,350,220,"STOP",red,B
Font #2
TouchVal(2)= 77:TouchVal(3)= 43:TouchVal(4)= 50:TouchVal(5)= 20
print @(0,0) "CMs Slider"
count=0
do
	Touch Check	' makes things faster
	redslider = TouchVal(2)
	yellowslider = TouchVal(4)
	blueslider = TouchVal(5)
	TouchVal(3) = yellowslider
	TouchVal(6) = blueslider
	' color white:print @(160,0) count
	color yellow:print @(250,225) yellowslider "  "
	color blue:print @(290,180) blueslider "  "
	if redslider > 180 then
		Touch Beep 150	'make short 150 ms Beep
		TouchVal(2) = 180
  endif

	if TouchVal(0) then
		Touch Remove All
		Cls black
		run "AUTORUN.BAS"
  endif
	count=count+1
loop

