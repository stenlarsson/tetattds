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
		needGraphicsInit(false)
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
		if(dialog != NULL)
		{
			needGraphicsInit = true;
			dialog->Repaint();
		}
	}
	
	void Driver::Tick()
	{
		if(dialog == NULL)
		{
			return;
		}
		
		SDL_Event event;
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
			case SDL_KEYDOWN:
				switch(event.key.keysym.sym) {
				case SDLK_SPACE:
					dialog->KeyA();
					break;
				case SDLK_BACKSPACE:
					dialog->KeyB();
					break;
				case SDLK_RETURN:
					dialog->KeyStart();
					break;
				case SDLK_UP:
					dialog->KeyUp();
					break;
				case SDLK_DOWN:
					dialog->KeyDown();
					break;
				case SDLK_LEFT:
					dialog->KeyLeft();
					break;
				case SDLK_RIGHT:
					dialog->KeyRight();
					break;
				default:
					break;
				}
				break;
				
			case SDL_MOUSEBUTTONDOWN:
				dialog->TouchDown(event.button.x, event.button.y);
				/* TODO
				   dialog->TouchHeld(x, y);
				*/
				break;
				
			case SDL_MOUSEBUTTONUP:
				dialog->TouchUp(event.button.x, event.button.y);
				break;

			default:
				break;
			}
		}
			  
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
}
