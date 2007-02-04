// server_gui.cpp : main project file.

#define _CRT_SECURE_NO_DEPRECATE
#include "MainForm.h"
#include <stdarg.h>

using namespace server_gui;

ref class GlobalObjects {
public:
	static MainForm^ form;
};

void PrintStatus(const char* format, ...)
{
	char buffer[256];

	va_list args;
	va_start( args, format );
	
	vsprintf( buffer, format, args );
	
	GlobalObjects::form->AddLine(buffer);

	va_end( args );
}

[STAThreadAttribute]
int main(array<System::String ^> ^args)
{
	// Enabling Windows XP visual effects before any controls are created
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false); 

	// Create the main window and run it
	GlobalObjects::form = gcnew MainForm();
	Application::Run(GlobalObjects::form);
	return 0;
}
