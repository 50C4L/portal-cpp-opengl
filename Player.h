#ifndef _PLAYER_H
#define _PLAYER_H

#include <glm/vec3.hpp>
#include <chrono>

#include "Physics.h"
#include "Portalable.h"

namespace portal
{
	class Camera;
	class Portal;

	///
	/// Simple player class
	/// Uses a capsule as the physics collider. Movement and jumping act on the capsule itself; the camera sits on the upper half.
	/// The capsule itself is locked so it can't rotate; look direction decides which way "forward" is.
	/// WASD for four-way movement, space to jump.
	/// Left click opens portal 1 (blue) on the target surface, right click portal 2 (orange)
	/// 
	class Player : public Portalable
	{
	public:
		struct PortalInfo
		{
			bool is_active = false;
			glm::vec3 position = glm::vec3( 0.f );
			glm::vec3 face_dir = glm::vec3( 0.f );
		};

		///
		/// Constructor
		/// 
		Player( physics::Physics& physics );
		~Player();

		virtual void Teleport( Portal& in_portal ) override;

		///
		/// Spawn the player at the given spot and initialize
		/// 
		/// @param position
		///		Spawn point
		/// 
		/// @param camera
		///		the camera that will be updated with the player
		/// 
		void Spawn( glm::vec3 position, std::shared_ptr<Camera> camera );
		
		///
		/// Must be called regularly to update the player's state
		/// 
		void Update();

		///
		/// Handle keyboard keys
		/// 
		/// @param key_map
		///		Reference to std::unordered_map<unsigned int, bool>
		/// 
		void HandleKeys( std::unordered_map<unsigned int, bool>& key_map );

		///
		/// Handle mouse buttons
		/// 
		/// @param button_map
		///		Reference to std::unordered_map<int, bool>
		/// 
		void HandleMouse( std::unordered_map<int, bool>& button_map, Portal& portal_left, Portal& portal_right );

		///
		/// Change look direction
		/// 
		/// @param yaw_angle
		///		Vertical angle
		/// 
		/// @param pitch_angle
		///		Horizontal angle
		/// 
		void Look( float yaw_angle, float pitch_angle );

		glm::vec3 GetPosition();
		glm::vec3 GetLookDirection();
		
	private:
		physics::Physics& mPhysics;
		std::unique_ptr<physics::Physics::Capsule> mCollisionCapsule; //< Capsule collider
		float mPreviousYPos;
		bool mIsGrounded;  //< Whether the player is standing on the "ground"

		// Ground check
		int mDownCastHitNumber; //< How many times the ground ray hit something

		std::shared_ptr<Camera> mMainCamera;
		bool mIsRunning;                     //< Whether we're running

		bool mMouseLeftPressed = false;
		bool mMouseRightPressed = false;
	};
}

#endif
