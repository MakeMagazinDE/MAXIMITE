/************************************************************************************************************************
Maximite

Video.c modified for use with 4.3" PSP TFT, Sharp LQ043 by Carsten Meyer, cm@ct.de

The video generator is based on an idea and code by Lucio Di Jasio presented in his excellent book
"Programming 32-bit Microcontrollers in C - Exploring the PIC32".
The colour technique and some of the code was developed by Dr Kilian Singer.

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


#include <p32xxxx.h>
#include <plib.h>
#include <string.h>

#include "MMBasic_Includes.h"
#include "Hardware_Includes.h"

// Parameters for VGA video with 31.5KHz horizontal scanning and 60Hz vertical refresh
// Graphics is 480x432 pixels.  This gives us 80 chars per line and 36 lines per screen
/*
#define VGA_VRES	 	432											// Vert graphics resolution (pixels)
#define VGA_HRES	 	480											// Horiz graphics resolution (pixels)
#define VGA_LINE_N   	525        									// number of lines in VGA frame
#define VGA_LINE_T   	2540       									// Tpb clock in a line (31.777us)
#define VGA_VSYNC_N  	2          									// V sync lines
#define VGA_VBLANK_N 	(VGA_LINE_N - VGA_VRES - VGA_VSYNC_N)  		// Nbr of blank lines
#define VGA_PREEQ_N  	((VGA_VBLANK_N/2) - 12)         			// Nbr blank lines at the bottom of the screen
#define VGA_POSTEQ_N 	VGA_VBLANK_N - VGA_PREEQ_N  				// Nbr blank lines at the top of the screen
#define VGA_PIX_T    	4          									// Tpb clock per pixel
#define VGA_HSYNC_T  	300        									// Tpb clock width horizontal pulse
#define VGA_BLANKPAD 	5											// number of zero bytes before sending data
*/

//##########################################################################################################################
// Parameters for PSP Display
// Graphics is 480x272 pixels.  This gives us 80 chars per line and 22 lines per screen
//#define TFT_HANN
#define TFT_PSP

#define VGA_VRES	 	272											// 272 Vert graphics resolution (pixels)
#define VGA_HRES	 	480											// 480 Horiz graphics resolution (pixels)
#define VGA_LINE_N   	286        									// 286 number of lines in VGA frame
#define VGA_PIX_T    	10         									// 10 Tpb clock per pixel => 8 MHz, 125 ns
#define VGA_LINE_T   	544*VGA_PIX_T								// 544 clock cycles in a line (66 us)
#define VGA_VSYNC_N  	10          								// 10 V sync lines
#define VGA_VBLANK_N 	14  										// 14 Nbr of blank lines
#define VGA_PREEQ_N  	2         									// 2 Nbr blank lines at the bottom of the screen
#define VGA_POSTEQ_N 	3 											// PSP,HANN: 2 Nbr blank lines at the top of the screen
#define VGA_HSYNC_T  	40*VGA_PIX_T                   				// HSYNC width), 41 SPI clock cycles width horizontal pulse

//##########################################################################################################################

// Common paramaters for Composite video
#define C_PIX_T      	10          								// Tpb clock per pixel
#define C_HSYNC_T    	374        									// Tpb clock width horizontal pulse
#define C_BLANKPAD   	8											// number of zero words (4 bytes each) before sending data
#define C_VSYNC_N    	3          									// V sync lines

// Parameters for PAL composite video
// Graphics is 304x216 pixels.  This gives us 50 chars per line and 18 lines per screen
#define PAL_VRES	    216											// Vert graphics resolution (pixels)
#define PAL_HRES	    304											// Horiz graphics resolution (pixels)
#define PAL_LINE_N     	312        									// number of lines in PAL frames
#define PAL_LINE_T     	5120       									// Tpb clock in a line (64us) 5115 //
#define PAL_VBLANK_N   	(PAL_LINE_N - PAL_VRES - C_VSYNC_N)  		// Nbr of blank lines
#define PAL_PREEQ_N    	((PAL_VBLANK_N/2) - 8)         				// Nbr blank lines at the bottom of the screen
#define PAL_POSTEQ_N   	(PAL_VBLANK_N - PAL_PREEQ_N)  				// Nbr blank lines at the top of the screen

// Parameters for NTSC composite video
// Graphics is 304x180 pixels.  This gives us 50 chars per line and 15 lines per screen
#define NTSC_VRES     	180											// Vert graphics resolution (pixels)
#define NTSC_HRES	    304											// Horiz graphics resolution (pixels)
#define NTSC_LINE_N     262	        								// number of lines in NTSC frames
#define NTSC_LINE_T     5080       									// Tpb clock in a line (63.55us)
#define NTSC_VBLANK_N   (NTSC_LINE_N - NTSC_VRES - C_VSYNC_N)  		// Nbr of blank lines
#define NTSC_PREEQ_N    ((NTSC_VBLANK_N/2) - 8)         			// Nbr blank lines at the bottom of the screen
#define NTSC_POSTEQ_N   (NTSC_VBLANK_N - NTSC_PREEQ_N)  			// Nbr blank lines at the top of the screen


// states of the vertical sync state machine
#define SV_PREEQ    0												// generating blank lines before the vert sync
#define SV_SYNC     1												// generating the vert sync
#define SV_POSTEQ   2												// generating blank lines after the vert sync
#define SV_LINE     3												// visible lines, send the video data out

// global variables used in other parts of the Maximite
int VRes, HRes;														// Global vert and horiz resolution in pixels on the screen
int screenWidth, screenHeight;

// define the video buffer
// note that this can differ from the pixel resolution, for example for composite HRes is not an even multiple of 32 where HBuf is
int VBuf, HBuf;														// Global vert and horiz resolution of the video buffer

#if defined(COLOUR)
    int *VideoBufRed = NULL;										// Image buffer for red
    int *VideoBufGrn = NULL;										// Image buffer for green
    int *VideoBufBlu = NULL;										// Image buffer for blue
    int ModeC, ModeP;                                               // set by the mode command
    int DefaultFgColour, DefaultBgColour;                           // default colour for text output
    int CurrentFgColour, CurrentBgColour;                           // current colour for text output
    int ConsoleFgColour, ConsoleBgColour;                           // current colour for text output at the command prompt
    char *CLine;                                                    // used for scan line colour control
#else
    int *VideoBuf = NULL;											// Image buffer for monochrome Maximite
#endif

int vga;															// true if we are using the internal VGA video
unsigned int SvSyncT;												// used to determine the timing for the SV_SYNC state;

volatile int VCount;												// counter for the number of lines in a frame
volatile int VState;												// the state of the state machine

int VS[4] = { SV_SYNC, SV_POSTEQ, SV_LINE, SV_PREEQ};				// the next state table
int VC[4];															// the next counter table (initialise in initVideo() below)

int zero[] = {0, 0, 0};

volatile int FlashSync;
const unsigned int PalVgaOption = 0xffffffff;						// use to hold the pal/vga setting in flash
const unsigned int VideoOption = 0xffffffff;						// use to hold the video off/on setting in flash

// configure a SPI channel and its associated DMA feed
void SetupSPI (int SPIchnl, int SPIint, int SPIpixt, void *SPIinput, int DMAchnl, int DMAsize, int *VBuffer) {
	// setup the DMA to send data to the SPI channels
    DmaChnOpen(DMAchnl, DMAchnl, DMA_OPEN_DEFAULT);
    DmaChnSetEventControl(DMAchnl, DMA_EV_START_IRQ_EN | DMA_EV_START_IRQ(SPIint));
	DmaChnSetTxfer(DMAchnl, (void*)VBuffer, (void *)SPIinput, DMAsize, 4, 4);


    // due to a bug in the PIC32 SPI (Microchip Errata #13) one or more
    // nops may or may not be required to get the colours into proper
    // registration on the VGA monitor.  Try compiling with the
    // following lines commented out or not commented - one of the
    // combinations should ensure that the colours are registered properly
    while(ReadCoreTimer() & 0b11111) nop;                            // ensure that each colour is started on the same clock phase
    nop;
    nop;
    nop;
    nop;
    // nop;
    
    // Due to a bug in the PIC32 SPI (Microchip Errata #13) each SPI channel (red, green, blue) can start at a slightly
    // different times causing the colours to not correctly align (ie, colour fringing on white text).
    // To correct for this we have to wait for the rising edge of the framing pulse then time the SPI open (using nops)
    // so that in future the framing input will not be coincident with a SPI clock.  The number of nops required will
    // vary with each compile so you will need to experiment by commenting/uncommenting the nops below.
    //SpiChnClose(SPIchnl);
    //while(PORTDbits.RD2);                                           // wait for the start of the synch pulse
    //while(!PORTDbits.RD2);                                          // wait for it to go high again
    //nop;
    //nop;
    //nop;
    //nop;
    
	// enable the SPI channel(s) which will clock the video data out.  Set master and framing mode.  The last arg sets the speed
    SpiChnOpen(SPIchnl, SPICON_ON | SPI_OPEN_ENHBUF | SPI_OPEN_TBE_NOT_FULL | SPICON_MSTEN |  SPICON_MODE32 | SPICON_FRMEN | SPICON_FRMSYNC | SPICON_FRMPOL, SPIpixt);
}    


/**************************************************************************************************
Initialise the video components
***************************************************************************************************/
void initVideo( void) {
	
	// first check if video output is disables, if so we can skip nearly everything
    if(GetFlashOption(&VideoOption) == CONFIG_OFF) {
        #if defined(COLOUR)
    	    VideoBufRed = VideoBufGrn = VideoBufBlu = NULL;         // set up the video pointer buffer but use zero size 
    	#else
    	    VideoBuf = NULL;                                        // set up the video pointer buffer but use zero size 
        #endif
	    VRes = VGA_VRES; HRes = VGA_HRES; VBuf = HBuf = 0;
	    return;										
	}

// check if we need to be in the VGA or composite mode
//    CNCONbits.ON = 1;       									    // turn on Change Notification module
//    P_VGA_COMP_PULLUP = P_ON;									    // turn on the pullup for pin C14 also called CN0
//    uSec(5);                                                        // allow the pullup to do its job
    vga = 1; // (P_VGA_COMP == P_VGA_SELECT) || (GetFlashOption(&PalVgaOption) == CONFIG_DISABLED);	// vga will be true if the jumper is NOT there or composite is disabled
    
    P_HORIZ_TRIS = P_OUTPUT;										// Horiz sync output
    P_VERT_TRIS = P_OUTPUT;											// Vert sync output used by VGA
    
        // set up for VGA output
	    VBuf = VRes = VGA_VRES;                                     // VBuf is the vert buffer size and VRes is that actual number of lines to use
	    HRes = VGA_HRES;                                            // HRes is the number of horiz pixels to use
	    HBuf = (((VGA_HRES + 31) / 32) * 32);                       // HBuf is the horiz buffer size
		VC[0] = VGA_VSYNC_N;										// setup the table used to count lines
		VC[1] = VGA_POSTEQ_N;
		VC[2] = VGA_VRES;
		VC[3] = VGA_PREEQ_N;

//##########################################################################################################################
// #define P_VID_OC_OPEN       OpenOC3					// the function used to open the output compare (see video.c)
// #define P_VID_OC_REG        OC3R                    // the output compare register
// enable the output compare which is used to time the width of the horiz sync pulse, OC3RS, OC3R
	    OpenOC3(OC_ON | OC_TIMER3_SRC | OC_CONTINUE_PULSE, 0, VGA_HSYNC_T);	
// enable timer 3 and set to the horizontal scanning frequency: (module ON  Prescaler  Internal Source, Timer Value)
	    OpenTimer3( T3_ON | T3_PS_1_1 | T3_SOURCE_INT, VGA_LINE_T-1);	    
//##########################################################################################################################
    mT3SetIntPriority(7);    										// set priority level 7 for the timer 3 interrupt to use shadow register set
    mT3IntEnable(1);												// Enable Timer3 Interrupt

    m_alloc(M_VIDEO, (VBuf * HBuf) / 8);                            // set up the video base (default) buffer - this is Red in the case of the colour MM
	VState = SV_PREEQ;												// initialise the state machine
    VCount = 1;                 									// set the count so that the first interrupt will increment the state
    screenWidth = HRes / (fontWidth * fontScale);
        
	CLine = NULL;
	ConsoleBgColour = CurrentBgColour = DefaultBgColour = 0;
	ModeC = 3;  ModeP = 0;                                              // set the default mode for VGA to all eight colours
	VideoBufGrn = getmemory((VBuf * HBuf) / 8);                         // allocate the additional buffers for green and blue in normal memory (the heap)    
	VideoBufBlu = getmemory((VBuf * HBuf) / 8);
	ConsoleFgColour = CurrentFgColour = DefaultFgColour = 7;            // default colour for text output
	SetupSPI(P_RED_SPI, P_RED_SPI_INTERRUPT, VGA_PIX_T, (void *)&P_RED_SPI_INPUT, 1, HBuf/8, VideoBufRed); // set up the Red channel
	SetupSPI(P_GRN_SPI, P_GRN_SPI_INTERRUPT, VGA_PIX_T, (void *)&P_GRN_SPI_INPUT, 0, HBuf/8, VideoBufGrn); // set up the Green channel
	SetupSPI(P_BLU_SPI, P_BLU_SPI_INTERRUPT, VGA_PIX_T, (void *)&P_BLU_SPI_INPUT, 3, HBuf/8, VideoBufBlu); // set up the Blue channel
}



/**************************************************************************************************
Timer 3 interrupt.
Used to generate the horiz and vert sync pulse under control of the state machine
modified for TFT use by cm
***************************************************************************************************/
void __ISR(_TIMER_3_VECTOR, ipl7) T3Interrupt(void) {
    #if defined(COLOUR)
	    static int *VPtrRed;
	    static int *VPtrGrn;
	    static int *VPtrBlu;
	    static int lcnt;
    #else
	    static int *VPtr;
    #endif

    switch ( VState) {    											// vertical state machine

#if defined(TFT_PSP)

        case SV_PREEQ:  // 0            							// prepare for the new horizontal line
                VPtrRed = VideoBufRed;
                VPtrGrn = VideoBufGrn;
                VPtrBlu = VideoBufBlu;
                lcnt = 0;
            break;
        case SV_SYNC:   // 1
            P_VERT_SET_LO;										    // start the vertical sync pulse for vga
            break;

        case SV_POSTEQ: // 2
            P_VERT_SET_HI;									        // end of the vertical sync pulse for vga
            break;

        case SV_LINE:   // 3
                if(VPtrRed) {
                   	DCH1SSA = KVA_TO_PA((void*) (VPtrRed));			// update the DMA1 source address (DMA1 is used for VGA data)
                    if(ModeC != 4 || lcnt & 1) VPtrRed += HBuf/32;	// move the pointers to the start of the next line (if mode 4 do this on uneven lines only)
                    if(CLine) {										 // depending on the scan line setting for red, enable the DMA transfer
                        if(CLine[lcnt] & RED) DmaChnEnable(1);
                    } else
                        DmaChnEnable(1);							// arm the DMA transfer
                }    
                if(VPtrGrn) {
                    DCH0SSA = KVA_TO_PA((void*) (VPtrGrn));         // update the DMA0 source address used for green
                    if(ModeC != 4 || lcnt & 1) VPtrGrn += HBuf/32;	// move the pointers to the start of the next line (if mode 4 do this on uneven lines only)
                    if(CLine) {                                     // depending on the scan line setting for green, enable the DMA transfer
                        if(CLine[lcnt] & GREEN) DmaChnEnable(0);
                    } else
                        DmaChnEnable(0);
                }    
                if(VPtrBlu) {
                    DCH3SSA = KVA_TO_PA((void*) (VPtrBlu));         // update the DMA3 source address used for blue
                    if(ModeC != 4 || lcnt & 1) VPtrBlu += HBuf/32;	// move the pointers to the start of the next line (if mode 4 do this on uneven lines only)
                    if(CLine) {                                     // depending on the scan line setting for blue, enable the DMA transfer
                        if(CLine[lcnt] & BLUE) DmaChnEnable(3);
                    } else
                        DmaChnEnable(3);
                    lcnt++;
                }    
            break;
#endif

#if defined(TFT_HANN)
        case SV_PREEQ:  // 0            							// prepare for the new horizontal line
                VPtrRed = VideoBufRed;
                VPtrGrn = VideoBufGrn;
                VPtrBlu = VideoBufBlu;
                lcnt = 0;
            break;

        case SV_SYNC:   // 1
            P_VERT_SET_LO;										    // start the vertical sync pulse for vga
            break;

        case SV_POSTEQ: // 2
            break;

        case SV_LINE:   // 3
				P_VERT_SET_HI;									    // end of the vertical sync pulse for vga
                if(VPtrRed) {
                   	DCH1SSA = KVA_TO_PA((void*) (VPtrRed));     // update the DMA1 source address (DMA1 is used for VGA data)
                    if(ModeC != 4 || lcnt & 1) VPtrRed += HBuf/32;	// move the pointers to the start of the next line (if mode 4 do this on uneven lines only)
                    if(CLine) {                                     // depending on the scan line setting for red, enable the DMA transfer
                        if(CLine[lcnt] & RED) DmaChnEnable(1);
                    } else
                        DmaChnEnable(1);							// arm the DMA transfer
                }    
                if(VPtrGrn) {
                    DCH0SSA = KVA_TO_PA((void*) (VPtrGrn));         // update the DMA0 source address used for green
                    if(ModeC != 4 || lcnt & 1) VPtrGrn += HBuf/32;	// move the pointers to the start of the next line (if mode 4 do this on uneven lines only)
                    if(CLine) {                                     // depending on the scan line setting for green, enable the DMA transfer
                        if(CLine[lcnt] & GREEN) DmaChnEnable(0);
                    } else
                        DmaChnEnable(0);
                }    
                if(VPtrBlu) {
                    DCH3SSA = KVA_TO_PA((void*) (VPtrBlu));         // update the DMA3 source address used for blue
                    if(ModeC != 4 || lcnt & 1) VPtrBlu += HBuf/32;	// move the pointers to the start of the next line (if mode 4 do this on uneven lines only)
                    if(CLine) {                                     // depending on the scan line setting for blue, enable the DMA transfer
                        if(CLine[lcnt] & BLUE) DmaChnEnable(3);
                    } else
                        DmaChnEnable(3);
                    lcnt++;
                }    
            break;
#endif
   }

    if (--VCount == 0) {											// count down the number of lines
        VCount = VC[VState&3];										// set the new count
        VState = VS[VState&3];    									// and advance the state machine

    }

    mT3ClearIntFlag();    											// clear the interrupt flag
}



#if defined(COLOUR)


/**************************************************************************************************
Calculate the colour channels to turn on or off for a specified colour
  colour = the colour requested
Returns:
  *con  = channels to turn on (bit 2 = red, 1 = green and bit 0 = blue)
  *coff = channels to turn off (as above)
***************************************************************************************************/
void SelectChannel(int colour, int *con, int *coff) {
    int x;
    
    static char m2map[7][8] = {                                     // map 3 bit colours to the 2 bit palette selected by the current MODE command
    // BLK  BLU  GRN  CYAN RED  PURP YELL WHITE
    // 000  001  010  011  100  101  110  111
      { 0,   0,   0,   0,   0,   0,   0,   0 },                     // blank padding
      { 0,   2,   2,   2,   4,   2,   6,   6 },                     // MODE 2, 1
      { 0,   1,   1,   1,   4,   5,   4,   5 },                     // MODE 2, 2
      { 0,   1,   1,   1,   4,   1,   4,   5 },                     // MODE 2, 3
      { 0,   1,   2,   3,   2,   1,   2,   3 },                     // MODE 2, 4
      { 0,   4,   2,   2,   2,   4,   2,   6 },                     // MODE 2, 5
      { 0,   1,   4,   1,   4,   1,   4,   5 }                      // MODE 2, 6
    };    
    
    if(colour < 0) {
        // if the request is to invert the video
        switch(ModeC) {
            case 4:
            case 3:
                *con = 7;                                           // in mode 3 or 4 all three channels should be inverted
                break;
            case 2:
                *con = m2map[ModeP][7];                             // lookup the bits that need to be toggled in mode 3
                break;
            case 1:
                if(ModeP & RED)                                     // in mode 1 just find one channel that is in use
                    *con = RED;
                else if(ModeP & GREEN)
                    *con = GREEN;
                else
                    *con = BLUE;
                break;
       }
        *coff = -1;                                                 // this just means that we invert the bits
        return;
    }
    
    // the request is for normal video output
    if(colour > 7) colour = 7;
    switch(ModeC) {
        case 4:
            break;
        case 3:
            *con = colour;                                          // in mode 3 or 4 all three channels are used
            *coff = ((~colour) & 7);
            break;
        case 2:
            *con = m2map[ModeP][colour];                            // map colours to that available in the 2 bit palette
            *coff = *con ^ m2map[ModeP][7];                         // and figure out what bits need to be turned off
            break;
        case 1:
            if(ModeP & RED)                                         // in mode 1 just find one channel that is in use
                x = RED;
            else if(ModeP & GREEN)
                x = GREEN;
            else
                x = BLUE;
            if(colour)
                { *con = x; *coff = 0; }
            else
                { *con = 0; *coff = x; }
            break;
   }
}   




/**************************************************************************************************
Turn on or off a single pixel in the graphics buffer
This is for colour only and it expects the caller to have mapped the colour channels as follows:
  con  = channels to turn on (bit 2 = red, 1 = green and bit 0 = blue)
  coff = channels to turn off (as above)
***************************************************************************************************/
void plotx(int x, int y, int con, int coff) {
    int w, xx;

    if(x >= 0 && x < HRes && y >= 0 && y < VRes && HBuf) {
        if(ModeC == 4) x += 16;     
        w = y * (HBuf/32) + x/32;
        xx = (0x80000000>>(x & 0x1f));
        
        if(coff == -1) {        
            // invert the pixels
            if(con & RED) VideoBufRed[w] ^= xx;                         // invert the red pixel
            if(con & GREEN) VideoBufGrn[w] ^= xx;                       // invert the green pixel
            if(con & BLUE) VideoBufBlu[w] ^= xx;                        // invert the blue pixel
        } else {                
            // draw the pixel
            if(con & RED) VideoBufRed[w] |= xx;                         // turn on the red pixel
            if(con & GREEN) VideoBufGrn[w] |= xx;                       // turn on the green pixel
            if(con & BLUE) VideoBufBlu[w] |= xx;                        // turn on the blue pixel
            xx = ~xx;
            if(coff & RED) VideoBufRed[w] &= xx;                        // turn off the red pixel
            if(coff & GREEN) VideoBufGrn[w] &= xx;                      // turn off the green pixel
            if(coff & BLUE) VideoBufBlu[w] &= xx;                       // turn off the blue pixel
        }    
    }    
}


/**************************************************************************************************
Turn on or off a single pixel in the graphics buffer
This is the colour version
***************************************************************************************************/
void plot(int x, int y, int b) {
    int coff, con;
    SelectChannel(b, &con, &coff);
    plotx(x, y, con, coff);
}

#endif    //COLOUR



#if !defined(COLOUR)
/**************************************************************************************************
Turn on or off a single pixel in the graphics buffer
This is the monochrome version
***************************************************************************************************/
void plot(int x, int y, int b) {
    
    if(x >= 0 && x < HRes && y >= 0 && y < VRes && HBuf) {
      	if(b == 0)
            VideoBuf[y * (HBuf/32) + x/32] &= ~(0x80000000>>(x & 0x1f));   		    // turn off the pixel
        else if(b == -1)
        	VideoBuf[y * (HBuf/32) + x/32] ^= (0x80000000>>(x& 0x1f));			    // invert the pixel
        else
    	    VideoBuf[y * (HBuf/32) + x/32] |= (0x80000000>>(x & 0x1f));			    // turn on the pixel
    }
}
#endif   // not COLOUR

    

/**************************************************************************************************
Get the value of a single pixel in the graphics buffer
***************************************************************************************************/
int pixel(int x, int y) {
    int p = 0;

    if(x >= 0 && x < HRes && y >= 0 && y < VRes && HBuf) {
        #if defined(COLOUR)
            if(ModeC == 4) x += 16;     
		    if(VideoBufRed) p |= ((VideoBufRed[y * (HBuf/32) + x/32]) & (0x80000000>>(x & 0x1f))) ? RED:0;
		    if(VideoBufGrn) p |= ((VideoBufGrn[y * (HBuf/32) + x/32]) & (0x80000000>>(x & 0x1f))) ? GREEN:0;
		    if(VideoBufBlu) p |= ((VideoBufBlu[y * (HBuf/32) + x/32]) & (0x80000000>>(x & 0x1f))) ? BLUE:0;
		    if(ModeC == 1 && p != 0) p = 1;                                             // monochrome Maximite compatibility mode
        #else
		    p = ((VideoBuf[y * (HBuf/32) + x/32]) & (0x80000000>>(x & 0x1f))) ? 1:0;
        #endif
    }
    return p;
}




/**************************************************************************************************
scroll the screen
if edit = true then do not scroll the bottom two lines and do not clear the new line (for the editor)
***************************************************************************************************/
void ScrollUp(int edit) {
   	int i;
	int *pd;
   	int *ps;
   	int res;
   	
    #if defined(COLOUR)
   	    int cnt = 0;
    #endif
   	
   	if(edit)
   	    res = 3;        // for the editor - reserve the bottom two lines
   	else
   	    res = 1;        // for DrawChar() - scroll everything
       	
    #if defined(COLOUR)
        if(VideoBufRed) {
    		pd = VideoBufRed;
           	ps = pd + (HBuf/32) * (fontHeight * fontScale);
           	for(i=0; i<(HBuf/32) * (VBuf - (fontHeight * fontScale) * res); i++) *pd++ = *ps++;	    // scroll up
           	if(!edit) for(i=0; i<(HBuf/32) * (fontHeight * fontScale); i++) *pd++ = 0;	            // clear the new line
           	if(++cnt == ModeC) return;
       	}
        if(VideoBufGrn && VideoBufGrn != VideoBufRed) {
    		pd = VideoBufGrn;
           	ps = pd + (HBuf/32) * (fontHeight * fontScale);
           	for(i=0; i<(HBuf/32) * (VBuf - (fontHeight * fontScale) * res); i++) *pd++ = *ps++;	    // scroll up
           	if(!edit) for(i=0; i<(HBuf/32) * (fontHeight * fontScale); i++) *pd++ = 0;	            // clear the new line
           	if(++cnt == ModeC) return;
       	}

		pd = VideoBufBlu;
       	ps = pd + (HBuf/32) * (fontHeight * fontScale);
       	for(i=0; i<(HBuf/32) * (VBuf - (fontHeight * fontScale) * res); i++) *pd++ = *ps++;	    // scroll up
       	if(!edit) for(i=0; i<(HBuf/32) * (fontHeight * fontScale); i++) *pd++ = 0;	            // clear the new line
 	
    #else
		pd = VideoBuf;
       	ps = pd + (HBuf/32) * (fontHeight * fontScale);
       	for(i=0; i<(HBuf/32) * (VBuf - (fontHeight * fontScale) * res); i++) *pd++ = *ps++;	        // scroll up
       	if(!edit) for(i=0; i<(HBuf/32) * (fontHeight * fontScale); i++) *pd++ = 0;	                // clear the new line
    #endif
}




/**************************************************************************************************
clear the screen
***************************************************************************************************/
void MMcls(void) {
    #if defined(COLOUR)
   	    int cnt = 0;
   	    int *p;
   	    char *pp;
    #endif
	lastx = lasty = MMPosX = MMPosY = 0;
	MMCharPos = 1;	
    #if defined(COLOUR)
	    if(VideoBufRed) {
            memset(VideoBufRed, (DefaultBgColour & RED)?0xff:0, VBuf * (HBuf/8));           // set the background
            if(ModeC == 4)                                                                  // for mode 4 we must zero the first 16 bits
                for(p = VideoBufRed; p < VideoBufRed + (VBuf * (HBuf/32)); p += HBuf/32) {  // for each line
                    pp = (char *)p;                                                         // point to the start of the line
                    pp[2] = 0; pp[3] = 0;                                                   // zero the first 16 bits
                }
	        if(++cnt == ModeC) return;
	        }
	    if(VideoBufGrn && VideoBufGrn != VideoBufRed) {
	        memset(VideoBufGrn, (DefaultBgColour & GREEN)?0xff:0, VBuf * (HBuf/8));         // set the background
            if(ModeC == 4)                                                                  // for mode 4 we must zero the first 16 bits
                for(p = VideoBufGrn; p < VideoBufGrn + (VBuf * (HBuf/32)); p += HBuf/32) {  // for each line
                    pp = (char *)p;                                                         // point to the start of the line
                    pp[2] = 0; pp[3] = 0;                                                   // zero the first 16 bits
                }
	        if(++cnt == ModeC) return;
	        }
	    memset(VideoBufBlu, (DefaultBgColour & BLUE)?0xff:0, VBuf * (HBuf/8));              // set the background
        if(ModeC == 4)                                                                      // for mode 4 we must zero the first 16 bits
            for(p = VideoBufBlu; p < VideoBufBlu + (VBuf * (HBuf/32)); p += HBuf/32) {      // for each line
                pp = (char *)p;                                                             // point to the start of the line
                pp[2] = 0; pp[3] = 0;                                                       // zero the first 16 bits
            }
    #else
	    memset(VideoBuf, 0, VBuf*(HBuf/8));
    #endif
	
}



#if defined(COLOUR)
// function to OR two video bitmaps - only used in cmd_mode below
void ORbitmaps(int *pd, int *ps) {
    int i;
   	for(i = 0; i < (VBuf*HBuf)/32; i++) pd[i] |= ps[i];
}



// switch the colour mode
// mc = colour mode
// mp = colour palette
void SetMode(int mc, int mp) {
	int *tp1, *tp2, *tp3;
	
    tp1 = tp2 = tp3 = NULL;
    if(!vga) return;
    
    FreeHeap(CLine);
    CLine = NULL;

    // using the previous mode as a guide set the pointers tp1, tp2 and tp3 to any memory that has been
    // allocated as video buffers or NULL if no memory was allocated
    switch(ModeC) {
        case 1:
        	if(VideoBufRed) tp1 = VideoBufRed;
        	if(VideoBufGrn) tp1 = VideoBufGrn;
        	if(VideoBufBlu) tp1 = VideoBufBlu;
        	break;
        case 2:
            switch(ModeP) {
                case 1: tp1 = VideoBufRed;
                        tp2 = VideoBufGrn;
                        break;
                case 2: tp1 = VideoBufRed;
                        tp2 = VideoBufBlu;
                        break;
                case 3: tp1 = VideoBufRed;
                        tp2 = VideoBufGrn;
                        break;
                case 4: tp1 = VideoBufGrn;
                        tp2 = VideoBufBlu;
                        break;
                case 5: tp1 = VideoBufRed;
                        tp2 = VideoBufGrn;
                        break;
                case 6: tp1 = VideoBufRed;
                        tp2 = VideoBufBlu;
                        break;
            }
            break;
        case 3:
            if(mc == 3) return;                                     // no need to do anything if we are already in the same mode
            tp1 = VideoBufRed;
            tp2 = VideoBufGrn;
            tp3 = VideoBufBlu;
            break;
        case 4:
            if(mc == 4) return;                                     // no need to do anything if we are already in the same mode
    		VBuf = VRes = VGA_VRES;
	        HRes = VGA_HRES;
	        HBuf = (((HRes + 31) / 32) * 32);
            VideoBufGrn = VideoBufBlu = tp1 = VideoBufRed;
            MMcls();
            // switch back to normal horizontal pixel clock speed
            SetupSPI(P_RED_SPI, P_RED_SPI_INTERRUPT, VGA_PIX_T, (void *)&P_RED_SPI_INPUT, 1, HBuf/8, VideoBufRed); // set up the Red channel
            SetupSPI(P_GRN_SPI, P_GRN_SPI_INTERRUPT, VGA_PIX_T, (void *)&P_GRN_SPI_INPUT, 0, HBuf/8, VideoBufGrn); // set up the Green channel
            SetupSPI(P_BLU_SPI, P_BLU_SPI_INTERRUPT, VGA_PIX_T, (void *)&P_BLU_SPI_INPUT, 3, HBuf/8, VideoBufBlu); // set up the Blue channel
            break;
    }        
    
    // at this point tp1 ALWAYS points to the permanent video buffer below PMemory and is NEVER set to NULL
    // tp2 and tp3 may point to valid memory depending on the previous mode.  If not, they are set to NULL
    
    // now, switch to the new mode
	switch(mc) {
    	case 1:
        	// monochrome Maximite compatible mode
        	VideoBufRed = (mp & RED) ? tp1 : NULL;
        	VideoBufGrn = (mp & GREEN) ? tp1 : NULL;
        	VideoBufBlu = (mp & BLUE) ? tp1 : NULL;
        	if(tp2) { ORbitmaps(tp1, tp2);  FreeHeap(tp2); }
        	if(tp3) { ORbitmaps(tp1, tp3);  FreeHeap(tp3); } 
        	CurrentFgColour = DefaultFgColour = mp;
        	CurrentBgColour = DefaultBgColour = BLACK;
        	break;
        case 2:
        // two bit colour mode.  made more complicated by the palette selection
            if(tp2 == NULL) tp2 = getmemory((VBuf * HBuf) / 8);
            if(tp3) { 
                ORbitmaps(tp2, tp3);
                FreeHeap(tp3);
            }
            ORbitmaps(tp1, tp2);
            ORbitmaps(tp2, tp1);

            switch(mp) {
                case 1: VideoBufRed = tp1;
                        VideoBufGrn = tp2;
                        VideoBufBlu = NULL;
                        break;
                case 2: VideoBufRed = tp1;
                        VideoBufGrn = NULL;
                        VideoBufBlu = tp2;
                        break;
                case 3: VideoBufRed = tp1;
                        VideoBufGrn = tp2;
                        VideoBufBlu = tp2;
                        break;
                case 4: VideoBufRed = NULL;
                        VideoBufGrn = tp1;
                        VideoBufBlu = tp2;
                        break;
                case 5: VideoBufRed = tp1;
                        VideoBufGrn = tp2;
                        VideoBufBlu = tp1;
                        break;
                case 6: VideoBufRed = tp1;
                        VideoBufGrn = tp1;
                        VideoBufBlu = tp2;
                        break;
            }
            break;
        case 3:
            // 3 bit colour mode
            if(ModeC == 2) {
                VideoBufRed = tp1;
                VideoBufGrn = tp2;
                ORbitmaps(tp1, tp2);
                ORbitmaps(tp2, tp1);
                VideoBufBlu = getmemory((VBuf * HBuf) / 8);
                ORbitmaps(VideoBufBlu, tp1);
            }    
            if(ModeC == 1 || ModeC == 4) {
                VideoBufRed = tp1;
                VideoBufGrn = getmemory((VBuf * HBuf) / 8);
                VideoBufBlu = getmemory((VBuf * HBuf) / 8);
                ORbitmaps(VideoBufGrn, tp1);
                ORbitmaps(VideoBufBlu, tp1);
            }    
    	    break;
    	case 4:
    	    // 3 bit colour, 240x216 pixels
    		VBuf = VRes = VGA_VRES/2;
	        HRes = VGA_HRES/2;
	        HBuf = (((HRes + 31) / 32) * 32) + 32;
        	if(tp2) FreeHeap(tp2);
        	if(tp3) FreeHeap(tp3);
            VideoBufRed = tp1;
            VideoBufGrn = (int *)((int)VideoBufRed + ((VBuf * HBuf) / 8));
            VideoBufBlu = (int *)((int)VideoBufGrn + ((VBuf * HBuf) / 8));
            MMcls();
            // set the horizontal pixel clock speed to half normal
            SetupSPI(P_RED_SPI, P_RED_SPI_INTERRUPT, VGA_PIX_T*2, (void *)&P_RED_SPI_INPUT, 1, HBuf/8 - 4, VideoBufRed); // set up the Red channel
            SetupSPI(P_GRN_SPI, P_GRN_SPI_INTERRUPT, VGA_PIX_T*2, (void *)&P_GRN_SPI_INPUT, 0, HBuf/8 - 4, VideoBufGrn); // set up the Green channel
            SetupSPI(P_BLU_SPI, P_BLU_SPI_INTERRUPT, VGA_PIX_T*2, (void *)&P_BLU_SPI_INPUT, 3, HBuf/8 - 4, VideoBufBlu); // set up the Blue channel
    	    break;
    }   	    

	ModeC = mc;
	ModeP = mp;
}	
#endif  // colour




/**************************************************************************************************
Draw a line on a the video output
	(x1, y1) - the start coordinate
	(x2, y2) - the end coordinate
	colour - zero for erase, non zero to draw
***************************************************************************************************/
#define abs( a)     (((a)> 0) ? (a) : -(a))

void MMline(int x1, int y1, int x2, int y2, int colour) {
    int  x, y, addx, addy, dx, dy;
    int P;
    int i;
    
    #if defined(COLOUR)
        int coff, con;
        SelectChannel(colour, &con, &coff);
    #else
        #define plotx(a, b, c, d)    plot(a, b, colour)
    #endif

    dx = abs(x2 - x1);
    dy = abs(y2 - y1);
    x = x1;
    y = y1;

    if(x1 > x2)
       addx = -1;
    else
       addx = 1;

    if(y1 > y2)
       addy = -1;
    else
       addy = 1;

    if(dx >= dy) {
       P = 2*dy - dx;
       for(i=0; i<=dx; ++i) {
          plotx(x, y, con, coff);
          if(P < 0) {
             P += 2*dy;
             x += addx;
          } else {
             P += 2*dy - 2*dx;
             x += addx;
             y += addy;
          }
       }
    } else {
       P = 2*dx - dy;
       for(i=0; i<=dy; ++i) {
          plotx(x, y, con, coff);
          if(P < 0) {
             P += 2*dx;
             y += addy;
          } else {
             P += 2*dx - 2*dy;
             x += addx;
             y += addy;
          }
       }
    }
}




/**********************************************************************************************
Draw a box on the video output
     (x1, y1) - the start coordinate
     (x2, y2) - the end coordinate
     fill  - 0 or 1
     colour - 0 or 1
***********************************************************************************************/
void MMbox(int x1, int y1, int x2, int y2, int fill, int colour) {

   if(fill) {
      int y, ymax;                          // Find the y min and max
      if(y1 < y2) {
         y = y1;
         ymax = y2;
      } else {
         y = y2;
         ymax = y1;
      }

      for(; y<=ymax; ++y)                    // Draw lines to fill the rectangle
         MMline(x1, y, x2, y, colour);
   } else {
      MMline(x1, y1, x2, y1, colour);      	// Draw the 4 sides
      MMline(x1, y2, x2, y2, colour);
      MMline(x1, y1, x1, y2, colour);
      MMline(x2, y1, x2, y2, colour);
   }
}





/***********************************************************************************************
Draw a circle on the video output
	(x,y) - the center of the circle
	radius - the radius of the circle
	fill - non zero
	colour - to use for both the circle and fill
	aspect - the ration of the x and y axis (a float).  0.83 gives an almost perfect circle
***********************************************************************************************/
void MMCircle(int x, int y, int radius, int fill, int colour, float aspect) {
   int a, b, P;
   int A, B;
   int asp;

   a = 0;
   b = radius;
   P = 1 - radius;
   asp = aspect * (float)(1 << 10);

   do {
     A = (a * asp) >> 10;
     B = (b * asp) >> 10;
     if(fill) {
         MMline(x-A, y+b, x+A, y+b, colour);
         MMline(x-A, y-b, x+A, y-b, colour);
         MMline(x-B, y+a, x+B, y+a, colour);
         MMline(x-B, y-a, x+B, y-a, colour);
      } else {
         plot(A+x, b+y, colour);
         plot(B+x, a+y, colour);
         plot(x-A, b+y, colour);
         plot(x-B, a+y, colour);
         plot(B+x, y-a, colour);
         plot(A+x, y-b, colour);
         plot(x-A, y-b, colour);
         plot(x-B, y-a, colour);
      }

      if(P < 0)
         P+= 3 + 2*a++;
      else
         P+= 5 + 2*(a++ - b--);
         
    } while(a <= b);
}
