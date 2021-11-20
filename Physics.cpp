#include "Physics.h"

using namespace portal;
using namespace portal::physics;
using namespace reactphysics3d;

namespace
{
	Vector3 glm_vec_to_rp3d_vec( const glm::vec3 glm_vec )
	{
		return Vector3{ glm_vec.x, glm_vec.y, glm_vec.z };
	}
}

///
/// Raycast implementation
/// 
Raycast::Raycast( glm::vec3 start, glm::vec3 stop, float continue_length, std::function<void()> callback )
	: mRay( Vector3{ start.x, start.y, start.z }, Vector3{ stop.x, stop.y, stop.z } )
	, mContinueLength( continue_length )
	, mCallback( std::move( callback ) )
{}

Raycast::~Raycast()
{
}

decimal 
Raycast::notifyRaycastHit( const reactphysics3d::RaycastInfo& info )
{
	if( mCallback )
	{
		mCallback();
	}
	return mContinueLength;
}

const Ray& 
Raycast::GetRay() const
{
	return mRay;
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

void 
Callback::onContact( const CallbackData& callbackData )
{
	if( !mCallback )
	{
		return;
	}
	if( callbackData.getNbContactPairs() > 0 )
	{
		mCallback( true );
	}
}

///
/// PhysicsObject implementation
/// 
Physics::PhysicsObject::PhysicsObject( glm::vec3 pos, 
									   reactphysics3d::PhysicsCommon& physics_comman, 
									   reactphysics3d::PhysicsWorld& world, 
									   bool is_rigid, 
									   Type type, 
									   physics::Callback callback )
	: mPhysicsCommon( physics_comman )
	, mWorld( world )
	, mType( type )
	, mCallback( std::move( callback ) )
	, mIsRigid( is_rigid )
{
	// 自定义析构函数
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
}

Physics::PhysicsObject::~PhysicsObject()
{}

glm::vec3
Physics::PhysicsObject::GetPosition() const
{
	const Transform& transform = mBody->getTransform();
	const Vector3& position = transform.getPosition();
	return { position.x, position.y, position.z };
}

void
Physics::PhysicsObject::Translate( glm::vec3 offset )
{
	glm::vec3 new_pos = GetPosition() + offset;
	Transform transform( Vector3{ new_pos.x, new_pos.y, new_pos.z }, Quaternion::identity() );
	mBody->setTransform( std::move( transform ) );
}

void
Physics::PhysicsObject::Update()
{
	mWorld.testCollision( mBody.get(), mCallback );
}

void 
Physics::PhysicsObject::SetPosition( glm::vec3 pos )
{
	Transform transform( Vector3{ pos.x, pos.y, pos.z }, Quaternion::identity() );
	mBody->setTransform( std::move( transform ) );
}

///
/// Box implementation
/// 
Physics::Box::Box( glm::vec3 pos, glm::vec3 size, PhysicsCommon& physics_comman, PhysicsWorld& world, bool is_rigid, Type type, Callback callback )
	: PhysicsObject( pos, physics_comman, world, is_rigid, type, std::move( callback ) )
{
	mBox = R3DBoxShape(
		mPhysicsCommon.createBoxShape( Vector3{ size.x /2.f ,size.y / 2.f, size.z / 2.f } ),
		[ this ]( reactphysics3d::BoxShape* box ){ mPhysicsCommon.destroyBoxShape( box ); }
	);
	mBody->addCollider( mBox.get(), Transform::identity() );
}

Physics::Box::~Box()
{}

///
/// Capsule implementation
/// 
Physics::Capsule::Capsule( glm::vec3 pos, float raidus, float height, PhysicsCommon& physics_comman, PhysicsWorld& world, bool is_rigid, Type type, Callback callback )
	: PhysicsObject( pos, physics_comman, world, is_rigid, type, std::move( callback ) )
{
	mCapsule = R3DCapsuleShape(
		mPhysicsCommon.createCapsuleShape( raidus, height ),
		[ this ]( reactphysics3d::CapsuleShape* capsule ){ mPhysicsCommon.destroyCapsuleShape( capsule ); }
	);
	mBody->addCollider( mCapsule.get(), Transform::identity() );
}

Physics::Capsule::~Capsule()
{}

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

#ifdef _DEBUG
	mWorld->setIsDebugRenderingEnabled( true );
	auto& debug_renderer = mWorld->getDebugRenderer();
	debug_renderer.setIsDebugItemDisplayed( DebugRenderer::DebugItem::COLLISION_SHAPE, true );
#endif
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

#ifdef _DEBUG
	// 超级无敌慢
	if( !mWorld->getIsDebugRenderingEnabled() )
	{
		return;
	}
	auto& triangles = mWorld->getDebugRenderer().getTriangles();
	if( triangles.size() > 0 )
	{
		std::vector<Vertex> vertices;
		const glm::vec4 red_color{ 1.f, 0.f, 0.f, 1.f };
		for( const DebugRenderer::DebugTriangle& tri : triangles )
		{
			vertices.emplace_back( Vertex{ { tri.point1.x, tri.point1.y, tri.point1.z }, red_color, {} } );
			vertices.emplace_back( Vertex{ { tri.point2.x, tri.point2.y, tri.point2.z }, red_color, {} } );
			vertices.emplace_back( Vertex{ { tri.point3.x, tri.point3.y, tri.point3.z }, red_color, {} } );
		}
		mDebugRenderable.reset();
		mDebugRenderable = std::make_unique<Renderer::Renderable>(
			std::move( vertices ),
			Renderer::DEBUG_PHYSICS_SHADER,
			0
		);
	}
#endif
}

Renderer::Renderable* 
Physics::GetDebugRenderable()
{
	return mDebugRenderable ? mDebugRenderable.get() : nullptr;
}

std::unique_ptr<Physics::Box>
Physics::CreateBox( glm::vec3 pos, glm::vec3 size, bool is_rigid, PhysicsObject::Type type, physics::Callback callback )
{
	return std::make_unique<Physics::Box>( pos, size, *mPhysicsCommon, *mWorld, is_rigid, type, std::move( callback ) );
}

std::unique_ptr<Physics::Capsule>
Physics::CreateCapsule( glm::vec3 pos, float raidus, float height, bool is_rigid, PhysicsObject::Type type, physics::Callback callback )
{
	return std::make_unique<Physics::Capsule>( pos, raidus, height, *mPhysicsCommon, *mWorld, is_rigid, type, std::move( callback ) );
}

void 
Physics::CastRay( physics::Raycast& ray )
{
	mWorld->raycast( ray.GetRay(), &ray );
}
