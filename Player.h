#ifndef _PLAYER_H
#define _PLAYER_H

#include <glm/vec3.hpp>

#include "Physics.h"

namespace portal
{
	class Player
	{
	public:
		Player();
		~Player();

		void Spawn( glm::vec3 position, Physics& physics );

	private:
		std::unique_ptr<Physics::Box> mCollisionBox;
		bool mIsActive;
	};
}

#endif
