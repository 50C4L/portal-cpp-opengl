#include "Physics.h"

#include <bullet/BulletCollision/NarrowPhaseCollision/btRaycastCallback.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#include "DebugRenderer.h"
#include "Renderer.h"

using namespace portal;
using namespace portal::physics;

namespace
{
	class PhysicsContactResultCallback : public btCollisionWorld::ContactResultCallback
	{
	public:
		PhysicsContactResultCallback( std::function<void()> callback )
			: mCallback( std::move( callback ) )
		{}

		virtual
		btScalar
		addSingleResult( btManifoldPoint& cp, 
						 const btCollisionObjectWrapper* colObj0Wrap, 
						 int partId0, 
						 int index0, 
						 const btCollisionObjectWrapper* colObj1Wrap, 
						 int partId1, 
						 int index1 )
		{
			if( mCallback )
			{
				mCallback();
			}
			return 0;
		}

	private:
		std::function<void()> mCallback;
	};
}

///
/// Callback implementation
/// 
Callback::Callback( std::function<void(bool)> callback )
	: mCallback( std::move( callback ) )
{
}

Callback::~Callback()
{}

///
/// PhysicsObject implementation
/// 
Physics::PhysicsObject::PhysicsObject( glm::vec3 pos,
									   btDiscreteDynamicsWorld& world, 
									   Type type,
									   physics::Callback callback )
	: mWorld( world )
	, mType( type )
	, mCallback( std::move( callback ) )
{
}

Physics::PhysicsObject::~PhysicsObject()
{
	if( mBody )
	{
		mWorld.removeRigidBody( mBody.get() );
	}
}

void 
Physics::PhysicsObject::BuildRigidBody( glm::vec3 pos, btCollisionShape* collision_shape, int group, int mask, bool is_ghost )
{
	btTransform box_transform;
	box_transform.setIdentity();
	box_transform.setOrigin( btVector3( pos.x, pos.y, pos.z ) );
	btDefaultMotionState* motion_state = new btDefaultMotionState( box_transform );

	btScalar mass = 1.f;
	btVector3 local_intertia( 0.f, 0.f, 0.f );
	if( mType == Type::STATIC )
	{
		mass = 0.f;
	}
	else
	{
		collision_shape->calculateLocalInertia( mass, local_intertia );
	}

	btRigidBody::btRigidBodyConstructionInfo rbInfo( mass, motion_state, collision_shape, local_intertia );
	
	mBody = std::make_unique<btRigidBody>( rbInfo );
	if( is_ghost )
	{
		mBody->setCollisionFlags( btCollisionObject::CF_NO_CONTACT_RESPONSE );
	}
	mWorld.addRigidBody( mBody.get(), group, mask );
}

glm::vec3
Physics::PhysicsObject::GetPosition() const
{
	btVector3& origin = mBody->getWorldTransform().getOrigin();
	return { origin.x(), origin.y(), origin.z() };
}

void
Physics::PhysicsObject::SetLinearVelocity( glm::vec3 velocity )
{
	mBody->setLinearVelocity( { velocity.x, velocity.y, velocity.z } );
}

glm::vec3 
Physics::PhysicsObject::GetLinearVelocity()
{
	auto veclocity = mBody->getLinearVelocity();
	return { veclocity.x(), veclocity.y(), veclocity.z() };
}

void 
Physics::PhysicsObject::SetAngularFactor( glm::vec3 factors )
{
	mBody->setAngularFactor( { factors.z, factors.y, factors.z } );
}

void 
Physics::PhysicsObject::SetDamping( float linear, float angular )
{
	mBody->setDamping( linear, angular );
}

void 
Physics::PhysicsObject::SetPosition( glm::vec3 pos )
{
	btTransform transform;
	transform.setIdentity();
	transform.setOrigin( btVector3( pos.x, pos.y, pos.z ) );
	mBody->setWorldTransform( std::move( transform ) );
}

void 
Physics::PhysicsObject::SetImpluse( glm::vec3 force, glm::vec3 pos )
{
	mBody->applyImpulse( { force.x, force.y, force.z }, { pos.x, pos.y, pos.z } );
}

void 
Physics::PhysicsObject::Activate()
{
	mBody->activate( true );
}

void 
Physics::PhysicsObject::SetTransform( glm::mat4 transform_mat )
{
	btTransform transform;
	transform.setFromOpenGLMatrix( glm::value_ptr( transform_mat ) );
	mBody->setWorldTransform( std::move( transform ) );
}

glm::mat4 
Physics::PhysicsObject::GetTransform()
{
	btTransform transform = mBody->getWorldTransform();
	float mat_val[16];
	transform.getOpenGLMatrix( mat_val );
	return glm::make_mat4x4( mat_val );
}

void 
Physics::PhysicsObject::SetIgnoireCollisionWith( const btCollisionObject* obj, bool flag )
{
	mBody->setIgnoreCollisionCheck( obj, flag );
}

bool 
Physics::PhysicsObject::IsCollideWith( btCollisionObject* obj )
{
	bool result = false;
	PhysicsContactResultCallback callback(
		[&](){ result = true; }
	);
	mWorld.contactPairTest( mBody.get(), obj, callback );
	return result;
}

btCollisionObject* 
Physics::PhysicsObject::GetCollisionObject()
{
	return mBody.get();
}

AABB 
Physics::PhysicsObject::GetAABB()
{
	btVector3 max, min;
	mBody->getAabb( min, max );
	return {
		{ max.x(), max.y(), max.z() },
		{ min.x(), min.y(), min.z() }
	};
}

///
/// Box implementation
/// 
Physics::Box::Box( glm::vec3 pos, glm::vec3 size, btDiscreteDynamicsWorld& world, Type type, int group, int mask, bool is_ghost, Callback callback )
	: PhysicsObject( pos, world, type, std::move( callback ) )
{
	mShape = std::make_unique<btBoxShape>( btVector3( size.x / 2.f, size.y / 2.f, size.z / 2.f ) );
	BuildRigidBody( std::move( pos ), mShape.get(), group, mask, is_ghost );
}

Physics::Box::~Box()
{}

///
/// Capsule implementation
/// 
Physics::Capsule::Capsule( glm::vec3 pos, float raidus, float height, btDiscreteDynamicsWorld& world,Type type, int group, int mask, bool is_ghost, Callback callback )
	: PhysicsObject( pos, world, type, std::move( callback ) )
{
	mShape = std::make_unique<btCapsuleShape>( raidus, height );
	BuildRigidBody( std::move( pos ), mShape.get(), group, mask, is_ghost );
}

Physics::Capsule::~Capsule()
{}

///
/// Physics class implementation
/// 
Physics::Physics( Renderer& renderer )
	: mPreviousUpdateTimepoint( std::chrono::steady_clock::now() )
	, mRenderer( renderer )
{}

Physics::~Physics()
{}

void 
Physics::Initialize( float dt )
{
	mConfiguration                     = std::make_unique<btDefaultCollisionConfiguration>();
	mCollisionDispatcher               = std::make_unique<btCollisionDispatcher>( mConfiguration.get() );
	mBroadphaseInterface               = std::make_unique<btDbvtBroadphase>();
	mSequentialImpulseConstraintSolver = std::make_unique<btSequentialImpulseConstraintSolver>();

	mWorld = std::make_unique<btDiscreteDynamicsWorld>( 
		mCollisionDispatcher.get(), 
		mBroadphaseInterface.get(), 
		mSequentialImpulseConstraintSolver.get(), 
		mConfiguration.get() );

	mWorld->setGravity( btVector3( 0, -20, 0 ) );
	mDebugRenderer = std::make_unique<DebugRenderer>( mRenderer );
	mWorld->setDebugDrawer( mDebugRenderer.get() );
}

void 
Physics::Update()
{
	auto current_time = std::chrono::steady_clock::now();
	float delta_seconds = std::chrono::duration<float, std::milli>( current_time - mPreviousUpdateTimepoint ).count() / 1000.f;
	mPreviousUpdateTimepoint = current_time;

	mWorld->stepSimulation( delta_seconds, 10 );
}

std::unique_ptr<Physics::Box>
Physics::CreateBox( glm::vec3 pos, glm::vec3 size, PhysicsObject::Type type, int group, int mask, bool is_ghost, physics::Callback callback )
{
	return std::make_unique<Physics::Box>( pos, size, *mWorld, type, group, mask, is_ghost, std::move( callback ) );
}

std::unique_ptr<Physics::Capsule>
Physics::CreateCapsule( glm::vec3 pos, float raidus, float height, PhysicsObject::Type type, int group, int mask, bool is_ghost, physics::Callback callback )
{
	return std::make_unique<Physics::Capsule>( pos, raidus, height, *mWorld, type, group, mask, is_ghost, std::move( callback ) );
}
 
void 
Physics::CastRay( glm::vec3 from, glm::vec3 to, int filter_group, std::function<void(bool, glm::vec3, glm::vec3, const btCollisionObject* )> callback )
{
	btVector3 from_v{ from.x, from.y, from.z };
	btVector3 to_v{ to.x, to.y, to.z };
	btCollisionWorld::ClosestRayResultCallback first_result( from_v, to_v );
	first_result.m_collisionFilterGroup = filter_group;
	mWorld->rayTest( from_v, to_v, first_result );

	if( callback )
	{
		callback( 
			first_result.hasHit(),
			{ first_result.m_hitPointWorld.x(), first_result.m_hitPointWorld.y(), first_result.m_hitPointWorld.z() },
			{ first_result.m_hitNormalWorld.x(), first_result.m_hitNormalWorld.y(), first_result.m_hitNormalWorld.z() },
			first_result.m_collisionObject
		);
	}
}

void
Physics::DebugRender()
{
	mWorld->debugDrawWorld();
}
