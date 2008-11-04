#include <stdio.h>
#include <unistd.h>
#include <SDL/SDL.h>
#include <driver.h>
#include <textentrydialog.h>

int main(int,char **)
{
	if(SDL_Init(SDL_INIT_VIDEO) == -1) {
		fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
		exit(1);
	}

	SDL_Surface* surface = SDL_SetVideoMode(
		640, //int width
		480, //int height
		0,   //int bitsperpixel
		SDL_HWSURFACE | SDL_DOUBLEBUF);  //Uint32 flags
	if(surface == NULL) {
		fprintf(stderr, "Failed to set SDL video mode: %s\n", SDL_GetError());
		exit(1);
	}
	
	FwGui::Driver* gui = new FwGui::Driver();
	FwGui::TextEntryDialog* dialog = new FwGui::TextEntryDialog("FWGUI TEST", "", 1024);
	gui->SetActiveDialog(dialog);

	while(!dialog->ok) {
		if(!gui->Tick()) {
			break;
		}
		SDL_Flip(surface);
		usleep(16666);
	}

	printf("Text entered: %s\n", dialog->GetText());

	gui->SetActiveDialog(NULL);
	delete dialog;
	delete gui;

	SDL_Quit();
	
	return 0;}
