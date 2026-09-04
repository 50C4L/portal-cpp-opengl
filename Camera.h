#ifndef _CAMERA_H
#define _CAMERA_H

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace portal
{
	///
	/// Camera class
	/// Provides the view matrix and the projection matrix
	/// 
	class Camera
	{
	public:
		///
		/// Constructor
		/// 
		/// @param view_width, view_height
		///		Viewport size
		/// 
		Camera( float view_width, float view_height );
		Camera( float view_width, 
				float view_height, 
				glm::vec3 position,
				glm::vec3 target,
				glm::vec3 front_dir );
		~Camera();

		///
		/// Get the view matrix
		/// 
		/// @return glm::mat4
		///		Yep, that's the view matrix
		/// 
		glm::mat4 GetViewMatrix();
		void UpdateViewMatrix();

		///
		/// Get the projection matrix
		/// 
		/// @return glm::mat4
		///		Yep, that's the projection matrix :)
		/// 
		glm::mat4 GetProjectionMatrix();
		void UpdateProjectionMatrix();

		///
		/// Update the camera
		/// 
		/// @param pitch
		///		Vertical rotation angle
		/// 
		/// @param yaw
		///		Horizontal rotation angle
		/// 
		/// @param translate
		///		Movement along XYZ
		/// 
		void UpdateCamera( float pitch = 0.f, float yaw = 0.f, glm::vec3 translate = glm::vec3{ 0.f } );

		///
		/// Set the look-at point; the view matrix updates automatically after this
		///	
		/// @param target
		///		Look-at point
		/// 
		void SetTarget( glm::vec3 target );
		glm::vec3 GetTarget();

		///
		/// Move the camera to a given point
		/// 
		/// @param pos
		///		Target coordinates
		/// 
		void SetPosition( glm::vec3 pos );
		glm::vec3 GetPosition();

		glm::vec3 GetLookDirection();
		glm::vec3 GetFrontDirection();
		glm::vec3 GetRightDirection();

	private:
		glm::vec3 mPosition;             ///< Camera position
		glm::vec3 mCameraFrontDirection; ///< Camera forward direction - Y stays put in FPS mode
		glm::vec3 mCameraUpDirection;    ///< Camera up direction
		glm::vec3 mCameraRightDirection; ///< Camera right
		glm::vec3 mTarget;               ///< Camera look-at point

		float mAspectRatio; ///< Aspect ratio
		float mFov;         ///< Field of view
		float mNearClip;    ///< Near clip plane
		float mFarClip;     ///< Far clip plane

		glm::mat4 mViewMat;       ///< View matrix
		glm::mat4 mProjectionMat; ///< Projection matrix
	};
}

#endif
