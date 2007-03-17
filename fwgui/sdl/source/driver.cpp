#include "fwgui.h"
#include "driver.h"
#include <SDL.h>
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
	
	bool Driver::Tick()
	{
		if(listener != NULL) {
			SDL_Event event;
			while(SDL_PollEvent(&event)) {
				Key key = (Key)-1;
				switch(event.type) {
				case SDL_KEYDOWN:
				case SDL_KEYUP:
					switch(event.key.keysym.sym) {
					case SDLK_SPACE:		key = FWKEY_A; break;
					case SDLK_ESCAPE:		key = FWKEY_B; break;
					case SDLK_BACKSPACE:	key = FWKEY_SELECT; break;
					case SDLK_RETURN:		key = FWKEY_START; break;
					case SDLK_RIGHT:		key = FWKEY_RIGHT; break;
					case SDLK_LEFT:			key = FWKEY_LEFT; break;
					case SDLK_UP:			key = FWKEY_UP; break;
					case SDLK_DOWN:			key = FWKEY_DOWN; break;
					case SDLK_RSHIFT:		key = FWKEY_R; break;
					case SDLK_LSHIFT:		key = FWKEY_L; break;
					case SDLK_x:			key = FWKEY_X; break;
					case SDLK_y:			key = FWKEY_Y; break;
					default:
						break;
					}

					if(key != -1) {
						if(event.type == SDL_KEYDOWN)
							listener->KeyDown(key);
						else if(event.type == SDL_KEYUP)
							listener->KeyUp(key);
					}
					break;
					
				case SDL_MOUSEBUTTONDOWN:
					listener->TouchDown(event.button.x, event.button.y);
					break;
					
				case SDL_MOUSEBUTTONUP:
					listener->TouchUp(event.button.x, event.button.y);
					break;

				case SDL_MOUSEMOTION:
					if(event.motion.state)
						listener->TouchDrag(event.motion.x, event.motion.y);
					break;

				case SDL_QUIT:
					return false;				

				default:
					break;
				}
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
				  
			SDL_BlitSurface(
				surface,               //SDL_Surface *src
				NULL,                  //SDL_Rect *srcrect
				SDL_GetVideoSurface(), //SDL_Surface *dst
				NULL);                 //SDL_Rect *dstrect
		}

		return true;
	}
}
