#ifndef _LEVEL_CONTROLLER_H
#define _LEVEL_CONTROLLER_H

#include <vector>
#include <unordered_map>
#include <memory>
#include <glm/vec3.hpp>

namespace portal
{
	class SceneBox;
	class Renderer;
	class Physics;

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

	private:
		std::unordered_map<std::string, std::unique_ptr<Level>> mLevels;
		Renderer& mRenderer;
		std::unique_ptr<Physics> mPhysics;
	};
}

#endif _LEVEL_CONTROLLER_H
