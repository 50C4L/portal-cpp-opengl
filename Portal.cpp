#include "Portal.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

using namespace portal;

namespace
{
	const float PORTAL_WIDTH = 15.f;
	const float PORTAL_HEIGHT = 16.f;

	std::vector<Vertex>
	generate_portal_mesh()
	{
		return {
			{ { -PORTAL_WIDTH / 2.f, -PORTAL_HEIGHT / 2.f, 0.f }, { 1.f, 1.f, 1.f, 1.f }, { 0.f, 0.f } },
			{ { -PORTAL_WIDTH / 2.f,  PORTAL_HEIGHT / 2.f, 0.f },  { 1.f, 1.f, 1.f, 1.f }, { 0.f, 1.f } },
			{ { PORTAL_WIDTH / 2.f,  -PORTAL_HEIGHT / 2.f, 0.f },  { 1.f, 1.f, 1.f, 1.f }, { 1.f, 0.f } },
			{ { PORTAL_WIDTH / 2.f,  -PORTAL_HEIGHT / 2.f, 0.f },  { 1.f, 1.f, 1.f, 1.f }, { 1.f, 0.f } },
			{ { -PORTAL_WIDTH / 2.f,  PORTAL_HEIGHT / 2.f, 0.f },  { 1.f, 1.f, 1.f, 1.f }, { 0.f, 1.f } },
			{ { PORTAL_WIDTH / 2.f,   PORTAL_HEIGHT / 2.f, 0.f },  { 1.f, 1.f, 1.f, 1.f }, { 1.f, 1.f } },
		};
	}
}

Portal::Portal( const std::string& shader, unsigned int texture )
	: Renderable( generate_portal_mesh(), shader, texture )
	, mFaceDir( 0.f, 0.f, 1.f )
	, mPosition( 0.f, 0.f, 0.f )
	, mOriginFaceDir( 0.f, 0.f, 1.f )
{}

Portal::~Portal()
{}

bool 
Portal::UpdatePosition( glm::vec3 pos, glm::vec3 dir )
{
	if( pos == mPosition && dir == mFaceDir )
	{
		return false;
	}

	// 求它们之间的夹角
	float theta = std::acos( glm::dot( mOriginFaceDir, dir ) );
	// 求垂直于它们的向量用作转轴
	glm::vec3 cross_product = glm::cross( mOriginFaceDir, dir );
	cross_product = glm::normalize( cross_product );

	mPosition = pos;
	mFaceDir = dir;

	Translate( pos + mFaceDir * 0.1f );
	Rotate( theta, cross_product );

	return true;
}
