#ifndef _PORTAL_H
#define _PORTAL_H

#include "Renderer.h"

namespace portal
{
	///
	/// 一个传送门本身只是一个贴图，以Sprite的形式“浮”在表面
	/// 
	class Portal
	{
	public:
		Portal( unsigned int texture );
		~Portal();

		bool UpdatePosition( glm::vec3 pos, glm::vec3 dir );

		Renderer::Renderable* GetFrameRenderable();
		Renderer::Renderable* GetHoleRenderable();

		glm::vec3 mFaceDir;
		glm::vec3 mPosition;
	private:
		glm::vec3 mOriginFaceDir;
		Renderer::Renderable mFrameRenderable;
		Renderer::Renderable mHoleRenderable;
	};
}

#endif
