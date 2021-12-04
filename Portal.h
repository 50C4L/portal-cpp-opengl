#ifndef _PORTAL_H
#define _PORTAL_H

#include "Renderer.h"
#include "Physics.h"

namespace portal
{
	class Portalable;
	///
	/// 传送门本身分为两部分
	/// 一部分时门框，只是一个带透明纸的贴图
	/// 另一部分是作为显示传送门内容的模型，这里用一个椭圆形来表示，详情看`generate_portal_ellipse_hole`
	/// 
	class Portal
	{
	public:
		///
		/// 构造函数
		/// 
		/// @param texture
		///		传送门用到的贴图（门框）
		/// 
		Portal( TextureInfo* texture, physics::Physics& physics );
		~Portal();

		///
		/// 设置与本传送门配对的传送门
		/// 
		/// @param paired_portal
		///		Pointer to Portal
		/// 
		void SetPair( Portal* paired_portal );

		///
		/// 根据提供的位置和方向更新传送门的方位
		/// 
		/// @param pos
		///		传送门的新位置
		/// 
		/// @param dir
		///		传送门面向的方向
		/// 
		/// @return 
		///		True表示有更新到
		/// 
		bool PlaceAt( glm::vec3 pos, glm::vec3 dir, const btCollisionObject* attched_surface_co );

		///
		/// 获取门框和门面的渲染体
		/// 
		Renderer::Renderable* GetFrameRenderable();
		Renderer::Renderable* GetHoleRenderable();

		///
		/// 传送门是否被放置
		/// 
		bool HasBeenPlaced();

		///
		/// 传送门是否可用
		/// 取决于配对的传送门是否已经被放置，以及玩家摄像机是否存在
		/// 
		/// @return 
		///		True if active
		/// 
		bool IsLinkActive();

		///
		/// 获取配对的传送门指针
		/// 
		Portal* GetPairedPortal();

		///
		///  获取传送门位置
		/// 
		glm::vec3 GetPosition();


		void CheckPortalable( Portalable* portalable );
		bool IsPortalableEntering( Portalable* portalable );

		glm::vec3 GetFaceDir();
		glm::vec3 GetUpDir();

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

		///
		/// 获取附着墙面的物理碰撞体
		/// 
		const btCollisionObject* GetAttachedCollisionObject();

		///
		/// 将提供的点转换到出口的相对位置
		/// 
		/// @param point
		///		要穿越的点（世界坐标）
		/// 
		glm::vec3 ConvertPointToOutPortal( glm::vec3 point );

		///
		/// 将提供的向量转换到出口的相对位置
		/// 
		/// @param direction
		///		向量
		/// 
		/// @param old_start_pos
		///		转换前的方向起始点
		/// 
		/// @param new_start_pos
		///		转换后的方向起始点
		/// 
		glm::vec3 ConvertDirectionToOutPortal( glm::vec3 direction, glm::vec3 old_start_pos, glm::vec3 new_start_pos );

	private:
		glm::vec3 mFaceDir;                    ///< 传送门面向的方向
		glm::vec3 mPosition;                   ///< 传送门位置
		glm::vec3 mOriginFaceDir;              ///< 传送门初始面向方向
		glm::vec3 mUpDir;                      ///< 传送门上方向
		glm::vec3 mRightDir;                   ///< 传送门的右方
		Renderer::Renderable mFrameRenderable; ///< 门框渲染体
		Renderer::Renderable mHoleRenderable;  ///< 门面渲染体
		bool mHasBeenPlaced;                   ///< 是否被放置

		Portal* mPairedPortal;                 ///< 配对的传送门指针
		// 当传送门被放置后，如果玩家在传送门的门口区域内，传送门附着的墙壁不能与玩家发生碰撞玩家才能穿过
		// 传送门。因此我们需要一圈的空气墙作为门框来挡住玩家
		std::vector<std::unique_ptr<physics::Physics::Box>> mFrameBoxes;
		std::unique_ptr<physics::Physics::Box> mEntryTrigger;
		std::unique_ptr<physics::Physics::Box> mTeleportTrigger;
		const btCollisionObject* mAttchedCO;

		physics::Physics& mPhysics;
	};
}

#endif
