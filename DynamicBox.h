#ifndef _DYNAMIC_BOX_H
#define _DYNAMIC_BOX_H

#include "Portalable.h"
#include "Physics.h"
#include "Renderer.h"

namespace portal
{
	class DynamicBox : public Portalable, public Renderer::Renderable
	{
	public:
		DynamicBox( physics::Physics& physics, glm::vec3 pos, TextureInfo* texture );
		~DynamicBox();

		virtual void Teleport( Portal& in_portal ) override;

		void Update();

		void SetPosition( glm::vec3 pos );
		void Launch( glm::vec3 force );

	private:
		std::unique_ptr<physics::Physics::Box> mCollisionBox;
		physics::Physics& mPhysics;
	};
}

#endif // !_DYNAMIC_BOX_H

