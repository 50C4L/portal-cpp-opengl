#ifndef _PHYSICS_H
#define _PHYSICS_H

#include <memory>
#include <functional>
#include <chrono>
#include <glm/vec3.hpp>

#include <bullet/btBulletCollisionCommon.h>
#include <bullet/btBulletDynamicsCommon.h>

#include "Renderer.h"

namespace portal
{
	namespace physics
	{
		///
		/// 射线
		/// 定义一条点到点的射线，如果它击中了任何物理物体，callback函数会被调用。
		/// 使用Physics::CastRay()来发射射线
		/// 
		/*class Raycast : public reactphysics3d::RaycastCallback
		{
			public:
				///
				/// 构造函数
				/// 
				/// @param start, stop
				///		射线开始和结束位置
				/// 
				/// @param continue_length
				///		击中后继续射线继续延展的单位
				/// 
				/// @param callback
				///		射线击中后调用的回调函数
				/// 
				Raycast( glm::vec3 start, glm::vec3 stop, float continue_length, std::function<void()> callback );
				~Raycast();

				///
				/// reactphysics3d::RaycastCallback::notifyRaycastHit
				///
				virtual reactphysics3d::decimal notifyRaycastHit( const reactphysics3d::RaycastInfo& info ) override;

				///
				/// 获取rp3d的射线实例引用
				/// 
				/// @return reactphysics3d::Ray&
				/// 
				const reactphysics3d::Ray& GetRay() const;

			private:
				reactphysics3d::Ray mRay;
				float mContinueLength;
				std::function<void()> mCallback;
		};*/

		///
		/// 物体碰撞回调
		/// 
		class Callback
		{
		public:
			Callback()
				: mCallback( nullptr )
			{}
			///
			/// 构造函数
			/// 
			/// @param callback
			///		碰撞发生时调用
			/// 
			Callback( std::function<void(bool)> callback );
			~Callback();

		private:
			std::function<void(bool)> mCallback;
		};

		class DebugRenderer;

		///
		/// 物理主类
		/// 封装reactphysics3d功能，请确保每个关卡只有一个实例
		/// 
		class Physics
		{
		public:
			///
			/// 物理物体基础
			/// 
			class PhysicsObject
			{
			public:
				///
				/// 刚体类型
				/// is_rigid 为 false时这个设置不起作用
				/// 
				enum class Type
				{
					STATIC,    //< 静态物体，静态物体之间不会碰撞，不会收到任何力的影响
					KINEMATIC, //< 永动物体，不会被阻挡不会收到任何力的影响
					DYNAMIC    //< 动态物体，与所有物体发生碰撞，受力影响
				};

				///
				/// 位置的getter和setter
				/// 
				glm::vec3 GetPosition() const;
				void SetPosition( glm::vec3 pos );

				///
				/// 移动位置
				/// 
				/// @param offset
				///		空间中移动的量
				/// 
				void Translate( glm::vec3 offset );

				///
				/// 如果想要physics::Callback工作正常
				/// 请以同样的频率调用这个函数以及Physics::Update()
				/// 
				void Update();

			protected:
				///
				/// 构造函数
				/// 
				/// @param pos
				///		位置
				/// 
				/// @param world
				///		Reference to btDiscreteDynamicsWorld
				/// 
				/// @param type
				///		刚体类型
				/// 
				/// @param callback
				///		本物体发生碰撞时调用的回调函数，必须确保Update()有定期被调用
				/// 
				PhysicsObject( glm::vec3 pos,
							   btDiscreteDynamicsWorld& world,
							   Type type,
							   physics::Callback callback );
				~PhysicsObject();

				///
				/// 创建刚体
				/// 
				/// @param pos
				///		中心位置
				/// 
				/// @param collision_shape
				///		btCollisionShape指针
				/// 
				void BuildRigidBody( glm::vec3 pos, btCollisionShape* collision_shape );

				btDiscreteDynamicsWorld& mWorld;
				Type mType;
				physics::Callback mCallback;
				bool mIsRigid;
				std::unique_ptr<btRigidBody> mBody;
				std::unique_ptr<btCollisionShape> mShape;
			};

			///
			/// 盒子型碰撞体
			/// 使用Physics::CreateBox来创建实例
			/// 
			class Box : public PhysicsObject
			{
			public:
				///
				/// 构造函数
				/// 
				/// @param pos
				///		中心位置
				/// 
				/// @param size
				///		长宽高
				/// 
				/// @param world
				///		Reference to btDiscreteDynamicsWorld
				/// 
				/// @param type
				///		刚体类型
				/// 
				/// @param callback
				///		本物体发生碰撞时调用的回调函数，必须确保Update()有定期被调用
				///  
				Box( glm::vec3 pos, 
					 glm::vec3 size, 
					 btDiscreteDynamicsWorld& world,
					 Type type,
					 physics::Callback callback );
				~Box();
			};

			///
			/// 胶囊型碰撞体
			/// 使用Physics::CreateCapsule来创建实例
			/// 
			class Capsule : public PhysicsObject
			{
			public:
				///
				/// 构造函数
				/// 
				/// @param pos
				///		中心位置
				/// 
				/// @param raidus
				///		胶囊两头球形的半径
				/// 
				/// @param height
				///		胶囊身体的长度
				/// 
				/// @param world
				///		Reference to btDiscreteDynamicsWorld
				/// 
				/// @param type
				///		刚体类型
				/// 
				/// @param callback
				///		本物体发生碰撞时调用的回调函数，必须确保Update()有定期被调用
				///  
				Capsule( glm::vec3 pos, 
						 float raidus, 
						 float height, 
						 btDiscreteDynamicsWorld& world, 
						 Type type,
						 physics::Callback callback );
				~Capsule();
			};

		public:
			///
			/// 构造函数
			/// 
			Physics( Renderer& renderer );
			~Physics();

			///
			/// 初始化，请只调用一次
			/// 
			/// @param dt
			///		物理更新的固定间隔 单位：秒
			/// 
			void Initialize( float dt );

			///
			/// 更新物理信息，调用频率越高结果越准确
			/// 
			/// @param enable_debug_draw
			///		是否渲染物理debug数据
			/// 
			void Update();

			///
			/// 创建盒子
			/// 
			/// 参数请见Box构造函数
			/// 
			std::unique_ptr<Box> CreateBox( glm::vec3 pos, glm::vec3 size, PhysicsObject::Type type, physics::Callback callback = {} );

			///
			/// 创建胶囊
			/// 
			/// 参数请见Capsule构造函数
			/// 
			std::unique_ptr<Capsule> CreateCapsule( glm::vec3 pos, float raidus, float height, PhysicsObject::Type type, physics::Callback callback = {} );

			///
			/// 发射射线
			/// 
			// void CastRay( physics::Raycast& ray );

			void DebugRender();

		private:
			std::unique_ptr<btDefaultCollisionConfiguration> mConfiguration;
			std::unique_ptr<btCollisionDispatcher> mCollisionDispatcher;
			std::unique_ptr<btBroadphaseInterface> mBroadphaseInterface;
			std::unique_ptr<btSequentialImpulseConstraintSolver> mSequentialImpulseConstraintSolver;
			std::unique_ptr<btDiscreteDynamicsWorld> mWorld;

			std::chrono::steady_clock::time_point mPreviousUpdateTimepoint; //< 上一次Update被调用的时间点

			std::unique_ptr<DebugRenderer> mDebugRenderer;
			Renderer& mRenderer;
		};
	}
}

#endif
