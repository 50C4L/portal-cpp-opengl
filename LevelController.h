#ifndef _LEVEL_CONTROLLER_H
#define _LEVEL_CONTROLLER_H

#include <vector>
#include <unordered_map>
#include <memory>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include "Physics.h"

namespace portal
{
	class SceneBox;
	class SceneSkyBox;
	class Renderer;
	class Camera;
	class Portal;
	class DynamicBox;
	class Player;

	///
	/// 简单关卡控制器
	/// 加载关卡和Gameplay逻辑都在里面
	/// TODO: Gameplay逻辑应该独立出来
	/// 
	class LevelController
	{
	public:
		class Level
		{
		public:
			///
			/// 关卡组成只有墙，所以就写这里了
			/// 
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

			/// 关卡不能被Copy
			Level( const Level& ) = delete;
			Level operator=( const Level& ) = delete;

			///
			/// 加墙，读取关卡文件时使用
			/// 
			void AddWall( Wall&& wall );
			std::vector<Wall>& GetWalls();

			///
			/// 设置玩家出生点，读取关卡文件时使用
			/// 
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

		void RenderPortals( glm::mat4 view_matrix, glm::mat4 projection_matrix, int current_recursion_level = 0 );
		void RenderBaseScene( glm::mat4 view_matrix, glm::mat4 projection_matrix );
		void RenderSkybox( glm::mat4 view_matrix, glm::mat4 projection_matrix );

		Renderer& mRenderer;
		std::unique_ptr<physics::Physics> mPhysics;
		std::unordered_map<std::string, std::unique_ptr<Level>> mLevels;
		std::unique_ptr<Player> mPlayer;
		std::shared_ptr<Camera> mMainCamera;
		int mMouseX;
		int mMouseY;
		std::unique_ptr<SceneSkyBox> mSkybox;
		std::unique_ptr<Portal> mPortals[2];
		Level* mCurrentLevel;
		glm::mat4 mMainCamProjMat;

		std::unique_ptr<DynamicBox> mDyBox;
		bool mShootBoxToggle = false;
		bool mRenderClone = false;
	};
}

#endif _LEVEL_CONTROLLER_H
