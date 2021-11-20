#ifndef _PHYSICS_H
#define _PHYSICS_H

#include <memory>
#include <functional>
#include <chrono>
#include <glm/vec3.hpp>

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
		class Raycast : public reactphysics3d::RaycastCallback
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
		};

		///
		/// 物体碰撞回调
		/// 
		class Callback : public reactphysics3d::CollisionCallback
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

			///
			/// reactphysics3d::CollisionCallback::onContact
			///
			virtual void onContact(const CallbackData& callbackData) override;

		private:
			std::function<void(bool)> mCallback;
		};

		///
		/// 物理主类
		/// 封装reactphysics3d功能，请确保每个关卡只有一个实例
		/// 
		class Physics
		{
		public:
			/// 利用RAII来自动释放
			using R3DCollisionBody = std::unique_ptr<reactphysics3d::CollisionBody, std::function<void(reactphysics3d::CollisionBody*)>>;
			using R3DRigidBody = std::unique_ptr<reactphysics3d::RigidBody, std::function<void(reactphysics3d::RigidBody*)>>;
			using R3DBoxShape = std::unique_ptr<reactphysics3d::BoxShape, std::function<void(reactphysics3d::BoxShape*)>>;
			using R3DCapsuleShape = std::unique_ptr<reactphysics3d::CapsuleShape, std::function<void(reactphysics3d::CapsuleShape*)>>;

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
				/// @param physics_common
				///		Reference to reactphysics3d::PhysicsCommon
				/// 
				/// @param world
				///		Reference to reactphysics3d::PhysicsWorld
				/// 
				/// @param is_rigid
				///		设置物体是否为刚体
				/// 
				/// @param type
				///		刚体类型，只有is_rigid为true时生效
				/// 
				/// @param callback
				///		本物体发生碰撞时调用的回调函数，必须确保Update()有定期被调用
				/// 
				PhysicsObject( glm::vec3 pos, 
							   reactphysics3d::PhysicsCommon& physics_common, 
							   reactphysics3d::PhysicsWorld& world, 
							   bool is_rigid, 
							   Type type,
							   physics::Callback callback );
				~PhysicsObject();

				reactphysics3d::PhysicsCommon& mPhysicsCommon;
				reactphysics3d::PhysicsWorld& mWorld;
				R3DCollisionBody mBody;
				Type mType;
				physics::Callback mCallback;
				bool mIsRigid;
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
				/// @param physics_common
				///		Reference to reactphysics3d::PhysicsCommon
				/// 
				/// @param world
				///		Reference to reactphysics3d::PhysicsWorld
				/// 
				/// @param is_rigid
				///		设置物体是否为刚体
				/// 
				/// @param type
				///		刚体类型，只有is_rigid为true时生效
				/// 
				/// @param callback
				///		本物体发生碰撞时调用的回调函数，必须确保Update()有定期被调用
				///  
				Box( glm::vec3 pos, 
					 glm::vec3 size, 
					 reactphysics3d::PhysicsCommon& physics_comman, 
					 reactphysics3d::PhysicsWorld& world, 
					 bool is_rigid, 
					 Type type,
					 physics::Callback callback );
				~Box();

			private:
				R3DBoxShape mBox;
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
				/// @param physics_common
				///		Reference to reactphysics3d::PhysicsCommon
				/// 
				/// @param world
				///		Reference to reactphysics3d::PhysicsWorld
				/// 
				/// @param is_rigid
				///		设置物体是否为刚体
				/// 
				/// @param type
				///		刚体类型，只有is_rigid为true时生效
				/// 
				/// @param callback
				///		本物体发生碰撞时调用的回调函数，必须确保Update()有定期被调用
				///  
				Capsule( glm::vec3 pos, 
						 float raidus, 
						 float height, 
						 reactphysics3d::PhysicsCommon& physics_comman, 
						 reactphysics3d::PhysicsWorld& world, 
						 bool is_rigid, 
						 Type type,
						 physics::Callback callback );
				~Capsule();

			private:
				R3DCapsuleShape mCapsule;
			};

		public:
			///
			/// 构造函数
			/// 
			Physics();
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
			void Update();

			///
			/// 获取用于渲染的debug渲染件
			/// 
			/// @return Renderer::Renderable*
			///		Poniter to Renderer::Renderable
			/// 
			Renderer::Renderable* GetDebugRenderable();

			///
			/// 创建盒子
			/// 
			/// 参数请见Box构造函数
			/// 
			std::unique_ptr<Box> CreateBox( glm::vec3 pos, glm::vec3 size, bool is_rigid, PhysicsObject::Type type, physics::Callback callback = {} );

			///
			/// 创建胶囊
			/// 
			/// 参数请见Capsule构造函数
			/// 
			std::unique_ptr<Capsule> CreateCapsule( glm::vec3 pos, float raidus, float height, bool is_rigid, PhysicsObject::Type type, physics::Callback callback = {} );

			///
			/// 发射射线
			/// 
			void CastRay( physics::Raycast& ray );

		private:
			using R3DPhysicsWorld = std::unique_ptr<reactphysics3d::PhysicsWorld, std::function<void(reactphysics3d::PhysicsWorld*)>>;

			std::unique_ptr<reactphysics3d::PhysicsCommon> mPhysicsCommon;
			R3DPhysicsWorld mWorld;

			float mUpdateInterval;  //< 物理更新间隔
			std::chrono::steady_clock::time_point mPreviousUpdateTimepoint; //< 上一次Update被调用的时间点
			float mTimeAccumulator; //< 累积非更新时间

			std::unique_ptr<Renderer::Renderable> mDebugRenderable;
		};
	}
}

#endif
