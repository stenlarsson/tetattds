#include <stdio.h>
#include <unistd.h>
#include <SDL.h>
#include <driver.h>
#include <textentrydialog.h>

int main()
{
	if(SDL_Init(SDL_INIT_VIDEO) == -1) {
		fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
		exit(1);
	}

	SDL_Surface* surface = SDL_SetVideoMode(
		256, //int width
		192, //int height
		0,   //int bitsperpixel
		0);  //Uint32 flags
	if(surface == NULL) {
		fprintf(stderr, "Failed to set SDL video mode: %s\n", SDL_GetError());
		exit(1);
	}

	FwGui::Driver* gui = new FwGui::Driver();
	FwGui::TextEntryDialog* dialog = new FwGui::TextEntryDialog("FWGUI TEST", "", 1024);
	gui->SetActiveDialog(dialog);

	while(!dialog->ok) {
		gui->Tick();
		SDL_Flip(surface);
		usleep(16666);
	}

	printf("Text entered: %s\n", dialog->GetText());

	gui->SetActiveDialog(NULL);
	delete dialog;
	delete gui;

	SDL_Quit();
}
