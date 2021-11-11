#include "Renderer.h"
#include <string>
#include <iostream>

#include <GL/glew.h>

using namespace portal;

namespace
{
	const std::string DEFAULT_VERTEX_SHADER = R"~~~(
		#version 330 core
		layout (location = 0) in vec3 in_pos;
		
		void main()
		{
			gl_Position = vec4( in_pos.x, in_pos.y, in_pos.z, 1.0 );
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

///
/// Renderable implementaitons
/// 
Renderer::Renderable::Renderable( std::vector<Vertex>&& vertices )
	: mVBO( 0 )
	, mVAO( 0 )
{
	glGenVertexArrays( 1, &mVAO ); 
	glBindVertexArray( mVAO );
	// 0. copy our vertices array in a buffer for OpenGL to use
	glGenBuffers( 1, &mVBO ); 
	glBindBuffer( GL_ARRAY_BUFFER, mVBO );
	glBufferData( GL_ARRAY_BUFFER, sizeof( vertices ), &vertices.front(), GL_STATIC_DRAW);
	// 1. then set the vertex attributes pointers
	glVertexAttribPointer( POSITION_INDEX, 3, GL_FLOAT, GL_FALSE, sizeof( Vertex ), (void*)0);
	glEnableVertexAttribArray( 0 );
}

Renderer::Renderable::~Renderable()
{
}

unsigned int
Renderer::Renderable::GetVAO()
{
	return mVAO;
}


///
/// Renderer implementations
/// 

Renderer::Renderer()
{
}

Renderer::~Renderer()
{
}

bool
Renderer::Initialize()
{
	return CompileShader( DEFAULT_SHADER, DEFAULT_VERTEX_SHADER, DEFAULT_FRAGMENT_SHADER );
}

bool 
Renderer::CompileShader( const std::string& name, std::string vertex_shader, std::string fragment_shader )
{
	// Lambda函数用于检查shader是否有编译错误
	static auto check_compile_error = []( unsigned int id ) -> bool
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
	if( !check_compile_error( vs_id ) )
	{
		return false;
	}

	// 编译片源shader
	unsigned int fs_id = glCreateShader( GL_FRAGMENT_SHADER );
	const char* fs_src = fragment_shader.c_str();
	glShaderSource( fs_id, 1, &fs_src, NULL );
	glCompileShader( fs_id );
	if( !check_compile_error( fs_id ) )
	{
		return false;
	}

	// 连接两个shader生成shader program
	unsigned int shader_program = glCreateProgram();
	glAttachShader( shader_program, vs_id );
	glAttachShader( shader_program, fs_id );
	glLinkProgram( shader_program );

	int success;
	constexpr int log_size = 512;
	char log[log_size];
	glGetProgramiv( shader_program, GL_LINK_STATUS, &success);
	if( !success ) 
	{
		glGetProgramInfoLog( shader_program, log_size, NULL, log );
		std::cerr << "ERROR: Failed to compile shader: \n" << log << std::endl;
		return false;
	}

	mCompiledShaders.emplace( name, shader_program );

	// 编译成功，可以释放顶点和片源的资源
	glDeleteShader( vs_id );
	glDeleteShader( fs_id );
	return true;
}

void 
Renderer::Render( Renderable& renderable_obj )
{
	glUseProgram( mCompiledShaders[ DEFAULT_SHADER ] );
	glBindVertexArray( renderable_obj.GetVAO() );
	glDrawArrays( GL_TRIANGLES, 0, 3 );
}
