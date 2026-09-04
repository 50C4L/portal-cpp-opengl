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
#include "LevelConstants.h"
#include "Utility.h"
#include "DynamicBox.h"
#include "Player.h"

using namespace portal;
using namespace portal::physics;
using namespace portal::level;

namespace
{
	constexpr int PORTAL_1 = 0;
	constexpr int PORTAL_2 = 1;
	const int MAX_PORTAL_RECURSION = 5;
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

	// Change the player spawn
	auto view_size = mRenderer.GetViewportSize();
	const float view_width = static_cast<float>( view_size.x );
	const float view_height = static_cast<float>( view_size.y );
	mMainCamera = std::make_shared<Camera>( view_width, view_height );
	mMainCamProjMat = mMainCamera->GetProjectionMatrix();
	mPlayer = std::make_unique<Player>( *mPhysics );
	mPlayer->Spawn( mCurrentLevel->GetSpawn(), mMainCamera );

	mSkybox = std::make_unique<SceneSkyBox>( mRenderer.GetResources().GetTextureInfo( "SKYBOX" ) );
	mSkybox->Rotate( glm::radians( 100.f ), { 0.f, 1.f, 0.f } );
	mPortals[PORTAL_1] = std::make_unique<Portal>( mRenderer.GetResources().GetTextureInfo( "resources/textures/blueportal.png" ), *mPhysics );
	mPortals[PORTAL_2] = std::make_unique<Portal>( mRenderer.GetResources().GetTextureInfo( "resources/textures/orangeportal.png" ), *mPhysics );
	mPortals[PORTAL_1]->SetPair( mPortals[PORTAL_2].get() );
	mPortals[PORTAL_2]->SetPair( mPortals[PORTAL_1].get() );

	// Build static objects from the level data
	auto& walls = mCurrentLevel->GetWalls();
	for( auto& wall : walls )
	{
		wall.render_instance = std::make_unique<SceneBox>(
			wall.position,
			wall.width,
			wall.height,
			wall.depth,
			wall.shader_name,
			mRenderer.GetResources().GetTextureInfo( wall.texture_path )
		);
		wall.mCollisionBox = mPhysics->CreateBox( 
			wall.position, 
			{ wall.width, wall.height, wall.depth }, 
			Physics::PhysicsObject::Type::STATIC, 
			static_cast<int>( PhysicsGroup::WALL ),
			static_cast<int>( PhysicsGroup::PLAYER ) | static_cast<int>( PhysicsGroup::RAY )
		);
	}
	mRenderer.UseCameraMatrix( mMainCamera.get() );
	mDyBox = std::make_unique<DynamicBox>( 
		*mPhysics,
		glm::vec3{ 0.f, 30.f, 0.f },
		mRenderer.GetResources().GetTextureInfo( "resources/textures/box.jpg" )
	);
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

	for( auto& portal : mPortals )
	{
		portal->CheckPortalable( mPlayer.get() );
		portal->CheckPortalable( mDyBox.get() );
		if( portal->IsPortalableEntering( mDyBox.get() ) )
		{
			// Clone!
			mDyBox->CloneAt( *portal );
			mRenderClone = true;
		}
	}
	if( !mPortals[PORTAL_1]->IsPortalableEntering( mDyBox.get() ) && !mPortals[PORTAL_2]->IsPortalableEntering( mDyBox.get() ) )
	{
		mRenderClone = false;
	}

	mDyBox->Update();
}

void 
LevelController::HandleKeys( std::unordered_map<unsigned int, bool>& key_map )
{
	mPlayer->HandleKeys( key_map );
	if( key_map['e'] != mShootBoxToggle )
	{
		mShootBoxToggle = key_map['e'];
		if( mShootBoxToggle )
		{
			auto pos = mPlayer->GetPosition();
			auto dir = glm::normalize( mPlayer->GetLookDirection() );
			mDyBox->SetPosition( pos + dir * 8.f );
			mDyBox->Launch( std::move( dir ) * 5000.f );
		}
	}
}

void 
LevelController::HandleMouseMove( int x, int y )
{
	float x_offset = static_cast<float>( mMouseX - x );
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
	mPlayer->HandleMouse( button_map, *mPortals[ PORTAL_1 ], *mPortals[ PORTAL_2 ] );
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
LevelController::RenderPortals( glm::mat4 view_matrix, glm::mat4 projection_matrix, int current_recursion_level )
{
	for( auto& portal : mPortals )
	{
		// Disable color and depth buffer writes
		glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );
		glDepthMask( GL_FALSE );
		glDisable( GL_DEPTH_TEST );

		// Enable stencil testing so portal contents only get drawn inside the portal
		glEnable( GL_DEPTH_TEST );
		// Stencil test is:
		// Pass when the stencil pixel value is not equal to current_recursion_level
		glStencilFunc( GL_NOTEQUAL, current_recursion_level, 0xFF );
		// Pixels that fail the test get stencil +1; everything else keeps its old value
		glStencilOp( GL_INCR, GL_KEEP, GL_KEEP );
		// All 8 bits of each pixel's stencil are usable (so portals can nest at most 255 times)
		glStencilMask( 0xFF );

		// Draw the portal window
		// e.g. here current_recursion_level = 0, the first layer
		// The screen pixels covered by the portal window fail the test because of glStencilFunc( GL_NOTEQUAL, current_recursion_level, 0xFF )
		// so their stencil values get current_recursion_level+1 via glStencilOp( GL_INCR, GL_KEEP, GL_KEEP )
		// Result: stencil buffer is 1 where the portal window is, 0 everywhere else
		mRenderer.SetViewMatrix( view_matrix );
		mRenderer.SetProjectionMatrix( projection_matrix );
		mRenderer.RenderOneoff( portal->GetHoleRenderable() );

		// Transform the current camera view matrix to the pose relative to the paired portal
		glm::mat4 portal_view = portal->ConvertView( view_matrix );
		// The new virtual camera sits behind the portal, so to keep the wall behind the portal from blocking the view, we set the projection's near plane at the portal
		glm::vec3 cam_pos = utility::extract_view_postion_from_matrix( portal_view );
		float distance_to_portal =  glm::length( cam_pos - portal->GetPairedPortal()->GetPosition() );
		glm::mat4 portal_cam_proj_mat = 
			glm::perspective( 
				glm::radians( 90.f ),
				16.f / 9.f,
				distance_to_portal - 1.1f,
				1000.f
			);

		// This is the bottom layer, render the innermost portal contents
		if( current_recursion_level == MAX_PORTAL_RECURSION )
		{
			// Allow color and depth writes
			glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
			glDepthMask( GL_TRUE );

			// Clear the depth buffer
			// Enable depth testing
			glClear( GL_DEPTH_BUFFER_BIT );
			glEnable( GL_DEPTH_TEST );

			// Enable stencil testing so we only draw inside the portal
			glEnable( GL_STENCIL_TEST );
			// Don't allow stencil writes anymore
			glStencilMask( 0x00 );
			// Only draw pixels that pass the stencil test (current_recursion_level + 1)
			glStencilFunc( GL_EQUAL, current_recursion_level + 1, 0xFF );

			RenderBaseScene( portal_view, portal_cam_proj_mat );
		}
		else
		{
			// If this isn't the bottom layer yet, recurse
			// Pass this portal's paired-portal camera into the recursive call, and bump the recursion level by 1 so it actually ends
			RenderPortals( portal_view, portal_cam_proj_mat, current_recursion_level + 1 );
		}

		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glDepthMask(GL_FALSE);

		glEnable(GL_STENCIL_TEST);
		glStencilMask(0xFF);

		// The portal interior we just rendered will fail this stencil test
		glStencilFunc( GL_NOTEQUAL, current_recursion_level + 1, 0xFF );

		// Pixels that fail get stencil -1, until we unwind back to the top of the recursion and the stencil buffer is all 0s
		glStencilOp( GL_DECR, GL_KEEP, GL_KEEP );

		mRenderer.SetProjectionMatrix( portal_cam_proj_mat );
		mRenderer.SetViewMatrix( view_matrix );
		for( auto& portal : mPortals )
		{
			mRenderer.RenderOneoff( portal->GetHoleRenderable() );
		}
	}
	
	// Disable stencil testing and color writes
	glDisable( GL_STENCIL_TEST );
	glStencilMask( 0x00 );
	glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );

	// Enable depth testing and color writes
	glEnable( GL_DEPTH_TEST );
	glDepthMask( GL_TRUE );

	// Depth test set to always pass, so everything gets written into the depth buffer
	glDepthFunc( GL_ALWAYS );
	glClear( GL_DEPTH_BUFFER_BIT );

	// Write both portal windows into the depth buffer
	mRenderer.SetProjectionMatrix( projection_matrix );
	mRenderer.SetViewMatrix( view_matrix );
	for( auto& portal : mPortals )
	{
		mRenderer.RenderOneoff( portal->GetHoleRenderable() );
	}
	// Put the depth test back to default (near occludes far)
	glDepthFunc( GL_LESS );

	// Enable stencil testing, disable writes to the stencil buffer
	glEnable(GL_STENCIL_TEST);
	glStencilMask(0x00);
	glStencilFunc( GL_LEQUAL, current_recursion_level, 0xFF );

	// Everything back to normal
	glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
	glDepthMask( GL_TRUE );
	glEnable(GL_DEPTH_TEST);
	// Draw the regular scene
	RenderBaseScene( view_matrix, projection_matrix );
	if( current_recursion_level != 0 )
	{
		RenderDebugInfo();
	}
}

void 
LevelController::RenderBaseScene( glm::mat4 view_matrix, glm::mat4 projection_matrix )
{
	RenderSkybox( view_matrix, projection_matrix );
	mRenderer.SetProjectionMatrix( std::move( projection_matrix ) );
	mRenderer.SetViewMatrix( std::move( view_matrix ) );
	// Draw everything except the "real portal"
	auto& walls = mCurrentLevel->GetWalls();
	for( auto& wall : walls )
	{
		mRenderer.RenderOneoff( wall.render_instance.get() );
	}
	// Draw the portal frames
	for( auto& portal : mPortals )
	{
		if( portal->HasBeenPlaced() )
		{
			mRenderer.RenderOneoff( portal->GetFrameRenderable() );
		}
	}
	mRenderer.RenderOneoff( mDyBox.get() );
	if( mRenderClone )
	{
		mRenderer.RenderOneoff( mDyBox->GetClone() );
	}
}

void
LevelController::RenderSkybox( glm::mat4 view_matrix, glm::mat4 projection_matrix )
{
	mRenderer.SetProjectionMatrix( std::move( projection_matrix ) );
	mRenderer.SetViewMatrix( std::move( view_matrix ) );
	glFrontFace( GL_CCW );
	mRenderer.RenderOneoff( mSkybox.get() );
	glFrontFace( GL_CW );
}
