#ifndef _PHYSICS_H
#define _PHYSICS_H

#include <memory>
#include <functional>
#include <chrono>
#include <glm/vec3.hpp>

namespace reactphysics3d
{
	class PhysicsCommon;
	class PhysicsWorld;
	class CollisionBody;
	class RigidBody;
	class BoxShape;
}

namespace portal
{
	class Physics
	{
	public:
		/// 利用RAII来自动释放
		using R3DCollisionBody = std::unique_ptr<reactphysics3d::CollisionBody, std::function<void(reactphysics3d::CollisionBody*)>>;
		using R3DRigidBody = std::unique_ptr<reactphysics3d::RigidBody, std::function<void(reactphysics3d::RigidBody*)>>;
		using R3DBoxShape = std::unique_ptr<reactphysics3d::BoxShape, std::function<void(reactphysics3d::BoxShape*)>>;

		class PhysicsObject
		{
		public:
			enum class Type
			{
				STATIC,
				KINEMATIC,
				DYNAMIC
			};

		protected:
			PhysicsObject( reactphysics3d::PhysicsCommon& physics_comman, reactphysics3d::PhysicsWorld& world, Type type )
				: mPhysicsCommon( physics_comman )
				, mWorld( world )
				, mType( type )
			{}

			reactphysics3d::PhysicsCommon& mPhysicsCommon;
			reactphysics3d::PhysicsWorld& mWorld;
			R3DCollisionBody mBody;
			Type mType;
		};

		class Box : public PhysicsObject
		{
		public:
			Box( glm::vec3 pos, glm::vec3 size, reactphysics3d::PhysicsCommon& physics_comman, reactphysics3d::PhysicsWorld& world, bool is_rigid, Type type );
			~Box();

			glm::vec3 GetPosition() const;

		private:
			R3DBoxShape mBox;
		};

	public:
		Physics();
		~Physics();

		void Initialize( float dt );
		void Update();

		std::unique_ptr<Box> CreateBox( glm::vec3 pos, glm::vec3 size, bool is_rigid, PhysicsObject::Type type );

	private:
		using R3DPhysicsWorld = std::unique_ptr<reactphysics3d::PhysicsWorld, std::function<void(reactphysics3d::PhysicsWorld*)>>;

		std::unique_ptr<reactphysics3d::PhysicsCommon> mPhysicsCommon;
		R3DPhysicsWorld mWorld;

		float mUpdateInterval;
		std::chrono::steady_clock::time_point mPreviousUpdateTimepoint;
		float mTimeAccumulator;
	};
}

#endif
