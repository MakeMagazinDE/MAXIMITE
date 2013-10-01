' Demo for some meters and scales
  Dim needle_arr(3)
  mode 3
  cls

  ParamName$="Ia mA"  
  plot_bar(40, 80, 267, 300)
  plot_bar_mid(100, 80, -10, 50)
  plot_bar(160, 80, 123, 300)
  plot_bar(220, 80, 67, 300)

  plot_scale(80, 290, 300)
  plot_needle(80, 290, 167, 300)

  plot_scale_mid(300, 290, 50)
  for i = -20 to 20
    plot_needle_mid(300, 290, i, 50, 2)
    pause 50
  next i

Touch Create 9,350,220,"EXIT",green,B

do
	if TouchVal(9) then
		Touch Remove All
		run "AUTORUN.BAS"
  endif

loop
    
end
  
  '##########################################################################

Sub plot_scale (center_x, center_y, max_s)
  ' Analog-Zeigerinstrument, Aufhängung der Nadel = Center
  Line (center_x - 84, center_y - 120) - (center_x + 84, center_y + 20), Blue, B
  Line (center_x - 83, center_y - 119) - (center_x + 83, center_y + 19), White, BF
  Font #1
  Color Black
  For i= 0 To 20
    rads=(i-10)*Pi/40
    x2=Sin(rads)*100 + center_x
    y2=-Cos(rads)*100 + center_y
    If i Mod 4 <> 0 Then
      x1=Sin(rads)*95 + center_x
      y1=-Cos(rads)*95 + center_y
    Else
      x1=Sin(rads)*90 + center_x
      y1=-Cos(rads)*90 + center_y
      Print @(x2-12,y2-14,1) (max_s*i/20)
    EndIf
    Line(x1,y1)-(x2,y2),Blue
  Next i
  Font #1
  Color Yellow
  Print @(center_x-84,center_y+25) ParamName$
End Sub
  
Sub plot_needle (center_x, center_y, needle_v, max_s, needle_idx)
  ' Analog-Zeigerinstrument, Aufhngung der Nadel = Center
  Local my_needle
  my_needle= needle_v * 100 / max_s
  ' needle value 0..100
  If my_needle <> needle_arr(needle_idx) Then
    Circle(center_x, center_y), 8, Blue
    rads= (needle_arr(needle_idx)-50)*Pi/200
    x2=Sin(rads)*90 + center_x
    y2=-Cos(rads)*90 + center_y
    Line(center_x,center_y)-(x2,y2),White
    
    rads= (my_needle-50)*Pi/200
    x2=Sin(rads)*90 + center_x
    y2=-Cos(rads)*90 + center_y
    Line(center_x,center_y)-(x2,y2),Red
    Color Red, Black
    Font #2
    Print @(center_x+15,center_y-9) Format$(needle_v, "%5.1f")
  EndIf
  needle_arr(needle_idx) = my_needle
End Sub
  
Sub plot_scale_mid (center_x, center_y, max_s)
  ' Analog-Zeigerinstrument, Aufhngung der Nadel = Center
  Line (center_x - 84, center_y - 120) - (center_x + 84, center_y + 20), Blue, B
  Line (center_x - 83, center_y - 119) - (center_x + 83, center_y + 19), White, BF
  Font #1
  Color Black
  For i= -10 To 10
    rads=(i)*Pi/40
    x2=Sin(rads)*100 + center_x
    y2=-Cos(rads)*100 + center_y
    If i Mod 4 <> 0 Then
      x1=Sin(rads)*95 + center_x
      y1=-Cos(rads)*95 + center_y
    Else
      x1=Sin(rads)*90 + center_x
      y1=-Cos(rads)*90 + center_y
      Print @(x2-10,y2-14,1) (max_s*i/10)
    EndIf
    Line(x1,y1)-(x2,y2),Blue
  Next i
  Font #1
  Color Yellow
  Print @(center_x-84,center_y+25) ParamName$
End Sub
  
Sub plot_needle_mid (center_x, center_y, needle_v, max_s, needle_idx)
  ' Analog-Zeigerinstrument, Aufhngung der Nadel = Center
  Local my_needle
  my_needle= needle_v * 50 / max_s
  ' needle value 0..100
  If my_needle <> needle_arr(needle_idx) Then
    Circle(center_x, center_y), 8, Blue
    rads= (needle_arr(needle_idx))*Pi/200
    x2=Sin(rads)*90 + center_x
    y2=-Cos(rads)*90 + center_y
    Line(center_x,center_y)-(x2,y2),White
    
    rads= (my_needle)*Pi/200
    x2=Sin(rads)*90 + center_x
    y2=-Cos(rads)*90 + center_y
    Line(center_x,center_y)-(x2,y2),Red
    Color Red, Black
    Font #2
    Print @(center_x+15,center_y-9) Format$(needle_v, "%5.1f")
  EndIf
  needle_arr(needle_idx) = my_needle
End Sub
  
  
Sub plot_grid (center_x, center_y, max_s)
  Font #1
  Color Blue
  Print @(center_x-78,center_y+40) 0
  Print @(center_x-78,center_y-10) max_s/2
  Print @(center_x-78,center_y-60) max_s
  Line (center_x - 84, center_y - 70) - (center_x + 84, center_y + 70), Blue, B
  For i= -2 To 2
    Line(center_x - 80, center_y + (i*25))-(center_x + 80, center_y + (i*25)), Blue
  Next i
  For i= -3 To 3
    Line(center_x + (i*25), center_y - 67)-(center_x + (i*25), center_y + 67), Blue
  Next i
  Font #1
  Color Yellow
  Print @(center_x-84,center_y+77) ParamName$
End Sub
  
Sub plot_graph (center_x, center_y, graph_v, graph_max)
  local my_point
  Font #1
  my_point= graph_v * 100 / graph_max
  
  BLIT center_x - 82, center_y - 50, center_x -83, center_y - 50, 167, 101, g
  Pixel(center_x + 83, center_y + 50 - my_point) = Green
End Sub
  
  
  '##########################################################################
   
Sub plot_bar (center_x, center_y, b_val, max_s)
  local my_val
  Font #1
  Color green
  my_val= b_val * 100 / max_s
  Print @(center_x+14,center_y+42) 0
  Print @(center_x+14,center_y+20) max_s/4
  Print @(center_x+14,center_y-4) max_s/2
  Print @(center_x+14,center_y-28) max_s*3/4
  Print @(center_x+14,center_y-50) max_s
  Line (center_x - 12, center_y - 52) - (center_x + 12, center_y + 52), Blue, B
  Font #1
  Line (center_x - 10, center_y + 50 - my_val) - (center_x + 10, center_y + 50), yellow, BF
  Line (center_x - 10, center_y + 49 - my_val) - (center_x + 10, center_y - 50), black, BF
  Color red, white
  Print @(center_x-10,center_y+54) ParamName$
  Print @(center_x-10,center_y+66) Format$(b_val, "%05.1f")
  Color white
End Sub

Sub plot_bar_mid (center_x, center_y, b_val, max_s)
  local my_val
  Font #1
  Color green
  my_val= b_val * 50 / max_s
  Print @(center_x+14,center_y+42) (-max_s)
  Print @(center_x+14,center_y+20) (-max_s/2)
  Print @(center_x+14,center_y-4) 0
  Print @(center_x+14,center_y-28) max_s/2
  Print @(center_x+14,center_y-50) max_s
  Line (center_x - 12, center_y - 52) - (center_x + 12, center_y + 52), Blue, B
  Font #1
  Line (center_x - 10, center_y - 49) - (center_x + 10, center_y +49), black, BF
  Line (center_x - 10, center_y - my_val-1) - (center_x + 10, center_y - my_val), yellow, B
  Color red, white
  Print @(center_x-10,center_y+54) ParamName$
  Print @(center_x-10,center_y+66) Format$(b_val, "%05.1f")
  Color white
End Sub
