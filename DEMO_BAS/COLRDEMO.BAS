''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
' Demonstration of the colour modes in Colour MMBasic
' Geoff Graham  May 2012
''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

Data "Black ", " Blue ", "Green ", " Cyan "
Data " Red  ", "Purple", "Yellow", "White "
Data Red, Yellow, Green, Red, Blue, Purple,    Red, Cyan, White
Data Green, Cyan, Blue,  Green, Purple, White, Yellow, Blue, White
Dim c$(8)
Dim p(9, 2)
For i = 0 To 7: Read c$(i) : Next
For i = 1 To 6: For j = 0 To 2 : Read p(i, j)  : Next i, j

Option usb off
Cls
mspc = 15
Mode 3
Colour 7
Font #1
''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Print "Monochrome Mode";
Line (0, MM.VPos+10)-(MM.HPos-2, MM.VPos+10), 7
Locate 0, MM.VPos + 5
Print "Any one colour can be selected for all output:"
Locate 0, MM.VPos + 5
For i = Blue To White
  Colour i
  Print " MODE 1," Str$(i) "  ";
Next
Print
For i = Blue To White
  Colour i
  Print "  " c$(i) "   ";
Next

'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Print @(0, MM.VPos + mspc) "Four colour mode";
Line (0, MM.VPos+10)-(MM.HPos-2, MM.VPos+10), 7
Locate 0, MM.VPos + 5
Print "Six colour palettes to chose from."
Print "Each palette consists of three colours plus black:" ;
For i = 1 To 6
  Print @(20, MM.VPos + 17) "Palette" i "  MODE 2," Str$(i) "   ";
  For j = 0 To 2
    Colour Black, p(i, j)
    Print "      ";
    Colour White, Black
    Print " ";
  Next
  Print "   (";
  For j = 0 To 2
    Colour p(i, j)
    Print " " c$(p(i, j)) " ";
  Next
  Colour White
  Print ")";
Next

''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Print @(0, MM.VPos + mspc) "Eight colour mode";
Line (0, MM.VPos+10)-(MM.HPos-2, MM.VPos+10), 7
Locate 0, MM.VPos + 5
Print "MODE 3    All colours can be used simultaneously"
line3 = MM.VPos
height3 = 40
For i = 0 To MM.HRes
  If (i \ (MM.HRes\8)) + 1 > 7 Then Exit For
  Line (i, line3)-(i + height3, line3 + height3), (i \ (MM.HRes\8)) + 1
Next


Touch Size 120,40	'set new bigger button size
Touch Create 0,350,220,"EXIT",red,B

do
	'Touch Check	' handle all buttons/switch events
	If TouchVal(0) then
		Touch Remove 0	' make touch items invalid, don't redraw
		cls black
		colour white, black
		font #2
		print: print "Good Bye"
		font #1
		Pause 1000
		run "autorun.bas"
	endif
	Pause 100	' we have some time left over for debounce, may be used for other things
loop
