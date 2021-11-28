#ifndef _SCENE_PRIMITIVES_H
#define _SCENE_PRIMITIVES_H

#include "Renderer.h"

namespace portal
{
	class SceneBox : public Renderer::Renderable
	{
	public:
		SceneBox( glm::vec3 position, float width, float height, float depth, std::string shader_name, TextureInfo* texture );
		~SceneBox() = default;
	};

	class SceneSkyBox : public Renderer::Renderable
	{
	public:
		SceneSkyBox( TextureInfo* cube_map_tex );
		~SceneSkyBox() = default;
	};
}

#endif
