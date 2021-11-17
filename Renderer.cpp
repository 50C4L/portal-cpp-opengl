#include "Renderer.h"
#include <string>
#include <iostream>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Camera.h"

using namespace portal;

namespace
{
	const std::string VIEW_MATRIX_UNIFORM_NAME = "view_mat";
	const std::string PROJECTION_MATRIX_UNIFORM_NAME = "projection_mat";

	const std::string DEFAULT_VERTEX_SHADER = R"~~~(
		#version 330 core
		layout (location = 0) in vec3 in_pos;
		layout (location = 1) in vec4 in_color;
		layout (location = 2) in vec2 in_uv;

		out vec2 tex_coord;
		out vec4 color;

		uniform mat4 view_mat;
		uniform mat4 projection_mat;
		
		void main()
		{
			mat4 model_mat = mat4( 1.0 );
			gl_Position = projection_mat * view_mat * model_mat * vec4( in_pos, 1.0 );
			tex_coord = in_uv;
			color = in_color;
		}
	)~~~";

	const std::string DEFAULT_FRAGMENT_SHADER = R"~~~(
		#version 330 core
		out vec4 frag_color;

		in vec2 tex_coord;

		// texture
		uniform sampler2D color_texture;
		
		void main()
		{
			frag_color = texture( color_texture, tex_coord );
		} 
	)~~~";

	const std::string DEBUG_PHYSICS_FRAGMENT_SHADER = R"~~~(
		#version 330 core
		out vec4 frag_color;
		in vec4 color;
		
		void main()
		{
			frag_color = color;
		} 
	)~~~";

	constexpr GLuint POSITION_INDEX = 0;
	constexpr GLuint COLOR_INDEX = 1;
	constexpr GLuint UV_INDEX = 2;
}

const std::string Renderer::DEFAULT_SHADER = "DEFLAULT_SHADER";
const std::string Renderer::DEBUG_PHYSICS_SHADER = "DEBUG_PHYSICS_SHADER";

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

Renderer::Shader::~Shader()
{
	glDeleteShader( mId );
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
Renderer::Renderable::Renderable( std::vector<Vertex>&& vertices, std::string shader_name, unsigned int texture_id, DrawType draw_type )
	: mVBO( 0 )
	, mVAO( 0 )
	, mNumberOfVertices( static_cast<int>( vertices.size() ) )
	, mShader( shader_name )
	, mTexture( texture_id )
	, mDrawType( draw_type )
{
	glGenVertexArrays( 1, &mVAO );
	glGenBuffers( 1, &mVBO ); 

	glBindVertexArray( mVAO );
	glBindBuffer( GL_ARRAY_BUFFER, mVBO );
	// 申请显存空间来放顶点数据
	glBufferData( GL_ARRAY_BUFFER, mNumberOfVertices * sizeof( Vertex ), &vertices.front(), GL_STATIC_DRAW );
	// 绑定顶点位置数据到 POSITION_INDEX
	glVertexAttribPointer( POSITION_INDEX, 3, GL_FLOAT, GL_FALSE, sizeof( Vertex ), (void*)0 );
	glEnableVertexAttribArray( POSITION_INDEX );
	// 绑定顶点颜色数据到 COLOR_INDEX
	glVertexAttribPointer( COLOR_INDEX, 4, GL_FLOAT, GL_FALSE, sizeof( Vertex ), (void*)( sizeof( glm::vec3 ) ) );
	glEnableVertexAttribArray( COLOR_INDEX );
	// 绑定顶点UV数据到 UV_INDEX
	glVertexAttribPointer( UV_INDEX, 2, GL_FLOAT, GL_FALSE, sizeof( Vertex ), (void*)( sizeof( glm::vec3 ) + sizeof( glm::vec4 ) ) );
	glEnableVertexAttribArray( UV_INDEX );
}

Renderer::Renderable::~Renderable()
{
	glDeleteBuffers( 1, &mVBO );
	glDeleteVertexArrays( 1, &mVAO );
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

unsigned int
Renderer::Renderable::GetTexture() const
{
	return mTexture;
}

Renderer::Renderable::DrawType 
Renderer::Renderable::GetDrawType() const
{
	return mDrawType;
}

///
/// Resources implementations
/// 
Renderer::Resources::Resources()
{
}

bool 
Renderer::Resources::LoadTexture( const std::string& path )
{
	bool success = false;
	int width, height, num_channels;
	unsigned char* data = stbi_load( path.c_str(), &width, &height, &num_channels, 0 );
	if( data )
	{
		unsigned int texture_id;
		glGenTextures( 1, &texture_id );
		glBindTexture( GL_TEXTURE_2D, texture_id );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );	
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data );
		glGenerateMipmap( GL_TEXTURE_2D );

		mLoadedTextures.emplace( path, texture_id );
		success = true;
	}
	else
	{
		std::cerr << "ERROR: Failed to load texture file " << path << std::endl;
	}
	stbi_image_free( data );
	return success;
}

unsigned int 
Renderer::Resources::GetTextureId( const std::string& path )
{
	auto itr = mLoadedTextures.find( path );
	if( itr != mLoadedTextures.end() )
	{
		return itr->second;
	}
	else
	{
		std::cerr << "ERROR: Cannot find the required texture " << path << std::endl;
		return 0; // TODO: return a default texture
	}
}

bool 
Renderer::Resources::CompileShader( const std::string& name, std::string vertex_shader, std::string fragment_shader )
{
	Shader shader{ std::move( vertex_shader ), std::move( fragment_shader ) };
	if( shader.IsValid() )
	{
		mCompiledShaders.emplace( name, std::move( shader ) );
		return true;
	}
	return false;
}

Renderer::Shader&
Renderer::Resources::GetShader( const std::string& name )
{
	auto itr = mCompiledShaders.find( name );
	if( itr != mCompiledShaders.end() )
	{
		return itr->second;
	}
	// 需求的shader不存在，改用默认shader
	else
	{
		return mCompiledShaders[ DEFAULT_SHADER ];
	}
}

///
/// Renderer implementations
/// 
Renderer::Renderer()
	: mActiveCamera( nullptr )
	, mProjectionMatrix( glm::mat4( 1.f ) )
{
	mResources = std::make_unique<Resources>();

	glEnable( GL_DEPTH_TEST );
	glEnable( GL_CULL_FACE );
	glCullFace( GL_BACK );
	glFrontFace( GL_CW );
	if( !mResources->CompileShader( DEFAULT_SHADER, DEFAULT_VERTEX_SHADER, DEFAULT_FRAGMENT_SHADER ) )
	{
		std::cerr << "ERROR: Failed to compile default shaders." << std::endl;
	}
	if( !mResources->CompileShader( DEBUG_PHYSICS_SHADER, DEFAULT_VERTEX_SHADER, DEBUG_PHYSICS_FRAGMENT_SHADER ) )
	{
		std::cerr << "ERROR: Failed to compile default shaders." << std::endl;
	}
}

Renderer::~Renderer()
{
}

void
Renderer::AddToRenderQueue( Renderable* renderable_obj )
{
	if( renderable_obj )
	{
		mRenderableList.emplace_back( std::move( renderable_obj ) );
	}
}

void 
Renderer::Render()
{
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	for( auto& renderable : mRenderableList )
	{
		RenderOneoff( renderable );
	}
}

void 
Renderer::RenderOneoff( Renderable* renderable_obj )
{
	if( !renderable_obj )
	{
		return;
	}
	glBindTexture( GL_TEXTURE_2D, renderable_obj->GetTexture() );
	auto shader = mResources->GetShader( renderable_obj->GetShaderName() );
	glUseProgram( shader.GetId() );
	if( mActiveCamera )
	{
		// 传递当前摄像机的视图投影矩阵到当前Shader
		shader.SetViewMatrix( mActiveCamera->GetViewMatrix() );
		shader.SetProjectionMatrix( mProjectionMatrix );
	}
	glBindVertexArray( renderable_obj->GetVAO() );
	glDrawArrays( 
		renderable_obj->GetDrawType() == Renderable::DrawType::TRIGANLES ? GL_TRIANGLES : GL_LINES, 
		0, 
		renderable_obj->GetNumberOfVertices() );
}

void 
Renderer::SetCameraAsActive( std::shared_ptr<Camera> camera )
{
	mActiveCamera = camera;
	mProjectionMatrix = mActiveCamera->GetProjectionMatrix();
}

Renderer::Resources&
Renderer::GetResources()
{
	return *mResources.get();
}
