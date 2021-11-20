#include "Player.h"

#include "Camera.h"

using namespace portal;
using namespace portal::physics;

Player::Player( Physics& physics )
	: mPhysics( physics )
	, mCapsulePrevPos( 0.f )
	, mIsActive( false )
	, mIsGrounded( false )
	, mPreviousUpdateTime( std::chrono::steady_clock::now() )
	, mFallingAccumulatedSec( 0.f )
	, mDownCastHitNumber( 0 )
	, mMoveVelocity( 15.0f )
	, mPositionDeltaPerSecond( 0.f )
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
		true, 
		Physics::PhysicsObject::Type::DYNAMIC,
		{ std::bind( &Player::OnCollision, this, std::placeholders::_1 ) }
	);
	mCapsulePrevPos = position;
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
	float delta_seconds = std::chrono::duration<float, std::milli>( current_time - mPreviousUpdateTime ).count() / 1000.f;
	mPreviousUpdateTime = current_time;

	glm::vec3 offset = mPositionDeltaPerSecond * delta_seconds;
	mCollisionCapsule->Translate( offset );

	if( !mIsGrounded )
	{
		mPositionDeltaPerSecond *= 0.8f;
	}
	else
	{
		mPositionDeltaPerSecond *= 0.0f;
	}
	

	auto pos = mCollisionCapsule->GetPosition();
	pos.y += 4.5f;
	mMainCamera->SetPosition( std::move( pos ) );
	
	mCollisionCapsule->Update();

	CastGroundCheckRay();
	if( mDownCastHitNumber == 1 )
	{
		mIsGrounded = false;
	}
}

void 
Player::Move( MoveDirection dir )
{
	if( !mIsActive )
	{
		return;
	}

	if( mMainCamera && mIsGrounded )
	{
		glm::vec3 forward_dir = mMainCamera->GetFrontDirection();
		glm::vec3 right_dir = mMainCamera->GetRightDirection();
		switch( dir )
		{
		case MoveDirection::LEFT:
			mPositionDeltaPerSecond -= right_dir * mMoveVelocity;
			break;
		case MoveDirection::RIGHT:
			mPositionDeltaPerSecond += right_dir * mMoveVelocity;
			break;
		case MoveDirection::FORWARD:
			mPositionDeltaPerSecond += forward_dir * mMoveVelocity;
			break;
		case MoveDirection::BACKWARD:
			mPositionDeltaPerSecond -= forward_dir * mMoveVelocity;
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
Player::CastGroundCheckRay()
{
	// 当玩家胶囊发生碰撞时，我们在玩家头顶高一点点的位置往正下方发射一根射线。
	// 射线会先击中玩家本身的胶囊，然后继续前进9个单位。
	// 如果它继续击中另一个物体，表示玩家站在一个东西上，看下面OnGroundRayHit
	static const float continue_value = 9.f;
	auto pos = mCollisionCapsule->GetPosition();
	mDownCastHitNumber = 0;
	Raycast ray(
		glm::vec3{ pos.x, pos.y + 5.5, pos.z },
		glm::vec3{ pos.x, pos.y - 5.5, pos.z },
		continue_value,
		[this]()
		{
			mDownCastHitNumber ++;
		}
	);
	mPhysics.CastRay( ray );
}

void
Player::OnCollision( bool is_collided )
{
	// 地面检测
	if( !mIsGrounded && mDownCastHitNumber >= 2)
	{
		mIsGrounded = true;
		mFallingAccumulatedSec = 0;
	}
}
