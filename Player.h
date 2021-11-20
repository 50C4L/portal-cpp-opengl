#ifndef _PLAYER_H
#define _PLAYER_H

#include <glm/vec3.hpp>
#include <chrono>

#include "Physics.h"

namespace portal
{
	class Camera;

	class Player
	{
	public:
		///
		/// 移动方向
		/// 
		enum class MoveDirection
		{
			FORWARD,
			BACKWARD,
			LEFT,
			RIGHT
		};

		///
		/// 构造函数
		/// 
		Player( physics::Physics& physics );
		~Player();

		///
		/// 玩家出生到指定地点并初始化
		/// 
		/// @param position
		///		出生点
		/// 
		/// @param camera
		///		the camera that will be updated with the player
		/// 
		void Spawn( glm::vec3 position, std::shared_ptr<Camera> camera );
		
		///
		/// 必须定期调用来更新玩家的状态
		/// 
		void Update();

		///
		/// 玩家按设定的速度向指定方向移动
		/// 
		/// @param dir
		///		方向
		/// 
		void Move( MoveDirection dir );

		///
		/// 改变观察方向
		/// 
		/// @param yaw_angle
		///		垂直角度
		/// 
		/// @param pitch_angle
		///		水平角度
		/// 
		void Look( float yaw_angle, float pitch_angle );
		
	private:
		void CastGroundCheckRay();
		void OnCollision( bool is_collided );

		physics::Physics& mPhysics;
		std::unique_ptr<physics::Physics::Capsule> mCollisionCapsule; //< 胶囊碰撞体
		glm::vec3 mCapsulePrevPos; //< 胶囊在上一个Update的位置，用于计算摄像机的offset
		bool mIsActive;    //< 是否被激活，玩家只有在调用Spawn()出生后才会被激活
		bool mIsGrounded;  //< 玩家是否站在“地面”上

		// time
		std::chrono::steady_clock::time_point mPreviousUpdateTime;
		float mFallingAccumulatedSec;

		// 地面检测
		std::unique_ptr<physics::Raycast> mGroudDetectionRay; //< 往正下方发射的射线
		int mDownCastHitNumber; //< 地面射线击中物体的次数

		std::shared_ptr<Camera> mMainCamera;
		float mMoveVelocity;               //< 移动速度
		glm::vec3 mPositionDeltaPerSecond; //< 累积每秒移动变量
	};
}

#endif
