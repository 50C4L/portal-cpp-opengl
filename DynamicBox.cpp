#include "DynamicBox.h"

#include "LevelConstants.h"
#include "Utility.h"
#include "Portal.h"

#include <glm/gtc/matrix_transform.hpp>

using namespace portal;
using namespace portal::physics;
using namespace portal::level;

DynamicBox::DynamicBox( physics::Physics& physics, glm::vec3 pos, TextureInfo* texture )
	: Renderer::Renderable( utility::generate_box_vertices( glm::vec3{ 0.f }, 5.f, 5.f, 5.f, 1.f ), Renderer::DEFAULT_SHADER, texture )
	, mClone( utility::generate_box_vertices( glm::vec3{ 0.f }, 5.f, 5.f, 5.f, 1.f ), Renderer::DEFAULT_SHADER, texture )
	, mPhysics( physics )
{
	mCollisionBox = mPhysics.CreateBox(
		pos,
		{ 5.f, 5.f, 5.f },
		Physics::PhysicsObject::Type::DYNAMIC,
		static_cast<int>( PhysicsGroup::BOX ),
		static_cast<int>( PhysicsGroup::WALL ) | static_cast<int>( PhysicsGroup::PORTAL_FRAME ) | static_cast<int>( PhysicsGroup::PLAYER )
	);
	//mCollisionBox->SetAngularFactor( { 0.f, 0.f, 0.f } );
	mPortalablePO = mCollisionBox.get();
}

DynamicBox::~DynamicBox()
{}

void
DynamicBox::Teleport( Portal& in_portal )
{
	// 计算传送后位置
	glm::vec3 prev_pos = mCollisionBox->GetPosition();
	glm::vec3 new_pos = in_portal.ConvertPointToOutPortal( prev_pos );
	mCollisionBox->SetPosition( new_pos );

	glm::vec3 velocity = mCollisionBox->GetLinearVelocity();
	velocity = in_portal.ConvertDirectionToOutPortal( std::move( velocity ), std::move( prev_pos ), std::move( new_pos ) );
	velocity *= 1.f - mCollisionBox->GetLinearDamping();
	mCollisionBox->SetLinearVelocity( std::move( velocity ) );

	glm::vec3 angular = mCollisionBox->GetAngularVelocity();
	angular = glm::rotate( glm::mat4( 1.f ), glm::radians( 180.f ), glm::vec3( 0.f, 1.f, 0.f ) ) * glm::vec4( std::move( angular ), 1.0 );
	mCollisionBox->SetAngularVelocity( std::move( angular ) );
}

void
DynamicBox::Update()
{
	mCollisionBox->Activate();
	SetTransform( mCollisionBox->GetTransform() );
}

void 
DynamicBox::SetPosition( glm::vec3 pos )
{
	mCollisionBox->SetPosition( std::move( pos ) );
}

void 
DynamicBox::Launch( glm::vec3 force )
{
	mCollisionBox->SetImpluse( std::move( force ), glm::vec3{ 0.f } );
}

void 
DynamicBox::CloneAt( Portal& in_portal )
{
	auto trans = in_portal.GetPairedPortal()->GetHoleRenderable()->GetTransform()
		* glm::rotate( glm::mat4( 1.f ), glm::radians( 180.f ), glm::vec3( 0.f, 1.f, 0.f ) ) 
		* glm::inverse( in_portal.GetHoleRenderable()->GetTransform() ) 
		* mCollisionBox->GetTransform();
	mClone.SetTransform( std::move( trans ) );
}

Renderer::Renderable* 
DynamicBox::GetClone()
{
	return &mClone;
}
