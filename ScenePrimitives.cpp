#include "ScenePrimitives.h"

using namespace portal;

namespace
{
	std::vector<Vertex>
	generate_box_vertices( glm::vec3 position, float width, float height, float depth, float repeat )
	{
		const glm::vec4 color{ 1.f, 1.f, 1.f, 1.f }; // White
		return {
			{ { position.x - width / 2.f, position.y + height / 2.f, position.z + depth / 2.f }, color, { 0.0f, repeat   }, { 0.f, 0.f, 1.f } },
			{ { position.x + width / 2.f, position.y + height / 2.f, position.z + depth / 2.f }, color, { repeat, repeat }, { 0.f, 0.f, 1.f } },
			{ { position.x + width / 2.f, position.y - height / 2.f, position.z + depth / 2.f }, color, { repeat, 0.0f   }, { 0.f, 0.f, 1.f } },
			{ { position.x + width / 2.f, position.y - height / 2.f, position.z + depth / 2.f }, color, { repeat, 0.0f   }, { 0.f, 0.f, 1.f } },
			{ { position.x - width / 2.f, position.y - height / 2.f, position.z + depth / 2.f }, color, { 0.0f, 0.0f     }, { 0.f, 0.f, 1.f } },
			{ { position.x - width / 2.f, position.y + height / 2.f, position.z + depth / 2.f }, color, { 0.0f, repeat   }, { 0.f, 0.f, 1.f } },

			{ { position.x - width / 2.f, position.y + height / 2.f, position.z + depth / 2.f }, color, { 0.0f, 0.0f     }, { 0.f, 1.f, 0.f } },
			{ { position.x - width / 2.f, position.y + height / 2.f, position.z - depth / 2.f }, color, { 0.0f, repeat   }, { 0.f, 1.f, 0.f } },
			{ { position.x + width / 2.f, position.y + height / 2.f, position.z - depth / 2.f }, color, { repeat, repeat }, { 0.f, 1.f, 0.f } },
			{ { position.x + width / 2.f, position.y + height / 2.f, position.z - depth / 2.f }, color, { repeat, repeat }, { 0.f, 1.f, 0.f } },
			{ { position.x + width / 2.f, position.y + height / 2.f, position.z + depth / 2.f }, color, { repeat, 0.0f   }, { 0.f, 1.f, 0.f } },
			{ { position.x - width / 2.f, position.y + height / 2.f, position.z + depth / 2.f }, color, { 0.0f, 0.0f     }, { 0.f, 1.f, 0.f } },

			{ { position.x - width / 2.f, position.y + height / 2.f, position.z + depth / 2.f }, color, { repeat, repeat }, { -1.f, 0.f, 0.f } },
			{ { position.x - width / 2.f, position.y - height / 2.f, position.z + depth / 2.f }, color, { repeat, 0.0f   }, { -1.f, 0.f, 0.f } },
			{ { position.x - width / 2.f, position.y - height / 2.f, position.z - depth / 2.f }, color, { 0.0f, 0.0f     }, { -1.f, 0.f, 0.f } },
			{ { position.x - width / 2.f, position.y - height / 2.f, position.z - depth / 2.f }, color, { 0.0f, 0.0f     }, { -1.f, 0.f, 0.f } },
			{ { position.x - width / 2.f, position.y + height / 2.f, position.z - depth / 2.f }, color, { 0.0f, repeat   }, { -1.f, 0.f, 0.f } },
			{ { position.x - width / 2.f, position.y + height / 2.f, position.z + depth / 2.f }, color, { repeat, repeat }, { -1.f, 0.f, 0.f } },

			{ { position.x + width / 2.f, position.y - height / 2.f, position.z + depth / 2.f }, color, { 0.0f, 0.0f     }, { 1.f, 0.f, 0.f } },
			{ { position.x + width / 2.f, position.y + height / 2.f, position.z + depth / 2.f }, color, { 0.0f, repeat   }, { 1.f, 0.f, 0.f } },
			{ { position.x + width / 2.f, position.y + height / 2.f, position.z - depth / 2.f }, color, { repeat, repeat }, { 1.f, 0.f, 0.f } },
			{ { position.x + width / 2.f, position.y + height / 2.f, position.z - depth / 2.f }, color, { repeat, repeat }, { 1.f, 0.f, 0.f } },
			{ { position.x + width / 2.f, position.y - height / 2.f, position.z - depth / 2.f }, color, { repeat, 0.0f   }, { 1.f, 0.f, 0.f } },
			{ { position.x + width / 2.f, position.y - height / 2.f, position.z + depth / 2.f }, color, { 0.0f, 0.0f     }, { 1.f, 0.f, 0.f } },

			{ { position.x - width / 2.f, position.y - height / 2.f, position.z - depth / 2.f }, color, { 0.0f, 0.0f     }, { 0.f, -1.f, 0.f } },
			{ { position.x - width / 2.f, position.y - height / 2.f, position.z + depth / 2.f }, color, { 0.0f, repeat   }, { 0.f, -1.f, 0.f } },
			{ { position.x + width / 2.f, position.y - height / 2.f, position.z + depth / 2.f }, color, { repeat, repeat }, { 0.f, -1.f, 0.f } },
			{ { position.x + width / 2.f, position.y - height / 2.f, position.z + depth / 2.f }, color, { repeat, repeat }, { 0.f, -1.f, 0.f } },
			{ { position.x + width / 2.f, position.y - height / 2.f, position.z - depth / 2.f }, color, { repeat, 0.0f   }, { 0.f, -1.f, 0.f } },
			{ { position.x - width / 2.f, position.y - height / 2.f, position.z - depth / 2.f }, color, { 0.0f, 0.0f     }, { 0.f, -1.f, 0.f } },

			{ { position.x + width / 2.f, position.y - height / 2.f, position.z - depth / 2.f }, color, { 0.0f, 0.0f     }, { 0.f, 0.f, -1.f } },
			{ { position.x + width / 2.f, position.y + height / 2.f, position.z - depth / 2.f }, color, { 0.0f, repeat   }, { 0.f, 0.f, -1.f } },
			{ { position.x - width / 2.f, position.y + height / 2.f, position.z - depth / 2.f }, color, { repeat, repeat }, { 0.f, 0.f, -1.f } },
			{ { position.x - width / 2.f, position.y + height / 2.f, position.z - depth / 2.f }, color, { repeat, repeat }, { 0.f, 0.f, -1.f } },
			{ { position.x - width / 2.f, position.y - height / 2.f, position.z - depth / 2.f }, color, { repeat, 0.0f   }, { 0.f, 0.f, -1.f } },
			{ { position.x + width / 2.f, position.y - height / 2.f, position.z - depth / 2.f }, color, { 0.0f, 0.0f     }, { 0.f, 0.f, -1.f } },
		};
	}
}

SceneBox::SceneBox( glm::vec3 position, float width, float height, float depth, std::string shader_name, TextureInfo* texture )
	: Renderer::Renderable( generate_box_vertices( position, width, height, depth, 4.f ), std::move( shader_name ), texture )
{
}

SceneSkyBox::SceneSkyBox( TextureInfo* cube_map_tex )
	: Renderer::Renderable(
		{
			// Top
			{{ -500.0f,  500.0f, -500.0f }, {}, {}, {} },
			{{  500.0f,  500.0f, -500.0f }, {}, {}, {} },
			{{  500.0f,  500.0f,  500.0f }, {}, {}, {} },
			{{  500.0f,  500.0f,  500.0f }, {}, {}, {} },
			{{ -500.0f,  500.0f,  500.0f }, {}, {}, {} },
			{{ -500.0f,  500.0f, -500.0f }, {}, {}, {} },
			// Bottom
			{{ -500.0f, -500.0f, -500.0f }, {}, {}, {} },
			{{ -500.0f, -500.0f,  500.0f }, {}, {}, {} },
			{{  500.0f, -500.0f, -500.0f }, {}, {}, {} },
			{{  500.0f, -500.0f, -500.0f }, {}, {}, {} },
			{{ -500.0f, -500.0f,  500.0f }, {}, {}, {} },
			{{  500.0f, -500.0f,  500.0f }, {}, {}, {} },
			// Left
			{{ -500.0f, -500.0f,  500.0f }, {}, {}, {} },
			{{ -500.0f, -500.0f, -500.0f }, {}, {}, {} },
			{{ -500.0f,  500.0f, -500.0f }, {}, {}, {} },
			{{ -500.0f,  500.0f, -500.0f }, {}, {}, {} },
			{{ -500.0f,  500.0f,  500.0f }, {}, {}, {} },
			{{ -500.0f, -500.0f,  500.0f }, {}, {}, {} },
			// Right
			{{ 500.0f, -500.0f, -500.0f }, {}, {}, {} },
			{{ 500.0f, -500.0f,  500.0f }, {}, {}, {} },
			{{ 500.0f,  500.0f,  500.0f }, {}, {}, {} },
			{{ 500.0f,  500.0f,  500.0f }, {}, {}, {} },
			{{ 500.0f,  500.0f, -500.0f }, {}, {}, {} },
			{{ 500.0f, -500.0f, -500.0f }, {}, {}, {} },
			// front
			{{ -500.0f, -500.0f,  500.0f }, {}, {}, {} },
			{{ -500.0f,  500.0f,  500.0f }, {}, {}, {} },
			{{  500.0f,  500.0f,  500.0f }, {}, {}, {} },
			{{  500.0f,  500.0f,  500.0f }, {}, {}, {} },
			{{  500.0f, -500.0f,  500.0f }, {}, {}, {} },
			{{ -500.0f, -500.0f,  500.0f }, {}, {}, {} },
			// back
			{{ -500.0f,  500.0f, -500.0f }, {}, {}, {} },
			{{ -500.0f, -500.0f, -500.0f }, {}, {}, {} },
			{{  500.0f, -500.0f, -500.0f }, {}, {}, {} },
			{{  500.0f, -500.0f, -500.0f }, {}, {}, {} },
			{{  500.0f,  500.0f, -500.0f }, {}, {}, {} },
			{{ -500.0f,  500.0f, -500.0f }, {}, {}, {} },
		}, Renderer::DEFAULT_SKYBOX_SHADER, cube_map_tex
	)
{}
