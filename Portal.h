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
		Portal( TextureInfo* texture, float view_width, float view_height );
		~Portal();

		void SetPair( Portal* paired_portal );

		bool UpdatePosition( glm::vec3 pos, glm::vec3 dir );

		Renderer::Renderable* GetFrameRenderable();
		Renderer::Renderable* GetHoleRenderable();

		glm::vec3 mFaceDir;
		glm::vec3 mPosition;

		bool HasBeenPlaced();

		///
		/// 传送门是否可用
		/// 取决于配对的传送门是否已经被放置，以及玩家摄像机是否存在
		/// 
		/// @return 
		///		True if active
		/// 
		bool IsLinkActive();

		Portal* GetPairedPortal();

		///
		/// 将提供的视图矩阵转换到配对的传送门相对位置的视图矩阵
		/// 
		/// @param view_matrix
		///		当前到本传送门的视图矩阵
		/// 
		/// @return
		///		转换后配对传送门后的视图矩阵
		/// 
		glm::mat4 ConvertView( const glm::mat4& view_matrix );

	private:
		glm::vec3 mOriginFaceDir;
		Renderer::Renderable mFrameRenderable;
		Renderer::Renderable mHoleRenderable;
		bool mHasBeenPlaced;

		Portal* mPairedPortal;
	};
}

#endif
