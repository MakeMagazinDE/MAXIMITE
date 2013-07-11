/***********************************************************************************************************************
MMBasic

MM_Custom.h

Include file that contains the globals and defines for MMCustom.c in the Maximite version of MMBasic.

************************************************************************************************************************/



/**********************************************************************************
 the C language function associated with commands, functions or operators should be
 declared here
**********************************************************************************/

// MMBASIC additions by Carsten Meyer (cm@ct.de) for the TFT Maximite with Touchscreen
// If it does not compile into Flash space, please alter
// #define MONOCHROME_NBR_PAGES        47                              // size of flash storage used for files in pages on the monochrome Maximite
// in files.h to less, i.e. 32

#define MAX_NBR_OF_BTNS		31	// defines array bounds

#define TOUCH_VRES	 	272		// 272 Vert graphics resolution (pixels)
#define TOUCH_HRES	 	480		// 480 Horiz graphics resolution (pixels)
#define TOUCH_OFS_X		63		// on our screen 63,  measured by ADC
#define TOUCH_MAX_X		955		// on our screen 955, measured by ADC
#define TOUCH_SCALE_X	(TOUCH_HRES*1000/(TOUCH_MAX_X-TOUCH_OFS_X))		// *TOUCH_SCALE_X div 1000

#define TOUCH_OFS_Y		115		// on our screen 115, measured by ADC
#define TOUCH_MAX_Y		860		// on our screen 860, measured by ADC
#define TOUCH_SCALE_Y	(TOUCH_VRES*1000/(TOUCH_MAX_Y-TOUCH_OFS_Y))		// *TOUCH_SCALE_X div 1000
#define TOUCH_OVERLAP		3		// additional touch bounds in pixel

#define TOUCH_TYPE_NONE		0		// for touch item list
#define TOUCH_TYPE_BUTTON	1		// button, single action, latched
#define TOUCH_TYPE_SWITCH	2		// switch, toggled
#define TOUCH_TYPE_RADIO	4		// radio button, only one active at a time
#define TOUCH_TYPE_CHECK	8		// checkbox, multiple choice, like switch


#if !defined(INCLUDE_COMMAND_TABLE) && !defined(INCLUDE_TOKEN_TABLE)
// format:
//      void cmd_???(void)
//      void fun_???(void)
//      void op_???(void)
	void cmd_touch(void);
	void cmd_touchval(void);
	void fun_touchval(void);
	
	
	void cmd_drawhbar(void);
	void cmd_drawvbar(void);
	void cmd_drawled(void);
#endif




/**********************************************************************************
 All command tokens tokens (eg, PRINT, FOR, etc) should be inserted in this table
**********************************************************************************/
#ifdef INCLUDE_COMMAND_TABLE
// type is always T_CMD
// and P is the precedence (which is only used for operators and not commands)
// the format is:
//    TEXT			TYPE		P	FUNCTION TO CALL
	{ "Touch Value(",		T_CMD | T_FUN,		0, 	cmd_touchval},
	{ "Touch",			T_CMD,		0, 	cmd_touch},
	
// TODO: merge to one WIDGET command
	{ "Graph HBar",		T_CMD,		0,	cmd_drawhbar},
	{ "Graph VBar",		T_CMD,		0,	cmd_drawvbar},
	{ "Graph LED",		T_CMD,		0,	cmd_drawled},
  
#endif


/**********************************************************************************
 All other tokens (keywords, functions, operators) should be inserted in this table
**********************************************************************************/
#ifdef INCLUDE_TOKEN_TABLE
// type is T_NA, T_FUN, T_FNA (no arguments) or T_OPER argumented by the types T_STR and/or T_NBR
// and P is the precedence (which is only used for operators)
// the format is:
//    TEXT			TYPE				P	FUNCTION TO CALL
	{ "Touch Value(",		T_FUN | T_NBR,		0,	fun_touchval},

#endif

