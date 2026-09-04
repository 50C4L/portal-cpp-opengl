#include "Application.h"

#include <iostream>

#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/freeglut.h>

#include "Camera.h"
#include "ScenePrimitives.h"
#include "LevelController.h"

using namespace portal;

namespace
{
	// Default window size
	constexpr int DEFAULT_WIDTH = 2560;
	constexpr int DEFAULT_HEIGHT = 1440;
	constexpr unsigned int UPDATE_TIME = 17; // Game logic updates 60 times a second, 16.66666ms interval
}

///
/// Because glutDisplayFunc and friends only support C-style callbacks,
/// we have to use a global Application instance to expose the wrapped interfaces Application::Render, Application::ResizeViewport
/// 
/// Then GLUTRenderCallback, GLUTResizeCallback and the other statics act as the actual callbacks and call those wrapped functions
/// 
std::shared_ptr<Application> Application::sInstance;

/*static*/
Application::Ptr
Application::CreateApp( Params params )
{
	if( sInstance )
	{
		return sInstance;
	}
	
	sInstance = std::make_shared<Application>( std::move( params ) );
	return sInstance;
}

/*static*/
void
Application::GLUTRenderCallback()
{
	if( sInstance )
	{
		sInstance->Render();
	}
	glutSwapBuffers();
}

/*static*/
void
Application::GLUTResizeCallback( int width, int height )
{
	if( sInstance ) 
	{
		sInstance->ResizeViewport( width, height );
	}
}

/*static*/
void
Application::GLUTUpdateCallback( int /*value*/ )
{
	if( sInstance ) 
	{
		sInstance->Update();
	}
	// Redraw screen
	glutPostRedisplay();
	// Schedule new update
	glutTimerFunc( UPDATE_TIME, GLUTUpdateCallback, 1 );
}

/*static*/
void 
Application::GLUTMouseMoveCallback( int x, int y )
{
	if( sInstance ) 
	{
		sInstance->MouseMoved( x, y );
	}
}

/*static*/
void 
Application::GLUTKeyboardUpCallback( unsigned char key, int x, int y )
{
	if( sInstance ) 
	{
		sInstance->KeyChanged( key, false );
	}
}

/*static*/
void 
Application::GLUTKeyboardDownCallback( unsigned char key, int x, int y )
{
	if( sInstance ) 
	{
		sInstance->KeyChanged( key, true );
	}
}

/*static*/ 
void 
Application::GLUTMousePressedCallback( int button, int state, int x, int y )
{
	if( sInstance ) 
	{
		sInstance->MousePresed( button, state == GLUT_DOWN );
	}
}

Application::Application( Params params )
	: mParams( params )
	, mWindowWidth( DEFAULT_WIDTH )
	, mWindowHeight( DEFAULT_HEIGHT )
{
	mKeyStatus.emplace( 'w', false );
	mKeyStatus.emplace( 'a', false );
	mKeyStatus.emplace( 's', false );
	mKeyStatus.emplace( 'd', false );
	mKeyStatus.emplace( ' ', false );
	mKeyStatus.emplace( 'e', false );

	mMouseButtonState.emplace( 1, false );
	mMouseButtonState.emplace( 2, false );
	mMouseButtonState.emplace( 3, false );
}

bool
Application::Initialize()
{
	// Initialize glut
	glutInit( &mParams.argc, mParams.argv);
	glutInitContextVersion( 3, 3 ); // At least OpenGL 3.3
	glutInitContextProfile( GLUT_CORE_PROFILE );
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
	glutInitWindowSize( DEFAULT_WIDTH, DEFAULT_HEIGHT );
	glutCreateWindow( "Shitty portal" );
	glutSetCursor( GLUT_CURSOR_NONE );

	// Initialize glew
	auto result = glewInit();
	if( result != GLEW_OK )
	{
		std::cerr << "ERROR: " << glewGetErrorString( result ) << std::endl;
		return false;
	}

	// Register glut callbacks
	glutDisplayFunc( GLUTRenderCallback );
	glutReshapeFunc( GLUTResizeCallback );
	constexpr int not_used_value = 0;
	glutTimerFunc( UPDATE_TIME, GLUTUpdateCallback, not_used_value );
	glutKeyboardFunc( GLUTKeyboardDownCallback );
	glutKeyboardUpFunc( GLUTKeyboardUpCallback );
	// Stick the mouse in the center of the window
	glutWarpPointer( mWindowWidth/2, mWindowHeight/2 );
	glutPassiveMotionFunc( GLUTMouseMoveCallback );
	glutMouseFunc( GLUTMousePressedCallback );

	// Initialize the renderer
	mRenderer = std::make_unique<Renderer>();
	mRenderer->ResizeViewport( { mWindowWidth, mWindowHeight } );

	// Load resources
	// TODO: Each level should load its own stuff
	mRenderer->GetResources().LoadTexture( "resources/textures/white_wall.jpg" );
	mRenderer->GetResources().LoadTexture( "resources/textures/blueportal.png" );
	mRenderer->GetResources().LoadTexture( "resources/textures/orangeportal.png" );
	mRenderer->GetResources().LoadTexture( "resources/textures/box.jpg" );
	mRenderer->GetResources().LoadCubeMaps( {
		"resources/textures/sky/right.jpg",
		"resources/textures/sky/left.jpg",
		"resources/textures/sky/top.jpg",
		"resources/textures/sky/bottom.jpg",
		"resources/textures/sky/front.jpg",
		"resources/textures/sky/back.jpg"
	}, "SKYBOX" );

	mLevelController = std::make_unique<LevelController>( *mRenderer );
	mLevelController->Initialize( UPDATE_TIME );
	if( mLevelController->LoadLevelFile( "resources/levels/level_intro.json" ) )
	{
		mLevelController->ChangeLevelTo( "resources/levels/level_intro.json" );
	}

	return true;
}

void
Application::Run()
{
	glutMainLoop();
}

void
Application::Update()
{
	if( mLevelController )
	{
		mLevelController->HandleKeys( mKeyStatus );
		mLevelController->Update();
	}
}

void
Application::Render()
{
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
	mLevelController->RenderScene();
}

void 
Application::ResizeViewport(int width, int height)
{
	mWindowWidth = width;
	mWindowHeight = height;
	if( mRenderer )
	{
		mRenderer->ResizeViewport( { width, height } );
	}
}

void 
Application::MouseMoved( int x, int y )
{
	if( mLevelController )
	{
		mLevelController->HandleMouseMove( x, y );
	}
}

void 
Application::KeyChanged( unsigned char key, bool is_down )
{
	mKeyStatus[ key ] = is_down;
}

void 
Application::MousePresed( int button, bool is_pressed )
{
	switch( button )
	{
	case GLUT_LEFT_BUTTON:
		mMouseButtonState[ 1 ] = is_pressed;
		break;
	case GLUT_RIGHT_BUTTON:
		mMouseButtonState[ 2 ] = is_pressed;
		break;
	case GLUT_MIDDLE_BUTTON:
		mMouseButtonState[ 3 ] = is_pressed;
		break;
	default:
		break;
	}

	if( mLevelController )
	{
		mLevelController->HandleMouseButton( mMouseButtonState );
	}
}
