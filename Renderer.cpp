#include "Renderer.h"
#include <string>
#include <iostream>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Camera.h"

using namespace portal;

namespace
{
	const std::string VIEW_MATRIX_UNIFORM_NAME = "view_mat";
	const std::string PROJECTION_MATRIX_UNIFORM_NAME = "projection_mat";

	const std::string DEFAULT_VERTEX_SHADER = R"~~~(
		#version 330 core
		layout (location = 0) in vec3 in_pos;

		uniform mat4 view_mat;
		uniform mat4 projection_mat;
		
		void main()
		{
			mat4 model_mat = mat4( 1.0 );
			gl_Position = projection_mat * view_mat * model_mat * vec4( in_pos, 1.0 );
		}
	)~~~";

	const std::string DEFAULT_FRAGMENT_SHADER = R"~~~(
		#version 330 core
		out vec4 frag_color;
		
		void main()
		{
			frag_color = vec4( 1.0f, 0.5f, 0.2f, 1.0f );
		} 
	)~~~";

	constexpr GLuint POSITION_INDEX = 0;
}

const std::string Renderer::DEFAULT_SHADER = "DEFLAULT_SHADER";

///
/// Shader implementaitons
/// 
Renderer::Shader::Shader()
	: Shader( DEFAULT_VERTEX_SHADER, DEFAULT_FRAGMENT_SHADER )
{}

Renderer::Shader::Shader( const std::string& vertex_shader, const std::string& fragment_shader )
{
	mIsValid = true;
	// Lambda函数用于检查shader是否有编译错误
	auto check_compile_error = []( unsigned int id ) -> bool
	{
		int success;
		constexpr int log_size = 512;
		char log[log_size];
		glGetShaderiv( id, GL_COMPILE_STATUS, &success );

		if( !success )
		{
			glGetShaderInfoLog( id, log_size, NULL, log);
			std::cerr << "ERROR: Failed to compile shader: \n" << log << std::endl;
		}
		return static_cast<bool>( success );
	};

	// 编译顶点shader
	unsigned int vs_id = glCreateShader( GL_VERTEX_SHADER );
	const char* vs_src = vertex_shader.c_str();
	glShaderSource( vs_id, 1, &vs_src, NULL );
	glCompileShader( vs_id );
	mIsValid = check_compile_error( vs_id );


	// 编译片源shader
	unsigned int fs_id = glCreateShader( GL_FRAGMENT_SHADER );
	const char* fs_src = fragment_shader.c_str();
	glShaderSource( fs_id, 1, &fs_src, NULL );
	glCompileShader( fs_id );
	mIsValid = check_compile_error( fs_id );

	// 连接两个shader生成shader program
	mId = glCreateProgram();
	glAttachShader( mId, vs_id );
	glAttachShader( mId, fs_id );
	glLinkProgram( mId );

	int success;
	constexpr int log_size = 512;
	char log[log_size];
	glGetProgramiv( mId, GL_LINK_STATUS, &success);
	if( !success ) 
	{
		glGetProgramInfoLog( mId, log_size, NULL, log );
		std::cerr << "ERROR: Failed to compile shader: \n" << log << std::endl;
		mIsValid = false;
	}

	// 获取矩阵变量在Shader中的位置
	mViewMatUniformLocation = glGetUniformLocation( mId, VIEW_MATRIX_UNIFORM_NAME.c_str() );
	mProjectionMatUniformLocation = glGetUniformLocation( mId, PROJECTION_MATRIX_UNIFORM_NAME.c_str() );

	// 编译结束，可以释放顶点和片源的资源
	glDeleteShader( vs_id );
	glDeleteShader( fs_id );
}

bool
Renderer::Shader::IsValid() const
{
	return mIsValid;
}

unsigned int
Renderer::Shader::GetId() const
{
	return mId;
}

void 
Renderer::Shader::SetViewMatrix( const glm::mat4& matrix )
{
	SetMat4( mViewMatUniformLocation, matrix );
}

void 
Renderer::Shader::SetProjectionMatrix( const glm::mat4& matrix )
{
	SetMat4( mProjectionMatUniformLocation, matrix );
}


void
Renderer::Shader::SetMat4( int location, const glm::mat4& matrix )
{
	glUniformMatrix4fv( location, 1, GL_FALSE, glm::value_ptr( matrix ) );
}


///
/// Renderable implementaitons
/// 
Renderer::Renderable::Renderable( std::vector<glm::vec3>&& vertices, std::string shader_name )
	: mVBO( 0 )
	, mVAO( 0 )
	, mNumberOfVertices( static_cast<int>( vertices.size() ) )
	, mShader( shader_name )
{
	glGenVertexArrays( 1, &mVAO );
	glGenBuffers( 1, &mVBO ); 

	glBindVertexArray( mVAO );
	glBindBuffer( GL_ARRAY_BUFFER, mVBO );
	// 申请显存空间来放顶点数据
	glBufferData( GL_ARRAY_BUFFER, mNumberOfVertices * sizeof( glm::vec3 ), &vertices.front(), GL_STATIC_DRAW );
	// 绑定顶点位置数据到 POSITION_INDEX
	glVertexAttribPointer( POSITION_INDEX, 3, GL_FLOAT, GL_FALSE, sizeof( glm::vec3 ), (void*)0);
	glEnableVertexAttribArray( POSITION_INDEX );
}

Renderer::Renderable::~Renderable()
{
}

unsigned int
Renderer::Renderable::GetVAO() const
{
	return mVAO;
}

int
Renderer::Renderable::GetNumberOfVertices() const
{
	return mNumberOfVertices;
}

std::string 
Renderer::Renderable::GetShaderName() const
{
	return mShader;
}


///
/// Renderer implementations
/// 
Renderer::Renderer()
	: mActiveCamera( nullptr )
	, mProjectionMatrix( glm::mat4( 1.f ) )
	, mLastRenderTimepoint( std::chrono::steady_clock::now() )
{
}

Renderer::~Renderer()
{
}

bool
Renderer::Initialize()
{
	glEnable( GL_DEPTH_TEST );
	return CompileShader( DEFAULT_SHADER, DEFAULT_VERTEX_SHADER, DEFAULT_FRAGMENT_SHADER );
}

bool 
Renderer::CompileShader( const std::string& name, std::string vertex_shader, std::string fragment_shader )
{
	Shader shader{ std::move( vertex_shader ), std::move( fragment_shader ) };
	if( shader.IsValid() )
	{
		mCompiledShaders.emplace( name, std::move( shader ) );
		return true;
	}
	return false;
}

void 
Renderer::Render( Renderable& renderable_obj )
{
	// 计算一帧用了多少毫秒
	int ms_since_last_call = static_cast<int>( std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - mLastRenderTimepoint ).count() );
	mLastRenderTimepoint = std::chrono::steady_clock::now();

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	auto shader = mCompiledShaders[ renderable_obj.GetShaderName() ];
	glUseProgram( shader.GetId() );
	if( mActiveCamera )
	{
		// 传递当前摄像机的视图投影矩阵到当前Shader
		shader.SetViewMatrix( mActiveCamera->GetViewMatrix() );
		shader.SetProjectionMatrix( mProjectionMatrix );
	}
	glBindVertexArray( renderable_obj.GetVAO() );
	glDrawArrays( GL_TRIANGLES, 0, renderable_obj.GetNumberOfVertices() );
}

void 
Renderer::SetCameraAsActive( std::shared_ptr<Camera> camera )
{
	mActiveCamera = camera;
	mProjectionMatrix = mActiveCamera->GetProjectionMatrix();
}
