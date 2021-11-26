#include "LevelController.h"

#include <GL/glew.h>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <string>
#include <fstream>
#include <iostream>

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
	const int MAX_PORTAL_RECURSION = 2;
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
	mPlayer = std::make_unique<Player>( *mPhysics );
	mPlayer->Spawn( { 0.f, 20.f, 0.f }, mMainCamera );

	mPortals[PORTAL_1] = std::make_unique<Portal>( mRenderer.GetResources().GetTextureId( "resources/textures/blueportal.png" ), view_width, view_height );
	mPortals[PORTAL_2] = std::make_unique<Portal>( mRenderer.GetResources().GetTextureId( "resources/textures/orangeportal.png" ), view_width, view_height );
	mPortals[PORTAL_1]->SetPair( mPortals[PORTAL_2].get(), mMainCamera.get() );
	mPortals[PORTAL_2]->SetPair( mPortals[PORTAL_1].get(), mMainCamera.get() );

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
	mRenderer.SetCameraAsActive( mMainCamera.get() );
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
		RenderPortals( mMainCamera.get() );
	}
	else
	{
		mRenderer.SetCameraAsActive( mMainCamera.get() );
		RenderBaseScene();
		RenderDebugInfo();
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
LevelController::RenderPortals( Camera* camera, int current_recursion_level )
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
		glStencilMask(0xFF);

		// 绘制传送门窗口
		// 比如这里时current_recursion_level = 0第一层
		// 屏幕上传送门窗口覆盖的位置会因为规则 glStencilFunc( GL_NOTEQUAL, current_recursion_level, 0xFF )
		// 不通过测试，因此它所覆盖的像素模板值会根据 glStencilOp( GL_INCR, GL_KEEP, GL_KEEP ) 进行current_recursion_level+1
		// 结果是模板缓存中除了传送门窗口的像素是1，其他都是0
		mRenderer.SetCameraAsActive( camera );
		mRenderer.RenderOneoff( portal->GetHoleRenderable() );

		if( current_recursion_level == MAX_PORTAL_RECURSION )
		{
		}
		else
		{
		}
	}
	// Disable the stencil test and stencil writing
	glDisable(GL_STENCIL_TEST);
	glStencilMask(0x00);

	// Disable color writing
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	// Enable the depth test, and depth writing.
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

	// Make sure we always write the data into the buffer
	glDepthFunc(GL_ALWAYS);

	// Clear the depth buffer
	glClear(GL_DEPTH_BUFFER_BIT);

	// Draw portals into depth buffer
	mRenderer.SetCameraAsActive( camera );
	for( auto& portal : mPortals )
	{
		mRenderer.RenderOneoff( portal->GetHoleRenderable() );
	}

	// Reset the depth function to the default
	glDepthFunc(GL_LESS);

	// Enable stencil test and disable writing to stencil buffer
	glEnable(GL_STENCIL_TEST);
	glStencilMask(0x00);

	// Draw at stencil >= recursionlevel
	// which is at the current level or higher (more to the inside)
	// This basically prevents drawing on the outside of this level.
	glStencilFunc(GL_LEQUAL, current_recursion_level, 0xFF);

	// Enable color and depth drawing again
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);

	// And enable the depth test
	glEnable(GL_DEPTH_TEST);

	// Draw scene objects normally, only at recursionLevel
	RenderBaseScene();
}

void 
LevelController::RenderBaseScene()
{
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
}
