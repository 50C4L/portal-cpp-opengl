#include "Player.h"

#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/vector_angle.hpp>

#include "Camera.h"
#include "LevelConstants.h"
#include "Portal.h"
#include "Utility.h"

using namespace portal;
using namespace portal::physics;
using namespace portal::level;

namespace
{
	const float PLAYER_CAPSULE_RAIDUS = 1.8f;
	const float PLAYER_CAPSULE_HEIGHT = 5.f;
	const float PLAYER_GROUND_DAMPING = 0.9f;
	const float PLAYER_AIR_DAMPING = 0.1f;
	const float PLAYER_MAX_VELOCITY = 20.f;
	const float PLAYER_JUMP_FORCE = 700.f;
	const float PLAYER_CAMERA_OFFSET = 4.f;

	const size_t PORTAL_1 = 0;
	const size_t PORTAL_2 = 1;

	float clip( float n, float lower, float upper )
	{
		return std::max( lower, std::min( n, upper ) );
	}
}

Player::Player( Physics& physics )
	: mPhysics( physics )
	, mPreviousYPos( 0.f )
	, mIsGrounded( false )
	, mDownCastHitNumber( 0 )
	, mIsRunning( false )
{}

Player::~Player()
{}

void 
Player::Spawn( glm::vec3 position, std::shared_ptr<Camera> camera )
{
	mPreviousYPos = position.y;
	mMainCamera = std::move( camera );
	mMainCamera->SetPosition( { position.x, position.y + 4.f, position.z } );
	mCollisionCapsule = mPhysics.CreateCapsule( 
		position, 
		PLAYER_CAPSULE_RAIDUS, PLAYER_CAPSULE_HEIGHT, 
		Physics::PhysicsObject::Type::DYNAMIC,
		static_cast<int>( PhysicsGroup::PLAYER ),
		static_cast<int>( PhysicsGroup::WALL ) | static_cast<int>( PhysicsGroup::PORTAL_FRAME )
	);
	mCollisionCapsule->SetAngularFactor( { 0.f, 0.f, 0.f } );
	mCollisionCapsule->SetDamping( PLAYER_AIR_DAMPING, 0.f );

	mCollisionCapsule->GetCollisionObject()->setUserPointer( static_cast<void*>( this ) );
	mPortalablePO = mCollisionCapsule.get();
}

void
Player::Update()
{
	// 如果Y值没有变就当作我们站在地上
	// TODO: 往下一个单位发射射线检测是否有碰撞体，这样更准确.
	// 不过目前bullet raytest好像有问题，有时候返回的碰撞法线是NaN导致错误
	auto new_pos = mCollisionCapsule->GetPosition();
	mIsGrounded = abs( mPreviousYPos - new_pos.y ) <= 0.001;
	mPreviousYPos = new_pos.y;

	new_pos.y += PLAYER_CAMERA_OFFSET;
	mMainCamera->UpdateCamera( 0.f, 0.f, new_pos - mMainCamera->GetPosition() );

	if( !mIsGrounded && !mIsRunning )
	{
		mCollisionCapsule->SetDamping( PLAYER_AIR_DAMPING, 0.2f );
	}
	else
	{
		if( mIsGrounded )
		{
			mCollisionCapsule->SetDamping( PLAYER_GROUND_DAMPING, 0.95f );
		}
	}
}

void 
Player::HandleKeys( std::unordered_map<unsigned int, bool>& key_map )
{
	if( mMainCamera && mIsGrounded )
	{
		const glm::vec3 forward_dir = mMainCamera->GetFrontDirection();
		const glm::vec3 right_dir = mMainCamera->GetRightDirection();
		glm::vec3 move_dir{ 0.f };
		if( key_map[ 'w' ] )
		{
			move_dir += forward_dir;
		}
		if( key_map[ 's' ] )
		{
			move_dir -= forward_dir;
		}
		if( key_map[ 'a' ] )
		{
			move_dir -= right_dir;
		}
		if( key_map[ 'd' ] )
		{
			move_dir += right_dir;
		}
		if( key_map[ ' ' ] )
		{
			mCollisionCapsule->SetImpluse( { 0.f, PLAYER_JUMP_FORCE, 0.f }, mCollisionCapsule->GetPosition() );
		}

		mIsRunning = move_dir != glm::vec3{ 0.f };

		if( mIsRunning )
		{
			mCollisionCapsule->Activate();
			mCollisionCapsule->SetDamping( 0.f, 0.f );
			const glm::vec3 velocity = move_dir * PLAYER_MAX_VELOCITY;
			mCollisionCapsule->SetLinearVelocity(
				{
					velocity.x,
					mCollisionCapsule->GetLinearVelocity().y,
					velocity.z
				}
			);
		}
	}
}

void 
Player::HandleMouse( std::unordered_map<int, bool>& button_map, Portal& portal_left, Portal& portal_right )
{
	// TODO Merge these
	if( mMouseLeftPressed != button_map[ 1 ] )
	{
		mMouseLeftPressed = button_map[ 1 ];
		if( mMouseLeftPressed )
		{
			auto look_dir = mMainCamera->GetLookDirection();
			look_dir *= 1000.f;
			mPhysics.CastRay( 
				mMainCamera->GetPosition(), look_dir, 
				static_cast<int>( PhysicsGroup::RAY ),
				[&, this]( bool is_hit, glm::vec3 hit_point, glm::vec3 hit_normal, const btCollisionObject* obj )
				{
					if( is_hit )
					{
						portal_left.PlaceAt( hit_point, hit_normal, obj );
					}
				}
			);
		}
	}

	if( mMouseRightPressed != button_map[ 2 ] )
	{
		mMouseRightPressed = button_map[ 2 ];
		if( mMouseRightPressed )
		{
			auto look_dir = mMainCamera->GetLookDirection();
			look_dir *= 1000.f;
			mPhysics.CastRay( 
				mMainCamera->GetPosition(), look_dir, 
				static_cast<int>( PhysicsGroup::RAY ),
				[&, this]( bool is_hit, glm::vec3 hit_point, glm::vec3 hit_normal, const btCollisionObject* obj )
				{
					if( is_hit )
					{
						portal_right.PlaceAt( hit_point, hit_normal, obj );
					}
				}
			);
		}
	}
}

void
Player::Look( float yaw_angle, float pitch_angle )
{
	mMainCamera->UpdateCamera( pitch_angle, yaw_angle, glm::vec3( 0.f ) );
}

void 
Player::Teleport( Portal& in_portal )
{
	// 计算传送后摄像机位置
	glm::vec3 pos = in_portal.ConvertPointToOutPortal( mMainCamera->GetPosition() );
	mMainCamera->SetPosition( pos );

	// 计算传送后的摄像机焦点
	glm::vec3 target = mMainCamera->GetTarget();
	mMainCamera->SetTarget( in_portal.ConvertPointToOutPortal( std::move( target ) ) );

	// 根据新的摄像机位置计算新的碰撞体位置
	pos.y -= PLAYER_CAMERA_OFFSET;
	pos += in_portal.GetPairedPortal()->GetFaceDir() * 0.1f;
	glm::vec3 prev_pos = mCollisionCapsule->GetPosition();
	mCollisionCapsule->SetPosition( pos );

	// 传送后保持玩家的线性运动惯性
	// 将方向根据出口的方向作出转换
	glm::vec3 velocity = mCollisionCapsule->GetLinearVelocity();
	velocity = in_portal.ConvertDirectionToOutPortal( std::move( velocity ), std::move( prev_pos ), std::move( pos ) );
	velocity *= 1.f - mCollisionCapsule->GetLinearDamping();
	mCollisionCapsule->SetLinearVelocity( std::move( velocity ) );
}

glm::vec3 
Player::GetPosition()
{
	return mCollisionCapsule->GetPosition();
}

glm::vec3 
Player::GetLookDirection()
{
	return mMainCamera->GetLookDirection();
}
