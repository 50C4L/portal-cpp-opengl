#ifndef _PLAYER_H
#define _PLAYER_H

#include <glm/vec3.hpp>
#include <chrono>

#include "Physics.h"

namespace portal
{
	class Camera;
	class Portal;

	///
	/// 简易玩家类
	/// 使用胶囊作为物理碰撞体，移动、跳跃均作用于胶囊本身，摄像机位置绑定在胶囊上半部分。
	/// 胶囊本身被设置为不能旋转，摄像机视线决定玩家向前移动的方向。
	/// WASD控制玩家四方向移动，空格键跳跃。
	/// 左键在目标表面开启传送门1（蓝色），右键传送门2（橙色）
	/// 
	class Player
	{
	public:
		struct PortalInfo
		{
			bool is_active = false;
			glm::vec3 position = glm::vec3( 0.f );
			glm::vec3 face_dir = glm::vec3( 0.f );
		};

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
		/// 处理键盘按键
		/// 
		/// @param key_map
		///		Reference to std::unordered_map<unsigned int, bool>
		/// 
		void HandleKeys( std::unordered_map<unsigned int, bool>& key_map );

		///
		/// 处理鼠标按键
		/// 
		/// @param button_map
		///		Reference to std::unordered_map<int, bool>
		/// 
		void HandleMouse( std::unordered_map<int, bool>& button_map, Portal& portal_left, Portal& portal_right );

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

		const std::vector<PortalInfo>& GetPortalInfo() const;

		void Teleport( glm::vec3 new_pos, glm::vec3 face_dir, glm::mat4 src_trans, glm::mat4 dst_trans );
		
	private:
		void CastGroundCheckRay( glm::vec3 pos );

		physics::Physics& mPhysics;
		std::unique_ptr<physics::Physics::Capsule> mCollisionCapsule; //< 胶囊碰撞体
		bool mIsActive;    //< 是否被激活，玩家只有在调用Spawn()出生后才会被激活
		bool mIsGrounded;  //< 玩家是否站在“地面”上

		// time
		std::chrono::steady_clock::time_point mPreviousUpdateTime;
		float mFallingAccumulatedSec;

		// 地面检测
		//std::unique_ptr<physics::Raycast> mGroudDetectionRay; //< 往正下方发射的射线
		int mDownCastHitNumber; //< 地面射线击中物体的次数

		std::shared_ptr<Camera> mMainCamera;
		glm::vec3 mMoveVelocity;             //< 移动速度
		glm::vec3 mMoveDirection;            //< 累积每秒移动变量
		bool mIsRunning;                     //< 是否在跑步

		std::vector<PortalInfo> mPortalInfo;

		bool mMouseLeftPressed = false;
		bool mMouseRightPressed = false;
	};
}

#endif
