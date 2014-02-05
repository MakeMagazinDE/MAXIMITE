/***********************************************************************************************************************
MMBasic

Touch.h

Include file that contains the globals and defines for Touch.c in the TFT Maximite version of MMBasic.

Note that this file is only used in building the firmware for the TFT Maximite which has a touch sensitive TFT LCD.
The TFT Maximite design, this file and MMBasic additions were created by Carsten Meyer (cm@ct.de) and was described 
in the German magazine c't in late 2013.

************************************************************************************************************************/

#define MAX_NBR_OF_BTNS		31	                // defines array bounds
#define TOUCH_CHECK         100                 // check for a touch every this number of commands executed

#define TOUCH_VRES	 	    272		            // 272 Vert graphics resolution (pixels)
#define TOUCH_HRES	 	    480		            // 480 Horiz graphics resolution (pixels)
#define TOUCH_OFS_X		    63		            // on our screen 63,  measured by ADC
#define TOUCH_MAX_X		    955		            // on our screen 955, measured by ADC
#define TOUCH_SCALE_X	    (TOUCH_HRES*1000/(TOUCH_MAX_X-TOUCH_OFS_X))		// *TOUCH_SCALE_X div 1000

#define TOUCH_OFS_Y		    115		            // on our screen 115, measured by ADC
#define TOUCH_MAX_Y		    860		            // on our screen 860, measured by ADC
#define TOUCH_SCALE_Y	    (TOUCH_VRES*1000/(TOUCH_MAX_Y-TOUCH_OFS_Y))		// *TOUCH_SCALE_X div 1000
#define TOUCH_OVERLAP		3		            // additional touch bounds in pixel

#define TOUCH_TYPE_NONE		0		            // for touch item list
#define TOUCH_TYPE_BUTTON	1		            // button, single action, latched
#define TOUCH_TYPE_SWITCH	2		            // switch, toggled
#define TOUCH_TYPE_RADIO	3		            // radio button, only one active at a time
#define TOUCH_TYPE_CHECK	4		            // checkbox, multiple choice, like switch
#define TOUCH_TYPE_PUSH		5		            // latching toggle pushbutton, like switch
#define TOUCH_TYPE_LED		9		            // LED or LED button
#define TOUCH_TYPE_VSLIDER	10		            // "analog" slider vertical
#define TOUCH_TYPE_HSLIDER	11		            // "analog" slider horizontal

#define TOUCH_ITEM_INVALID	0		            // item status
#define TOUCH_ITEM_INACTIVE	1					// inactive, greyed out
#define TOUCH_ITEM_ACTIVE	2					// active, highlighted

#if !defined(INCLUDE_COMMAND_TABLE) && !defined(INCLUDE_TOKEN_TABLE)
	void cmd_touch(void);
	void cmd_touchval(void);
	void cmd_drawgraph(void);
	void fun_touchval(void);
	void fun_touched(void);
// calibration constants for TFT -cm
	extern int touch_ofs_x, touch_ofs_y, touch_max_x, touch_max_y, touch_scale_x, touch_scale_y;

    extern int touch_active;
    extern int TouchTimer;
    extern char *OnTouchGOSUB;
    extern void InitTouchLCD(int col, int pos);
    extern int checktouch(void);
    extern int item_active[MAX_NBR_OF_BTNS];
	extern void WriteRTCram(int reg, int data);
	extern int ReadRTCram(int reg);
#endif


/**********************************************************************************
 All command tokens tokens (eg, PRINT, FOR, etc) should be inserted in this table
**********************************************************************************/
#ifdef INCLUDE_COMMAND_TABLE
	{ "TouchVal(",	T_CMD | T_FUN,	0, 	cmd_touchval},
	{ "Touch",		T_CMD,		    0, 	cmd_touch},
#endif


/**********************************************************************************
 All other tokens (keywords, functions, operators) should be inserted in this table
**********************************************************************************/
#ifdef INCLUDE_TOKEN_TABLE
	{ "TouchVal(",	T_FUN | T_NBR,	0,	fun_touchval},
	{ "Touched(",	T_FUN | T_NBR,	0,	fun_touched},
#endif

