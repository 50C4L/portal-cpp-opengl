#ifndef _PORTAL_H
#define _PORTAL_H

#include "Renderer.h"
#include "Physics.h"

namespace portal
{
	class Portalable;
	///
	/// A portal itself is two parts
	/// One part is the frame, just a texture with transparency
	/// The other is the model that actually shows the portal contents, represented here as an ellipse — see `generate_portal_ellipse_hole`
	/// 
	class Portal
	{
	public:
		///
		/// Constructor
		/// 
		/// @param texture
		///		Texture used by the portal (the frame)
		/// 
		Portal( TextureInfo* texture, physics::Physics& physics );
		~Portal();

		///
		/// Pair this portal with another one
		/// 
		/// @param paired_portal
		///		Pointer to Portal
		/// 
		void SetPair( Portal* paired_portal );

		///
		/// Update the portal's pose from the given position and direction
		/// 
		/// @param pos
		///		The portal's new position
		/// 
		/// @param dir
		///		The direction the portal faces
		/// 
		/// @return 
		///		True if it actually got updated
		/// 
		bool PlaceAt( glm::vec3 pos, glm::vec3 dir, const btCollisionObject* attched_surface_co );

		///
		/// Get the frame and hole renderables
		/// 
		Renderer::Renderable* GetFrameRenderable();
		Renderer::Renderable* GetHoleRenderable();

		///
		/// Whether the portal has been placed
		/// 
		bool HasBeenPlaced();

		///
		/// Whether the portal is usable
		/// Depends on whether the paired portal has been placed, and whether the player camera exists
		/// 
		/// @return 
		///		True if active
		/// 
		bool IsLinkActive();

		///
		/// Get a pointer to the paired portal
		/// 
		Portal* GetPairedPortal();

		///
		///  Get the portal position
		/// 
		glm::vec3 GetPosition();


		void CheckPortalable( Portalable* portalable );
		bool IsPortalableEntering( Portalable* portalable );

		glm::vec3 GetFaceDir();
		glm::vec3 GetUpDir();

		///
		/// Convert the given view matrix to the view matrix at the paired portal's relative pose
		/// 
		/// @param view_matrix
		///		Current view matrix looking at this portal
		/// 
		/// @return
		///		The converted view matrix looking out the paired portal
		/// 
		glm::mat4 ConvertView( const glm::mat4& view_matrix );

		///
		/// Get the physics collider of the attached wall
		/// 
		const btCollisionObject* GetAttachedCollisionObject();

		///
		/// Convert the given point to the exit portal's relative position
		/// 
		/// @param point
		///		The point to send through (world space)
		/// 
		glm::vec3 ConvertPointToOutPortal( glm::vec3 point );

		///
		/// Convert the given vector to the exit portal's relative position
		/// 
		/// @param direction
		///		The vector
		/// 
		/// @param old_start_pos
		///		Direction origin before the conversion
		/// 
		/// @param new_start_pos
		///		Direction origin after the conversion
		/// 
		glm::vec3 ConvertDirectionToOutPortal( glm::vec3 direction, glm::vec3 old_start_pos, glm::vec3 new_start_pos );

	private:
		glm::vec3 mFaceDir;                    ///< Direction the portal faces
		glm::vec3 mPosition;                   ///< Portal position
		glm::vec3 mOriginFaceDir;              ///< Portal's original facing direction
		glm::vec3 mUpDir;                      ///< Portal up
		glm::vec3 mRightDir;                   ///< Portal right
		Renderer::Renderable mFrameRenderable; ///< Frame renderable
		Renderer::Renderable mHoleRenderable;  ///< Hole renderable
		bool mHasBeenPlaced;                   ///< Whether it's been placed

		Portal* mPairedPortal;                 ///< Pointer to the paired portal
		// Once a portal is placed, if the player is in the doorway region, the wall the portal is stuck to
		// can't collide with the player — otherwise they couldn't walk through.
		// So we need a ring of air walls around the frame to keep the player from clipping past
		std::vector<std::unique_ptr<physics::Physics::Box>> mFrameBoxes;
		std::unique_ptr<physics::Physics::Box> mEntryTrigger;
		std::unique_ptr<physics::Physics::Box> mTeleportTrigger;
		const btCollisionObject* mAttchedCO;

		physics::Physics& mPhysics;
	};
}

#endif
