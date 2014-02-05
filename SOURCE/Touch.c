/***********************************************************************************************************************
MMBasic

Touch.c

25-Jan-2014 added TOUCH DISABLE() and TOUCH ENABLE() commands
04-Sep-2013 added TOUCHED() function, revised touch detection and getXY
            moved TOUCHVAL(#X) etc. to TOUCHED(#X) etc.
29-Aug-2013 added hor and vert slider as touch items -cm
21-Aug-2013 some minor changes, look for -cm

Handles all the touch related commands and functions for the TFT Maximite version of MMBasic.

Note that this file is only used in building the firmware for the TFT Maximite which has a touch sensitive TFT LCD.
The TFT Maximite design, this file and MMBasic additions were created by Carsten Meyer (cm@ct.de) and was described 
in the German magazine c't in late 2013.

************************************************************************************************************************/

#include <stdio.h>
#include <p32xxxx.h>								// device specific defines
#include <plib.h>									// peripheral libraries

#include "MMBasic_Includes.h"
#include "Hardware_Includes.h"


void SelectChannel(int colour, int *con, int *coff);
void getcoord(char *p, int *x, int *y);

// List of 32 touch items
int item_active[MAX_NBR_OF_BTNS],item_touched[MAX_NBR_OF_BTNS],item_value[MAX_NBR_OF_BTNS], item_type[MAX_NBR_OF_BTNS],item_colour[MAX_NBR_OF_BTNS],
	item_top[MAX_NBR_OF_BTNS],item_left[MAX_NBR_OF_BTNS],item_bottom[MAX_NBR_OF_BTNS],item_right[MAX_NBR_OF_BTNS],
	item_max[MAX_NBR_OF_BTNS],item_flags[MAX_NBR_OF_BTNS];
char item_text[MAX_NBR_OF_BTNS][15];

int	item_sizex = 80; 
int item_sizey = 25;
int	last_touchx = -1; // for saving ccords used by TouchVal(#X) etc. -cm
int	last_touchy = -1; 
int last_item_hit = -1; // last item touched/hit
int touch_active = 0;
int TouchTimer = 0;
char *OnTouchGOSUB = NULL;
int x_raw, y_raw;

// calibration constants for TFT -cm
int touch_ofs_x, touch_ofs_y, touch_max_x, touch_max_y, touch_scale_x, touch_scale_y;

// ######################################################################################
// functions for RTC NVRAM used by TFT Touchscreen constants -cm
// ######################################################################################

// write integer value to RTC's NVRAM, reg = 0..55
void WriteRTCram(int reg, int data) {
	int value, idx;
	idx = (reg * 2) + 8;
	value = (data & 0x00FF);		// LSB
	WriteRTC(idx,value);
	value = (data >> 8);			// MSB
	idx += 1;
	WriteRTC(idx,value);
}

// read integer value from RTC's NVRAM, reg = 0..55
int ReadRTCram(int reg) {
	int value, idx;
	idx = (reg * 2) + 8;
	value = ReadRTC(idx);
	idx += 1;
	value = value + (ReadRTC(idx) << 8);			// MSB
	return value;
}

//get touchscreen calibration constants -cm
void InitTouchLCD(int col, int pos) {
    int i;
    char tmp[MAXSTRLEN];
    
	i = ReadRTCram(4);
	if (i != 0xAA55) {					// NVRAM was never used before
		touch_ofs_x = TOUCH_OFS_X;  	// set up defaults
		touch_max_x = TOUCH_MAX_X;
		touch_ofs_y = TOUCH_OFS_Y;
		touch_max_y = TOUCH_MAX_Y;
		WriteRTCram(0,touch_ofs_x);
		WriteRTCram(1,touch_max_x);
		WriteRTCram(2,touch_ofs_y);
		WriteRTCram(3,touch_max_y);
		WriteRTCram(4,0xAA55);			// is now initialized
        CurrentFgColour = col;
        strcpy(tmp, "Touchscreen Not Calibrated - Using Defaults");
	} else {
		touch_ofs_x = ReadRTCram(0);
		touch_max_x = ReadRTCram(1);
		touch_ofs_y = ReadRTCram(2);
		touch_max_y = ReadRTCram(3);
        strcpy(tmp, "TFT & Touchscreen by Carsten Meyer, cm@ct.de");
	}
   	MMPosX = (480 - strlen(tmp)*6)/2; MMPosY = pos;
   	MMPrintString(tmp);
	touch_scale_x =	TOUCH_HRES*1000/(touch_max_x-touch_ofs_x);
	touch_scale_y =	TOUCH_VRES*1000/(touch_max_y-touch_ofs_y);
}

// ######################################################################################
// Touch interface - uses some non-portable port defines, also: 
// X- = A0 I/O MMBASIC Pin 35 = PIC32MX795 Pin 32 RB8/AN8
// X+ = A1 I/O MMBASIC Pin 36 = PIC32MX795 Pin 33 RB9/AN9
// Y- = A2 I/O MMBASIC Pin 37 = PIC32MX795 Pin 34 RB10/AN10
// Y+ = A3 I/O MMBASIC Pin 38 = PIC32MX795 Pin 35 RB11/AN11

// setup the analog (ADC) function in main.c initEXTIO(), scheme 17.5.10 clocked conversion trigger
//	AD1CON1 = 0x00E0;       										// SSRC=111, automatic conversion after sampling
// 	AD1CSSL = 0;       												// no scanning required
//	AD1CON2 = 0;       												// use MUXA, use AVdd   &   AVss as Vref+/-
//	AD1CON3 = 0x1F3F;  												// 0001 1111 0011 1111, Tsamp = 32 x Tad;
//	AD1CON1bits.ADON = 1; 											// turn on the ADC
// ######################################################################################


int DetectTouch(void) {
// also sets all rails as inputs on exit. Returns true if touched
	int value;
	LATBCLR = 0x0F00; 	                                            // all low - IMPORTANT, otherwise switching port dir takes >2 ms!
	ODCBCLR = 0x0F00; 	                                            // all NOT open drain
	TRISBSET = 0x0F00;	                                            // all inputs RB8..RB11 = 0x0F00

	TRISBbits.TRISB10 = P_OUTPUT; 	                                // reset value on Y rails cap
	LATBCLR = (1 << 10); 			                                // reset value on Y rails cap
	uSec(5);							                            // 5 us cap discharge
	TRISBbits.TRISB10 = P_INPUT;		                            // cap keeps pin low until read if not touched

                                                                    // set X plane high to detect touch
	TRISBbits.TRISB8 = P_OUTPUT; 	                                // output on X rails
	TRISBbits.TRISB9 = P_OUTPUT;
	LATBSET = (1 << 8);			                                    // set to Ref Voltage
	LATBSET = (1 << 9);			                                    // set to Ref Voltage

	uSec(10);							                            // RC settling delay
	value = PORTBbits.RB10;	                                        // return 1 if touched

	LATBCLR = 0x0F00; 	                                            // all low - IMPORTANT, otherwise switching port dir takes >2 ms!
	TRISBSET = 0x0F00;	                                            // all inputs
	return value;
}


int GetTouchX(void) {
// For analog returns the reading as a 10 bit number with 0b1111111111 = 3.3V
	int value = 0;

	LATBCLR = 0x0C00; 	                                            // Y low - IMPORTANT, otherwise switching port dir takes >2 ms!
	TRISBCLR = 0x0C00;	                                            // Prepare switching, Y rails become outputs
	uSec(5);								                        // settling delay
	TRISBSET = 0x0C00;	                                            // Y rails become inputs
   	AD1CHSbits.CH0SA = 10;
	AD1PCFGCLR = (1 << 10);	                                        // enable analog function

	TRISBCLR = 0x0300;	                                            // X rails outputs
	LATBSET = (1 << 9);	 			                                // set X+ line to Ref 3V3
	uSec(25);								                        // settling delay
                                                                    // read analog voltage from Y- pin
	AD1CON1SET = 0x0002;       				                        // start conversion
	while (!AD1CON1bits.DONE && !MMAbort);                          // wait conversion complete
	value = ADC1BUF0;						                        // and get the result
	AD1PCFGSET = (1 << 10);	            // disable analog function
	LATBCLR = 0x0F00; 	                                            // all low - IMPORTANT, otherwise switching port dir takes >2 ms!
	TRISBSET = 0x0F00;	                                            // all inputs
	return value;
}


int GetTouchY(void) {
// For analog returns the reading as a 10 bit number with 0b1111111111 = 3.3V
	int value = 0;

	LATBCLR = 0x0300; 	                                            // X low - IMPORTANT, otherwise switching port dir takes >2 ms!
	TRISBCLR = 0x0300;	                                            // Prepare switching, X rails become outputs
	uSec(5);								                        // settling delay
	TRISBSET = 0x0300;	                                            // X rails become inputs
   	AD1CHSbits.CH0SA = 8;
	AD1PCFGCLR = (1 << 8);	                                        // enable analog function

	TRISBCLR = 0x0C00;	                                            // Y rails outputs
	LATBSET = (1 << 11);	 			                            // set Y+ line to Ref 3V3
	uSec(25);								                        // settling delay
                                                                    // read analog voltage from X- pin
	AD1CON1SET = 0x0002;       				                        // start conversion
	while (!AD1CON1bits.DONE && !MMAbort);                          // wait conversion complete
	value = ADC1BUF0;						                        // and get the result
	AD1PCFGSET = (1 << 8);	                                        // disable analog function
	LATBCLR = 0x0F00; 	                                            // all low - IMPORTANT, otherwise switching port dir takes >2 ms!
	TRISBSET = 0x0F00;	                                            // all inputs
	return value;
}

int getXY(void) {
// returns true with new x_raw, last_touchx, y_raw, last_touchy values if touch sensed
// returns false if no touch sensed
	int x, y, x_temp, y_temp, i;
	long temp;

	if (!DetectTouch()) return false;		// return "no touch"

	x_temp = 0;
	y_temp = 0;
    for(i = 0; i < 4; i++) {				// integrate 4 values
		x_temp = x_temp + GetTouchX();
		y_temp = y_temp + GetTouchY();
		if (!DetectTouch()) return false;	// premature end of touch, exit
	}	    

	x_temp = x_temp / 4;
	y_temp = y_temp / 4;

// small hysteresis
	if (x_temp > x_raw + 2) x_raw = x_temp;
	if (x_temp < x_raw - 2) x_raw = x_temp;
	if (y_temp > y_raw + 2) y_raw = y_temp;
	if (y_temp < y_raw - 2) y_raw = y_temp;

	x = x_raw - touch_ofs_x;			// offset X
	if (x < 0) x = 0;
	if (x > touch_max_x) x = touch_max_x;
	temp = ((long)x * touch_scale_x) / 1000;
	last_touchx = (int)temp;

	y = y_raw - touch_ofs_y;			// offset Y
	if (y < 0) y = 0;
	if (y > touch_max_y) y = touch_max_y;
	temp = ((long)y * touch_scale_y) / 1000;
	last_touchy = (int)temp;			// save last touch coord -cm
	return true;
}

// ######################################################################################
// graphic primitives: button, slider etc.
// ######################################################################################

void fillbox(int x1, int y1, int x2, int y2, int colour) {
// simply fill a box, faster than MMbox; assumes x2 > x1 and y2 > y1
	int coff, con, x, y;
	if ((x2 < x1) || (y2 < y1)) return;
	SelectChannel(colour, &con, &coff);
	for(y = y1; y<=y2; ++y) {
		for(x = x1; x<=x2; ++x) plotx(x, y, con, coff);	// Draw pixels to fill the rectangle
	}
}

void fillboxdither(int x1, int y1, int x2, int y2, int colour1, int colour2) {
// simply fill a box with dithered colour; faster than MMbox; assumes x2>x1 and y2>y1
	int coff1, con1, coff2, con2, x, y;
	if ((x2 < x1) || (y2 < y1)) return;
	SelectChannel(colour1, &con1, &coff1);
	SelectChannel(colour2, &con2, &coff2);
	for(y = y1; y<=y2; ++y) {
		for(x = x1; x<=x2; ++x){				// Draw pixels to fill the rectangle
			if((x ^ y) & 1)  {
				plotx(x, y, con1, coff1);
			}
			else plotx(x, y, con2, coff2);		// every second pixel black
		}
	}
}

void greyboxdither(int x1, int y1, int x2, int y2, int colour1) {
// grey out a box by dithering to screen bg colour; assumes x2>x1 and y2>y1
	int coff1, con1, x, y;
	if ((x2 < x1) || (y2 < y1)) return;
	SelectChannel(colour1, &con1, &coff1);
	for(y = y1; y<=y2; ++y) {
		for(x = x1; x<=x2; ++x){				// Draw pixels to fill the rectangle
			if((x ^ y) & 1)  {
				plotx(x, y, con1, coff1);
			}
		}
	}
}

void rectBtn(int x1, int y1, int x2, int y2, int colour1, int colour2) {
// just draw a simple rect button with corners, nothing else
	fillboxdither(x1, y1, x2, y2, colour1, colour2);
	MMbox(x1, y1, x2, y2, 0, 7);	// draw a surrounding white box
	plot(x1, y1, DefaultBgColour);	// bg corners
	plot(x2, y1, DefaultBgColour);
	plot(x1, y2, DefaultBgColour);
	plot(x2, y2, DefaultBgColour);
	x1 += 1;
	x2 -= 1;
	y1 += 1;
	y2 -= 1;
	MMline(x1, y1, x2, y1, 7);		// draw a highlight line
	MMline(x1, y1, x1, y2, 7);		// draw a highlight line
	x1 += 1;
	y1 += 1;
	MMline(x1, y2, x2, y2, 0);		// draw a shadow line
	MMline(x2, y1, x2, y2, 0);		// draw a shadow line
}

void drawButton(int index) {
 	int i,x1,x2,y1,y2,colour, pressed;
	char *s;
	s = item_text[index];	
	x1=item_left[index];
	y1=item_top[index];
	x2=item_right[index];
    y2=item_bottom[index];
	colour=item_colour[index];
	pressed=item_value[index];
	
 	int mm_x, mm_y, slen, item_sizex_half, item_sizey_half, x1_insert, x2_insert, y1_insert, y2_insert;
	int colour1, colour2, fontNbr_save, fontScale_save;
	colour1 = colour & 7;
	colour2 = (colour >> 8);
	if (!colour2) colour2 = colour1;	// if no secondary colour specified, use main colour
	colour2 = colour2 & 7;

	fontNbr_save = fontNbr;
	fontScale_save =fontScale;

	item_sizex_half = (x2-x1)/2; 
	item_sizey_half = (y2-y1)/2;
	x1_insert = x1+1;
	y1_insert = y1+1;
	x2_insert = x2-1;
	y2_insert = y2-1;
	MMbox(x1, y1, x2, y2, 0, 7);						            // draw a surrounding box
	plot(x1, y1, DefaultBgColour);							        // corners
	plot(x2, y1, DefaultBgColour);
	plot(x1, y2, DefaultBgColour);
	plot(x2, y2, DefaultBgColour);
 
	if (pressed){
		MMline(x1_insert, y1_insert, x2_insert, y1_insert, 0);		// draw a insert line
		MMline(x1_insert, y1_insert, x1_insert, y2_insert, 0);		// draw a insert line
		MMline(x1+2, y2_insert, x2_insert, y2_insert, colour1);		// draw a insert line
		MMline(x2_insert, y1+2, x2_insert, y2_insert, colour1);		// draw a insert line
		fillbox(x1+2, y1+2, x2-2, y2-2, 0);						    // draw a insert coloured box
	}	else {
		MMline(x1_insert, y1_insert, x2_insert, y1_insert, 7);		// draw a insert line
		MMline(x1_insert, y1_insert, x1_insert, y2_insert, 7);		// draw a insert line
		MMline(x1+2, y2_insert, x2_insert, y2_insert, 0);			// draw a insert line
		MMline(x2_insert, y1+2, x2_insert, y2_insert, 0);			// draw a insert line
		fillboxdither(x1+2, y1+2, x2-2, y2-2, colour1, colour2); // draw a insert coloured box
	}
	slen = strlen(s);
	mm_x = MMPosX;
	mm_y = MMPosY;

	SetFont(1, 1, 0);
 	MMPosX = x1+item_sizex_half-(5*slen);	                        // center caption
	if (!(slen & 1)) MMPosX -= 3;
	MMPosY = y1+item_sizey_half-8;

	if (pressed) {
		CurrentFgColour = colour1;
		CurrentBgColour = 0;
	} else {
		if (colour1 <= 1) 
			CurrentFgColour = 7;
		else
			CurrentFgColour = 0;
		CurrentBgColour = colour1;
	}
  for(i = *s++; i > 0; i--) VideoPutc(*s++);		                // print string (s is a MMBasic string)
	MMPosX = mm_x;
	MMPosY = mm_y;
	SetFont(fontNbr_save, fontScale_save, 0);
	CurrentFgColour = DefaultFgColour;
	CurrentBgColour = DefaultBgColour;
	if (pressed){
		MMbox(x1+3, y1+3, x2-2, y2-2, 0, colour1);				    // draw a insert coloured box
		} else {
		MMbox(x1+2, y1+2, x2-2, y2-2, 0, 7);				        // draw a insert coloured box
		plot(x1+2, y1+2, colour1);							        // corners
		plot(x2-2, y1+2, colour1);
		plot(x1+2, y2-2, colour1);
		plot(x2-2, y2-2, colour1);
	}
}

void drawPushButton(int index) {
  	int i,x1,x2,y1,y2,colour, pressed;
	char *s;
	s = item_text[index];	
	x1=item_left[index];
	y1=item_top[index];
	x2=item_right[index];
    y2=item_bottom[index];
	colour=item_colour[index];
	pressed=item_value[index];

 	int mm_x, mm_y, slen, item_sizex_half, item_sizey_half, x1_insert, x2_insert, y1_insert, y2_insert;
	int colour1, colour2, fontNbr_save, fontScale_save;
	colour1 = colour & 7;
	colour2 = (colour >> 8);
	if (!colour2) colour2 = colour1;	// if no secondary colour specified, use main colour
	colour2 = colour2 & 7;

	fontNbr_save = fontNbr;
	fontScale_save =fontScale;

	item_sizex_half = (x2-x1)/2; 
	item_sizey_half = (y2-y1)/2;
	x1_insert = x1+1;
	y1_insert = y1+1;
	x2_insert = x2-1;
	y2_insert = y2-1;
	MMbox(x1, y1, x2, y2, 0, 7);						            // draw a surrounding box
	plot(x1, y1, DefaultBgColour);							        // corners
	plot(x2, y1, DefaultBgColour);
	plot(x1, y2, DefaultBgColour);
	plot(x2, y2, DefaultBgColour);
 
	if (pressed){
		MMline(x1_insert, y1_insert, x2_insert, y1_insert, 0);		// draw a insert line
		MMline(x1_insert, y1_insert, x1_insert, y2_insert, 0);		// draw a insert line
		MMline(x1+2, y2_insert, x2_insert, y2_insert, 7);			// draw a insert line
		MMline(x2_insert, y1+2, x2_insert, y2_insert, 7);			// draw a insert line
		fillboxdither(x1+2, y1+2, x2-2, y2-2, colour1, colour2);	// draw a insert coloured box
	}	else {
		MMline(x1_insert, y1_insert, x2_insert, y1_insert, 7);		// draw a insert line
		MMline(x1_insert, y1_insert, x1_insert, y2_insert, 7);		// draw a insert line
		MMline(x1+2, y2_insert, x2_insert, y2_insert, 0);			// draw a insert line
		MMline(x2_insert, y1+2, x2_insert, y2_insert, 0);			// draw a insert line
		fillboxdither(x1+2, y1+2, x2-2, y2-2, colour1, 0); 			// draw a insert coloured box, dimmed
	}
	slen = strlen(s);
	mm_x = MMPosX;
	mm_y = MMPosY;

	SetFont(1, 1, 0);
 	MMPosX = x1+item_sizex_half-(5*slen);	                        // center caption
	if (!(slen & 1)) MMPosX -= 3;
	MMPosY = y1+item_sizey_half-8;
	if (pressed) {
		CurrentFgColour = 0;
		CurrentBgColour = 7;
		MMbox(x1+3, y1+3, x2-2, y2-2, 0, colour1);				    // draw a insert coloured box
	} else {
		MMbox(x1+2, y1+2, x2-2, y2-2, 0, 7);				        // draw a insert coloured box
		plot(x1+2, y1+2, colour1);							        // corners
		plot(x2-2, y1+2, colour1);
		plot(x1+2, y2-2, colour1);
		plot(x2-2, y2-2, colour1);
		if (colour1 == 0) colour1 = 7;
		CurrentFgColour = colour1;
		CurrentBgColour = 0;
	}
	for(i = *s++; i > 0; i--) VideoPutc(*s++);		                // print string (s is a MMBasic string)
	MMPosX = mm_x;
	MMPosY = mm_y;
	SetFont(fontNbr_save, fontScale_save, 0);
	CurrentFgColour = DefaultFgColour;
	CurrentBgColour = DefaultBgColour;
}

void drawRadioButton(int index) {
 	int i,x1,x2,y1,y2,colour, pressed;
	char *s;
	s = item_text[index];	
	x1=item_left[index];
	y1=item_top[index];
	x2=item_right[index];
    y2=item_bottom[index];
	colour=item_colour[index] & 7;
	pressed=item_value[index];

 	int  mm_x, mm_y, slen, item_size_third, item_size_half;
	int fontNbr_save, fontScale_save;
	fontNbr_save = fontNbr;
	fontScale_save =fontScale;

	item_size_half = (y2-y1)/2;
	item_size_third = (y2-y1)/3;

//	MMCircle(x, y, radius, fill, colour, aspect);
	MMCircle(x1+item_size_half, y1+item_size_half, item_size_half, 1, colour, 1);				// draw a filled white circle
 	MMCircle(x1+item_size_half, y1+item_size_half, item_size_third, (pressed && 1), 0, 1);		// draw a black filled circle

	slen = strlen(s);
	mm_x = MMPosX;
	mm_y = MMPosY;

	SetFont(1, 1, 0);
 	MMPosX = x1+item_size_half+item_size_half+6;	                // align caption
	MMPosY = y1+item_size_half-8;					                // Text height center

	for(i = *s++; i > 0; i--) VideoPutc(*s++);		                // print string (s is a MMBasic string)
	MMPosX = mm_x;
	MMPosY = mm_y;
	SetFont(fontNbr_save, fontScale_save, 0);
}

void drawLedButton(int index) {
 	int i,x1,x2,y1,y2,colour, pressed;
	char *s;
	s = item_text[index];	
	x1=item_left[index];
	y1=item_top[index];
	x2=item_right[index];
    y2=item_bottom[index];
	colour=item_colour[index] & 7;
	pressed=item_value[index];

 	int  mm_x, mm_y, slen,  item_size, item_size_third, item_size_half;
	int fontNbr_save, fontScale_save;
	fontNbr_save = fontNbr;
	fontScale_save =fontScale;
	item_size = (y2-y1);
	item_size_half = item_size/2;
	item_size_third = item_size/3;

	slen = strlen(s);
	mm_x = MMPosX;
	mm_y = MMPosY;
	SetFont(1, 1, 0);
 	MMPosX = x1+item_size+8;											// align caption
	MMPosY = y1+item_size_half-8;					                // Text height center
	for(i = *s++; i > 0; i--) VideoPutc(*s++);		                // print string (s is a MMBasic string)
	MMPosX = mm_x;
	MMPosY = mm_y;
	SetFont(fontNbr_save, fontScale_save, 0);
//	MMCircle(x, y, radius, fill, colour, aspect);

	x1+=item_size_half;
	y1+=item_size_half;
	MMCircle(x1, y1, item_size_half, 0, 7, 1);						// draw a surrounding circle
	MMCircle(x1, y1, item_size_half-1, 0, 7, 1);					// draw a surrounding circle
 	if(pressed) {
    		MMCircle(x1, y1, item_size_half-3, 1, colour, 1);		// draw insert filled circle
		    MMCircle(x1, y1, item_size_third-2, 1, 7, 1);			// shadow circle
		    MMCircle(x1+1, y1+1, item_size_third-2, 1, colour, 1);
    } else {
    		MMCircle(x1, y1, item_size_half-4, 1, 0, 1);			// erase insert circle
    		MMCircle(x1, y1, item_size_half-3, 0, colour, 1);		// draw insert circle
		    MMCircle(x1, y1, item_size_third-2, 1, colour, 1);		// shadow circle
		    MMCircle(x1+1, y1+1, item_size_third-2, 1, 0, 1);
    }
}

void drawCheckbox(int index) {
 	int i,x1,x2,y1,y2, colour, pressed;
	char *s;
	s = item_text[index];	
	x1=item_left[index];
	y1=item_top[index];
	x2=item_right[index];
    y2=item_bottom[index];
	colour=item_colour[index] & 7;
	pressed=item_value[index];

 	int  mm_x, mm_y, slen, item_size, item_size_half;
	int fontNbr_save, fontScale_save;
	fontNbr_save = fontNbr;
	fontScale_save =fontScale;

	item_size = y2-y1;
	item_size_half = item_size/2;

	fillbox(x1, y1, x1+item_size, y2, colour);	// draw a filled colour box
	MMbox(x1+1, y1+1, x1+item_size-1, y2-1, 0, 0);  // draw a insert black box
 	if (pressed) { // draw V
		MMline(x1+4, y1+item_size_half, x1+item_size_half, y2-4, 0);
		MMline(x1+item_size_half, y2-4, x1+item_size-4, y1+4, 0);
        // double line for better visability
		MMline(x1+5, y1+item_size_half, x1+item_size_half+1, y2-4, 0);
		MMline(x1+item_size_half+1, y2-4, x1+item_size-3, y1+4, 0);
	}

	slen = strlen(s);
	mm_x = MMPosX;
	mm_y = MMPosY;

	SetFont(1, 1, 0);
 	MMPosX = x1+item_size+6;	                                    // align caption
	MMPosY = y1+item_size_half-8;					                // Text height center

	for(i = *s++; i > 0; i--) VideoPutc(*s++);		                // print string (s is a MMBasic string)
	MMPosX = mm_x;
	MMPosY = mm_y;
	SetFont(fontNbr_save, fontScale_save, 0);
	CurrentFgColour = DefaultFgColour;
	CurrentBgColour = DefaultBgColour;
}


void drawSwitch(int index) {
 	int x1,x2,y1,y2,colour, pressed;
//	char *s;
//	s = item_text[index];	
	x1=item_left[index];
	y1=item_top[index];
	x2=item_right[index];
    y2=item_bottom[index];
	colour=item_colour[index];
	pressed=item_value[index];

 	int  mm_x, mm_y, item_sizex_half, item_sizey_half, x1_insert, x2_insert, y1_insert, y2_insert;
	int colour1, colour2, fontNbr_save, fontScale_save;
	colour1 = colour & 7;
	colour2 = (colour >> 8);
	if (!colour2) colour2 = colour1;	// if no secondary colour specified, use main colour
	colour2 = colour2 & 7;

	fontNbr_save = fontNbr;
	fontScale_save =fontScale;

	x1_insert = x1+1;
	y1_insert = y1+1;
	x2_insert = x2-1;
	y2_insert = y2-1;
 
	MMbox(x1, y1, x2, y2, 0, 7);						            // draw a surrounding box
	plot(x1, y1, DefaultBgColour);							        // corners
	plot(x2, y1, DefaultBgColour);
	plot(x1, y2, DefaultBgColour);
	plot(x2, y2, DefaultBgColour);

	fillboxdither(x1_insert, y1_insert, x2_insert, y2_insert, colour1, colour2);	// draw insert box
	mm_x = MMPosX;
	mm_y = MMPosY;
	SetFont(1, 1, 0);
	item_sizex_half = (x2-x1)/2; 
	item_sizey_half = (y2-y1)/2; 
	if (pressed){
		fillbox(x1+item_sizex_half-1, y1+2, x2-2, y2-2, 7);		// draw a white box
		CurrentFgColour = 0;
		CurrentBgColour = 7;
		MMPosX = x1+item_sizex_half+(item_sizey_half/2)+2;
		MMPosY = y1+item_sizey_half-8;
		DisplayString("ON");				// print string
		MMline(x1+item_sizex_half, y2_insert, x2_insert, y2_insert, DefaultBgColour);	// draw shadow line
		MMline(x2_insert, y1+2, x2_insert, y2_insert, DefaultBgColour);		            // draw shadow line
	} else {
		if (colour1 <= 1) 
			CurrentFgColour = 7;	// white lettering if btn is black or blue for readability
		else 
			CurrentFgColour = 0;
		CurrentBgColour = colour1;
	  	MMPosX = x1+(item_sizey_half/2)-3;			   // center caption
		MMPosY = y1+item_sizey_half-8;
		DisplayString("OFF");				// print string
		MMbox(x1+2, y1+2, x1+item_sizex_half+2, y2-2, 0, 7);					             // draw a unfilled white box
		MMline(x1+3, y2_insert, x1+item_sizex_half+3, y2_insert, DefaultBgColour);		     // draw shadow line
		MMline(x1+item_sizex_half+3, y1+2, x1+item_sizex_half+3, y2_insert, DefaultBgColour);// draw shadow line
	}
    MMPosX = mm_x;
    MMPosY = mm_y;
	SetFont(fontNbr_save, fontScale_save, 0);
	CurrentFgColour = DefaultFgColour;
	CurrentBgColour = DefaultBgColour;
}


int updateHorSlider(int index, int x_new) {
// draws/updates horizontal slider with fixed dimensions and returns input value within limits
// colour bit 3 (+8): fill left scroll area, colour bit 4 (+16): fill right scroll area
// colour bit 5 (+32: draw line instead of button

 	int x1,x2,y1,y2,colour,value, x1_insert, x2_insert, y1_insert, y2_insert, x_btnsize, x_knobhalf, 
		colour1, colour2, x_max, flags;
	x1=item_left[index];
	y1=item_top[index];
	x2=item_right[index];
    y2=item_bottom[index];
	value=item_value[index];
	x_max=item_max[index];
	flags=item_flags[index];
	colour=item_colour[index];
	colour1 = colour & 7;
	colour2 = (colour >> 8);
	if (!colour2) colour2 = colour1;	// if no secondary colour specified, use main colour
	colour2 = colour2 & 7;

	MMbox(x1, y1, x2, y2, 0, 7);		// draw a white surrounding box
	x1_insert = x1+1;
	y1_insert = y1+1;
	x2_insert = x2-1;
	y2_insert = y2-1;

	if (flags & 32){
		x_btnsize = 1;					// slider knob X dimension = height Y
		x_knobhalf = 0;					// mid of slider knob
	} else {
		x_btnsize = ((y2 - y1)*3)/4;	// slider knob X dimension = height Y
		x_knobhalf = x_btnsize/2;		// mid of slider knob
	}

	if (x_new >= 0) value = (x_new - x_knobhalf) - x1;
	if (value < 0) value = 0;
	if (value > x_max) value = x_max;

	if (flags & 8)
		fillboxdither(x1_insert, y1_insert, x1_insert+value-1, y2_insert, colour1, 0);	// fill left scrolling area
	else
		fillbox(x1_insert, y1_insert, x1_insert+value-1, y2_insert, DefaultBgColour);			// clear left scrolling area
	x1_insert += value;
	if (flags & 16)
		fillboxdither(x1_insert+x_btnsize+1, y1_insert, x2_insert, y2_insert, colour1, 0);// fill right scrolling area
	else
		fillbox(x1_insert+x_btnsize+1, y1_insert, x2_insert, y2_insert, DefaultBgColour);		// clear right scrolling area

	if (flags & 32){
		fillbox(x1_insert, y1_insert, x1_insert+1, y2_insert, 7);								// just a bold line
	} else {
		x2_insert = x1_insert + x_btnsize;
		rectBtn(x1_insert, y1_insert, x2_insert, y2_insert,  colour1, colour2);
		x1_insert += 7;
		x2_insert -= 7;
		y1_insert += 5;
		y2_insert -= 5;	
		fillboxdither(x1_insert, y1_insert, x2_insert, y2_insert, colour, 7);			// knob insert
	}

	return value;

}


int updateVertSlider(int index, int y_new) {
// draws/updates vertical slider with fixed dimensions and returns input value within limits
// colour bit 3 (8): fill upper scroll area, colour bit 4 (16): fill lower scroll area
 
	int x1,x2,y1,y2, colour, value, x1_insert, x2_insert, y1_insert, y2_insert, y_btnsize, y_knobhalf, 
		colour1, colour2, y_max, flags;

	x1=item_left[index];
	y1=item_top[index];
	x2=item_right[index];
    y2=item_bottom[index];
	y_max=item_max[index];
	value=y_max-item_value[index];
	flags=item_flags[index];
	colour=item_colour[index];
	colour1 = colour & 7;
	colour2 = (colour >> 8);
	if (!colour2) colour2 = colour1;	// if no secondary colour specified, use main colour
	colour2 = colour2 & 7;

	if (flags & 32){
		y_btnsize = 1;		// slider knob X dimension = height Y
		y_knobhalf = 0;		// mid of slider knob
	} else {
		y_btnsize = ((x2 - x1)*3)/4;	// slider knob X dimension = height Y
		y_knobhalf = y_btnsize/2;		// mid of slider knob
	}
	MMbox(x1, y1, x2, y2, 0, 7);					            // draw a white surrounding box
	x1_insert = x1+1;
	y1_insert = y1+1;
	x2_insert = x2-1;
	y2_insert = y2-1;

	if (y_new >= 0) {
		value = (y_new - y_knobhalf) - y1;
	}
	if (value < 0) value = 0;
	if (value > y_max) value = y_max;
	if (flags & 8)
		fillboxdither(x1_insert, y1_insert, x2_insert, y1_insert+value-1, colour1, 0);	// fill upper scrolling area
	else
		fillbox(x1_insert, y1_insert, x2_insert, y1_insert+value-1, DefaultBgColour);			// clear upper scrolling area
	y1_insert += value;
	if (flags & 16)
		fillboxdither(x1_insert, y1_insert+y_btnsize+1, x2_insert, y2_insert, colour1, 0);// fill lower scrolling area
	else
		fillbox(x1_insert, y1_insert+y_btnsize+1, x2_insert, y2_insert, DefaultBgColour);		// clear lower scrolling area
	y2_insert = y1_insert + y_btnsize;

	if (flags & 32){
		fillbox(x1_insert, y1_insert, x2_insert, y1_insert+1, 7);								// just a bold line
	} else {
		rectBtn(x1_insert, y1_insert, x2_insert, y2_insert, colour1, colour2);
		x1_insert += 5;
		x2_insert -= 5;
		y1_insert += 7;
		y2_insert -= 7;
		fillboxdither(x1_insert, y1_insert, x2_insert, y2_insert, colour1, 7);			// knob insert
	}
	return y_max-value;

}



void drawItemIdx(int index) {
// draw touch item by index from 0 to 31
	int colour, colour1, colour2;
	if (item_active[index] == TOUCH_ITEM_INVALID) return;		                                            // was not inited/active
	switch(item_type[index]) {
		case TOUCH_TYPE_NONE:
			colour = item_colour[index];	// if colour specified, draw a filled rect with this colour
			if (colour) {
				colour1 = colour & 7;
				colour2 = colour >> 8;
				if (!colour2) {
					fillbox(item_left[index], item_top[index],item_right[index],item_bottom[index], colour1);
				} else {
					colour2 = colour2 & 7;
					fillboxdither(item_left[index], item_top[index],item_right[index],item_bottom[index], colour1, colour2);
				}
			}
			break;
		case TOUCH_TYPE_BUTTON:
			drawButton(index);
			break;
		case TOUCH_TYPE_SWITCH:
			drawSwitch(index);	
			break;
		case TOUCH_TYPE_RADIO:
			drawRadioButton(index);
			break;
		case TOUCH_TYPE_CHECK:
			drawCheckbox(index);
			break;
		case TOUCH_TYPE_PUSH:
			drawPushButton(index);
			break;
		case TOUCH_TYPE_LED:
			drawLedButton(index);
			break;
		case TOUCH_TYPE_HSLIDER:
			item_value[index] = updateHorSlider(index,-1); // -1 => do not check for X update
			break;
		case TOUCH_TYPE_VSLIDER:
			item_value[index] = updateVertSlider(index,-1); // -1 => do not check for Y update
			break;
	}
}


int checkItem(int x, int y, int index) {
// check if Item (idx) is touched - so x and y fall into item's coord scope
	if (item_active[index] < TOUCH_ITEM_ACTIVE) return 0;	// was not active
	if (x < item_left[index]-TOUCH_OVERLAP) return 0;		// not in bounds
	if (x > item_right[index]+TOUCH_OVERLAP) return 0;
	if (y < item_top[index]-TOUCH_OVERLAP) return 0;
	if (y > item_bottom[index]+TOUCH_OVERLAP) return 0;

	return 1;
}
	
void touch_index_error(void) {
	error("Index out of range (touch item)");
}

void touch_syntax_error(void) {
	error("Invalid syntax (touch)");
}

void touch_missing_error(void) {
	error("Missing value or parameter (touch)");
}


void cmd_touchval(void) {
// TouchItem(<itemnbr>) = 1 or 0 - set button/switch nbr on or off
int index, value;
	index = getinteger(cmdline);
    if(index < 0 || index > MAX_NBR_OF_BTNS) touch_index_error();

	while(*cmdline && tokenfunction(*cmdline) != op_equal) cmdline++;
	if(!*cmdline) touch_syntax_error();
	++cmdline;
	if(!*cmdline) touch_missing_error();
	value = getinteger(cmdline);
	item_value[index] = value;
	if (item_type[index] < TOUCH_TYPE_VSLIDER) {
		if(value) 
		    item_value[index] = 1;	// button/switch ON
		else 
		    item_value[index] = 0;
	}
	drawItemIdx(index);	
}


void fun_touched(void) {
// new by -cm, get current touch state; returns 1 if touch to this item happened	
	int index, value;
	char *p;

	if((p = checkstring(ep, "#X")) != NULL) {
		value = getXY();
		fret = (float)last_touchx;
		return;
	}
	if((p = checkstring(ep, "#Y")) != NULL) {
		value = getXY();
		fret = (float)last_touchy;
		return;
	}
	if((p = checkstring(ep, "#I")) != NULL) {
		fret = (float)last_item_hit;
		return;
	}
	if((p = checkstring(ep, "#S")) != NULL) {			// Screen touched?
		fret = (float)DetectTouch();
		return;
	}

	index = getinteger(ep);	                            // only one param
    if(index < 0 || index > MAX_NBR_OF_BTNS) touch_index_error();

	if (item_active[index] < TOUCH_ITEM_ACTIVE) {
		fret = 0;										// return 0 if not active
		return;
	}
	value = item_touched[index];			            // get value formerly set by CheckTouch
	item_touched[index] = 0;
	fret = (float)value;		
}

void fun_touchval(void) {
// new by -cm, simplified, get touch item value (0/1 or slider value), automatic button reset	
	int index, value;

	index = getinteger(ep);	                            // only one param
    if(index < 0 || index > MAX_NBR_OF_BTNS) touch_index_error();

	if (item_active[index] < TOUCH_ITEM_ACTIVE) {
		fret = 0;										// return 0 if not initialised
		return;
	}
	value = item_value[index];			                // get value formerly set by CheckTouch
	if (!(item_flags[index] & 128))						// if I'm a button, but not disabled
		if (item_type[index] == TOUCH_TYPE_BUTTON)
			if (value) {
				item_value[index] = 0;					// was read, is now OFF	
				drawItemIdx(index);						// redraw me
			}
	fret = (float)value;		
}

void ticknoise(int usec) {
// click noise on beeper
	ODCECLR = (1 << 4);
	TRISECLR = (1 << 4); 	                    
	LATESET = (1 << 4);			
	uSec(usec);
	LATECLR = (1 << 4);
}


int checktouch(void) {
// run one touchscreen cycle and check if button/switch hit
	int index, myitemtype, radio_hit, old_val;

	if (!touch_active)	return false;	// leave if not activated
	if (!getXY()) return false;			// leave if not touched
// now last_touchx and last_touchy hold the current touch coordinates

	radio_hit = 0;
	last_item_hit = -1;

// we now have the XY touch coordinates and can check if it falls in range of one item
    for(index = 0; index <= MAX_NBR_OF_BTNS; index++) {
		myitemtype = item_type[index];
		if (checkItem(last_touchx, last_touchy, index)) {	// have we found an item?
			last_item_hit = index;
			item_touched[index] = 1;						// mark touched even if disabled
			if (item_flags[index] & 128) return false;		// was disabled, do nothing else
			if (myitemtype < TOUCH_TYPE_VSLIDER) ticknoise(2500);
			switch(myitemtype) {
				case TOUCH_TYPE_RADIO:		            // is it a radio button?
					radio_hit = 1;						// mark as radio button to erase others
					break;								// other action done in radio btn redraw loop
				case TOUCH_TYPE_BUTTON:
					item_value[index] = 1;	            // is now ON	
					drawItemIdx(index);		            // redraw me
					while (DetectTouch()) CheckAbort(); // wait for touch released
					uSec(100000);    // "debounce" time to ensure that the touch has finished
					break;
				case TOUCH_TYPE_LED:					// is it a LED?
				case TOUCH_TYPE_SWITCH:		            // is it a switch?
				case TOUCH_TYPE_PUSH:		            // is it a pushbutton?
				case TOUCH_TYPE_CHECK:					// is it a check box?
					item_value[index] ^= 1;				// Toggle	
					drawItemIdx(index);		            // redraw me
					uSec(100000);    // "debounce" time to ensure that the touch has finished -cm
					while (DetectTouch()) CheckAbort();	// wait for touch released
					break;
				case TOUCH_TYPE_HSLIDER:
					old_val = item_value[index];
					item_value[index] = updateHorSlider(index, last_touchx);	// redraw me with new value
					if (old_val != item_value[index]) ticknoise(500);
					return false;
					break;
				case TOUCH_TYPE_VSLIDER:
					old_val = item_value[index];
					item_value[index] = updateVertSlider(index, last_touchy);	// redraw me with new value
					if (old_val != item_value[index]) ticknoise(500);
					return false;
					break;
				case TOUCH_TYPE_NONE:					// is it an empty box?
					item_value[index] = 1;	            // is now ON	
			}
		}
	}	
// set the radio btn touched and cancel all other radio buttons in redraw loop
	if (radio_hit) {
	    for(index = 0; index <= MAX_NBR_OF_BTNS; index++) {
			if (item_type[index] == TOUCH_TYPE_RADIO) {
				if (index == last_item_hit) 
					item_value[index] = 1;	                                        // only last one is now ON
				else
					item_value[index] = 0;	// shut off all other radio btns			
				drawItemIdx(index);			// redraw radio btns
			}
		}
		while (DetectTouch()) CheckAbort();                                     // wait for touch released
	}

	return true;
}

void remove_item(int index) {
// erase removed item to gackground color, make invalid
	if (item_active[index] && (item_type[index] != TOUCH_TYPE_NONE)) {
		fillbox(item_left[index], item_top[index], item_right[index], item_bottom[index], DefaultBgColour);
	}
	item_value[index] = 0;	// default OFF
	item_max[index] = 1;
	item_colour[index] = 0;
	item_touched[index] = 0;
	item_flags[index] = 0;
	item_active[index] = TOUCH_ITEM_INVALID;	// de-validize item		
}

void enable_item(int index) {
// redraw item
	if (item_active[index]) {	// > inactive (0)
		if (item_type[index] != TOUCH_TYPE_NONE) {
			drawItemIdx(index);
		}
		item_active[index] = TOUCH_ITEM_ACTIVE;		
	}
}

void disable_item(int index) {
// grey out item to make inactive state visible
	if (item_active[index]) {	// > inactive (0)
		if (item_type[index] != TOUCH_TYPE_NONE) {
			greyboxdither(item_left[index], item_top[index], item_right[index], item_bottom[index], DefaultBgColour);
		}
		item_active[index] = TOUCH_ITEM_INACTIVE;	// not active, but stays on screen greyed out		
	}
}

void cmd_touch(void) {
   	int i, x, y, c, temp, type, flags;
	char *s, *p;
  
	if((p = checkstring(cmdline, "CREATE")) != NULL) {
	    getargs(&p, 13, ",");
		if(argc < 8) touch_syntax_error();

    	i = getinteger(argv[0]);   	
	    x = getinteger(argv[2]);
	    y = getinteger(argv[4]);		

		c = getinteger(argv[8]);
	    
		if(i > MAX_NBR_OF_BTNS || i < 0) touch_index_error();
		s = "OK";
		if(*argv[6]) {
			s = getstring(argv[6]);			// the caption string
		}
		
		type = TOUCH_TYPE_NONE;				// default empty graphic
		remove_item(i);						// reset item vals
		item_active[i] = 0x55AA;				// validize item
		item_left[i] = x;
		item_right[i] = x+item_sizex;
		item_top[i] = y;
		item_bottom[i] = y+item_sizey;
		item_colour[i] = c;
		flags = 0;
		if(argc > 11) {
			if (strchr(argv[12], 't') != NULL || strchr(argv[12], 'T') != NULL) flags |= 8;		// fill Top side of slider knob
			if (strchr(argv[12], 'b') != NULL || strchr(argv[12], 'B') != NULL) flags |= 16;	// fill Bottom side of knob
			if (strchr(argv[12], 'l') != NULL || strchr(argv[12], 'L') != NULL) flags |= 8;		// fill Left side of knob
			if (strchr(argv[12], 'r') != NULL || strchr(argv[12], 'R') != NULL) flags |= 16;	// fill Right side of knob
			if (strchr(argv[12], 'n') != NULL || strchr(argv[12], 'N') != NULL) flags |= 32;	// No slider knob
			if (strchr(argv[12], 'd') != NULL || strchr(argv[12], 'D') != NULL) flags |= 128;	// Disable touch automatic handling
		}				
		if(argc >= 11) {
			if (strchr(argv[10], 'b') != NULL || strchr(argv[10], 'B') != NULL) type = TOUCH_TYPE_BUTTON;
			if (strchr(argv[10], 's') != NULL || strchr(argv[10], 'S') != NULL) type = TOUCH_TYPE_SWITCH;
			if (strchr(argv[10], 'r') != NULL || strchr(argv[10], 'R') != NULL) type = TOUCH_TYPE_RADIO;
			if (strchr(argv[10], 'c') != NULL || strchr(argv[10], 'C') != NULL) type = TOUCH_TYPE_CHECK;
			if (strchr(argv[10], 'h') != NULL || strchr(argv[10], 'H') != NULL) {
				type = TOUCH_TYPE_HSLIDER;
				item_max[i] = item_sizex;
				if (flags & 32){
					item_right[i] = x+item_sizex+3;									// adjust size to fit scroll area		
				} else {
					item_right[i] = x+item_sizex+(item_sizey*3/4)+2;				// adjust size to fit scroll area		
				}
			}
			if (strchr(argv[10], 'v') != NULL || strchr(argv[10], 'V') != NULL) {
				type = TOUCH_TYPE_VSLIDER;
				item_max[i] = item_sizey;
				if (flags & 32){
					item_bottom[i] = y+item_sizey+3;								// adjust size to fit scroll area		
				} else {
					item_bottom[i] = y+item_sizey+(item_sizex*3/4)+2;				// adjust size to fit scroll area		
				}
			}
			if (strchr(argv[10], 'l') != NULL || strchr(argv[10], 'L') != NULL) type = TOUCH_TYPE_LED;
			if (strchr(argv[10], 'p') != NULL || strchr(argv[10], 'P') != NULL) type = TOUCH_TYPE_PUSH;
		}

		item_flags[i] = flags;		
		item_type[i] = type;
		strncpy(item_text[i],s,14);
		drawItemIdx(i);
		touch_active = 1;
		return;
	}
	if((p = checkstring(cmdline, "REMOVE")) != NULL) {
    	int k;    
    	getargs(&p, (MAX_ARG_COUNT * 2) - 1, ",");		                        // getargs macro must be the first executable stmt in a block
        if(checkstring(argv[0], "ALL")) {
		    for(i = 0; i <= MAX_NBR_OF_BTNS; i++) remove_item(i);
			return;
		}		
       	for(i = 0; i < argc; i += 2) {
       		k = getinteger(argv[i]);
       	    if(k < 0 || k > MAX_NBR_OF_BTNS) touch_index_error();
			remove_item(k);
   		}
		touch_active = 0;	
	    for(i = 0; i <= MAX_NBR_OF_BTNS; i++) {
			if (item_active[i] >= TOUCH_ITEM_ACTIVE) touch_active = 1;	// any item still active?			
		}	    
		return;
	}
	if((p = checkstring(cmdline, "ENABLE")) != NULL) {
    	int k;    
    	getargs(&p, (MAX_ARG_COUNT * 2) - 1, ",");		// getargs macro must be the first executable stmt in a block
        if(checkstring(argv[0], "ALL")) {
		    for(i = 0; i <= MAX_NBR_OF_BTNS; i++) enable_item(i);
			return;
		}		
       	for(i = 0; i < argc; i += 2) {
       		k = getinteger(argv[i]);
       	    if(k < 0 || k > MAX_NBR_OF_BTNS) touch_index_error();
			enable_item(k);
   		}
		return;
	}
	if((p = checkstring(cmdline, "DISABLE")) != NULL) {
    	int k;   
    	getargs(&p, (MAX_ARG_COUNT * 2) - 1, ",");		// getargs macro must be the first executable stmt in a block
        if(checkstring(argv[0], "ALL")) {
		    for(i = 0; i <= MAX_NBR_OF_BTNS; i++) disable_item(i);
			return;
		}		
       	for(i = 0; i < argc; i += 2) {
       		k = getinteger(argv[i]);
       	    if(k < 0 || k > MAX_NBR_OF_BTNS) touch_index_error();
			disable_item(k);
   		}
		return;
	}
	if((p = checkstring(cmdline, "SIZE")) != NULL) {
		getargs(&p, 3, ",");
		if(argc != 3) touch_syntax_error();
		item_sizex = getinteger(argv[0]);
		item_sizey = getinteger(argv[2]);
		if(item_sizex < 10 || item_sizey < 10) error("Item too small (touch)");
		return;	    
	}
	if((p = checkstring(cmdline, "CHECK")) != NULL) {
		checktouch();
		return; 
	}
	if((p = checkstring(cmdline, "RELEASE")) != NULL) {
		if (DetectTouch()) {
			while (DetectTouch()) CheckAbort();                                     // wait for touch released
		}
		last_touchx = -1;			// invalidize touch coord -cm
		last_touchy = -1;
		return;
	}
	if((p = checkstring(cmdline, "WAIT")) != NULL) {
		if (!DetectTouch()) {
			while (!DetectTouch()) CheckAbort();                                    // wait for touch occured
			checktouch();
		}
		return;
	}
	if((p = checkstring(cmdline, "INTERRUPT")) != NULL) {
		skipspace(p);
		if(*p == '0' && !isdigit(*(p+1))) 
		    OnTouchGOSUB = NULL;                                                // the program wants to turn the interrupt off
		else {
			OnTouchGOSUB = GetIntAddress(p);					                // get a pointer to the interrupt routine
    	    InterruptUsed = true;
		}	
		return;
	}
	if((p = checkstring(cmdline, "BEEP")) != NULL) {
		// make touch tick or beep noise @ 1kHz, parameter duration
		skipspace(p);
		temp = getinteger(p);				// only one param, duration in mSec
	    for(i = 0; i <= temp; i++) {
			ticknoise(500);
			uSec(500);
		}	    
		return;
	}
	if((p = checkstring(cmdline, "CALIBRATE")) != NULL) {
		// new function to calibrate touch screen by letting the user touch two dots
	    for(i = 0; i <= MAX_NBR_OF_BTNS; i++) {
			item_active[i] = 0;				// de-validize all items		
		}	    
    DefaultBgColour = 0;				// overide the background colour
    MMcls();							// clear screen and home cursor
		MMCircle(4, 4, 4, 1, 7, 1);
		SetFont(0, 1, 0);
		MMPosX = 12;
		MMPosY = 10;
		DisplayString("TOUCH CIRCLE");		// print string

		while (!getXY()) CheckAbort();		// wait for touch occured and get XY

		touch_ofs_x = x_raw - 7;			// substract small offset due to circle diameter
		touch_ofs_y = y_raw - 12;			// substract small offset due to circle diameter

		if (DetectTouch()) {
			while (DetectTouch()) CheckAbort();		// wait for touch released
		}
    MMcls();							// clear screen and home cursor
		MMCircle((TOUCH_HRES-5), (TOUCH_VRES-5), 4, 1, 7, 1);
		MMPosX = (TOUCH_HRES-65);
		MMPosY = TOUCH_VRES-20;
		DisplayString("NOW HERE");			// print string

		while (!getXY()) CheckAbort();		// wait for touch occured and get XY

		touch_max_x = x_raw + 7;			// add small offset due to circle diameter
		touch_max_y = y_raw + 12;			// add small offset due to circle diameter

		if (DetectTouch()) {
			while (DetectTouch()) CheckAbort();			// wait for touch released
		}

		WriteRTCram(0,touch_ofs_x);
		WriteRTCram(1,touch_max_x);
		WriteRTCram(2,touch_ofs_y);
		WriteRTCram(3,touch_max_y);
		WriteRTCram(4,0xAA55);				// is now initialized
		touch_scale_x =	TOUCH_HRES*1000/(touch_max_x-touch_ofs_x);
		touch_scale_y =	TOUCH_VRES*1000/(touch_max_y-touch_ofs_y);
    MMcls();							// clear screen and home cursor
		DisplayString("Touch Calibration Values Written to NVRAM");				// print string
		SetFont((GetFlashOption(&FontOption) == CONFIG_FONT1) ? 0:1, 1, 0);
		return;
	}
	touch_missing_error();
}
