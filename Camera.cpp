#include "Camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace portal;

Camera::Camera( float view_width, float view_height )
	: mPosition( 0.f, 10.f, 1.f )
	, mCameraFrontDirection( 0.f, 0.f, -1.f )
	, mCameraUpDirection( 0.f, 1.f, 0.f )
	, mCameraRightDirection( glm::normalize( glm::cross( mCameraFrontDirection, mCameraUpDirection ) ) )
	, mAspectRatio( view_width / view_height )
	, mFov( 90.f )
	, mNearClip( 0.1f )
	, mFarClip( 1000.f )
	, mMoveVelocity( 1.0f )
	, mPositionDeltaPerSecond( 0.f )
	, mYawAngle( 0.f )
	, mPitchAngle( 0.f )
{
}

Camera::~Camera()
{}

glm::mat4 
Camera::GetViewMatrix()
{
	return glm::lookAt( 
		mPosition,
		mPosition + mCameraFrontDirection,
		mCameraUpDirection
	);
}

glm::mat4 
Camera::GetProjectionMatrix()
{
	return glm::perspective( 
		glm::radians( mFov ),
		mAspectRatio,
		mNearClip,
		mFarClip
	);
}

void
Camera::Update( int ms_passed )
{
	// 更新右边方向
	mCameraRightDirection = glm::normalize( glm::cross( mCameraFrontDirection, mCameraUpDirection ) );

	// 根据更新间隔和速度更新摄像机位置
	const float sec_passed = ms_passed / 1000.f;
	mPosition += mPositionDeltaPerSecond * sec_passed;

	// 让镜头慢慢停下
	mPositionDeltaPerSecond *= 0.8f;
}

void
Camera::Move( MovementDirection dir )
{
	switch( dir )
	{
	case MovementDirection::UP:
		break;
	case MovementDirection::DOWN:
		break;
	case MovementDirection::LEFT:
		mPositionDeltaPerSecond -= mCameraRightDirection * mMoveVelocity;
		break;
	case MovementDirection::RIGHT:
		mPositionDeltaPerSecond += mCameraRightDirection * mMoveVelocity;
		break;
	case MovementDirection::FORWARD:
		mPositionDeltaPerSecond += mCameraFrontDirection * mMoveVelocity;
		break;
	case MovementDirection::BACKWARD:
		mPositionDeltaPerSecond -= mCameraFrontDirection * mMoveVelocity;
		break;
	default:
		break;
	}
}

void
Camera::Look( float yaw_angle, float pitch_angle )
{
	mYawAngle += yaw_angle;
	mPitchAngle += pitch_angle;

	// 限制垂直角度，不然摄像机会翻转
	if( mPitchAngle > 89.0f )
	{
		mPitchAngle = 89.0f;
	}
	else if( mPitchAngle < -89.0f )
	{
		mPitchAngle = -89.0f;
	}

	// 画个图将摄像机三维分解为多个二维图，通过三角函数就能算出镜头转动后的方向
	glm::vec3 front;
	front.x = cos( glm::radians( mYawAngle ) ) * cos( glm::radians( mPitchAngle ) );
	front.y = sin( glm::radians( mPitchAngle ) );
	front.z = sin( glm::radians( mYawAngle ) ) * cos( glm::radians( mPitchAngle ) );
	mCameraFrontDirection = glm::normalize( front );
}
