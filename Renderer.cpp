#include "Renderer.h"
#include <string>
#include <iostream>
#include <filesystem>

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
	const std::string MODEL_MATRIX_UNIFORM_NAME = "model_mat";
	const std::string VIEW_MATRIX_UNIFORM_NAME = "view_mat";
	const std::string PROJECTION_MATRIX_UNIFORM_NAME = "projection_mat";
	constexpr GLuint POSITION_INDEX = 0;
	constexpr GLuint COLOR_INDEX = 1;
	constexpr GLuint UV_INDEX = 2;
	constexpr GLuint NORMAL_INDEX = 3;

	const std::string DEFAULT_VERTEX_SHADER = R"~~~(
		#version 330 core
		layout (location = 0) in vec3 in_pos;
		layout (location = 1) in vec4 in_color;
		layout (location = 2) in vec2 in_uv;
		layout (location = 3) in vec3 in_normal;

		out vec2 tex_coord;
		out vec4 color;
		out vec3 frag_pos;
		out vec3 normal;

		uniform mat4 model_mat;
		uniform mat4 view_mat;
		uniform mat4 projection_mat;
		
		void main()
		{
			gl_Position = projection_mat * view_mat * model_mat * vec4( in_pos, 1.0 );
			frag_pos = vec3( model_mat * vec4( in_pos, 1.0 ) );
			tex_coord = in_uv;
			color = in_color;
			vec4 temp_normal = model_mat * vec4( in_normal, 1.0 );
			normal = temp_normal.xyz;
		}
	)~~~";

	const std::string DEFAULT_FRAGMENT_SHADER = R"~~~(
		#version 330 core
		out vec4 frag_color;

		in vec2 tex_coord;
		in vec3 frag_pos;
		in vec3 normal;

		// texture
		uniform sampler2D color_texture;

		void main()
		{
			vec3 light_color = vec3( 1.0, 1.0, 1.0 );
			vec3 light_pos = vec3( 1000.0, 1000.0, 500.0 );

			// Ambient
			vec3 ambient = 0.15 * light_color;

			// Diffuse
			vec3 n_normal = normalize( normal );
			vec3 light_dir = normalize( light_pos - frag_pos );
			float diff_f = max( dot( n_normal, light_dir ), 0.0 );
			vec3 diffuse = diff_f * light_color;
			
			// Final
			vec4 tex_color = texture( color_texture, tex_coord );
			vec3 result = ( ambient + diffuse ) * tex_color.rgb;
			frag_color = vec4( result, tex_color.a );
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

	const std::string PORTAL_HOLE_FRAGMENT_SHADER = R"~~~(
		#version 330 core
		out vec4 frag_color;
		in vec4 color;
		
		void main()
		{
			frag_color = color;
		} 
	)~~~";

	const std::string PORTAL_FRAME_FRAGMENT_SHADER = R"~~~(
		#version 330 core
		out vec4 frag_color;
		in vec2 tex_coord;

		uniform sampler2D color_texture;
		
		void main()
		{
			frag_color = texture( color_texture, tex_coord );
		} 
	)~~~";

	int get_gl_draw_mode( Renderer::Renderable::DrawType type )
	{
		switch( type )
		{
		case Renderer::Renderable::DrawType::LINES:
			return GL_LINES;
		case Renderer::Renderable::DrawType::TRIANGLE_FANS:
			return GL_TRIANGLE_FAN;
		case Renderer::Renderable::DrawType::TRIANGLES:
		default:
			return GL_TRIANGLES;
		}
	}
}

const std::string Renderer::DEFAULT_SHADER = "DEFLAULT_SHADER";
const std::string Renderer::DEBUG_PHYSICS_SHADER = "DEBUG_PHYSICS_SHADER";
const std::string Renderer::PORTAL_HOLE_SHADER = "PORTAL_HOLE_SHADER";
const std::string Renderer::PORTAL_FRAME_SHADER = "PORTAL_FRAME_SHADER";

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
	mModelMatUniformLocation = glGetUniformLocation( mId, MODEL_MATRIX_UNIFORM_NAME.c_str() );
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
Renderer::Shader::SetModelMatrix( const glm::mat4& matrix )
{
	SetMat4( mModelMatUniformLocation, matrix );
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
	, mTranslation( 0.f )
	, mRotation( 0.f )
	, mTransform( 1.f )
	, mIsDirty( false )
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
	// 绑定顶点法线数据到 NORMAL_INDEX
	glVertexAttribPointer( NORMAL_INDEX, 3, GL_FLOAT, GL_FALSE, sizeof( Vertex ), (void*)( sizeof( glm::vec3 ) + sizeof( glm::vec4 ) + sizeof( glm::vec2 ) ) );
	glEnableVertexAttribArray( NORMAL_INDEX );
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

void 
Renderer::Renderable::Translate( glm::vec3 offset )
{
	mIsDirty = true;
	mTranslation = std::move( offset );
}

void 
Renderer::Renderable::Rotate( float angle, glm::vec3 axis )
{
	mIsDirty = true;
	mRotation = axis * angle;
}

glm::mat4
Renderer::Renderable::GetTransform()
{
	if( !mIsDirty )
	{
		return mTransform;
	}
	else
	{
		glm::mat4 trans( 1.f );
		trans = glm::translate( trans, mTranslation );
		trans = glm::rotate( trans, mRotation.x, { 1.f, 0.f, 0.f} );
		trans = glm::rotate( trans, mRotation.y, { 0.f, 1.f, 0.f} );
		trans = glm::rotate( trans, mRotation.z, { 0.f, 0.f, 1.f} );
		mTransform = std::move( trans );
		mIsDirty = false;
		return mTransform;
	}
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
	auto gl_color_channel = num_channels == 3 ? GL_RGB : GL_RGBA;
	if( data )
	{
		unsigned int texture_id;
		glGenTextures( 1, &texture_id );
		glBindTexture( GL_TEXTURE_2D, texture_id );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );	
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexImage2D( GL_TEXTURE_2D, 0, gl_color_channel, width, height, 0, gl_color_channel, GL_UNSIGNED_BYTE, data );
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
	, mViewportSize( { 0, 0 } )
{
	mResources = std::make_unique<Resources>();

	glEnable( GL_DEPTH_TEST );
	glEnable( GL_CULL_FACE );
	glCullFace( GL_BACK );
	glFrontFace( GL_CW );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ); 
	if( !mResources->CompileShader( DEFAULT_SHADER, DEFAULT_VERTEX_SHADER, DEFAULT_FRAGMENT_SHADER ) )
	{
		std::cerr << "ERROR: Failed to compile default shaders." << std::endl;
	}
	if( !mResources->CompileShader( DEBUG_PHYSICS_SHADER, DEFAULT_VERTEX_SHADER, DEBUG_PHYSICS_FRAGMENT_SHADER ) )
	{
		std::cerr << "ERROR: Failed to compile default shaders." << std::endl;
	}
	if( !mResources->CompileShader( PORTAL_HOLE_SHADER, DEFAULT_VERTEX_SHADER, PORTAL_HOLE_FRAGMENT_SHADER ) )
	{
		std::cerr << "ERROR: Failed to compile default shaders." << std::endl;
	}
	if( !mResources->CompileShader( PORTAL_FRAME_SHADER, DEFAULT_VERTEX_SHADER, PORTAL_FRAME_FRAGMENT_SHADER ) )
	{
		std::cerr << "ERROR: Failed to compile default shaders." << std::endl;
	}
}

Renderer::~Renderer()
{
}

void 
Renderer::ResizeViewport( glm::ivec2 size )
{
	// 重新设置viewport
	glViewport( 0, 0, size.x, size.y );
	mViewportSize = size;
}

glm::ivec2 
Renderer::GetViewportSize()
{
	return mViewportSize;
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
		shader.SetModelMatrix( renderable_obj->GetTransform() );
		shader.SetViewMatrix( mActiveCamera->GetViewMatrix() );
		shader.SetProjectionMatrix( mProjectionMatrix );
	}
	glBindVertexArray( renderable_obj->GetVAO() );
	glDrawArrays( 
		get_gl_draw_mode( renderable_obj->GetDrawType() ), 
		0, 
		renderable_obj->GetNumberOfVertices() );
}

void 
Renderer::SetCameraAsActive( Camera* camera )
{
	mActiveCamera = camera;
	mProjectionMatrix = mActiveCamera->GetProjectionMatrix();
}

Renderer::Resources&
Renderer::GetResources()
{
	return *mResources.get();
}
