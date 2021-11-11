#include "Application.h"

#include <iostream>

#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/freeglut.h>

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
}

Application::Application( Params params )
	: mParams( params )
	, mWindowWidth( DEFAULT_WIDTH )
	, mWindowHeight( DEFAULT_HEIGHT )
	, mRenderer( std::make_unique<Renderer>() )
{
}

bool
Application::Initialize()
{
	// 初始化glut
	glutInit( &mParams.argc, mParams.argv);
	glutInitContextVersion( 3, 3 ); // 至少是OpenGL 3.3
	glutInitContextProfile( GLUT_CORE_PROFILE );
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL );
	glutInitWindowSize( DEFAULT_WIDTH, DEFAULT_HEIGHT );
	glutCreateWindow( "Shitty portal" );
	glutSetCursor( GLUT_CURSOR_NONE );
	ResizeViewport( DEFAULT_WIDTH, DEFAULT_HEIGHT );

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
	// 设置窗口中心点
	glutWarpPointer( mWindowWidth/2, mWindowHeight/2 );

	// 加载资源
	mRenderer->Initialize();
	mTriangle = std::make_unique<Renderer::Renderable>( 
		std::vector<Renderer::Vertex>{
			{ -0.5f, -0.5f, 0.0f },
			{ 0.5f, -0.5f, 0.0f },
			{ 0.0f,  0.5f, 0.0f }
		} );

	return true;
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
{
	//glClear( GL_DEPTH_BUFFER_BIT );
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	mRenderer->Render( *mTriangle.get() );
}

void 
Application::ResizeViewport(int width, int height)
{
	// 重新设置viewport
	glViewport( 0, 0, (GLsizei)width, (GLsizei)height );
	mWindowWidth = width;
	mWindowHeight = height;
}
