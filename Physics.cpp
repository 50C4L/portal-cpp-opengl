#include "Physics.h"
#include "DebugRenderer.h"

using namespace portal;
using namespace portal::physics;

//namespace
//{
//	Vector3 glm_vec_to_rp3d_vec( const glm::vec3 glm_vec )
//	{
//		return Vector3{ glm_vec.x, glm_vec.y, glm_vec.z };
//	}
//}
//
/////
///// Raycast implementation
///// 
//Raycast::Raycast( glm::vec3 start, glm::vec3 stop, float continue_length, std::function<void()> callback )
//	: mRay( Vector3{ start.x, start.y, start.z }, Vector3{ stop.x, stop.y, stop.z } )
//	, mContinueLength( continue_length )
//	, mCallback( std::move( callback ) )
//{}
//
//Raycast::~Raycast()
//{
//}
//
//decimal 
//Raycast::notifyRaycastHit( const reactphysics3d::RaycastInfo& info )
//{
//	if( mCallback )
//	{
//		mCallback();
//	}
//	return mContinueLength;
//}
//
//const Ray& 
//Raycast::GetRay() const
//{
//	return mRay;
//}
//
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
Physics::PhysicsObject::BuildRigidBody( glm::vec3 pos, btCollisionShape* collision_shape )
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
	mWorld.addRigidBody( mBody.get() );
}

glm::vec3
Physics::PhysicsObject::GetPosition() const
{
	btVector3& origin = mBody->getWorldTransform().getOrigin();
	return { origin.x(), origin.y(), origin.z() };
}

void
Physics::PhysicsObject::Translate( glm::vec3 offset )
{
	/*glm::vec3 new_pos = GetPosition() + offset;
	Transform transform( Vector3{ new_pos.x, new_pos.y, new_pos.z }, Quaternion::identity() );
	mBody->setTransform( std::move( transform ) );*/
}

void
Physics::PhysicsObject::Update()
{
	/*mWorld.testCollision( mBody.get(), mCallback );*/
}

void 
Physics::PhysicsObject::SetPosition( glm::vec3 pos )
{
	btTransform transform;
	transform.setIdentity();
	transform.setOrigin( btVector3( pos.x, pos.y, pos.z ) );
	mBody->setWorldTransform( std::move( transform ) );
}

///
/// Box implementation
/// 
Physics::Box::Box( glm::vec3 pos, glm::vec3 size, btDiscreteDynamicsWorld& world, Type type, Callback callback )
	: PhysicsObject( pos, world, type, std::move( callback ) )
{
	mShape = std::make_unique<btBoxShape>( btVector3( size.x / 2.f, size.y / 2.f, size.z / 2.f ) );
	BuildRigidBody( std::move( pos ), mShape.get() );
}

Physics::Box::~Box()
{}

///
/// Capsule implementation
/// 
Physics::Capsule::Capsule( glm::vec3 pos, float raidus, float height, btDiscreteDynamicsWorld& world,Type type, Callback callback )
	: PhysicsObject( pos, world, type, std::move( callback ) )
{
	mShape = std::make_unique<btCapsuleShape>( raidus, height );
	BuildRigidBody( std::move( pos ), mShape.get() );
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

	mWorld->setGravity( btVector3( 0, -10, 0 ) );
	mDebugRenderer = std::make_unique<DebugRenderer>( mRenderer );
	mWorld->setDebugDrawer( mDebugRenderer.get() );
}

void 
Physics::Update()
{
	auto current_time = std::chrono::steady_clock::now();
	float delta_seconds = std::chrono::duration<float, std::milli>( current_time - mPreviousUpdateTimepoint ).count() / 1000.f;
	mPreviousUpdateTimepoint = current_time;

	mWorld->stepSimulation( delta_seconds, 1 );

//#ifdef _DEBUG
//	// 超级无敌慢
//	if( !mWorld->getIsDebugRenderingEnabled() )
//	{
//		return;
//	}
//	auto& triangles = mWorld->getDebugRenderer().getTriangles();
//	if( triangles.size() > 0 )
//	{
//		std::vector<Vertex> vertices;
//		const glm::vec4 red_color{ 1.f, 0.f, 0.f, 1.f };
//		for( const DebugRenderer::DebugTriangle& tri : triangles )
//		{
//			vertices.emplace_back( Vertex{ { tri.point1.x, tri.point1.y, tri.point1.z }, red_color, {} } );
//			vertices.emplace_back( Vertex{ { tri.point2.x, tri.point2.y, tri.point2.z }, red_color, {} } );
//			vertices.emplace_back( Vertex{ { tri.point3.x, tri.point3.y, tri.point3.z }, red_color, {} } );
//		}
//		mDebugRenderable.reset();
//		mDebugRenderable = std::make_unique<Renderer::Renderable>(
//			std::move( vertices ),
//			Renderer::DEBUG_PHYSICS_SHADER,
//			0
//		);
//	}
//#endif
}

std::unique_ptr<Physics::Box>
Physics::CreateBox( glm::vec3 pos, glm::vec3 size, PhysicsObject::Type type, physics::Callback callback )
{
	return std::make_unique<Physics::Box>( pos, size, *mWorld, type, std::move( callback ) );
}

std::unique_ptr<Physics::Capsule>
Physics::CreateCapsule( glm::vec3 pos, float raidus, float height, PhysicsObject::Type type, physics::Callback callback )
{
	return std::make_unique<Physics::Capsule>( pos, raidus, height, *mWorld, type, std::move( callback ) );
}

//void 
//Physics::CastRay( physics::Raycast& ray )
//{
//	mWorld->raycast( ray.GetRay(), &ray );
//}

void
Physics::DebugRender()
{
	mWorld->debugDrawWorld();
}
