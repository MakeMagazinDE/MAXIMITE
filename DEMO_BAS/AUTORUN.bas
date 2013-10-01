' AUTORUN uses file selection utility by Carsten Meyer, cm@ct.de 8/2013
option usb off
FileSelect("*.bas", 0)	' open a .bas file

Mode 3:	Cls: 	Font #1: Color white, black
touch remove all
if filename$="Cancel" then
	run
endif
run filename$


sub FileSelect(FileMask$, saveBtn)
' file selection utility by Carsten Meyer, cm@ct.de 8/2013
' returns filename$ (is "NewFile" if NEW selected and saveBtn=1)
	local i, x, x_old, y, y_old, firststart
	dim file_arr$(20)
	settick 0, 0	' disable Interrupt
	file_arr$(0)= dir$(FileMask$,FILE)	'change file extension to your needs
	for i = 1 to 20
		file_arr$(i)=dir$()
	next i
	cls:font #2:color white, black:locate 0,0
	for i = 10 to 19
		print tab(20) format$(i, "%02.0f") ": " file_arr$(i)
	next i
	locate 0,0
	for i = 0 to 9
		print format$(i, "%02.0f") ": " file_arr$(i)
	next i
	font #1:Color green, black
	print @(0,250) "touch file to select..."
	touch release
	touch remove all
	touch size 462,185	'set dummy button size
	touch create 9,5,5,"",0,n,d	' None (dummy button), disable automatic touch handling
	touch size 100,35	'set new bigger button size
	if saveBtn then
		touch create 2,150,220,"NEW",red,B
		touch create 1,260,220,"SAVE",green,B
		touch create 0,370,220,"CANCEL",cyan,B
	else
		touch create 1,260,220,"OPEN",green,B
		touch create 0,370,220,"CANCEL",cyan,B
	endif
	line(0,0)-(239,19),-1,bf
	filenum = 0 : x_old = 0 : y_old = 0 : firststart = 1
	do
		'touch Check	' handle all buttons/switch events
		font #2
		colour green, black
		print @(0,220) time$
		if touched(9) then ' dummy button
			x = touched(#X) \ 240	' recent coordinates, integer division
			y = touched(#Y) \ 20
			if y < 10 then
				x = x * 10
				if x <> x_old or y <> y_old or firststart then
					' select file by touch, invert rect
					line(x_old*24,y_old*20)-(x_old*24+239,y_old*20+19),-1,bf
					font #2
					line(x*24,y*20)-(x*24+239,y*20+19),-1,bf
					if  x+y <> filenum then
						touch Beep (1)
					endif
					filenum = x+y
					firststart = 0
				endif
			endif
			x_old = x : y_old = y
		endif
		if touchval(0) then
			filename$="Cancel"
      erase file_arr$
      touch remove all
		  exit sub
		endif
		if touchval(1) then
			' return a filename
			filename$=file_arr$(filenum)
			if filename$<>"" then
        erase file_arr$
        touch remove all
			  exit sub
			endif
		endif
		if touchval(2) then	'might be -1 for not initialised
			' New, return filename "NewFile"
			filename$="NewFile"
      erase file_arr$
      touch remove all
		  exit sub
		endif
		pause 50	' we have some time left over, may be used for other things
	loop
end sub

end
