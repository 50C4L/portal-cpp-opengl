#include "LevelController.h"

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

	mPortals[0] = { false, std::make_unique<Portal>( Renderer::DEFAULT_SHADER, mRenderer.GetResources().GetTextureId( "resources/blueportal.png" ) ) };
	mPortals[1] = { false, std::make_unique<Portal>( Renderer::DEFAULT_SHADER, mRenderer.GetResources().GetTextureId( "resources/orangeportal.png" ) ) };
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

	// 改变玩家出生点
	auto view_size = mRenderer.GetViewportSize();
	mMainCamera = std::make_shared<Camera>( static_cast<float>( view_size.x ), static_cast<float>( view_size.y ), Camera::Type::FPS );
	mPlayer = std::make_unique<Player>( *mPhysics );
	mPlayer->Spawn( { 0.f, 20.f, 0.f }, mMainCamera );

	// 把关卡加入到渲染队列
	auto& walls = itr->second->GetWalls();
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
		mRenderer.AddToRenderQueue( wall.render_instance.get() );
	}
	mRenderer.SetCameraAsActive( mMainCamera );
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
		if( portal_info[i].is_active && mPortals[i].portal->UpdatePosition( portal_info[i].position, portal_info[i].face_dir ) )
		{
			mRenderer.RemoveFromRenderQueue( mPortals[i].portal.get() );
			mRenderer.AddToRenderQueue( mPortals[i].portal.get() );
		}
		// TODO check portal position
		mPortals[i].is_active = true;
	}
}
