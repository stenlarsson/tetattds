#include "fwgui.h"
#include "driver.h"
#include <SDL/SDL.h>
#include <stdio.h>
#include <string.h>
#include "font.h"
#include "gunship_12_bin.h"
#include "vera_11_bin.h"
#include "dialog.h"
#include "theme.h"

#define TOUCH_DELTA 100

namespace FwGui
{
	Driver::Driver()
	:	framebuffer(NULL),
		dialog(NULL),
		lastX(0),
		lastY(0),
		firstTouch(true),
		needGraphicsInit(false),
		listener(NULL)
	{
		Font::GUNSHIP_12 = new Font(gunship_12_bin);
		Font::VERA_11 = new Font(vera_11_bin);
		
		framebuffer = SDL_CreateRGBSurface(
			SDL_SWSURFACE, //Uint32 flags
			256,           //int width
			192,           //int height
			15,            //int bitsPerPixel
			0x0000001f,    //Uint32 Rmask
			0x000003e0,    //Uint32 Gmask
			0x00007c00,    //Uint32 Bmask
			0x00000000);   //Uint32 Amask
		
		if(framebuffer == NULL) {
			fprintf(stderr, "CreateRGBSurface failed: %s\n", SDL_GetError());
			exit(1);
		}
	}
	
	Driver::~Driver()
	{
		delete Font::GUNSHIP_12;
		delete Font::VERA_11;
		
		SDL_FreeSurface((SDL_Surface*)framebuffer);
	}
	
	void Driver::SetActiveDialog(Dialog* dialog)
	{
		this->dialog = dialog;
		SetListener(dialog);
		if(dialog != NULL)
		{
			needGraphicsInit = true;
			dialog->Repaint();
		}
	}

	void Driver::SetListener(InputListener* listener)
	{
		this->listener = listener;
	}
	
	/**
	 * Maps SDL keyboard codes to FWKEY_* constants.
	 * Returns FWKEY_NONE for unrecognized keys.
	 */
	static inline Key MapKey(int sdlkey) {
		switch(sdlkey) {
		case SDLK_SPACE:     return FWKEY_A;
		case SDLK_ESCAPE:    return FWKEY_B;
		case SDLK_BACKSPACE: return FWKEY_SELECT;
		case SDLK_RETURN:    return FWKEY_START;
		case SDLK_RIGHT:     return FWKEY_RIGHT;
		case SDLK_LEFT:      return FWKEY_LEFT;
		case SDLK_UP:        return FWKEY_UP;
		case SDLK_DOWN:      return FWKEY_DOWN;
		case SDLK_RSHIFT:    return FWKEY_R;
		case SDLK_LSHIFT:    return FWKEY_L;
		case SDLK_x:         return FWKEY_X;
		case SDLK_y:         return FWKEY_Y;
		default:             return FWKEY_NONE;
		}
	}
	
	/**
	 * Maps SDL joystick buttons to FWKEY_* constants.
	 * Returns FWKEY_NONE for unrecognized buttons.
	 */
	static inline Key MapButton(int button) {
		switch(button) {
		case 0:  return FWKEY_A;
		case 1:  return FWKEY_B;
		case 4:  return FWKEY_SELECT;
		case 7:  return FWKEY_START;
		case 5:  return FWKEY_R;
		case 6:  return FWKEY_L;
		case 2:  return FWKEY_X;
		case 3:  return FWKEY_Y;
		default: return FWKEY_NONE;
		}
	}

	bool Driver::Tick()
	{
		SDL_Event event;
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				if(listener != NULL) {
					Key key = MapKey(event.key.keysym.sym);
					if(key != FWKEY_NONE) {
						if(event.type == SDL_KEYDOWN)
							listener->KeyDown(key);
						else if(event.type == SDL_KEYUP)
							listener->KeyUp(key);
					}						
				}
				break;
				
			case SDL_MOUSEBUTTONDOWN:
				if(listener != NULL)
					listener->TouchDown(event.button.x, event.button.y-192);
				break;
				
			case SDL_MOUSEBUTTONUP:
				if(listener != NULL)
					listener->TouchUp(event.button.x, event.button.y-192);
				break;

			case SDL_MOUSEMOTION:
				if(listener != NULL && event.motion.state)
					listener->TouchDrag(event.motion.x, event.motion.y-192);
				break;
				
			case SDL_JOYAXISMOTION:
				if(listener != NULL) {
#define THRESHOLD 4096
					if(event.jaxis.axis == 1) {
						static int last = 0;
						if(last < THRESHOLD && event.jaxis.value >= THRESHOLD) {
							listener->KeyDown(FWKEY_UP);
						}
						if(last >= THRESHOLD && event.jaxis.value < THRESHOLD) {
							listener->KeyUp(FWKEY_UP);
						}
						if(last > -THRESHOLD && event.jaxis.value <= -THRESHOLD) {
							listener->KeyDown(FWKEY_DOWN);
						}
						if(last <= -THRESHOLD && event.jaxis.value > -THRESHOLD) {
							listener->KeyUp(FWKEY_DOWN);
						}
						last = event.jaxis.value;
					} else if(event.jaxis.axis == 0) {
						static int last = 0;
						if(last < THRESHOLD && event.jaxis.value >= THRESHOLD) {
							listener->KeyDown(FWKEY_RIGHT);
						}
						if(last >= THRESHOLD && event.jaxis.value < THRESHOLD) {
							listener->KeyUp(FWKEY_RIGHT);
						}
						if(last > -THRESHOLD && event.jaxis.value <= -THRESHOLD) {
							listener->KeyDown(FWKEY_LEFT);
						}
						if(last <= -THRESHOLD && event.jaxis.value > -THRESHOLD) {
							listener->KeyUp(FWKEY_LEFT);
						}
						last = event.jaxis.value;
					}
				}
				break;

			case SDL_JOYBUTTONDOWN:
			case SDL_JOYBUTTONUP:
				if(listener != NULL) {
					Key key = MapButton(event.jbutton.button);
					if(key != FWKEY_NONE) {
						if(event.jbutton.state == 1)
							listener->KeyDown(key);
						else
							listener->KeyUp(key);
					}						
				}
				break;

			case SDL_JOYHATMOTION:
				static int joyhatstate = 0;
				if(listener != NULL) {
					int changes = joyhatstate ^ event.jhat.value;

#define CHECK_HAT(x)											\
					if(changes & SDL_HAT_##x) {					\
						if(event.jhat.value & SDL_HAT_##x) {	\
							listener->KeyDown(FWKEY_##x);		\
						} else {								\
							listener->KeyUp(FWKEY_##x);			\
						}										\
					}

					CHECK_HAT(UP);
					CHECK_HAT(RIGHT);
					CHECK_HAT(DOWN);
					CHECK_HAT(LEFT);
				}
				joyhatstate = event.jhat.value;
				break;


			case SDL_QUIT:
				return false;

			default:
				break;
			}
		}
			  
		if(dialog != NULL)
		{
			SDL_Surface* surface = (SDL_Surface*)framebuffer;

			if(dialog->NeedsRepaint())
			{
				if(backgroundImage == NULL)
				{
					SDL_FillRect(
						surface,
						NULL,
						SDL_MapRGB(
							surface->format,
							backgroundColor.r,
							backgroundColor.g,
							backgroundColor.b));
				}
				else
				{
					SDL_BlitSurface(
						(SDL_Surface*)backgroundImage, NULL, surface, NULL);
				}
				
				Graphics* graphics = new Graphics((uint16_t*)surface->pixels, 256, 192);
				dialog->Paint(graphics);
				delete graphics;
				dialog->SetRepaint(false);
			}
			
			SDL_Rect dstrect = {0, 192, 256, 192};
			SDL_BlitSurface(
				surface,               //SDL_Surface *src
				NULL,                  //SDL_Rect *srcrect
				SDL_GetVideoSurface(), //SDL_Surface *dst
				&dstrect);             //SDL_Rect *dstrect
		}

		return true;
	}
}
