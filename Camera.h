#ifndef _CAMERA_H
#define _CAMERA_H

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace portal
{
	///
	/// 摄像机类
	/// 提供视图矩阵和投影矩阵
	/// 
	class Camera
	{
	public:
		///
		/// 摄像机运动方向
		/// 
		enum class MovementDirection
		{
			UP,
			DOWN,
			LEFT,
			RIGHT,
			FORWARD,
			BACKWARD
		};

		Camera( float view_width, float view_height );
		~Camera();

		///
		/// 获取视图矩阵
		/// 
		/// @return glm::mat4
		///		就是视图矩阵
		/// 
		glm::mat4 GetViewMatrix();

		///
		/// 获取投影矩阵
		/// 
		/// @return glm::mat4
		///		就是投影矩阵 :)
		/// 
		glm::mat4 GetProjectionMatrix();

		///
		/// 更新摄像机参数，需要定期调用（例如放在主循环内）
		/// 
		/// @param ms_passed
		///		距离上次调用过去的时间（毫秒），用于根据速度计算摄像机位置
		/// 
		void Update( int ms_passed );

		///
		/// 移动摄像机
		/// 
		/// @param dir
		///		方向
		/// 
		void Move( MovementDirection dir );

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
		glm::vec3 mPosition;             ///< 摄像机位置
		glm::vec3 mCameraFrontDirection; ///< 摄像机面朝方向
		glm::vec3 mCameraUpDirection;    ///< 摄像机头顶方向
		glm::vec3 mCameraRightDirection; ///< 摄像机右方

		float mAspectRatio; ///< 画面比例
		float mFov;         ///< 视野
		float mNearClip;    ///< 近裁切面
		float mFarClip;     ///< 远裁切面

		// 摄像机运动
		float mMoveVelocity;               ///< 移动速度
		glm::vec3 mPositionDeltaPerSecond; ///< 累积每秒移动变量
		float mYawAngle;                   ///< 垂直角度
		float mPitchAngle;                 ///< 水平角度
	};
}

#endif
