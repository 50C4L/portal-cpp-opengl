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
#include "BuiltInShaders.h"

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
const std::string Renderer::DEFAULT_SKYBOX_SHADER = "DEFAULT_SKYBOX_SHADER";
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
Renderer::Renderable::Renderable( std::vector<Vertex>&& vertices, std::string shader_name, TextureInfo* texture_id, DrawType draw_type )
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

void 
Renderer::Renderable::SetTransform( glm::mat4 trans )
{
	mIsDirty = false;
	mTransform = std::move( trans );
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

TextureInfo*
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

		mLoadedTextures.emplace( path, TextureInfo{ texture_id, GL_TEXTURE_2D } );
		success = true;
	}
	else
	{
		std::cerr << "ERROR: Failed to load texture file " << path << std::endl;
	}
	stbi_image_free( data );
	return success;
}

bool 
Renderer::Resources::LoadCubeMaps( std::vector<std::string> files, const std::string& name )
{
	TextureInfo tex_info{};
	tex_info.texture_id = GL_TEXTURE_CUBE_MAP;
	glGenTextures( 1, &tex_info.texture_id );
	glBindTexture( GL_TEXTURE_CUBE_MAP, tex_info.texture_id );

	int width, height, nrChannels;
	for (unsigned int i = 0; i < files.size(); i++)
	{
		unsigned char *data = stbi_load( files[i].c_str(), &width, &height, &nrChannels, 0 );
		if (data)
		{
			glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
						  0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);
			stbi_image_free(data);
		}
		else
		{
			std::cerr << "ERROR: Cubemap tex failed to load at path: " << files[i] << std::endl;
			stbi_image_free(data);
			return false;
		}
	}
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );

	mLoadedTextures.emplace( name, std::move( tex_info ) );

	return true;
}


TextureInfo*
Renderer::Resources::GetTextureInfo( const std::string& path )
{
	auto itr = mLoadedTextures.find( path );
	if( itr != mLoadedTextures.end() )
	{
		return &(itr->second);
	}
	else
	{
		// TODO: 返回一个紫色之类的默认贴图
		static TextureInfo default_tex{ 0, GL_TEXTURE_2D };
		std::cerr << "ERROR: Cannot find the required texture " << path << std::endl;
		return &default_tex; 
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
	: mProjectionMatrix( glm::mat4( 1.f ) )
	, mViewMatrix( glm::mat4( 1.f ) )
	, mViewportSize( { 0, 0 } )
{
	mResources = std::make_unique<Resources>();

	glEnable( GL_DEPTH_TEST );
	glEnable( GL_CULL_FACE );
	glCullFace( GL_BACK );
	glFrontFace( GL_CW );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ); 

	// 编译内置shader
	// TODO: 从文件加载Shader
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
	if( !mResources->CompileShader( DEFAULT_SKYBOX_SHADER, DEFAULT_SKYBOX_VERTEX_SHADER, DEFAULT_SKYBOX_FRAGMENT_SHADER ) )
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
	if( auto tex_ptr =  renderable_obj->GetTexture() )
	{
		glBindTexture( tex_ptr->tex_type, tex_ptr->texture_id );
	}
	auto shader = mResources->GetShader( renderable_obj->GetShaderName() );
	glUseProgram( shader.GetId() );
	shader.SetModelMatrix( renderable_obj->GetTransform() );
	shader.SetViewMatrix( mViewMatrix );
	shader.SetProjectionMatrix( mProjectionMatrix );
	glBindVertexArray( renderable_obj->GetVAO() );
	glDrawArrays( 
		get_gl_draw_mode( renderable_obj->GetDrawType() ), 
		0, 
		renderable_obj->GetNumberOfVertices() );
}

void 
Renderer::UseCameraMatrix( Camera* camera )
{
	mViewMatrix = camera->GetViewMatrix();
	mProjectionMatrix = camera->GetProjectionMatrix();
}

void
Renderer::SetViewMatrix( glm::mat4 view )
{
	mViewMatrix = std::move( view );
}

void
Renderer::SetProjectionMatrix( glm::mat4 projection )
{
	mProjectionMatrix = std::move( projection );
}

Renderer::Resources&
Renderer::GetResources()
{
	return *mResources.get();
}
