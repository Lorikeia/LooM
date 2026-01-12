// DESCRIPTION:
//	DOOM graphics stuff for (soon to be) SDL3!!

#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>


#include <SDL3/SDL.h>	// Can't be using the usual boolean with you around.

#include <stdarg.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <errno.h>
#include <signal.h>

#include "doomstat.h"
#include "i_system.h"
#include "v_video.h"
#include "m_argv.h"
#include "d_main.h"

#include "doomdef.h"


SDL_Event		S_event;
SDL_Palette		*S_cmap;	// Kinda. Sorta.
uint8_t		*screen;
SDL_Window	*window;
SDL_Renderer	*renderer;
SDL_Texture	*texture;
int		S_width;
int		S_height;

// Fake mouse handling.
// This cannot work properly w/o DGA.
// Needs an invisible mouse cursor at least.
boolean		grabMouse;


//
//  Translates the key currently in X_event
//

int Slatekey(void)	{
	SDL_Keycode rc;
	switch(rc = S_event.key.key)	{	// ^
		case SDLK_LEFT:		return KEY_LEFTARROW;
		case SDLK_RIGHT:	return KEY_RIGHTARROW;
		case SDLK_DOWN:		return KEY_DOWNARROW;
		case SDLK_UP:		return KEY_UPARROW;
		case SDLK_ESCAPE:	return KEY_ESCAPE;
		case SDLK_RETURN:	return KEY_ENTER;
		case SDLK_TAB:	return KEY_TAB;
		case SDLK_F1:	return KEY_F1;
		case SDLK_F2:	return KEY_F2;
		case SDLK_F3:	return KEY_F3;
		case SDLK_F4:	return KEY_F4;
		case SDLK_F5:	return KEY_F5;
		case SDLK_F6:	return KEY_F6;
		case SDLK_F7:	return KEY_F7;
		case SDLK_F8:	return KEY_F8;
		case SDLK_F9:	return KEY_F9;
		case SDLK_F10:	return KEY_F10;
		case SDLK_F11:	return KEY_F11;
		case SDLK_F12:	return KEY_F12;
	
		case SDLK_BACKSPACE:
		case SDLK_DELETE:	return KEY_BACKSPACE;

		case SDLK_PAUSE:	return KEY_PAUSE;

		case SDLK_KP_EQUALS:
		case SDLK_EQUALS:	return KEY_EQUALS;

		case SDLK_KP_MINUS:
		case SDLK_MINUS:		return KEY_MINUS;

		case SDLK_LSHIFT:
		case SDLK_RSHIFT:	return KEY_RSHIFT;
	
		case SDLK_LCTRL:
		case SDLK_RCTRL:	return KEY_RCTRL;
	
		case SDLK_LALT:
		case SDLK_LMETA:
		case SDLK_RALT:
		case SDLK_RMETA:	return KEY_RALT;
	
		default:
	/*if (rc >= XK_space && rc <= XK_asciitilde)
	    rc = rc - XK_space + ' ';
	if (rc >= 'A' && rc <= 'Z')
	    rc = rc - 'A' + 'a';*/ // [LIC] FIGURE THIS OUT
	break;
    }

    return rc;

}

void I_ShutdownGraphics(void)	{
	SDL_DestroyPalette(S_cmap);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}



//
// I_StartFrame
//
void I_StartFrame (void)
{
    // er?

}

void I_GetEvent(void)	{
}

// I_StartTic
//
void I_StartTic(void)	{
	event_t event;
    // put event-grabbing stuff in here
	while(SDL_PollEvent(&S_event)) {
		switch(S_event.type)	{
		case SDL_EVENT_QUIT:	I_Quit();	break;	// Risky?
		case SDL_EVENT_KEY_DOWN:
			event.type = ev_keydown;
			event.data1 = Slatekey();
			D_PostEvent(&event);
			break;
		case SDL_EVENT_KEY_UP:
			event.type = ev_keyup;
			event.data1 = Slatekey();
			D_PostEvent(&event);
			break;
		case SDL_EVENT_MOUSE_BUTTON_DOWN:	// Nephew so long...
		case SDL_EVENT_MOUSE_BUTTON_UP:
			event.type = ev_mouse;
			event.data1 = 0;	// Needed? Check.
			if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_MASK(1)) event.data1 |= 1;
			if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_MASK(2)) event.data1 |= 4;
			if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_MASK(3)) event.data1 |= 2;
			event.data2 = event.data3 = 0;	// Ewww look at that MASK.
			D_PostEvent(&event);
			break;
		case SDL_EVENT_MOUSE_MOTION:
			if(S_event.motion.xrel || S_event.motion.yrel) {
				event.type = ev_mouse;
				event.data1 =	0;
				if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_MASK(1)) event.data1 |= 1;
				if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_MASK(2)) event.data1 |= 4;
				if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_MASK(3)) event.data1 |= 2;	// C'MON!!
				event.data2 = S_event.motion.xrel	* 2;
				event.data3 = -S_event.motion.yrel	* 2;	// Don't get tempted now...
				D_PostEvent(&event);
			}
			break;	// ^
		default:	break;
		}
	}
}


//
// I_UpdateNoBlit
//
void I_UpdateNoBlit (void)
{
    // what is this?
}

//
// I_FinishUpdate
//
void I_FinishUpdate (void)
{

    static int	lasttic;
    int		tics;
    int		i;
    // UNUSED static unsigned char *bigscreen=0;

    // draws little dots on the bottom of the screen
	if(devparm)	{
		i = I_GetTime();
		tics = i - lasttic;
		lasttic = i;
		if (tics > 20) tics = 20;
		for(i=0 ; i<tics*2 ; i+=2)	screens[0][ (SCREENHEIGHT-1)*SCREENWIDTH + i] = 0xff;
		for ( ; i<20*2 ; i+=2)	screens[0][ (SCREENHEIGHT-1)*SCREENWIDTH + i] = 0x0;
	}
	SDL_UpdateTexture(texture, NULL, screens[0], SCREENWIDTH);
	SDL_RenderClear(renderer);
	SDL_RenderTexture(renderer, texture, NULL, NULL);	// 'cause resizin'!
	SDL_RenderPresent(renderer);
}


//
// I_ReadScreen
//
void I_ReadScreen(byte* scr)	{
    SDL_memcpy(scr, screens[0], SCREENWIDTH * SCREENHEIGHT);
}


//
// I_SetPalette
//
void I_SetPalette (byte* palette)	{
	SDL_Color	colors[256];

	// set the S(DL) colormap entries
	for(int i = 0; i < 256; i++)	{
		colors[i].r = gammatable[usegamma][*palette++];
		colors[i].g = gammatable[usegamma][*palette++];
		colors[i].b = gammatable[usegamma][*palette++];
		colors[i].a = SDL_ALPHA_OPAQUE;	// ALPHA?? Yes, alpha.
	}

	SDL_SetPaletteColors(S_cmap, colors, 0, 256);	// Just like the good ol' times, guys!
	SDL_SetTexturePalette(texture, S_cmap);			// Sniffle sniffle...
}



void I_InitGraphics(void){
    char*		d;
    int			n;
    int			pnum;
    int			x=0;
    int			y=0;
    
    // warning: char format, different type arg
    char		xsign=' ';
    char		ysign=' ';
    

	

	if(M_CheckParm("-2") || M_CheckParm("-3") || M_CheckParm("-4")) printf("Getting to that... I'll add an actual parameter to scale to a specific size... soon!!!\n");

    S_width = SCREENWIDTH;	// Only here for when we implement said size parameter.
    S_height = SCREENHEIGHT;

    // check for command-line display name
    if(M_CheckParm("-disp"))	printf("C'mon, man. Do it yourself. export DISPLAY, man. C'mon.\n");

    // check for command-line geometry
    if(M_CheckParm("-geom"))	printf("Go home, man.");

	// Sound too, bro...
	if(SDL_Init(SDL_INIT_VIDEO) < 0) I_Error("I_InitGraphics() FAILED @SDL_Init: %s", SDL_GetError());	// See how corporate we are?

	window = SDL_CreateWindow("Alright... GET READY!!!", S_width, S_height, SDL_WINDOW_RESIZABLE);
		//| M_CheckParm("-grabmouse") ? SDL_WINDOW_MOUSE_RELATIVE_MODE : 0);	// I thought this code was super cool and awesome... let's try to use it.
	if(!window) I_Error("What the helly.");

	renderer = SDL_CreateRenderer(window, NULL);
	if(!renderer) __asm__("hlt");	// You did something wrong. Not me.

	S_cmap = SDL_CreatePalette(256);

	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_INDEX8, SDL_TEXTUREACCESS_STREAMING, SCREENWIDTH, SCREENHEIGHT);
	if(!texture) puts("hi");

	SDL_SetRenderLogicalPresentation(renderer, SCREENWIDTH, SCREENHEIGHT, SDL_LOGICAL_PRESENTATION_LETTERBOX);	// Stretch might be kinda cool...

	SDL_SetWindowRelativeMouseMode(window, M_CheckParm("-grabmouse"));	// We'll probably change this to "nomouse" because most people... have a mouse.

	screen = (uint8_t*)malloc(SCREENWIDTH * SCREENHEIGHT);
	screens[0] = screen;
}
