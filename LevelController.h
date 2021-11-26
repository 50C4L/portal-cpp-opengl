#ifndef _LEVEL_CONTROLLER_H
#define _LEVEL_CONTROLLER_H

#include <vector>
#include <unordered_map>
#include <memory>
#include <glm/vec3.hpp>

#include "Physics.h"
#include "Player.h"

namespace portal
{
	class SceneBox;
	class Renderer;
	class Camera;
	class Portal;

	class LevelController
	{
	public:
		class Level
		{
		public:
			struct Wall
			{
				glm::vec3 position = glm::vec3( 0.f );
				float width        = 0.f;
				float height       = 0.f;
				float depth        = 0.f;
				std::string texture_path;
				std::string shader_name;

				std::unique_ptr<SceneBox> render_instance;
				std::unique_ptr<physics::Physics::Box> mCollisionBox;
			};

			Level();
			~Level() = default;

			Level( const Level& ) = delete;
			Level operator=( const Level& ) = delete;

			void AddWall( Wall&& wall );

			std::vector<Wall>& GetWalls();

			void SetSpawn( glm::vec3 point );
			glm::vec3 GetSpawn() const;

		private:
			std::vector<Wall> mWalls;
			bool mIsBuilt;
			glm::vec3 mSpawnPoint;
		};

		LevelController( Renderer& renderer );
		~LevelController();

		void Initialize( int update_interval_ms );

		bool LoadLevelFile( const std::string& path );
		void ChangeLevelTo( const std::string& path );

		void Update();
		void HandleKeys( std::unordered_map<unsigned int, bool>& key_map );
		void HandleMouseMove( int x, int y );
		void HandleMouseButton( std::unordered_map<int, bool>& button_map );

		void RenderScene();

	private:
		void RenderDebugInfo();
		void UpdatePortalState();

		void RenderPortals( Camera* camera, int current_recursion_level = 0 );
		void RenderBaseScene();

		Renderer& mRenderer;
		std::unique_ptr<physics::Physics> mPhysics;
		std::unordered_map<std::string, std::unique_ptr<Level>> mLevels;
		std::unique_ptr<Player> mPlayer;
		std::shared_ptr<Camera> mMainCamera;
		int mMouseX;
		int mMouseY;
		std::unique_ptr<Portal> mPortals[2];
		Level* mCurrentLevel;
	};
}

#endif _LEVEL_CONTROLLER_H
