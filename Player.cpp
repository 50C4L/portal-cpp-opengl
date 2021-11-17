#include "Player.h"

using namespace portal;

Player::Player()
	: mIsActive( false )
{}

Player::~Player()
{}

void 
Player::Spawn( glm::vec3 position, Physics& physics )
{
	mCollisionBox = physics.CreateBox( position, { 2.f, 5.f, 2.f }, true, Physics::PhysicsObject::Type::DYNAMIC );
	mIsActive = true;
}
