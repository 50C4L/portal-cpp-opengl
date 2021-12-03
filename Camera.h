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
		/// 构造函数
		/// 
		/// @param view_width, view_height
		///		视口的大小
		/// 
		Camera( float view_width, float view_height );
		Camera( float view_width, 
				float view_height, 
				glm::vec3 position,
				glm::vec3 target,
				glm::vec3 front_dir );
		~Camera();

		///
		/// 获取视图矩阵
		/// 
		/// @return glm::mat4
		///		就是视图矩阵
		/// 
		glm::mat4 GetViewMatrix();
		void UpdateViewMatrix();

		///
		/// 获取投影矩阵
		/// 
		/// @return glm::mat4
		///		就是投影矩阵 :)
		/// 
		glm::mat4 GetProjectionMatrix();
		void UpdateProjectionMatrix();

		///
		/// 更新摄像机
		/// 
		/// @param pitch
		///		垂直方向转动的角度
		/// 
		/// @param yaw
		///		水平方向转动的角度
		/// 
		/// @param translate
		///		XYZ方向的移动
		/// 
		void UpdateCamera( float pitch = 0.f, float yaw = 0.f, glm::vec3 translate = glm::vec3{ 0.f } );

		///
		/// 设置焦点，设置后会自动更新视图矩阵
		///	
		/// @param target
		///		焦点
		/// 
		void SetTarget( glm::vec3 target );
		glm::vec3 GetTarget();

		///
		/// 把摄像机移动到指定点
		/// 
		/// @param pos
		///		目标坐标
		/// 
		void SetPosition( glm::vec3 pos );
		glm::vec3 GetPosition();

		glm::vec3 GetLookDirection();
		glm::vec3 GetFrontDirection();
		glm::vec3 GetRightDirection();

	private:
		glm::vec3 mPosition;             ///< 摄像机位置
		glm::vec3 mCameraFrontDirection; ///< 摄像机向前方向 - FPS模式下Y轴不变
		glm::vec3 mCameraUpDirection;    ///< 摄像机头顶方向
		glm::vec3 mCameraRightDirection; ///< 摄像机右方
		glm::vec3 mTarget;               ///< 摄像机的焦点

		float mAspectRatio; ///< 画面比例
		float mFov;         ///< 视野
		float mNearClip;    ///< 近裁切面
		float mFarClip;     ///< 远裁切面

		glm::mat4 mViewMat;       ///< 视图矩阵
		glm::mat4 mProjectionMat; ///< 投影矩阵
	};
}

#endif
