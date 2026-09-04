#ifndef _PHYSICS_H
#define _PHYSICS_H

#include <memory>
#include <functional>
#include <chrono>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include <bullet/btBulletCollisionCommon.h>
#include <bullet/btBulletDynamicsCommon.h>


namespace portal
{
	class Renderer;

	namespace physics
	{
		struct AABB
		{
			glm::vec3 max;
			glm::vec3 min;

			bool IsContain( const glm::vec3& point )
			{
				return (point.x >= min.x && point.x <= max.x) &&
					   (point.y >= min.y && point.y <= max.y) &&
					   (point.z >= min.z && point.z <= max.z);
			}
		};

		///
		/// Object collision callback
		/// 
		class Callback
		{
		public:
			Callback()
				: mCallback( nullptr )
			{}
			///
			/// Constructor
			/// 
			/// @param callback
			///		Called when a collision happens
			/// 
			Callback( std::function<void(bool)> callback );
			~Callback();

		private:
			std::function<void(bool)> mCallback;
		};

		class DebugRenderer;

		///
		/// Main physics class
		/// Wraps reactphysics3d; please make sure each level has only one instance
		/// 
		class Physics
		{
		public:
			///
			/// Physics object base
			/// 
			class PhysicsObject
			{
			public:
				///
				/// Rigid body type
				/// This setting does nothing when is_rigid is false
				/// 
				enum class Type
				{
					STATIC,    //< Static body, statics don't collide with each other and aren't affected by any forces
					KINEMATIC, //< Perpetual-motion body, won't be stopped, won't be affected by any forces
					DYNAMIC    //< Dynamic body, collides with everything, affected by forces
				};

				///
				/// Position getter and setter
				/// 
				glm::vec3 GetPosition() const;
				void SetPosition( glm::vec3 pos );

				///
				/// Set linear velocity
				/// 
				/// @param velocity
				///		Velocity
				/// 
				void SetLinearVelocity( glm::vec3 velocity );
				glm::vec3 GetLinearVelocity();

				void SetAngularVelocity( glm::vec3 velocity );
				glm::vec3 GetAngularVelocity();

				///
				/// Set angular factor
				/// 
				/// @param factors [0.0 - 1.0]
				///		x, y, z are the angular factors on the three axes; 0 means can't rotate, 1 means 1:1 rotation
				///
				void SetAngularFactor( glm::vec3 factors );

				///
				/// Set damping
				/// 
				/// @param linear
				///		Linear damping
				/// 
				/// @param angular
				///		Angular damping
				/// 
				void SetDamping( float linear, float angular );
				float GetLinearDamping();

				///
				/// Apply an impulse
				/// 
				/// @param force
				///		Magnitude and direction of the force
				/// 
				/// @param pos
				///		Point the force is applied at
				/// 
				void SetImpluse( glm::vec3 force, glm::vec3 pos );

				///
				/// Wake the body up
				/// Bodies go to sleep when fully at rest, to cut down on computation
				/// 
				void Activate();

				void SetTransform( glm::mat4 transform_mat );
				glm::mat4 GetTransform();

				void SetIgnoireCollisionWith( const btCollisionObject* obj, bool flag );

				bool IsCollideWith( btCollisionObject* obj );

				btCollisionObject* GetCollisionObject();

				AABB GetAABB();

			protected:
				///
				/// Constructor
				/// 
				/// @param pos
				///		Position
				/// 
				/// @param world
				///		Reference to btDiscreteDynamicsWorld
				/// 
				/// @param type
				///		Rigid body type
				/// @param callback
				///		Callback fired when this body collides; make sure Update() is being called regularly
				/// 
				PhysicsObject( glm::vec3 pos,
							   btDiscreteDynamicsWorld& world,
							   Type type,

							   physics::Callback callback );
				~PhysicsObject();

				///
				/// Create the rigid body
				/// 
				/// @param pos
				///		Center position
				/// 
				/// @param collision_shape
				///		btCollisionShape pointer
				/// 
				/// @param group
				///		Collision group
				/// 
				/// @param mask
				///		Collision mask, used to filter collisions
				///
				/// @param is_ghost
				///		Whether to only detect collisions, with no physics response
				/// 
				void BuildRigidBody( glm::vec3 pos, btCollisionShape* collision_shape, int group, int mask, bool is_ghost );

				btDiscreteDynamicsWorld& mWorld;
				Type mType;
				physics::Callback mCallback;
				std::unique_ptr<btRigidBody> mBody;
				std::unique_ptr<btCollisionShape> mShape;
			};

			///
			/// Box collider
			/// Use Physics::CreateBox to create an instance
			/// 
			class Box : public PhysicsObject
			{
			public:
				///
				/// Constructor
				/// 
				/// @param pos
				///		Center position
				/// 
				/// @param size
				///		Width, height, depth
				/// 
				/// @param world
				///		Reference to btDiscreteDynamicsWorld
				/// 
				/// @param type
				///		Rigid body type
				///
				/// @param group
				///		Collision group
				/// 
				/// @param mask
				///		Collision mask, used to filter collisions
				/// 
				/// @param callback
				///		Callback fired when this body collides; make sure Update() is being called regularly
				///  
				Box( glm::vec3 pos, 
					 glm::vec3 size, 
					 btDiscreteDynamicsWorld& world,
					 Type type,
					 int group,
					 int mask,
					 bool is_ghost,
					 physics::Callback callback );
				~Box();
			};

			///
			/// Capsule collider
			/// Use Physics::CreateCapsule to create an instance
			/// 
			class Capsule : public PhysicsObject
			{
			public:
				///
				/// Constructor
				/// 
				/// @param pos
				///		Center position
				/// 
				/// @param raidus
				///		Radius of the spheres at both ends of the capsule
				/// 
				/// @param height
				///		Length of the capsule body
				/// 
				/// @param world
				///		Reference to btDiscreteDynamicsWorld
				/// 
				/// @param type
				///		Rigid body type
				///
				/// @param group
				///		Collision group
				/// 
				/// @param mask
				///		Collision mask, used to filter collisions
				/// 
				/// @param callback
				///		Callback fired when this body collides; make sure Update() is being called regularly
				///  
				Capsule( glm::vec3 pos, 
						 float raidus, 
						 float height, 
						 btDiscreteDynamicsWorld& world, 
						 Type type,
						 int group,
						 int mask,
						 bool is_ghost,
						 physics::Callback callback );
				~Capsule();
			};

		public:
			///
			/// Constructor
			/// 
			Physics( Renderer& renderer );
			~Physics();

			///
			/// Initialize, please only call this once
			/// 
			/// @param dt
			///		Fixed physics timestep, in seconds
			/// 
			void Initialize( float dt );

			///
			/// Update physics; the more often you call this, the more accurate the result
			/// 
			/// @param enable_debug_draw
			///		Whether to render physics debug data
			/// 
			void Update();

			///
			/// Create a box
			/// 
			/// See the Box constructor for parameters
			/// 
			std::unique_ptr<Box> CreateBox( 
				glm::vec3 pos, 
				glm::vec3 size, 
				PhysicsObject::Type type, 
				int group, 
				int mask, 
				bool is_ghost = false, 
				physics::Callback callback = {} );
			 
			///
			/// Create a capsule
			/// 
			/// See the Capsule constructor for parameters
			/// 
			std::unique_ptr<Capsule> CreateCapsule( 
				glm::vec3 pos, 
				float raidus, 
				float height, 
				PhysicsObject::Type type, 
				int group, 
				int mask, 
				bool is_ghost = false, 
				physics::Callback callback = {} );

			///
			/// Cast a ray
			/// 
			void CastRay( glm::vec3 from, glm::vec3 to, int filter_group, std::function<void(bool, glm::vec3, glm::vec3, const btCollisionObject* )> callback = nullptr );

			///
			/// Render physics debug info
			/// 
			void DebugRender();

		private:
			// Bits Bullet3 physics needs
			std::unique_ptr<btDefaultCollisionConfiguration> mConfiguration;
			std::unique_ptr<btCollisionDispatcher> mCollisionDispatcher;
			std::unique_ptr<btBroadphaseInterface> mBroadphaseInterface;
			std::unique_ptr<btSequentialImpulseConstraintSolver> mSequentialImpulseConstraintSolver;
			std::unique_ptr<btDiscreteDynamicsWorld> mWorld;

			std::chrono::steady_clock::time_point mPreviousUpdateTimepoint; //< The last time Update was called

			std::unique_ptr<DebugRenderer> mDebugRenderer;
			Renderer& mRenderer;
		};
	}
}

#endif
