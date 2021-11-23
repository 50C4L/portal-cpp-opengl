#include "Player.h"

#include "Camera.h"

using namespace portal;
using namespace portal::physics;

namespace
{
	const float PLAYER_CAPSULE_RAIDUS = 1.8f;
	const float PLAYER_CAPSULE_HEIGHT = 5.f;
	const float PLAYER_GROUND_DAMPING = 0.9f;
	const float PLAYER_AIR_DAMPING = 0.1f;
	const float PLAYER_MAX_VELOCITY = 20.f;
	const float PLAYER_JUMP_FORCE = 2.f;

	float clip( float n, float lower, float upper )
	{
		return std::max( lower, std::min( n, upper ) );
	}
}

Player::Player( Physics& physics )
	: mPhysics( physics )
	, mIsActive( false )
	, mIsGrounded( false )
	, mPreviousUpdateTime( std::chrono::steady_clock::now() )
	, mFallingAccumulatedSec( 0.f )
	, mDownCastHitNumber( 0 )
	, mMoveVelocity( 0.f )
	, mMoveDirection( 0.f )
	, mIsRunning( false )
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
		PLAYER_CAPSULE_RAIDUS, PLAYER_CAPSULE_HEIGHT, 
		Physics::PhysicsObject::Type::DYNAMIC
	);
	mCollisionCapsule->SetAngularFactor( { 0.f, 0.f, 0.f } );
	mCollisionCapsule->SetDamping( PLAYER_AIR_DAMPING, 0.f );
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

	auto pos = mCollisionCapsule->GetPosition();
	pos.y += 4.5f;
	mMainCamera->SetPosition( std::move( pos ) );

	CastGroundCheckRay();
	if( mIsGrounded )
	{
		mCollisionCapsule->SetDamping( mIsRunning ? 0.f : PLAYER_GROUND_DAMPING, 0.f );
	}
	else
	{
		mCollisionCapsule->SetDamping( PLAYER_AIR_DAMPING, 0.f );
	}
}

void 
Player::HandleKeys( std::unordered_map<unsigned int, bool>& key_map )
{
	if( !mIsActive )
	{
		return;
	}

	if( mMainCamera && mIsGrounded )
	{
		glm::vec3 forward_dir = mMainCamera->GetFrontDirection();
		glm::vec3 right_dir = mMainCamera->GetRightDirection();
		if( key_map[ 'w' ] )
		{
			mMoveDirection += forward_dir;
		}
		if( key_map[ 's' ] )
		{
			mMoveDirection -= forward_dir;
		}
		if( key_map[ 'a' ] )
		{
			mMoveDirection -= right_dir;
		}
		if( key_map[ 'd' ] )
		{
			mMoveDirection += right_dir;
		}
		if( key_map[ ' ' ] )
		{
			mCollisionCapsule->SetImpluse( { 0.f, PLAYER_JUMP_FORCE, 0.f }, mCollisionCapsule->GetPosition() );
		}

		mIsRunning = key_map[ 'w' ] || key_map[ 's' ] || key_map[ 'a' ] || key_map[ 'd' ];

		if( mIsRunning )
		{
			mMoveDirection = mMoveDirection == glm::vec3{ 0.f } ? mMoveDirection :glm::normalize( mMoveDirection );
			mMoveVelocity = mMoveDirection * PLAYER_MAX_VELOCITY;
			mCollisionCapsule->SetLinearVelocity( mMoveVelocity );
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
	auto pos = mCollisionCapsule->GetPosition();
	glm::vec3 from{ 
		pos.x, 
		pos.y,
		pos.z
	};
	glm::vec3 to = from;
	to.y -= PLAYER_CAPSULE_HEIGHT / 2.f + PLAYER_CAPSULE_RAIDUS + 1.f;
	mPhysics.CastRay( 
		std::move( from ), std::move( to ),
		[this]( bool is_hit, glm::vec3 /*hit_point*/ )
		{
			mIsGrounded = is_hit;
		}
	);
}
