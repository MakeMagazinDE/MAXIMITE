DIM needle_arr(3)

Mode 3
Cls
FONT #1

SetPin 1,1
SetPin 2,1
SetPin 3,1

ParamName$ = "DEATH RATE"
plot_grid 0, 0, 300
ParamName$ = "GERBIL SPEED"
plot_scale 200, 0, 100
sinrad=0

Touch Size 80,25
Touch Create 9,390,240,"EXIT",green,B
Touch Size 25,100	'set vertical slider size - drawed item is somewhat longer due to knob 
Touch Create 5,430,5,"",blue,V,B	  ' Vertical slider, fill Bottom side
Touch Size 165,30	'set horizontal slider size
Touch Create 3,0,180,"",green,H,LND		' fill left side, no knob, disable automatic touch (-)
Touch Size 80,20
Touch Create 7,20,220,"Faster!",white,C	'create and draw a checkbox

do
	my_sine=sin(sinrad)
	if TouchVal(5) > 80 then
		Touch Beep 100
		TouchVal(5)=80
	endif
	TouchVal(3)=my_sine*20 + 30
	plot_graph 0, 0, my_sine*50+50, 300
	plot_needle 200, 0, TouchVal(5)+my_sine*15+15, 100, 2
	sinrad=sinrad + 0.1 + TouchVal(7)*0.2
	
	' pause 20

	if TouchVal(9) then
		Touch Remove All
		run "AUTORUN.BAS"
  endif

loop
  
  
sub plot_scale (x, y, max_s)
  local center_x, center_y
	center_x = x + 84
	center_y = y + 120
  ' Analog-Zeigerinstrument, Aufhängung der Nadel = Center
  line (center_x - 84, center_y - 120) - (center_x + 84, center_y + 20), blue, B
  line (center_x - 83, center_y - 119) - (center_x + 83, center_y + 19), white, BF
  FONT #1
  color black
  For i= 0 To 20
    rads=(i-10)*pi/40
    x2=Sin(rads)*100 + center_x
    y2=-Cos(rads)*100 + center_y
    If i Mod 4 <> 0 Then
      x1=Sin(rads)*95 + center_x
      y1=-Cos(rads)*95 + center_y
    Else
      x1=Sin(rads)*90 + center_x
      y1=-Cos(rads)*90 + center_y
      print @(x2-12,y2-14,1) (max_s*i/20)
    EndIf
    Line(x1,y1)-(x2,y2),blue
  Next i
  FONT #2
  color yellow
  print @(center_x-84,center_y+25) ParamName$
end sub
  
sub plot_needle (x, y, needle_v, max_s, needle_idx)
  ' Analog-Zeigerinstrument, Aufhängung der Nadel = Center
  local my_needle, center_x, center_y
	center_x = x + 84
	center_y = y + 120
  my_needle= needle_v * 100 / max_s
  ' needle value 0..100
  if my_needle <> needle_arr(needle_idx) then
    circle(center_x, center_y), 8, blue
    rads= (needle_arr(needle_idx)-50)*pi/200
    x2=Sin(rads)*90 + center_x
    y2=-Cos(rads)*90 + center_y
    Line(center_x,center_y)-(x2,y2),white
    
    rads= (my_needle-50)*pi/200
    x2=Sin(rads)*90 + center_x
    y2=-Cos(rads)*90 + center_y
    Line(center_x,center_y)-(x2,y2),red
    color red, black
    FONT #2
    print @(center_x+15,center_y-9) format$(needle_v, "%5.1f")
  endif
  needle_arr(needle_idx) = my_needle
end sub
  
sub plot_grid (x, y, max_s)
  local center_x, center_y
	center_x = x + 84
	center_y = y + 70

  FONT #1
  color blue
  print @(center_x-78,center_y+40) 0
  print @(center_x-78,center_y-10) max_s/2
  print @(center_x-78,center_y-60) max_s
  line (center_x - 84, center_y - 70) - (center_x + 84, center_y + 70), blue, B
  For i= -2 To 2
    Line(center_x - 80, center_y + (i*25))-(center_x + 80, center_y + (i*25)), blue
  Next i
  For i= -3 To 3
    Line(center_x + (i*25), center_y - 67)-(center_x + (i*25), center_y + 67), blue
  Next i
  FONT #2
  color yellow
  print @(center_x-84,center_y+77) ParamName$
end sub
  
sub plot_graph (x, y, graph_v, graph_max)
  local center_x, center_y
	center_x = x + 84
	center_y = y + 70
  FONT #1
  BLIT center_x - 82, center_y - 50, center_x -83, center_y - 50, 167, 101, g
  PIXEL(center_x + 83, center_y + 50 - graph_v) = green
  
end sub
  
  
