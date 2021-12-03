#include "Camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#include "Utility.h"

using namespace portal;

Camera::Camera( float view_width, float view_height )
	: Camera(
		view_width,
		view_height,
		{ 0.f, 10.f, 1.f },
		{ 0.f, 0.f, -100.f },
		{ 0.f, 0.f, -1.f }
	)
{}
Camera::Camera( float view_width,
				float view_height,
				glm::vec3 position,
				glm::vec3 target,
				glm::vec3 front_dir )
	: mPosition( std::move( position ) )
	, mCameraFrontDirection( std::move( front_dir ) )
	, mCameraUpDirection( 0.f, 1.f, 0.f )
	, mCameraRightDirection( glm::normalize( glm::cross( mCameraFrontDirection, mCameraUpDirection ) ) )
	, mTarget( std::move( target ) )
	, mAspectRatio( view_width / view_height )
	, mFov( 90.f )
	, mNearClip( 0.1f )
	, mFarClip( 1000.f )
{
	UpdateViewMatrix();
	UpdateProjectionMatrix();
}

Camera::~Camera()
{}

void
Camera::UpdateViewMatrix()
{
	mViewMat = glm::lookAt( 
		mPosition,
		mTarget,
		mCameraUpDirection
	);
}

glm::mat4
Camera::GetViewMatrix()
{
	return mViewMat;
}

void
Camera::UpdateProjectionMatrix()
{
	mProjectionMat = glm::perspective( 
		glm::radians( mFov ),
		mAspectRatio,
		mNearClip,
		mFarClip
	);
}

void 
Camera::UpdateCamera( float pitch, float yaw, glm::vec3 translate )
{
	mPosition += translate;
	mTarget += translate;
	// 从当前的视图矩阵获取右向量
	glm::mat4 camera = glm::inverse( mViewMat );
	mCameraRightDirection = glm::vec3( camera[0] );
	// 绕右向量转动
	mTarget = mPosition + glm::vec3( glm::rotate( glm::mat4{ 1.f }, glm::radians( pitch ), mCameraRightDirection ) * glm::vec4( mTarget - mPosition, 1.f ) );
	// 绕上向量转动
	mTarget = mPosition + glm::vec3( glm::rotate( glm::mat4{ 1.f }, glm::radians( yaw ), mCameraUpDirection ) * glm::vec4( mTarget - mPosition, 1.f ) );
	UpdateViewMatrix();

	// 更新前进方向，FPS摄像机只能水平移动
	const glm::vec3 look_dir = GetLookDirection();
	mCameraFrontDirection.x = look_dir.x;
	mCameraFrontDirection.z = look_dir.z;
	mCameraFrontDirection = glm::normalize( mCameraFrontDirection );
}

void 
Camera::SetTarget( glm::vec3 target )
{
	mTarget = std::move( target );
	UpdateCamera();
}

glm::vec3 
Camera::GetTarget()
{
	return mTarget;
}

glm::mat4
Camera::GetProjectionMatrix()
{
	return mProjectionMat;
}

void 
Camera::SetPosition( glm::vec3 pos )
{
	mPosition = std::move( pos );
	UpdateCamera();
}

glm::vec3 
Camera::GetPosition()
{
	return mPosition;
}

glm::vec3 
Camera::GetLookDirection()
{
	return mTarget - mPosition;;
}

glm::vec3 
Camera::GetFrontDirection()
{
	return mCameraFrontDirection;
}

glm::vec3 
Camera::GetRightDirection()
{
	return mCameraRightDirection;
}
