/***********************************************************************************************************************
MMBasic

MM_Custom.c

Handles all the custom commands and functions in the Maximite implementation of MMBasic.  These are commands and functions
that are not normally part of the Maximite.  This is a good place to insert your own customised commands.

************************************************************************************************************************/

#include <stdio.h>
#include <p32xxxx.h>								// device specific defines
#include <plib.h>									// peripheral libraries

#include "MMBasic_Includes.h"
#include "Hardware_Includes.h"

/********************************************************************************************************************************************
 custom commands and functions
 each function is responsible for decoding a command
 all function names are in the form cmd_xxxx() (for a basic command) or fun_xxxx() (for a basic function) so, if you want to search for the
 function responsible for the NAME command look for cmd_name

 There are 4 items of information that are setup before the command is run.
 All these are globals.

 int cmdtoken	This is the token number of the command (some commands can handle multiple
				statement types and this helps them differentiate)

 char *cmdline	This is the command line terminated with a zero char and trimmed of leading
				spaces.  It may exist anywhere in memory (or even ROM).

 char *nextstmt	This is a pointer to the next statement to be executed.  The only thing a
				command can do with it is save it or change it to some other location.

 char *CurrentLinePtr  This is read only and is set to NULL if the command is in immediate mode.

 The only actions a command can do to change the program flow is to change nextstmt or
 execute longjmp(mark, 1) if it wants to abort the program.

 ********************************************************************************************************************************************/


void getcoord(char *p, int *x, int *y);

// List of 32 touch items
int item_init[MAX_NBR_OF_BTNS],item_value[MAX_NBR_OF_BTNS],item_type[MAX_NBR_OF_BTNS],item_colour[MAX_NBR_OF_BTNS],
	item_top[MAX_NBR_OF_BTNS],item_left[MAX_NBR_OF_BTNS],item_bottom[MAX_NBR_OF_BTNS],item_right[MAX_NBR_OF_BTNS];
char item_text[MAX_NBR_OF_BTNS][15];
int	item_sizex = 80; 
int item_sizey = 25;
int last_item_hit = -1; // last item touched/hit
int touch_active = 0;

void mSec(int mstime) {
	IntPauseTimer = 0;
	if (mstime == 0) return;
	while(IntPauseTimer < mstime) CheckAbort();
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

#define TOUCH_BEEPER			34	// MMBASIC Beeper output pin PIC32MX795 Pin 32 RB8/AN8

#define TOUCHX_MINUS_PIN		35	// X- = PIC32MX795 Pin 32 RB8/AN8
#define TOUCHX_PLUS_PIN			36	// X+ = PIC32MX795 Pin 33 RB9/AN9
#define TOUCHY_MINUS_PIN		37	// Y- = PIC32MX795 Pin 34 RB10/AN10
#define TOUCHY_PLUS_PIN			38	// Y+ = PIC32MX795 Pin 35 RB11/AN11

int DetectTouch(void) {
// also sets all rails as inputs on exit. Returns true if touched
	int value;
	ODCBCLR = 0x0F00; 	// all NOT open drain
	LATBCLR = 0x0F00; 	// all low - IMPORTANT, otherwise switching port dir takes >2 ms!
	TRISBSET = 0x0F00;	// all inputs RB8..RB11 = 0x0F00

	PinSetOutput(TOUCHY_MINUS_PIN); 	// reset value on Y rails cap
	PinLow(TOUCHY_MINUS_PIN); 			// reset value on Y rails cap
	uSec(5);							// 5 us cap discharge
	PinSetInput(TOUCHY_MINUS_PIN);		// cap keeps pin low until read if not touched

// set X plane high to detect touch
	PinSetOutput(TOUCHX_MINUS_PIN); 	// output on X rails
	PinSetOutput(TOUCHX_PLUS_PIN);
	PinHigh(TOUCHX_MINUS_PIN);			// set to Ref Voltage
	PinHigh(TOUCHX_PLUS_PIN);			// set to Ref Voltage

	uSec(50);							// RC settling delay
	value = PinRead(TOUCHY_MINUS_PIN);	// return 1 if touched

	LATBCLR = 0x0F00; 	// all low - IMPORTANT, otherwise switching port dir takes >2 ms!
	TRISBSET = 0x0F00;	// all inputs
	return value;
}

int GetTouchX(void) {
// For analog returns the reading as a 10 bit number with 0b1111111111 = 3.3V
	int value = 0;

   	AD1CHSbits.CH0SA = GetPinBit(TOUCHY_MINUS_PIN);
	AD1PCFGCLR = (1 << GetPinBit(TOUCHY_MINUS_PIN));	// enable analog function

	TRISBCLR = 0x0300;	// X rails outputs
	PinHigh(TOUCHX_PLUS_PIN);	 			// set X+ line to Ref 3V3
// read analog voltage from Y- pin
	uSec(50);								// settling delay

	AD1CON1SET = 0x0002;       				// start conversion
	while (!AD1CON1bits.DONE && !MMAbort);  // wait conversion complete
	value = ADC1BUF0;						// and get the result
	AD1PCFGSET = (1 << GetPinBit(TOUCHY_MINUS_PIN));	// disable analog function
	return value;
}

int GetTouchY(void) {
// For analog returns the reading as a 10 bit number with 0b1111111111 = 3.3V
	int value = 0;

   	AD1CHSbits.CH0SA = GetPinBit(TOUCHX_MINUS_PIN);
	AD1PCFGCLR = (1 << GetPinBit(TOUCHX_MINUS_PIN));	// enable analog function

	TRISBCLR = 0x0C00;	// Y rails outputs
	PinHigh(TOUCHY_PLUS_PIN);	 			// set Y+ line to Ref 3V3
// read analog voltage from X- pin
	uSec(50);								// settling delay

	AD1CON1SET = 0x0002;       				// start conversion
	while (!AD1CON1bits.DONE && !MMAbort);  // wait conversion complete
	value = ADC1BUF0;						// and get the result
	AD1PCFGSET = (1 << GetPinBit(TOUCHX_MINUS_PIN));	// disable analog function
	return value;
}


// ######################################################################################
// graphic primitives: button, slider etc.
// ######################################################################################

void drawButton(int x1, int y1, int x2, int y2, int colour, int pressed, char *s) {
 	int  mm_x, mm_y, slen, item_sizex_half, item_sizey_half, x1_insert, x2_insert, y1_insert, y2_insert;

	item_sizex_half = (x2-x1)/2; 
	item_sizey_half = (y2-y1)/2;
	x1_insert = x1+1;
	y1_insert = y1+1;
	x2_insert = x2-1;
	y2_insert = y2-1;

	MMbox(x1, y1, x2, y2, 0, 7);						// draw a surrounding box
	plot(x1, y1, DefaultBgColour);							// corners
	plot(x2, y1, DefaultBgColour);
	plot(x1, y2, DefaultBgColour);
	plot(x2, y2, DefaultBgColour);
 
	if (pressed){
		MMline(x1_insert, y1_insert, x2_insert, y1_insert, 0);						// draw a insert line
		MMline(x1_insert, y1_insert, x1_insert, y2_insert, 0);						// draw a insert line
		MMline(x1+2, y2_insert, x2_insert, y2_insert, colour);				// draw a insert line
		MMline(x2_insert, y1+2, x2_insert, y2_insert, colour);				// draw a insert line
		MMbox(x1+2, y1+2, x2-2, y2-2, 1, 0);					// draw a insert coloured box
	}	else {
		MMline(x1_insert, y1_insert, x2_insert, y1_insert, 7);						// draw a insert line
		MMline(x1_insert, y1_insert, x1_insert, y2_insert, 7);						// draw a insert line
		MMline(x1+2, y2_insert, x2_insert, y2_insert, 0);						// draw a insert line
		MMline(x2_insert, y1+2, x2_insert, y2_insert, 0);						// draw a insert line
		MMbox(x1+2, y1+2, x2-2, y2-2, 1, colour);			// draw a insert coloured box
	}
	slen = strlen(s);
	mm_x = MMPosX;
	mm_y = MMPosY;

	SetFont(1, 1, 0);
 	MMPosX = x1+item_sizex_half-(5*slen);	// center caption
	MMPosY = y1+item_sizey_half-8;

	if (pressed) {
		CurrentFgColour = colour;
		CurrentBgColour = 0;
	} else {
		CurrentFgColour = 0;
		CurrentBgColour = colour;
	}
	DisplayString(s);				// print string (s is a MMBasic string)
	MMPosX = mm_x;
	MMPosY = mm_y;
	SetFont(0, 1, 0);
	CurrentFgColour = DefaultFgColour;
	CurrentBgColour = DefaultBgColour;
	if (pressed){
		MMbox(x1+3, y1+3, x2-2, y2-2, 0, colour);				// draw a insert coloured box
		} else {
		MMbox(x1+2, y1+2, x2-2, y2-2, 0, 7);				// draw a insert coloured box
		plot(x1+2, y1+2, colour);							// corners
		plot(x2-2, y1+2, colour);
		plot(x1+2, y2-2, colour);
		plot(x2-2, y2-2, colour);
	}
}

void drawRadioButton(int x1, int y1, int x2, int y2, int colour, int pressed, char *s) {
 	int  mm_x, mm_y, slen, item_size_third, item_size_half;

	item_size_half = (y2-y1)/2;
	item_size_third = (y2-y1)/3;

//	MMCircle(x, y, radius, fill, colour, aspect);
	MMCircle(x1+item_size_half, y1+item_size_half, item_size_half, 1, colour, 1);				// draw a filled white circle
 	MMCircle(x1+item_size_half, y1+item_size_half, item_size_third, (pressed && 1), 0, 1);		// draw a black filled circle

	slen = strlen(s);
	mm_x = MMPosX;
	mm_y = MMPosY;

	SetFont(1, 1, 0);
 	MMPosX = x1+item_size_half+item_size_half+6;	// align caption
	MMPosY = y1+item_size_half-8;					// Text height center

	DisplayString(s);								// print string (s is a MMBasic string)
	MMPosX = mm_x;
	MMPosY = mm_y;
	SetFont(0, 1, 0);
}

void drawCheckbox(int x1, int y1, int x2, int y2, int colour, int pressed, char *s) {
 	int  mm_x, mm_y, slen, item_size, item_size_half;

	item_size = y2-y1;
	item_size_half = item_size/2;

	MMbox(x1, y1, x1+item_size, y2, 1, colour);			// draw a filled colour box
	MMbox(x1+1, y1+1, x1+item_size-1, y2-1, 0, 0);	// draw a insert black box
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
 	MMPosX = x1+item_size+6;	// align caption
	MMPosY = y1+item_size_half-8;					// Text height center

	DisplayString(s);								// print string (s is a MMBasic string)
	MMPosX = mm_x;
	MMPosY = mm_y;
	SetFont(0, 1, 0);
}

void drawSwitch(int x1, int y1, int x2, int y2, int colour, int pressed) {
 	int  mm_x, mm_y, item_sizex_half, x1_insert, x2_insert, y1_insert, y2_insert;

	x1_insert = x1+1;
	y1_insert = y1+1;
	x2_insert = x2-1;
	y2_insert = y2-1;
 
	MMbox(x1, y1, x2, y2, 0, 7);						// draw a surrounding box
	plot(x1, y1, DefaultBgColour);							// corners
	plot(x2, y1, DefaultBgColour);
	plot(x1, y2, DefaultBgColour);
	plot(x2, y2, DefaultBgColour);

	MMbox(x1_insert, y1_insert, x2_insert, y2_insert, 1, colour);						// draw insert box
	mm_x = MMPosX;
	mm_y = MMPosY;
	SetFont(1, 1, 0);
	MMPosY = y1+4;
	item_sizex_half = (x2-x1)/2; 
	if (pressed){
		MMbox(x1+item_sizex_half-1, y1+2, x2-2, y2-2, 1, 7);					// draw a white box
  	MMPosX = x1+item_sizex_half+6;
		CurrentFgColour = 0;
		CurrentBgColour = 7;
		DisplayString("ON");				// print string
		MMline(x1+item_sizex_half, y2_insert, x2_insert, y2_insert, DefaultBgColour);	// draw shadow line
		MMline(x2_insert, y1+2, x2_insert, y2_insert, DefaultBgColour);		// draw shadow line
	} else {
  	MMPosX = x1+3;
		CurrentFgColour = 0;
		CurrentBgColour = colour;
		DisplayString("OFF");				// print string
		MMbox(x1+2, y1+2, x1+item_sizex_half+2, y2-2, 0, 7);					// draw a unfilled white box
		MMline(x1+3, y2_insert, x1+item_sizex_half+3, y2_insert, DefaultBgColour);		// draw shadow line
		MMline(x1+item_sizex_half+3, y1+2, x1+item_sizex_half+3, y2_insert, DefaultBgColour);		// draw shadow line
	}
    MMPosX = mm_x;
    MMPosY = mm_y;
	SetFont(0, 1, 0);
	CurrentFgColour = DefaultFgColour;
	CurrentBgColour = DefaultBgColour;
}

void drawItemIdx(int index) {
// index from 0 to 31
	if (item_init[index] != 0x55AA) return;		// was not inited/active
	switch(item_type[index]) {
		case TOUCH_TYPE_BUTTON:
			drawButton(item_left[index], item_top[index], item_right[index], item_bottom[index], item_colour[index], item_value[index], item_text[index]);
			break;
		case TOUCH_TYPE_SWITCH:
			drawSwitch(item_left[index], item_top[index], item_right[index], item_bottom[index], item_colour[index], item_value[index]);	
			break;
		case TOUCH_TYPE_RADIO:
			drawRadioButton(item_left[index], item_top[index], item_right[index], item_bottom[index], item_colour[index], item_value[index], item_text[index]);
			break;
		case TOUCH_TYPE_CHECK:
			drawCheckbox(item_left[index], item_top[index], item_right[index], item_bottom[index], item_colour[index], item_value[index], item_text[index]);
			break;
	}
}


int checkItem(int x, int y, int index) {
// check if Item (idx) is touched - so x and y fall into item's coord scope
	if (item_init[index] != 0x55AA) return 0;			// was not active
	if (x < item_left[index]-TOUCH_OVERLAP) return 0;	// not in bounds
	if (x > item_right[index]+TOUCH_OVERLAP) return 0;
	if (y < item_top[index]-TOUCH_OVERLAP) return 0;
	if (y > item_bottom[index]+TOUCH_OVERLAP) return 0;

	return 1;
}
	
	
void cmd_touchval(void) {
// TouchItem(<itemnbr>) = 1 or 0 - set button/switch nbr on or off
int index, value;
	index = getinteger(cmdline);
    if(index < 0 || index > MAX_NBR_OF_BTNS) error("Touch index out of range");

	while(*cmdline && tokenfunction(*cmdline) != op_equal) cmdline++;
	if(!*cmdline) error("Invalid syntax");
	++cmdline;
	if(!*cmdline) error("Missing value");
	value = getinteger(cmdline);
	if(value) item_value[index] = 1;	// button ON
	else item_value[index] = 0;
	drawItemIdx(index);	
}


void fun_touchval(void) {
	int index, value;
	long temp;
	char *p;
	
	if((p = checkstring(ep, "#X")) != NULL) {
		if (!DetectTouch())	{
			fret = -1;
			return;
		}
		value = GetTouchX();
		if (!DetectTouch())	{
			fret = -1;
			return;
		}
		value -= TOUCH_OFS_X;	// Offset Y
		if (value > TOUCH_MAX_X) value = TOUCH_MAX_X;
		if (value < 0) value = 0;
		temp = ((long)value * TOUCH_SCALE_X) / 1000;
	
		fret = (float)temp;
		return;
	}
	if((p = checkstring(ep, "#Y")) != NULL) {
		if (!DetectTouch())	{
			fret = -1;
			return;
		}
		value = GetTouchY();
		if (!DetectTouch())	{
			fret = -1;
			return;
		}
		value -= TOUCH_OFS_Y;	// Offset Y
		if (value > TOUCH_MAX_Y) value = TOUCH_MAX_Y;
		if (value < 0) value = 0;
		temp = ((long)value * TOUCH_SCALE_Y) / 1000;
	
		fret = (float)temp;
		return;
	}
	if((p = checkstring(ep, "#I")) != NULL) {
		fret = (float)last_item_hit;
		return;
	}

	index = getinteger(ep);	// only one param
    if(index < 0 || index > MAX_NBR_OF_BTNS) error("Touch index out of range");

	if (item_init[index] != 0x55AA) {
		fret = -1;
		return;
	}
	value = item_value[index];			// get value formerly set by CheckTouch
	if (item_type[index] == TOUCH_TYPE_BUTTON || item_type[index] == TOUCH_TYPE_NONE) 
		if (value) {
			item_value[index] = 0;	// was read, is now OFF	
			drawItemIdx(index);		// redraw me
		}
	fret = (float)value;
		
}


void checktouch(void) {
// run one touchscreen cycle and check if button/switch hit
	int index, x, y, myitemtype, radio_hit;
	long temp;

	if (!touch_active)	return;	// not activated
	if (!DetectTouch())	return;	// nothing happened

	x = GetTouchX();
	LATBCLR = 0x0F00; 	// all low - IMPORTANT, otherwise switching port dir takes >2 ms!
	TRISBSET = 0x0F00;	// all inputs
	y = GetTouchY();

	if (!DetectTouch())	return;	// premature end of touch

	x -= TOUCH_OFS_X;	// Offset X
	if (x < 0) x = 0;
	if (x > TOUCH_MAX_X) x = TOUCH_MAX_X;
	temp = ((long)x * TOUCH_SCALE_X) / 1000;
	x = (int)temp;

	y -= TOUCH_OFS_Y;	// Offset Y
	if (y < 0) y = 0;
	if (y > TOUCH_MAX_Y) y = TOUCH_MAX_Y;
	temp = ((long)y * TOUCH_SCALE_Y) / 1000;
	y = (int)temp;
	radio_hit = 0;
	last_item_hit = -1;
// we now have the XY touch coordinates and can check if it falls in range of one item
    for(index = 0; index <= MAX_NBR_OF_BTNS; index++) {
		myitemtype = item_type[index];
		if (checkItem(x, y, index)) {	// have we found an item?
			PinOpenCollectorOff(TOUCH_BEEPER);
			PinSetOutput(TOUCH_BEEPER); 	// click noise on beeper
			PinHigh(TOUCH_BEEPER);
			uSec(2500);
			PinLow(TOUCH_BEEPER);
			last_item_hit = index;
			switch(myitemtype) {
				case TOUCH_TYPE_BUTTON:
					item_value[index] = 1;	// is now ON	
					drawItemIdx(index);		// redraw me
					while (DetectTouch()) mSec(50);  // wait for touch released
					break;
				case TOUCH_TYPE_SWITCH:		// is it a switch?
					item_value[index] ^= 1;	// Toggle	
					drawItemIdx(index);		// redraw me
					while (DetectTouch()) mSec(50);  // wait for touch released
					break;
				case TOUCH_TYPE_RADIO:		// is it a radio button?
					item_value[index] = 1;	// is now ON	
					drawItemIdx(index);		// redraw me for responsiveness
					radio_hit = 1;
					break;
				case TOUCH_TYPE_CHECK:		// is it a check box?
					item_value[index] ^= 1;	// Toggle	
					drawItemIdx(index);		// redraw me
					while (DetectTouch()) mSec(50);  // wait for touch released
					break;
				case TOUCH_TYPE_NONE:		// is it a empty box?
					item_value[index] = 1;	// is now ON	
			}
		}
	}	
// cancel all other radio buttons
	if (radio_hit) {
	    for(index = 0; index <= MAX_NBR_OF_BTNS; index++) {
			if (item_type[index] == TOUCH_TYPE_RADIO) item_value[index] = 0;	// shut off all radio btns			
		}
		item_value[last_item_hit] = 1;	// only last one is now ON
	    for(index = 0; index <= MAX_NBR_OF_BTNS; index++) {
			if (item_type[index] == TOUCH_TYPE_RADIO)	drawItemIdx(index);		// redraw all radio btns
		}
		while (DetectTouch()) mSec(50);  // wait for touch released and redraw all
	}
}

void cmd_touch(void) {
/* example for using multiple cmd defs: SPRITE MOVE n, x, y[, colour]    
	char *p;
   	int i, x, y, c;
	if((p = checkstring(cmdline, "MOVE")) != NULL) {
	    getargs(&p, 7, ",");
	    if(!(argc == 5 || argc == 7)) error("Invalid number of parameters");
    	i = getinteger(argv[0]);
	    x = getinteger(argv[2]);
	    y = getinteger(argv[4]);
	    if(argc == 7) 
	        c = getinteger(argv[6]);
	    else
	        c = 0;
*/

   	int i, x, y, c, temp;
	int is_button = 0;
	int is_switch = 0;
	int is_radio = 0;
	int is_check = 0;
	char *s, *p;
    
	if((p = checkstring(cmdline, "ITEM CREATE")) != NULL) {
	    getargs(&p, 11, ",");
		if(argc < 8) error("Invalid touch syntax");

    	i = getinteger(argv[0]);
    	
	    x = getinteger(argv[2]);
	    y = getinteger(argv[4]);		
/*
// TODO: Will that work?
		p = argv[xxx];
		// the start point is specified
		if(*p != '(') error("Expected coordinate opening bracket");
		getcoord(p + 1, &x, &y);
		p = getclosebracket(p + 1) + 1;
*/

		c = getinteger(argv[8]);

	    
		if(i > MAX_NBR_OF_BTNS || i < 0) error("Touch index out of range");

		s = "OK";
		if(*argv[6]) {
			s = getstring(argv[6]);									// the caption string
		}
		
		if(argc == 11) {
			is_button = (strchr(argv[10], 'b') != NULL || strchr(argv[10], 'B') != NULL);
			is_switch = (strchr(argv[10], 's') != NULL || strchr(argv[10], 'S') != NULL);
			is_radio = (strchr(argv[10], 'r') != NULL || strchr(argv[10], 'R') != NULL);
			is_check = (strchr(argv[10], 'c') != NULL || strchr(argv[10], 'C') != NULL);
		}		
		
		item_init[i] = 0x55AA;	// validize button
		item_left[i] = x;
		item_right[i] = x+item_sizex;
		item_top[i] = y;
		item_bottom[i] = y+item_sizey;
		item_value[i] = 0;	// default OFF
		temp = TOUCH_TYPE_NONE;								// default empty graphic
		if(is_button)
			temp = TOUCH_TYPE_BUTTON;	// is a button
		if(is_switch)
			temp = TOUCH_TYPE_SWITCH;	// is a switch
		if(is_radio)
			temp = TOUCH_TYPE_RADIO;	// is a radio button
		if(is_check)
			temp = TOUCH_TYPE_CHECK;	// is a ckeckbox
		item_type[i] = temp;
		item_colour[i] = c;
		strncpy(item_text[i],s,14);
		drawItemIdx(i);
		touch_active = 1;
		return;
	}
	if((p = checkstring(cmdline, "ITEM REMOVE")) != NULL) {
    	int k;
    
    	getargs(&p, (MAX_ARG_COUNT * 2) - 1, ",");		// getargs macro must be the first executable stmt in a block

        if(checkstring(argv[0], "ALL")) {			// if the argument is ALL load all active sprites into the array
		    for(i = 0; i <= MAX_NBR_OF_BTNS; i++) {
				item_init[i] = 0;	// de-validize all items		
			}	    
			return;
		}
		
       	for(i = 0; i < argc; i += 2) {					// step through the arguments and put the sprite numbers into the array
       		k = getinteger(argv[i]);
       	    if(k < 0 || k > MAX_NBR_OF_BTNS) error("Touch index out of range");
			item_init[k] = 0;	// de-validize item
   		}

		touch_active = 0;	
	    for(i = 0; i <= MAX_NBR_OF_BTNS; i++) {
			if (item_init[i] == 0x55AA) touch_active = 1;	// any item still active?			
		}	    
		return;
	}
	if((p = checkstring(cmdline, "ITEM SIZE")) != NULL) {
		getargs(&p, 3, ",");
		if(argc != 3) error("Invalid syntax");
		item_sizex = getinteger(argv[0]);
		item_sizey = getinteger(argv[2]);
		if(item_sizex < 10 || item_sizey < 10) error("Touch item too small");
		return;	    
	}
	if((p = checkstring(cmdline, "CHECK")) != NULL) {
		checktouch();
		return;
 
	}
	if((p = checkstring(cmdline, "RELEASE")) != NULL) {
		if (DetectTouch()) {
			while (DetectTouch()) mSec(50);  // wait for touch released
		}
		return;
	}
	if((p = checkstring(cmdline, "WAIT")) != NULL) {
		if (!DetectTouch()) {
			while (!DetectTouch()) mSec(50);  // wait for touch occured
			checktouch();
		}
		return;
	}
	error("Expected ITEMCREATE/REMOVE/SIZE, CHECK, RELEASE or WAIT");
}


// ######################################################################################
// Some graphic widgets
// ######################################################################################

// TODO: merge to one GRAPH command
// GRAPH HBar(x,y),Wert,Farbe,Fill
void cmd_drawhbar(void) {
	int x1, y1, y2, colour, myval, fill, box, scale;
	char *p;
	getargs(&cmdline, 7, ",");

	if(argc < 2) error("Invalid widget syntax");
	x1 = lastx; y1 = lasty; colour = DefaultFgColour;		// set the defaults for optional components
	p = argv[0];
	// the start point is specified
	if(*p != '(') error("Expected opening bracket");
	getcoord(p + 1, &x1, &y1);
	p = getclosebracket(p + 1) + 1;


	colour = 7;
	fill = true;
	if(argc > 3 && *argv[4])
		colour = getinteger(argv[4]);

	myval = getinteger(argv[2]);
	if(myval > 100) myval=100;
	if(myval < 0) myval=0;
	lastx = x1; lasty = y1;			// save in case the user wants the last value

	box = 0;
	fill = 0;
	scale = 0;
	if(argc == 7) {
		box = (strchr(argv[6], 'b') != NULL || strchr(argv[6], 'B') != NULL);
		fill = (strchr(argv[6], 'f') != NULL || strchr(argv[6], 'F') != NULL);
		scale = (strchr(argv[6], 's') != NULL || strchr(argv[6], 'S') != NULL);
	}
	if (box) MMbox(x1, y1, x1+104, y1+25, 0, 7);						// draw a surrounding box

	if (scale) {
		x1 += 2;
		y1 += 25;
		y2 = y1 + 5;
		MMline(x1, y1, x1, y2+3, 7);					// draw a scale line
		x1 = x1+12;
		MMline(x1, y1, x1, y2, 7);						// draw a scale line
		x1 = x1+13;
		MMline(x1, y1, x1, y2+3, 7);					// draw a scale line
		x1 = x1+12;
		MMline(x1, y1, x1, y2, 7);						// draw a scale line
		x1 = x1+13;
		MMline(x1, y1, x1, y2+3, 7);					// draw a scale line
		x1 = x1+12;
		MMline(x1, y1, x1, y2, 7);						// draw a scale line
		x1 = x1+13;
		MMline(x1, y1, x1, y2+3, 7);					// draw a scale line
		x1 = x1+12;
		MMline(x1, y1, x1, y2, 7);						// draw a scale line
		x1 = x1+13;
		MMline(x1, y1, x1, y2+3, 7);					// draw a scale line
	}

	x1 = lastx+2;
	y1 = lasty+2;
	if (fill) {
		MMbox(x1+myval, y1, x1+100, y1+21, 1, 0);		// erase rest of insert coloured box
		MMbox(x1, y1, x1+myval, y1+21, 1, colour);		// draw a insert coloured box
	} else {
		MMbox(x1, y1, x1+100, y1+21, 1, 0);		// erase insert coloured box
		MMline(x1+myval, y1, x1+myval, y1+21, colour);		// draw a line
	}

}

// GRAPH VBar(x,y),Wert,Farbe,BFS
void cmd_drawvbar(void) {
	int x1, y1, x2, colour, myval, fill, box, scale;
	char *p;
	getargs(&cmdline, 7, ",");

	if(argc < 2) error("Invalid widget syntax");
	x1 = lastx; y1 = lasty; colour = DefaultFgColour;		// set the defaults for optional components
	p = argv[0];
	// the start point is specified
	if(*p != '(') error("Expected opening bracket");
	getcoord(p + 1, &x1, &y1);
	p = getclosebracket(p + 1) + 1;

	colour = 7;
	if(argc > 3 && *argv[4])
		colour = getinteger(argv[4]);
	myval = getinteger(argv[2]);

	if(myval > 100) myval=100;
	if(myval < 0) myval=0;
	lastx = x1; lasty = y1;			// save in case the user wants the last value

	box = 0;
	fill = 0;
	scale = 0;
	if(argc == 7) {
		box = (strchr(argv[6], 'b') != NULL || strchr(argv[6], 'B') != NULL);
		fill = (strchr(argv[6], 'f') != NULL || strchr(argv[6], 'F') != NULL);
		scale = (strchr(argv[6], 's') != NULL || strchr(argv[6], 'S') != NULL);
	}
	if (box) MMbox(x1, y1, x1+25, y1+104, 0, 7);						// draw a surrounding box

	if (scale) {
		x1 += 25;
		y1 += 2;
		x2 = x1 + 5;
		MMline(x1, y1, x2+3, y1, 7);					// draw a scale line
		y1 = y1+12;
		MMline(x1, y1, x2, y1, 7);						// draw a scale line
		y1 = y1+13;
		MMline(x1, y1, x2+3, y1, 7);					// draw a scale line
		y1 = y1+12;
		MMline(x1, y1, x2, y1, 7);						// draw a scale line
		y1 = y1+13;
		MMline(x1, y1, x2+3, y1, 7);					// draw a scale line
		y1 = y1+12;
		MMline(x1, y1, x2, y1, 7);						// draw a scale line
		y1 = y1+13;
		MMline(x1, y1, x2+3, y1, 7);					// draw a scale line
		y1 = y1+12;
		MMline(x1, y1, x2, y1, 7);						// draw a scale line
		y1 = y1+13;
		MMline(x1, y1, x2+3, y1, 7);					// draw a scale line
	}

	x1 = lastx+2;
	y1 = lasty+2;
	if (fill) {
		MMbox(x1, y1, x1+21, y1-myval+100, 1, 0);		// erase rest of insert coloured box
		y1 = y1+100;
		MMbox(x1, y1, x1+21, y1-myval, 1, colour);		// draw a insert coloured box
	} else {
		MMbox(x1, y1, x1+21, y1+100, 1, 0);		// erase insert coloured box
		y1 = y1+100;
		MMline(x1, y1-myval, x1+21, y1-myval, colour);		// draw a line
	}
}

void cmd_drawled(void) {
// GRAPH LED draws a LED, usage GRAPH LED(x,y),value,colour
	int x1, y1, colour, myval;
	char *p;
	getargs(&cmdline, 5, ",");

	if(argc < 2) error("Invalid widgetsyntax");
	x1 = lastx; y1 = lasty; colour = DefaultFgColour;		// set the defaults for optional components
	p = argv[0];
	// the start point is specified
	if(*p != '(') error("Expected opening bracket");
	getcoord(p + 1, &x1, &y1);
	p = getclosebracket(p + 1) + 1;

	colour = 7;
	if(argc > 3 && *argv[4])
		colour = getinteger(argv[4]);

	lastx = x1; lasty = y1;			// save in case the user wants the last value
	x1 += 10;	// offset radius
	y1 += 10;
//	MMCircle(x, y, radius, fill, colour, aspect);
	MMCircle(x1, y1, 10, 0, 7, 1);						// draw a surrounding circle
	myval = getinteger(argv[2]);
	if(myval) 
		{
		MMCircle(x1, y1, 8, 1, colour, 1);				// draw insert circle
		MMline(x1-5, y1, x1-3, y1-3, 7);				// draw a shadow
		MMline(x1-3, y1-3, x1, y1-5, 7);				// draw a shadow
		}
	else
		{
		MMCircle(x1, y1, 7, 1, 0, 1);					// erase insert circle
		MMCircle(x1, y1, 8, 0, colour, 1);				// draw insert circle
		MMline(x1-5, y1, x1-3, y1-3, colour);			// draw a shadow
		MMline(x1-3, y1-3, x1, y1-5, colour);			// draw a shadow
		}
}
