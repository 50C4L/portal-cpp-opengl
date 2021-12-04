#include "Portal.h"
#include <iostream>

#define _USE_MATH_DEFINES
#include <math.h>

#include <glm/gtc/matrix_transform.hpp>

#include "LevelConstants.h"
#include "Player.h"
#include "Utility.h"

using namespace portal;
using namespace portal::physics;
using namespace portal::level;
using namespace portal::utility;

namespace
{
	const float PORTAL_FRAME_WIDTH = 19.f;
	const float PORTAL_FRAME_HEIGHT = 16.f;
	const float PORTAL_GUT_WIDTH = 4.5f;
	const float PORTAL_GUT_HEIGHT = 6.8f;

	std::vector<Vertex>
	generate_portal_frame()
	{
		return {
			{ { -PORTAL_FRAME_WIDTH / 2.f, -PORTAL_FRAME_HEIGHT / 2.f, 0.f }, { 1.f, 1.f, 1.f, 1.f }, { 0.f, 0.f }, { 0.f, 0.f, 1.f } },
			{ { -PORTAL_FRAME_WIDTH / 2.f,  PORTAL_FRAME_HEIGHT / 2.f, 0.f }, { 1.f, 1.f, 1.f, 1.f }, { 0.f, 1.f }, { 0.f, 0.f, 1.f } },
			{ { PORTAL_FRAME_WIDTH / 2.f,  -PORTAL_FRAME_HEIGHT / 2.f, 0.f }, { 1.f, 1.f, 1.f, 1.f }, { 1.f, 0.f }, { 0.f, 0.f, 1.f } },
			{ { PORTAL_FRAME_WIDTH / 2.f,  -PORTAL_FRAME_HEIGHT / 2.f, 0.f }, { 1.f, 1.f, 1.f, 1.f }, { 1.f, 0.f }, { 0.f, 0.f, 1.f } },
			{ { -PORTAL_FRAME_WIDTH / 2.f,  PORTAL_FRAME_HEIGHT / 2.f, 0.f }, { 1.f, 1.f, 1.f, 1.f }, { 0.f, 1.f }, { 0.f, 0.f, 1.f } },
			{ { PORTAL_FRAME_WIDTH / 2.f,   PORTAL_FRAME_HEIGHT / 2.f, 0.f }, { 1.f, 1.f, 1.f, 1.f }, { 1.f, 1.f }, { 0.f, 0.f, 1.f } },
		};
	}

	// 用三角扇画一个椭圆面，用作传送门的门
	const int ELLIPSE_NUM_SIDES = 20;
	std::vector<Vertex>
	generate_portal_ellipse_hole( float radius_x, float radius_y )
	{
		const glm::vec4 black{ 0.f, 0.f, 0.f, 1.f };
		const glm::vec2 fake_uv( 0.f, 0.f );
		const glm::vec3 normal{ 0.f, 0.f, 1.f };

		constexpr int num_vertices = ELLIPSE_NUM_SIDES + 2;
		const float two_pi = 2.f * static_cast<float>( M_PI );

		std::vector<Vertex> vertices;
		vertices.reserve( num_vertices );
		vertices.emplace_back( Vertex{ { 0.f, 0.f, 0.f }, black, fake_uv, normal } );
		
		for( int i = 0; i < num_vertices; i++ )
		{
			float rad = (num_vertices - i) * two_pi / ELLIPSE_NUM_SIDES;
			vertices.emplace_back( Vertex{
				{ cos( rad ) * radius_x, sin( rad ) * radius_y, 0.f },
				black, fake_uv, normal
			});
		}
		return vertices;
	}

	const float PORTAL_FRAME_UP_OFFSET = 1.25f * PORTAL_GUT_HEIGHT;
	const float PORTAL_FRAME_RIGHT_OFFSET = 1.5f * PORTAL_GUT_WIDTH;
	const float PORTAL_FRAME_TICKNESS = 4.f;
	const float PORTAL_ENTRY_TRIGGER_DEPTH = 1.f;
	const float PORTAL_ENTRY_TRIGGER_OFFSET = PORTAL_ENTRY_TRIGGER_DEPTH / 2;
}

Portal::Portal( TextureInfo* texture, physics::Physics& physics )
	: mFaceDir( 0.f, 0.f, 1.f )
	, mPosition( 500.f, 500.f, 500.f )
	, mOriginFaceDir( 0.f, 0.f, 1.f )
	, mUpDir( 0.f, 1.f, 0.f )
	, mRightDir( -1.f, 0.f, 0.f )
	, mFrameRenderable( generate_portal_frame(), Renderer::PORTAL_FRAME_SHADER, texture )
	, mHoleRenderable( generate_portal_ellipse_hole( PORTAL_GUT_WIDTH, PORTAL_GUT_HEIGHT ), Renderer::PORTAL_HOLE_SHADER, nullptr, Renderer::Renderable::DrawType::TRIANGLE_FANS )
	, mHasBeenPlaced( false )
	, mPairedPortal( nullptr )
	, mAttchedCO( nullptr )
	, mPhysics( physics )
{
	// 创建门框碰撞体
	const glm::vec3 front_offset = mFaceDir * PORTAL_FRAME_TICKNESS / 2.f;
	mFrameBoxes.emplace_back(
		physics.CreateBox( 
			mPosition + mUpDir * PORTAL_FRAME_UP_OFFSET - front_offset,
			{ 4 * PORTAL_GUT_WIDTH, PORTAL_GUT_HEIGHT / 2.f, PORTAL_FRAME_TICKNESS }, 
			Physics::PhysicsObject::Type::STATIC, 
			static_cast<int>( PhysicsGroup::PORTAL_FRAME ),
			static_cast<int>( PhysicsGroup::PLAYER ) ) );
	mFrameBoxes.emplace_back(
		physics.CreateBox( 
			mPosition +  mUpDir * -PORTAL_FRAME_UP_OFFSET - front_offset,
			{ 4 * PORTAL_GUT_WIDTH, PORTAL_GUT_HEIGHT / 2.f, PORTAL_FRAME_TICKNESS }, 
			Physics::PhysicsObject::Type::STATIC, 
			static_cast<int>( PhysicsGroup::PORTAL_FRAME ),
			static_cast<int>( PhysicsGroup::PLAYER ) ) );
	mFrameBoxes.emplace_back(
		physics.CreateBox( 
			mPosition + mRightDir * -PORTAL_FRAME_RIGHT_OFFSET - front_offset,
			{ PORTAL_GUT_WIDTH, 2 * PORTAL_GUT_HEIGHT, PORTAL_FRAME_TICKNESS }, 
			Physics::PhysicsObject::Type::STATIC, 
			static_cast<int>( PhysicsGroup::PORTAL_FRAME ),
			static_cast<int>( PhysicsGroup::PLAYER ) ) );
	mFrameBoxes.emplace_back(
		physics.CreateBox( 
			mPosition + mRightDir * PORTAL_FRAME_RIGHT_OFFSET - front_offset,
			{ PORTAL_GUT_WIDTH, 2 * PORTAL_GUT_HEIGHT, PORTAL_FRAME_TICKNESS }, 
			Physics::PhysicsObject::Type::STATIC, 
			static_cast<int>( PhysicsGroup::PORTAL_FRAME ),
			static_cast<int>( PhysicsGroup::PLAYER ) ) );

	// 创建两个触发区
	const bool is_ghost = true;
	mEntryTrigger = physics.CreateBox( 
			mPosition +  mFaceDir * 3.f * PORTAL_ENTRY_TRIGGER_OFFSET,
			{ 2 * PORTAL_GUT_WIDTH, 2 * PORTAL_GUT_HEIGHT, 8.f * PORTAL_ENTRY_TRIGGER_DEPTH }, 
			Physics::PhysicsObject::Type::STATIC, 
			static_cast<int>( PhysicsGroup::PORTAL_FRAME ),
			static_cast<int>( PhysicsGroup::PLAYER ),
			is_ghost
		);
	mTeleportTrigger = physics.CreateBox( 
			mPosition - mFaceDir * PORTAL_ENTRY_TRIGGER_OFFSET,
			{ 2 * PORTAL_GUT_WIDTH, 2 * PORTAL_GUT_HEIGHT, PORTAL_ENTRY_TRIGGER_DEPTH }, 
			Physics::PhysicsObject::Type::STATIC, 
			static_cast<int>( PhysicsGroup::PORTAL_FRAME ),
			static_cast<int>( PhysicsGroup::PLAYER ),
			is_ghost
		);
}

Portal::~Portal()
{}

void 
Portal::SetPair( Portal* paired_portal )
{
	mPairedPortal = paired_portal;
}

bool 
Portal::PlaceAt( glm::vec3 pos, glm::vec3 dir, const btCollisionObject* attched_surface_co )
{
	// 使用`is_vector_has_nan_value`来检测`pos`和`dir`是否含有NaN值，
	// Bullet物理引擎在做射线检测时有时候结果会带有NaN值，就很烦 :(
	if( !attched_surface_co || is_vector_has_nan_value( pos ) || is_vector_has_nan_value( dir ) )
	{
		return false;
	}

	mAttchedCO = attched_surface_co;
	// Bullet物理引擎的射线检测碰撞法线有误差 大概是 < 0.00015
	// 这里小于这个值的都当作0
	dir = round_vector_to_zero( std::move( dir ) );
	
	glm::vec3 rot_axis = mUpDir;
	if( mOriginFaceDir != dir && mOriginFaceDir != dir * -1.f )
	{
		// 如果门被放在水平位置，上和右方向要换过来
		if( abs( dir.y ) >= abs( dir.z ) && abs( dir.y ) >= abs( dir.x ) )
		{
			mRightDir = glm::cross( mOriginFaceDir, dir );
			mUpDir = glm::cross( mRightDir, dir  );
			rot_axis = mRightDir;
		}
		else
		{
			mUpDir = glm::cross( mOriginFaceDir, dir );
			mRightDir = glm::normalize( glm::cross( dir, mUpDir ) );
			rot_axis = mUpDir;
		}
	}
	else
	{
		mUpDir = { 0.f, 1.f, 0.f };
		mRightDir = glm::normalize( glm::cross( dir, mUpDir ) );
	}

	mPosition = pos;
	mFaceDir = dir;

	// 根据上面求得的位置和旋转变量来更新门口和门面的模型矩阵
	// 求它们之间的夹角
	float theta = std::acos( glm::dot( mOriginFaceDir, dir ) );
	mFrameRenderable.Translate( pos + mFaceDir * 0.2f );
	mFrameRenderable.Rotate( theta, rot_axis );
	mHoleRenderable.Translate( pos + mFaceDir * 0.1f );
	mHoleRenderable.Rotate( theta, rot_axis );

	mHasBeenPlaced = true;
	
	// 确保门框也做同样的位移和旋转
	const glm::vec3 front_offset = mFaceDir * PORTAL_FRAME_TICKNESS / 2.f;
	for( size_t i = 0; i < mFrameBoxes.size(); i ++)
	{
		glm::mat4 trans( 1.f );
		switch(i)
		{
		case 0:
			trans = glm::translate( trans, pos + mUpDir * PORTAL_FRAME_UP_OFFSET - front_offset );
			break;
		case 1:
			trans = glm::translate( trans, pos + mUpDir * -PORTAL_FRAME_UP_OFFSET - front_offset );
			break;
		case 2:
			trans = glm::translate( trans, pos + mRightDir * -PORTAL_FRAME_RIGHT_OFFSET - front_offset );
			break;
		case 3:
			trans = glm::translate( trans, pos + mRightDir * PORTAL_FRAME_RIGHT_OFFSET - front_offset );
			break;
		default:
			// You are fucked if it hits here.
			break;
		}
		trans = glm::rotate( trans, theta, rot_axis );

		mFrameBoxes[i]->SetTransform( std::move( trans ) );
	}
	// 门口触发区
	{
		glm::mat4 trans( 1.f );
		trans = glm::translate( trans, pos + mFaceDir * 3.f *  PORTAL_ENTRY_TRIGGER_OFFSET );
		trans = glm::rotate( trans, theta, rot_axis );
		mEntryTrigger->SetTransform( std::move( trans ) );
	}
	// 传送触发区
	{
		glm::mat4 trans( 1.f );
		trans = glm::translate( trans, pos - mFaceDir * PORTAL_ENTRY_TRIGGER_OFFSET );
		trans = glm::rotate( trans, theta, rot_axis );
		mTeleportTrigger->SetTransform( std::move( trans ) );
	}

	return true;
}

Renderer::Renderable*
Portal::GetFrameRenderable()
{
	return &mFrameRenderable;
}

Renderer::Renderable* 
Portal::GetHoleRenderable()
{
	return &mHoleRenderable;
}

bool 
Portal::HasBeenPlaced()
{
	return mHasBeenPlaced;
}

bool
Portal::IsLinkActive()
{
	if( mPairedPortal )
	{
		return mHasBeenPlaced && mPairedPortal->HasBeenPlaced();
	}
	return false;
}

Portal*
Portal::GetPairedPortal()
{
	return mPairedPortal;
}

glm::vec3 
Portal::GetPosition()
{
	return mPosition;
}

void 
Portal::CheckPortalable( Portalable* portalable )
{
	if( mHasBeenPlaced && mPairedPortal && mPairedPortal->HasBeenPlaced() && portalable && portalable->GetPhysicsObject() )
	{
		auto physics_object = portalable->GetPhysicsObject();
		const bool is_detected = IsPortalableEntering( portalable );
		if( mAttchedCO )
		{
			// 当物体在传送门判定区内，关闭物体与传送门附着面的碰撞检测，使得物体可以“穿过”传送门
			// 当两个门在同一面墙时，物体进入任意一个门的警戒区都会关闭与墙壁的碰撞
			if( mAttchedCO == mPairedPortal->GetAttachedCollisionObject() &&
				is_detected != mPairedPortal->IsPortalableEntering( portalable ) )
			{
				physics_object->SetIgnoireCollisionWith( mAttchedCO, true );
			}
			else
			{
				physics_object->SetIgnoireCollisionWith( mAttchedCO, is_detected );
			}
		}
		if( is_detected )
		{
			auto aabb = mTeleportTrigger->GetAABB();
			if( aabb.IsContain( physics_object->GetPosition() ) )
			{
				portalable->Teleport( *this );
			}
		}
	}
}

bool 
Portal::IsPortalableEntering( Portalable* portalable )
{
	if( !portalable || !portalable->GetPhysicsObject() )
	{
		return false;
	}
	return mEntryTrigger->GetAABB().IsContain( portalable->GetPhysicsObject()->GetPosition() );
}

glm::vec3 
Portal::GetFaceDir()
{
	return mFaceDir;
}

glm::vec3 
Portal::GetUpDir()
{
	return mUpDir;
}

const btCollisionObject*
Portal::GetAttachedCollisionObject()
{
	return mAttchedCO;
}

glm::mat4 
Portal::ConvertView( const glm::mat4& view_matrix )
{
	// 先将视图矩阵转换到本传送门的本地空间，再旋转180度，然后用出口的逆变换矩阵转换到世界空间
	glm::mat4 model_view = view_matrix * mHoleRenderable.GetTransform();
	glm::mat4 final_view = model_view
						   * glm::rotate( glm::mat4( 1.f ), glm::radians( 180.f ), glm::vec3( 0.f, 1.f, 0.f ) )
						   * glm::inverse( mPairedPortal->GetHoleRenderable()->GetTransform() );
	return final_view;
}

glm::vec3 
Portal::ConvertPointToOutPortal( glm::vec3 point )
{
	if( !mPairedPortal )
	{
		return glm::vec3{ 0.f };
	}

	// 同ConvertView
	return mPairedPortal->GetHoleRenderable()->GetTransform()
		* glm::rotate( glm::mat4( 1.f ), glm::radians( 180.f ), glm::vec3( 0.f, 1.f, 0.f ) ) 
		* glm::inverse( mHoleRenderable.GetTransform() ) 
		* glm::vec4( std::move( point ), 1.f );
}

glm::vec3 
Portal::ConvertDirectionToOutPortal( glm::vec3 direction, glm::vec3 old_start_pos, glm::vec3 new_start_pos )
{
	// 由于转换的是方向不是一个点，这里先找出从出发点向给与方向上的一个点
	// 然后把该点进行变换，得到新的目标点后与新的出发点组成一个新的单位方向
	// 再乘以老方向的"长度“
	glm::vec3 target = old_start_pos + glm::normalize( direction );
	target = ConvertPointToOutPortal( std::move( target ) );
	return glm::normalize( target - new_start_pos ) * glm::length( direction );
}
