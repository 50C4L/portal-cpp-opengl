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
	// 默认窗口大小
	constexpr int DEFAULT_WIDTH = 2560;
	constexpr int DEFAULT_HEIGHT = 1440;
	constexpr unsigned int UPDATE_TIME = 17; // 游戏逻辑每秒更新60次, 16.66666ms间隔
}

///
/// 因为glutDisplayFunc等函数只支持C样式的回调函数
/// 这里只能用一个全局的Application instance来提供封装好的接口 Application::Render, Application::ResizeViewport
/// 
/// 然后通过 GLUTRenderCallback, GLUTResizeCallback这些静态函数作为回调函数再调用上面的封装函数
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

Application::Application( Params params )
	: mParams( params )
	, mWindowWidth( DEFAULT_WIDTH )
	, mWindowHeight( DEFAULT_HEIGHT )
{
	mKeyStatus.emplace( 'w', false );
	mKeyStatus.emplace( 'a', false );
	mKeyStatus.emplace( 's', false );
	mKeyStatus.emplace( 'd', false );
}

bool
Application::Initialize()
{
	// 初始化glut
	glutInit( &mParams.argc, mParams.argv);
	glutInitContextVersion( 3, 3 ); // 至少是OpenGL 3.3
	glutInitContextProfile( GLUT_CORE_PROFILE );
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
	glutInitWindowSize( DEFAULT_WIDTH, DEFAULT_HEIGHT );
	glutCreateWindow( "Shitty portal" );
	glutSetCursor( GLUT_CURSOR_NONE );

	// 初始化glew
	auto result = glewInit();
	if( result != GLEW_OK )
	{
		std::cerr << "ERROR: " << glewGetErrorString( result ) << std::endl;
		return false;
	}

	// 注册glut回调
	glutDisplayFunc( GLUTRenderCallback );
	glutReshapeFunc( GLUTResizeCallback );
	constexpr int not_used_value = 0;
	glutTimerFunc( UPDATE_TIME, GLUTUpdateCallback, not_used_value );
	glutKeyboardFunc( GLUTKeyboardDownCallback );
	glutKeyboardUpFunc( GLUTKeyboardUpCallback );
	// 鼠标设在窗口在中心
	glutWarpPointer( mWindowWidth/2, mWindowHeight/2 );
	glutPassiveMotionFunc( GLUTMouseMoveCallback );

	// 初始化渲染器
	mRenderer = std::make_unique<Renderer>();
	mRenderer->ResizeViewport( { mWindowWidth, mWindowHeight } );

	// 加载资源
	mRenderer->GetResources().LoadTexture( "resources/white_wall.jpg" );
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
	mRenderer->Render();
	mLevelController->RenderDebugInfo();
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
		mLevelController->HandleMouse( x, y );
	}
}

void 
Application::KeyChanged( unsigned char key, bool is_down )
{
	mKeyStatus[ key ] = is_down;
}
