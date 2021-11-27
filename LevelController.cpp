#include "LevelController.h"

#include <GL/glew.h>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <string>
#include <fstream>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

#include "ScenePrimitives.h"
#include "Renderer.h"
#include "Camera.h"
#include "Portal.h"

using namespace portal;
using namespace portal::physics;

namespace
{
	constexpr int PORTAL_1 = 0;
	constexpr int PORTAL_2 = 1;
	const int MAX_PORTAL_RECURSION = 4;

	glm::vec3
	extract_view_postion_from_matrix( const glm::mat4 view_matrix )
	{
		glm::mat4 model_view_t = glm::transpose( view_matrix );
  
		// Get plane normals 
		glm::vec3 n1( model_view_t[0] );
		glm::vec3 n2( model_view_t[1] );
		glm::vec3 n3( model_view_t[2] );

		// Get plane distances
		float d1( model_view_t[0].w );
		float d2( model_view_t[1].w );
		float d3( model_view_t[2].w );

		// Get the intersection of these 3 planes
		glm::vec3 n2n3 = cross( n2, n3 );
		glm::vec3 n3n1 = cross( n3, n1 );
		glm::vec3 n1n2 = cross( n1, n2 );

		glm::vec3 top = ( n2n3 * d1 ) + ( n3n1 * d2 ) + ( n1n2 * d3 );
		float denom = dot( n1, n2n3 );

		return top / -denom;
	}
}

///
/// Level class implementations
/// 
LevelController::Level::Level()
	: mIsBuilt( false )
	, mSpawnPoint( 0.f )
{}

void
LevelController::Level::AddWall( Wall&& wall )
{
	mWalls.emplace_back( std::move( wall ) );
}

std::vector<LevelController::Level::Wall>& 
LevelController::Level::GetWalls()
{
	return mWalls;
}

void
LevelController::Level::SetSpawn( glm::vec3 point )
{
	mSpawnPoint = std::move( point );
}

glm::vec3 
LevelController::Level::GetSpawn() const
{
	return mSpawnPoint;
}

///
/// LevelController implementations
/// 
LevelController::LevelController( Renderer& renderer )
	: mRenderer( renderer )
	, mMouseX( 0 )
	, mMouseY( 0 )
	, mCurrentLevel( nullptr )
	, mMainCamProjMat( glm::mat4( 1.f ) )
{
}

LevelController::~LevelController()
{
}

void
LevelController::Initialize( int update_interval_ms )
{
	mPhysics = std::make_unique<Physics>( mRenderer );
	mPhysics->Initialize( update_interval_ms / 1000.f );
}

bool 
LevelController::LoadLevelFile( const std::string& path )
{
	std::ifstream ifs{ path };
	if( !ifs.is_open() )
	{
		std::cerr << "ERROR: Failed to open level json file " << path << std::endl;
		return false;
	}

	rapidjson::IStreamWrapper isw{ ifs };
	rapidjson::Document json_doc;
	json_doc.ParseStream( isw );
	if( json_doc.HasParseError() )
	{
		std::cerr << "ERROR: Failed to parse json file level: " << path << ", msg: " << std::to_string( static_cast<int>( json_doc.GetParseError() ) ) << std::endl;
		return false;
	}

	std::unique_ptr<Level> level = std::make_unique<Level>();
	if( json_doc.HasMember( "Spawn" ) )
	{
		level->SetSpawn(
			glm::vec3{
				json_doc[ "Spawn" ][ "x" ].GetFloat(),
				json_doc[ "Spawn" ][ "y" ].GetFloat(),
				json_doc[ "Spawn" ][ "z" ].GetFloat()
			}
		);
	}

	if( json_doc.HasMember( "Walls" ) )
	{
		std::string texture_path;
		std::string shader_name;
		rapidjson::Value& walls_obj = json_doc[ "Walls" ];
		if( walls_obj.HasMember( "texture" ) )
		{
			texture_path = walls_obj[ "texture" ].GetString();
		}

		if( walls_obj.HasMember( "shader" ) )
		{
			shader_name = walls_obj[ "shader" ].GetString();
		}

		if( walls_obj.HasMember( "build" ) )
		{
			rapidjson::Value& build_obj = walls_obj[ "build" ];
			for( auto itr = build_obj.MemberBegin(); itr != build_obj.MemberEnd(); itr++ )
			{
				Level::Wall wall;
				wall.texture_path = texture_path;
				wall.shader_name = shader_name;
				if( itr->value.HasMember( "pos" ) )
				{
					wall.position = glm::vec3( 
						itr->value[ "pos" ][ "x" ].GetFloat(),
						itr->value[ "pos" ][ "y" ].GetFloat(),
						itr->value[ "pos" ][ "z" ].GetFloat()
					);
				}
				if( itr->value.HasMember( "width" ) )
				{
					wall.width = itr->value[ "width" ].GetFloat();
				}
				if( itr->value.HasMember( "height" ) )
				{
					wall.height = itr->value[ "height" ].GetFloat();
				}
				if( itr->value.HasMember( "depth" ) )
				{
					wall.depth = itr->value[ "depth" ].GetFloat();
				}
				level->AddWall( std::move( wall ) );
			}
		}
	}
	mLevels[ path ] = std::move( level );

	return true;
}

void 
LevelController::ChangeLevelTo( const std::string& path )
{
	auto itr = mLevels.find( path );
	if( itr == mLevels.end() )
	{
		std::cerr << "ERROR: Failed to change to level " << path << ", level is not loaded." << std::endl;
		return;
	}

	// TODO: Release the previous level
	mCurrentLevel = itr->second.get();

	// 改变玩家出生点
	auto view_size = mRenderer.GetViewportSize();
	const float view_width = static_cast<float>( view_size.x );
	const float view_height = static_cast<float>( view_size.y );
	mMainCamera = std::make_shared<Camera>( view_width, view_height, Camera::Type::FPS );
	mMainCamProjMat = mMainCamera->GetProjectionMatrix();
	mPlayer = std::make_unique<Player>( *mPhysics );
	mPlayer->Spawn( { 0.f, 20.f, 0.f }, mMainCamera );

	mPortals[PORTAL_1] = std::make_unique<Portal>( mRenderer.GetResources().GetTextureId( "resources/textures/blueportal.png" ), view_width, view_height );
	mPortals[PORTAL_2] = std::make_unique<Portal>( mRenderer.GetResources().GetTextureId( "resources/textures/orangeportal.png" ), view_width, view_height );
	mPortals[PORTAL_1]->SetPair( mPortals[PORTAL_2].get() );
	mPortals[PORTAL_2]->SetPair( mPortals[PORTAL_1].get() );

	// 根据关卡数据生成静态物体
	auto& walls = mCurrentLevel->GetWalls();
	for( auto& wall : walls )
	{
		wall.render_instance = std::make_unique<SceneBox>(
			wall.position,
			wall.width,
			wall.height,
			wall.depth,
			wall.shader_name,
			mRenderer.GetResources().GetTextureId( wall.texture_path )
		);
		wall.mCollisionBox = mPhysics->CreateBox( wall.position, { wall.width, wall.height, wall.depth }, Physics::PhysicsObject::Type::STATIC );
	}
	mRenderer.UseCameraMatrix( mMainCamera.get() );
}

void
LevelController::Update()
{
	if( mPlayer )
	{
		mPlayer->Update();
	}
	if( mPhysics )
	{
		mPhysics->Update();
	}

	UpdatePortalState();
}

void 
LevelController::HandleKeys( std::unordered_map<unsigned int, bool>& key_map )
{
	mPlayer->HandleKeys( key_map );
}

void 
LevelController::HandleMouseMove( int x, int y )
{
	float x_offset = static_cast<float>( x - mMouseX );
	float y_offset = static_cast<float>( mMouseY - y );
	mMouseX = x;
	mMouseY = y;

	const float sensitivitiy = 0.3f;
	x_offset *= sensitivitiy;
	y_offset *= sensitivitiy;
	mPlayer->Look( x_offset, y_offset );
}

void 
LevelController::HandleMouseButton( std::unordered_map<int, bool>& button_map )
{
	mPlayer->HandleMouse( button_map );
}

void
LevelController::RenderScene()
{
	if( mPortals[ PORTAL_1 ]->IsLinkActive() )
	{
		RenderPortals( mMainCamera.get()->GetViewMatrix(), mMainCamProjMat );
	}
	else
	{
		RenderBaseScene( mMainCamera.get()->GetViewMatrix(), mMainCamProjMat );
	}
}

void
LevelController::RenderDebugInfo()
{
	if( mPhysics )
	{
		mPhysics->DebugRender();
	}
}

void
LevelController::UpdatePortalState()
{
	const auto& portal_info = mPlayer->GetPortalInfo();
	for( int i = 0; i < portal_info.size(); i++ )
	{
		if( portal_info[i].is_active )
		{
			mPortals[i]->UpdatePosition( portal_info[i].position, portal_info[i].face_dir );
		}
	}
}

void 
LevelController::RenderPortals( glm::mat4 view_matrix, glm::mat4 projection_matrix, int current_recursion_level )
{
	for( auto& portal : mPortals )
	{
		// 关闭颜色和深度缓存写入
		glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );
		glDepthMask( GL_FALSE );
		glDisable( GL_DEPTH_TEST );

		// 开启模板测试，确保传送门的内容只画在传送门里面
		glEnable( GL_DEPTH_TEST );
		// 设置模板测试为：
		// 当模板像素值不等于current_recursion_level时，测试通过
		glStencilFunc( GL_NOTEQUAL, current_recursion_level, 0xFF );
		// 测试不同通过的像素模板值+1，其他情况保持原有值
		glStencilOp( GL_INCR, GL_KEEP, GL_KEEP );
		// 表示每个像素8位的模板值都可用（就是传送门最多可以嵌套255次)
		glStencilMask( 0xFF );

		// 绘制传送门窗口
		// 比如这里时current_recursion_level = 0第一层
		// 屏幕上传送门窗口覆盖的位置会因为规则 glStencilFunc( GL_NOTEQUAL, current_recursion_level, 0xFF )
		// 不通过测试，因此它所覆盖的像素模板值会根据 glStencilOp( GL_INCR, GL_KEEP, GL_KEEP ) 进行current_recursion_level+1
		// 结果是模板缓存中除了传送门窗口的像素是1，其他都是0
		mRenderer.SetViewMatrix( view_matrix );
		mRenderer.SetProjectionMatrix( projection_matrix );
		mRenderer.RenderOneoff( portal->GetHoleRenderable() );

		// 将当前的摄像机视图矩阵变换到配对的传送门后相对的位置
		glm::mat4 portal_view = portal->ConvertView( view_matrix );
		// 因为新的虚拟摄像机在传送门后，为了不被传送门后的墙挡住视线，我们将投影矩阵的近裁切面设置在传送门的位置
		glm::vec3 cam_pos = extract_view_postion_from_matrix( portal_view );
		float distance_to_portal =  glm::length( cam_pos - portal->GetPairedPortal()->mPosition );
		glm::mat4 portal_cam_proj_mat = 
			glm::perspective( 
				glm::radians( 90.f ),
				16.f / 9.f,
				distance_to_portal,
				1000.f
			);

		// 这是最底层了，渲染最底层的传送门内容
		if( current_recursion_level == MAX_PORTAL_RECURSION )
		{
			// 允许颜色和深度写入
			glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
			glDepthMask( GL_TRUE );

			// 清理深度缓存
			// 开启深度测试
			glClear( GL_DEPTH_BUFFER_BIT );
			glEnable( GL_DEPTH_TEST );

			// 开启模板测试，确保我们只在传送门内绘制
			glEnable( GL_STENCIL_TEST );
			// 不再允许对模板进行写入
			glStencilMask( 0x00 );
			// 只对通过模板测试（current_recursion_level + 1)的像素进行绘制
			glStencilFunc( GL_EQUAL, current_recursion_level + 1, 0xFF );

			RenderBaseScene( portal_view, portal_cam_proj_mat );
		}
		else
		{
			// 如果这还不是最底层，我们进行递归
			// 把这个传送门配对传送门的摄像机传到递归函数中进行绘制，并且将递归层数+1确保递归会结束
			RenderPortals( portal_view, portal_cam_proj_mat, current_recursion_level + 1 );
		}

		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glDepthMask(GL_FALSE);

		glEnable(GL_STENCIL_TEST);
		glStencilMask(0xFF);

		// 上面渲染的传送门内部会不通过这个模板测试
		glStencilFunc( GL_NOTEQUAL, current_recursion_level + 1, 0xFF );

		// 不通过测试的像素模板值会-1，直到退回到递归最高层时我们最终的模板缓存会全部变为0
		glStencilOp( GL_DECR, GL_KEEP, GL_KEEP );

		mRenderer.SetProjectionMatrix( portal_cam_proj_mat );
		mRenderer.SetViewMatrix( view_matrix );
		for( auto& portal : mPortals )
		{
			mRenderer.RenderOneoff( portal->GetHoleRenderable() );
		}
	}
	
	// 关闭模板测试和颜色写入
	glDisable( GL_STENCIL_TEST );
	glStencilMask( 0x00 );
	glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );

	// 开启深度测试和颜色写入
	glEnable( GL_DEPTH_TEST );
	glDepthMask( GL_TRUE );

	// 深度测试设置为通过，也就是所有东西都会被写入到深度缓存里
	glDepthFunc( GL_ALWAYS );
	glClear( GL_DEPTH_BUFFER_BIT );

	// 将两个传送门的窗口写入到深度缓存
	mRenderer.SetProjectionMatrix( projection_matrix );
	mRenderer.SetViewMatrix( view_matrix );
	for( auto& portal : mPortals )
	{
		mRenderer.RenderOneoff( portal->GetHoleRenderable() );
	}
	// 将深度测试设回默认（近的挡住远的）
	glDepthFunc( GL_LESS );

	// 开启模板测试，关闭对模板缓存的写入
	glEnable(GL_STENCIL_TEST);
	glStencilMask(0x00);
	glStencilFunc( GL_LEQUAL, current_recursion_level, 0xFF );

	// 一切恢复正常
	glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
	glDepthMask( GL_TRUE );
	glEnable(GL_DEPTH_TEST);
	// 绘制正常的场景
	RenderBaseScene( view_matrix, projection_matrix );
}

void 
LevelController::RenderBaseScene( glm::mat4 view_matrix, glm::mat4 projection_matrix )
{
	mRenderer.SetProjectionMatrix( std::move( projection_matrix ) );
	mRenderer.SetViewMatrix( std::move( view_matrix ) );
	// 绘制除了“真传送门”以外的场景
	auto& walls = mCurrentLevel->GetWalls();
	for( auto& wall : walls )
	{
		mRenderer.RenderOneoff( wall.render_instance.get() );
	}
	// 绘制传送门的框
	for( auto& portal : mPortals )
	{
		if( portal->HasBeenPlaced() )
		{
			mRenderer.RenderOneoff( portal->GetFrameRenderable() );
		}
	}
	//RenderDebugInfo();
}
