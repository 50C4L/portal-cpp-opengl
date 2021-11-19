#include "Player.h"

#include "Camera.h"

using namespace portal;
using namespace portal::physics;

Player::Player( Physics& physics )
	: mPhysics( physics )
	, mIsActive( false )
	, mGravityAcc( -9.81f )
	, mIsGrounded( false )
	, mPreviousUpdateTime( std::chrono::steady_clock::now() )
	, mFallingAccumulatedSec( 0.f )
	, mDownCastHitNumber( 0 )
	, mMoveVelocity( 10.0f )
{}

Player::~Player()
{}

void 
Player::Spawn( glm::vec3 position, std::shared_ptr<Camera> camera )
{
	mMainCamera = std::move( camera );
	mMainCamera->SetPosition( { position.x, position.y + 4.5f, position.z } );
	mMainCamera->SetMoveVelocity( mMoveVelocity );
	mCollisionCapsule = mPhysics.CreateCapsule( 
		position, 
		1.8f, 5.f, 
		false, 
		Physics::PhysicsObject::Type::STATIC,
		{ std::bind( &Player::OnCollision, this, std::placeholders::_1 ) }
	);
	mIsActive = true;
	mPreviousUpdateTime = std::chrono::steady_clock::now();
}

void
Player::Update()
{
	if( !mIsActive )
	{
		return;
	}

	auto current_time = std::chrono::steady_clock::now();
	long long delta_ms = std::chrono::duration_cast<std::chrono::milliseconds>( current_time - mPreviousUpdateTime ).count();
	float delta_sec = delta_ms / 1000.f;
	mPreviousUpdateTime = current_time;

	auto camera_offset = mMainCamera->Update( static_cast<int>( delta_ms ) );
	if( !mIsGrounded )
	{
		mFallingAccumulatedSec += delta_sec;
		float current_velocity = mGravityAcc * mFallingAccumulatedSec;
		float offset = current_velocity * delta_sec;
		mMainCamera->Translate( { 0.f, offset, 0.f } );
		camera_offset.y += offset;
	}
	
	mCollisionCapsule->Translate( camera_offset );
	mCollisionCapsule->Update();
}

void 
Player::SetGravity( float gravity_acc )
{
	mGravityAcc = gravity_acc;
}

void 
Player::Move( MoveDirection dir )
{
	if( !mIsActive )
	{
		return;
	}

	if( mMainCamera )
	{
		switch( dir )
		{
		case MoveDirection::LEFT:
			mMainCamera->Move( Camera::MovementDirection::LEFT );
			break;
		case MoveDirection::RIGHT:
			mMainCamera->Move( Camera::MovementDirection::RIGHT );
			break;
		case MoveDirection::FORWARD:
			mMainCamera->Move( Camera::MovementDirection::FORWARD );
			break;
		case MoveDirection::BACKWARD:
			mMainCamera->Move( Camera::MovementDirection::BACKWARD );
			break;
		default:
			break;
		}
	}
}

void
Player::Look( float yaw_angle, float pitch_angle )
{
	if( !mIsActive )
	{
		return;
	}

	mMainCamera->Look( yaw_angle, pitch_angle );
}

void
Player::OnCollision( bool is_collided )
{
	// 地面检测
	// 当玩家胶囊发生碰撞时，我们在玩家头顶高一点点的位置往正下方发射一根射线。
	// 射线会先击中玩家本身的胶囊，然后继续前进9个单位。
	// 如果它继续击中另一个物体，表示玩家站在一个东西上，看下面OnGroundRayHit
	static const float continue_value = 9.f;
	if( !mGroudDetectionRay )
	{
		auto pos = mCollisionCapsule->GetPosition();
		mGroudDetectionRay = std::make_unique<Raycast>( 
			glm::vec3{ pos.x, pos.y + 5.5, pos.z },
			glm::vec3{ pos.x, pos.y - 5.5, pos.z },
			continue_value,
			std::bind( &Player::OnGroundRayHit, this )
		);
		mPhysics.CastRay( *mGroudDetectionRay );
	}
}

void 
Player::OnGroundRayHit()
{
	if( !mIsGrounded )
	{
		mDownCastHitNumber ++;
		if( mDownCastHitNumber >= 2)
		{
			mIsGrounded = true;
			mFallingAccumulatedSec = 0;
			mDownCastHitNumber = 0;
		}
	}
}
