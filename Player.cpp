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
	, mMoveVelocity( 2.0f )
	, mPositionDeltaPerSecond( 0.f )
{}

Player::~Player()
{}

void 
Player::Spawn( glm::vec3 position, std::shared_ptr<Camera> camera )
{
	mMainCamera = std::move( camera );
	mMainCamera->SetPosition( { position.x, position.y + 4.5f, position.z } );
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
	auto current_time = std::chrono::steady_clock::now();
	float delta_sec = std::chrono::duration<float, std::milli>( current_time - mPreviousUpdateTime ).count() / 1000.f;
	mPreviousUpdateTime = current_time;

	auto pos = mCollisionCapsule->GetPosition();
	if( !mIsGrounded )
	{
		mFallingAccumulatedSec += delta_sec;
		float current_velocity = mGravityAcc * mFallingAccumulatedSec;
		float current_offset = current_velocity * delta_sec;

		pos.y += current_offset;
		mCollisionCapsule->SetPosition( std::move( pos ) );
	}
	mCollisionCapsule->Update();

	pos.y += 4.5f;
	mMainCamera->SetPosition( std::move( pos ) );
}

void 
Player::SetGravity( float gravity_acc )
{
	mGravityAcc = gravity_acc;
}

void 
Player::Move( MoveDirection dir )
{
	/*switch( dir )
	{
	case MoveDirection::LEFT:
		mPositionDeltaPerSecond -= mCameraRightDirection * mMoveVelocity;
		break;
	case MoveDirection::RIGHT:
		mPositionDeltaPerSecond += mCameraRightDirection * mMoveVelocity;
		break;
	case MoveDirection::FORWARD:
		mPositionDeltaPerSecond += mCameraFrontDirection * mMoveVelocity;
		break;
	case MoveDirection::BACKWARD:
		mPositionDeltaPerSecond -= mCameraFrontDirection * mMoveVelocity;
		break;
	default:
		break;
	}*/
}

void
Player::Look( float yaw_angle, float pitch_angle )
{
	mMainCamera->Look( yaw_angle, pitch_angle );
}

void
Player::OnCollision( bool is_collided )
{
	if( !mGroudDetectionRay )
	{
		auto pos = mCollisionCapsule->GetPosition();
		mGroudDetectionRay = std::make_unique<Raycast>( 
			glm::vec3{ pos.x, pos.y + 5.5, pos.z },
			glm::vec3{ pos.x, pos.y - 5.5, pos.z },
			9.0f,
			std::bind( &Player::OnGroundRayHit, this )
		);
		mPhysics.CastRay( *mGroudDetectionRay );
	}
}

void 
Player::OnGroundRayHit()
{
	mDownCastHitNumber ++;
	if( mDownCastHitNumber >= 2)
	{
		mIsGrounded = true;
		mFallingAccumulatedSec = 0;
		mDownCastHitNumber = 0;
	}
}
