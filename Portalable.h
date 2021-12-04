#ifndef _PORTALABLE_H
#define _PORTALABLE_H

#include "Physics.h"

namespace portal
{
	class Portal;

	class Portalable
	{
	public:
		Portalable() = default;
		~Portalable() = default;

		virtual void Teleport( Portal& in_portal ) = 0;

		physics::Physics::PhysicsObject* GetPhysicsObject()
		{
			return mPortalablePO;
		}

	protected:
		physics::Physics::PhysicsObject* mPortalablePO = nullptr;
	};
}

#endif
