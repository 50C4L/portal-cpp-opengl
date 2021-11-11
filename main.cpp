#include "Application.h"

int main( int argc, char** argv )
{
	auto app = portal::Application::CreateApp( { argc, argv } );
	if( app->Initialize() )
	{
		app->Run();
	}

	return 0;
}
