#ifndef _PORTAL_H
#define _PORTAL_H

#include "Renderer.h"
#include "Camera.h"

namespace portal
{
	///
	/// 一个传送门本身只是一个贴图，以Sprite的形式“浮”在表面
	/// 
	class Portal
	{
	public:
		Portal( unsigned int texture, float view_width, float view_height );
		~Portal();

		void SetPair( Portal* paired_portal, Camera* player_camera );

		bool UpdatePosition( glm::vec3 pos, glm::vec3 dir );

		Renderer::Renderable* GetFrameRenderable();
		Renderer::Renderable* GetHoleRenderable();

		glm::vec3 mFaceDir;
		glm::vec3 mPosition;

		Camera* GetCamera();
		bool HasBeenPlaced();

		///
		/// 传送门是否可用
		/// 取决于配对的传送门是否已经被放置，以及玩家摄像机是否存在
		/// 
		/// @return 
		///		True if active
		/// 
		bool IsLinkActive();

	private:
		glm::vec3 mOriginFaceDir;
		Renderer::Renderable mFrameRenderable;
		Renderer::Renderable mHoleRenderable;
		Camera mCamera;
		bool mHasBeenPlaced;

		Portal* mPairedPortal;
		Camera* mPlayerCamera;
	};
}

#endif
