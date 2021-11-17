#include "Physics.h"

///
/// reactphysics3d 这个库一堆warning
/// 这里在编译它的头文件时暂时关闭那些warning
/// 
#pragma warning( push )
#pragma warning(disable : 4996)
#pragma warning(disable : 4267)
#pragma warning(disable : 4099)
#pragma warning(disable : 4244)
#include <reactphysics3d/reactphysics3d.h>
#pragma warning( pop ) 

using namespace portal;
using namespace reactphysics3d;

///
/// Box implementation
/// 
Physics::Box::Box( glm::vec3 pos, glm::vec3 size, PhysicsCommon& physics_comman, PhysicsWorld& world, bool is_rigid, Type type )
	: PhysicsObject( physics_comman, world, type )
{
	auto body_deleter = [ this ]( reactphysics3d::CollisionBody* body ) -> void
	{
		if( auto rigid_body = dynamic_cast<reactphysics3d::RigidBody*>( body ) )
		{
			mWorld.destroyRigidBody( rigid_body );
		}
		else
		{
			mWorld.destroyCollisionBody( body );
		}
	};

	if( is_rigid )
	{
		mBody = R3DCollisionBody( 
			mWorld.createRigidBody( Transform{ Vector3{ pos.x, pos.y, pos.z }, Quaternion::identity() } ),
			std::move( body_deleter )
		);
	}
	else
	{
		mBody = R3DCollisionBody( 
			mWorld.createCollisionBody( Transform{ Vector3{ pos.x, pos.y, pos.z }, Quaternion::identity() } ),
			std::move( body_deleter )
		);
	}

	if( is_rigid )
	{
		auto body = dynamic_cast<RigidBody*>( mBody.get() );
		switch( type )
		{
		case Type::DYNAMIC:
			body->setType( BodyType::DYNAMIC );
			break;
		case Type::KINEMATIC:
			body->setType( BodyType::KINEMATIC );
			break;
		case Type::STATIC:
		default:
			body->setType( BodyType::STATIC );
			break;
		}
	}

	mBox = R3DBoxShape(
		mPhysicsCommon.createBoxShape( Vector3{ size.x /2.f ,size.y / 2.f, size.z / 2.f } ),
		[ this ]( reactphysics3d::BoxShape* box ){ mPhysicsCommon.destroyBoxShape( box ); }
	);
	mBody->addCollider( mBox.get(), Transform::identity() );
}

Physics::Box::~Box()
{}

glm::vec3
Physics::Box::GetPosition() const
{
	const Transform& transform = mBody->getTransform();
	const Vector3& position = transform.getPosition();
	return { position.x, position.y, position.z };
}

///
/// Physics class implementation
/// 
Physics::Physics()
	: mPhysicsCommon( std::make_unique<PhysicsCommon>() )
	, mUpdateInterval( 1.f / 60.f )
	, mPreviousUpdateTimepoint( std::chrono::steady_clock::now() )
	, mTimeAccumulator( 0.f )
{}

Physics::~Physics()
{}

void 
Physics::Initialize( float dt )
{
	mUpdateInterval = dt;
	PhysicsWorld::WorldSettings settings;
	settings.gravity = Vector3( 0, -9.81f, 0 );
	mWorld = R3DPhysicsWorld( 
		mPhysicsCommon->createPhysicsWorld( settings ),
		[ this ]( reactphysics3d::PhysicsWorld* world ) -> void
		{
			mPhysicsCommon->destroyPhysicsWorld( world );
		}
	);
}

void 
Physics::Update()
{
	auto current_time = std::chrono::steady_clock::now();
	float delta_seconds = std::chrono::duration<float, std::milli>( current_time - mPreviousUpdateTimepoint ).count() / 1000.f;
	mPreviousUpdateTimepoint = current_time;

#ifdef _DEBUG
	// std::cout << "Physics::Update() call interval: " << delta_seconds << "s." << std::endl; 
#endif

	mTimeAccumulator += delta_seconds;
	while( mTimeAccumulator >= mUpdateInterval )
	{
		mWorld->update( mUpdateInterval );
		mTimeAccumulator -= mUpdateInterval;
	}
}

std::unique_ptr<Physics::Box>
Physics::CreateBox( glm::vec3 pos, glm::vec3 size, bool is_rigid, PhysicsObject::Type type )
{
	return std::make_unique<Physics::Box>( pos, size, *mPhysicsCommon, *mWorld, is_rigid, type );
}
