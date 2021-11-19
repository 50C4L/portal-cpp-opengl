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
		enum class MoveDirection
		{
			FORWARD,
			BACKWARD,
			LEFT,
			RIGHT
		};

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
		/// 设置玩家重力
		/// 
		/// @param gravity_acc
		///		单位/秒^2
		/// 
		void SetGravity( float gravity_acc );

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
		void OnCollision( bool is_collided );
		void OnGroundRayHit();

		physics::Physics& mPhysics;
		std::unique_ptr<physics::Physics::Capsule> mCollisionCapsule;
		bool mIsActive;
		float mGravityAcc;
		bool mIsGrounded;

		// time
		std::chrono::steady_clock::time_point mPreviousUpdateTime;
		float mFallingAccumulatedSec;

		std::unique_ptr<physics::Raycast> mGroudDetectionRay;
		int mDownCastHitNumber;

		std::shared_ptr<Camera> mMainCamera;

		float mMoveVelocity;               ///< 移动速度
		glm::vec3 mPositionDeltaPerSecond; ///< 累积每秒移动变量
	};
}

#endif
