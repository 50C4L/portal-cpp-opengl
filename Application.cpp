#include "Application.h"

#include <GL/glew.h>
#include <GL/glut.h>

using namespace portal;

namespace
{
	// 默认窗口大小
	constexpr int DEFAULT_WIDTH = 1920;
	constexpr int DEFAULT_HEIGHT = 1080;
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
}

Application::Application( Params params )
	: mParams( params )
	, mWindowWidth( DEFAULT_WIDTH )
	, mWindowHeight( DEFAULT_HEIGHT )
{
}

void
Application::Initialize()
{
	// 初始化glut
	glutInit( &mParams.argc, mParams.argv);
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL );
	glutInitWindowSize( DEFAULT_WIDTH, DEFAULT_HEIGHT );
	glutCreateWindow( "Shitty portal" );
	glutSetCursor( GLUT_CURSOR_NONE );
	ResizeViewport( DEFAULT_WIDTH, DEFAULT_HEIGHT );

	// 初始化glew
	glewInit();

	// 注册glut回调
	glutDisplayFunc( GLUTRenderCallback );
	glutReshapeFunc( GLUTResizeCallback );
	constexpr int not_used_value = 0;
	glutTimerFunc( UPDATE_TIME, GLUTUpdateCallback, not_used_value );
	// 设置窗口中心点
	glutWarpPointer( mWindowWidth/2, mWindowHeight/2 );
}

void
Application::Run()
{
	glutMainLoop();
}

void
Application::Update()
{}

void
Application::Render()
{}

void 
Application::ResizeViewport(int width, int height)
{

}
