#include "ScenePrimitives.h"

#include "Utility.h"

using namespace portal;

const float SKYBOX_SIZE = 500.f;

SceneBox::SceneBox( glm::vec3 position, float width, float height, float depth, std::string shader_name, TextureInfo* texture )
	: Renderer::Renderable( utility::generate_box_vertices( position, width, height, depth, 4.f ), std::move( shader_name ), texture )
{
}

SceneSkyBox::SceneSkyBox( TextureInfo* cube_map_tex )
	: Renderer::Renderable(
		{
			// Top
			{{ -SKYBOX_SIZE,  SKYBOX_SIZE, -SKYBOX_SIZE }, {}, {}, {} },
			{{  SKYBOX_SIZE,  SKYBOX_SIZE, -SKYBOX_SIZE }, {}, {}, {} },
			{{  SKYBOX_SIZE,  SKYBOX_SIZE,  SKYBOX_SIZE }, {}, {}, {} },
			{{  SKYBOX_SIZE,  SKYBOX_SIZE,  SKYBOX_SIZE }, {}, {}, {} },
			{{ -SKYBOX_SIZE,  SKYBOX_SIZE,  SKYBOX_SIZE }, {}, {}, {} },
			{{ -SKYBOX_SIZE,  SKYBOX_SIZE, -SKYBOX_SIZE }, {}, {}, {} },
			// Bottom
			{{ -SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE }, {}, {}, {} },
			{{ -SKYBOX_SIZE, -SKYBOX_SIZE,  SKYBOX_SIZE }, {}, {}, {} },
			{{  SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE }, {}, {}, {} },
			{{  SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE }, {}, {}, {} },
			{{ -SKYBOX_SIZE, -SKYBOX_SIZE,  SKYBOX_SIZE }, {}, {}, {} },
			{{  SKYBOX_SIZE, -SKYBOX_SIZE,  SKYBOX_SIZE }, {}, {}, {} },
			// Left
			{{ -SKYBOX_SIZE, -SKYBOX_SIZE,  SKYBOX_SIZE }, {}, {}, {} },
			{{ -SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE }, {}, {}, {} },
			{{ -SKYBOX_SIZE,  SKYBOX_SIZE, -SKYBOX_SIZE }, {}, {}, {} },
			{{ -SKYBOX_SIZE,  SKYBOX_SIZE, -SKYBOX_SIZE }, {}, {}, {} },
			{{ -SKYBOX_SIZE,  SKYBOX_SIZE,  SKYBOX_SIZE }, {}, {}, {} },
			{{ -SKYBOX_SIZE, -SKYBOX_SIZE,  SKYBOX_SIZE }, {}, {}, {} },
			// Right
			{{ SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE }, {}, {}, {} },
			{{ SKYBOX_SIZE, -SKYBOX_SIZE,  SKYBOX_SIZE }, {}, {}, {} },
			{{ SKYBOX_SIZE,  SKYBOX_SIZE,  SKYBOX_SIZE }, {}, {}, {} },
			{{ SKYBOX_SIZE,  SKYBOX_SIZE,  SKYBOX_SIZE }, {}, {}, {} },
			{{ SKYBOX_SIZE,  SKYBOX_SIZE, -SKYBOX_SIZE }, {}, {}, {} },
			{{ SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE }, {}, {}, {} },
			// front
			{{ -SKYBOX_SIZE, -SKYBOX_SIZE,  SKYBOX_SIZE }, {}, {}, {} },
			{{ -SKYBOX_SIZE,  SKYBOX_SIZE,  SKYBOX_SIZE }, {}, {}, {} },
			{{  SKYBOX_SIZE,  SKYBOX_SIZE,  SKYBOX_SIZE }, {}, {}, {} },
			{{  SKYBOX_SIZE,  SKYBOX_SIZE,  SKYBOX_SIZE }, {}, {}, {} },
			{{  SKYBOX_SIZE, -SKYBOX_SIZE,  SKYBOX_SIZE }, {}, {}, {} },
			{{ -SKYBOX_SIZE, -SKYBOX_SIZE,  SKYBOX_SIZE }, {}, {}, {} },
			// back
			{{ -SKYBOX_SIZE,  SKYBOX_SIZE, -SKYBOX_SIZE }, {}, {}, {} },
			{{ -SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE }, {}, {}, {} },
			{{  SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE }, {}, {}, {} },
			{{  SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE }, {}, {}, {} },
			{{  SKYBOX_SIZE,  SKYBOX_SIZE, -SKYBOX_SIZE }, {}, {}, {} },
			{{ -SKYBOX_SIZE,  SKYBOX_SIZE, -SKYBOX_SIZE }, {}, {}, {} },
		}, Renderer::DEFAULT_SKYBOX_SHADER, cube_map_tex
	)
{}
