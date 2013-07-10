/************************************************************************************************************************
Maximite

Video.h

Copyright 2011, 2012 Geoff Graham.  All Rights Reserved.

This file and modified versions of this file are supplied to specific individuals or organisations under the following 
provisions:

- This file, or any files that comprise the MMBasic source (modified or not), may not be distributed or copied to any other
  person or organisation without written permission.

- Object files (.o and .hex files) generated using this file (modified or not) may not be distributed or copied to any other
  person or organisation without written permission.

- This file is provided in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of 
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

************************************************************************************************************************/

// Global variables provided by Video.c and DrawChar.c
extern int VRes, HRes;												// Global vert and horiz resolution in pixels on the screen
extern int screenWidth, screenHeight;								// Global vert and horiz resolution in the currently selected font
extern int VBuf, HBuf;												// Global vert and horiz resolution of the video buffer
extern int MMPosX, MMPosY;
extern int MMCharPos;
extern int ListCnt;													// line count used by the LIST and FILES commands
extern int vga;														// true if we are using the internal monochrome vga
extern int fontNbr, fontWidth, fontHeight, fontScale;

#if defined(COLOUR)
    #define BLACK   0b000
    #define BLUE    0b001
    #define GREEN   0b010
    #define CYAN    0b011
    #define RED     0b100
    #define PURPLE  0b101
    #define YELLOW  0b110
    #define WHITE   0b111
    extern int ModeC, ModeP;                                        // set by the mode command
    extern int *VideoBufRed;										// video buffer for red
    extern int *VideoBufGrn;										// video buffer for green
    extern int *VideoBufBlu;										// video buffer for blue
    extern int DefaultFgColour, DefaultBgColour;                    // default colour for text output
    extern int CurrentFgColour, CurrentBgColour;                    // current colour for text output
    extern int ConsoleFgColour, ConsoleBgColour;                    // current colour for text output at the command prompt
    extern char *CLine;
    extern void plotx(int x, int y, int con, int coff);
    extern void SelectChannel(int colour, int *con, int *coff);
#else
    extern int *VideoBuf;											// video buffer for monochrome
    #define DefaultFgColour 1
#endif

#define VCHARS (VRes / (fontHeight * fontScale))                    // number of lines in a screenfull

#define NBRFONTS	10

struct s_font {
	void *p;
	char width, height;
	char start, end;
};

extern struct s_font ftbl[NBRFONTS];	

// Facilities provided by Video.c and DrawChar.c
extern void MMCursor(int b);
extern void MMcls(void);
extern void initVideo(void);
//extern void haltVideo(void);
extern void plot(int x, int y, int b); 
extern int pixel(int x, int y);
extern void MMline(int x1, int y1, int x2, int y2, int colour) ;
extern void MMCircle(int x, int y, int radius, int fill, int colour, float aspect) ;
extern void MMbox(int x1, int y1, int x2, int y2, int fill, int colour);
extern void SetMode(int mc, int mp);

//extern void initDrawChar(void);
extern void VideoPutc(char c);
extern void DisplayString(char *p);
extern void SetFont(int font, int scale, int reverse);
extern void initFont(void);
extern void UnloadFont(int font);
extern void ScrollUp(int res);

// cursor definition
void ShowCursor(int show);
typedef enum {C_OFF = 0, C_STANDARD, C_INSERT } Cursor_e ;
extern Cursor_e Cursor;
extern int AutoLineWrap;		// true if auto line wrap is on
extern int PrintPixelMode;      // global used to track the pixel mode when we are printing

// definitions related to setting NTSC and PAL options
#if defined(COLOUR) || defined(DUINOMITE)
    #define CONFIG_DISABLED	0b111   // disabled is the default for the Colour Maximite and Duinomite
    #define CONFIG_PAL		0b001
    #define CONFIG_NTSC		0b010
#else
    #define CONFIG_PAL		0b111   // PAL is the default for the Maximite
    #define CONFIG_NTSC		0b001
    #define CONFIG_DISABLED	0b010
#endif
extern const unsigned int PalVgaOption;

// definitions related to setting video off and on
#define CONFIG_ON		0b111
#define CONFIG_OFF		0b010
extern const unsigned int VideoOption;

extern char putPicaso(int len, char *p);

