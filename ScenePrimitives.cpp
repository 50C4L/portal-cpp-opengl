#include "ScenePrimitives.h"

using namespace portal;

namespace
{
	std::vector<Vertex>
	generate_box_vertices( glm::vec3 position, float width, float height, float depth, float repeat )
	{
		return {
			{ { position.x - width / 2.f, position.y + height / 2.f, position.z + depth / 2.f }, { 0.0f, repeat } },
			{ { position.x + width / 2.f, position.y + height / 2.f, position.z + depth / 2.f }, { repeat, repeat } },
			{ { position.x + width / 2.f, position.y - height / 2.f, position.z + depth / 2.f }, { repeat, 0.0f } },
			{ { position.x + width / 2.f, position.y - height / 2.f, position.z + depth / 2.f }, { repeat, 0.0f } },
			{ { position.x - width / 2.f, position.y - height / 2.f, position.z + depth / 2.f }, { 0.0f, 0.0f } },
			{ { position.x - width / 2.f, position.y + height / 2.f, position.z + depth / 2.f }, { 0.0f, repeat } },

			{ { position.x - width / 2.f, position.y + height / 2.f, position.z + depth / 2.f }, { 0.0f, 0.0f } },
			{ { position.x - width / 2.f, position.y + height / 2.f, position.z - depth / 2.f }, { 0.0f, repeat } },
			{ { position.x + width / 2.f, position.y + height / 2.f, position.z - depth / 2.f }, { repeat, repeat } },
			{ { position.x + width / 2.f, position.y + height / 2.f, position.z - depth / 2.f }, { repeat, repeat } },
			{ { position.x + width / 2.f, position.y + height / 2.f, position.z + depth / 2.f }, { repeat, 0.0f } },
			{ { position.x - width / 2.f, position.y + height / 2.f, position.z + depth / 2.f }, { 0.0f, 0.0f } },

			{ { position.x - width / 2.f, position.y + height / 2.f, position.z + depth / 2.f }, { repeat, repeat } },
			{ { position.x - width / 2.f, position.y - height / 2.f, position.z + depth / 2.f }, { repeat, 0.0f } },
			{ { position.x - width / 2.f, position.y - height / 2.f, position.z - depth / 2.f }, { 0.0f, 0.0f } },
			{ { position.x - width / 2.f, position.y - height / 2.f, position.z - depth / 2.f }, { 0.0f, 0.0f } },
			{ { position.x - width / 2.f, position.y + height / 2.f, position.z - depth / 2.f }, { 0.0f, repeat } },
			{ { position.x - width / 2.f, position.y + height / 2.f, position.z + depth / 2.f }, { repeat, repeat } },

			{ { position.x + width / 2.f, position.y - height / 2.f, position.z + depth / 2.f }, { 0.0f, 0.0f } },
			{ { position.x + width / 2.f, position.y + height / 2.f, position.z + depth / 2.f }, { 0.0f, repeat } },
			{ { position.x + width / 2.f, position.y + height / 2.f, position.z - depth / 2.f }, { repeat, repeat } },
			{ { position.x + width / 2.f, position.y + height / 2.f, position.z - depth / 2.f }, { repeat, repeat } },
			{ { position.x + width / 2.f, position.y - height / 2.f, position.z - depth / 2.f }, { repeat, 0.0f } },
			{ { position.x + width / 2.f, position.y - height / 2.f, position.z + depth / 2.f }, { 0.0f, 0.0f } },

			{ { position.x - width / 2.f, position.y - height / 2.f, position.z - depth / 2.f }, { 0.0f, 0.0f } },
			{ { position.x - width / 2.f, position.y - height / 2.f, position.z + depth / 2.f }, { 0.0f, repeat } },
			{ { position.x + width / 2.f, position.y - height / 2.f, position.z + depth / 2.f }, { repeat, repeat } },
			{ { position.x + width / 2.f, position.y - height / 2.f, position.z + depth / 2.f }, { repeat, repeat } },
			{ { position.x + width / 2.f, position.y - height / 2.f, position.z - depth / 2.f }, { repeat, 0.0f } },
			{ { position.x - width / 2.f, position.y - height / 2.f, position.z - depth / 2.f }, { 0.0f, 0.0f } },

			{ { position.x + width / 2.f, position.y - height / 2.f, position.z - depth / 2.f }, { 0.0f, 0.0f } },
			{ { position.x + width / 2.f, position.y + height / 2.f, position.z - depth / 2.f }, { 0.0f, repeat } },
			{ { position.x - width / 2.f, position.y + height / 2.f, position.z - depth / 2.f }, { repeat, repeat } },
			{ { position.x - width / 2.f, position.y + height / 2.f, position.z - depth / 2.f }, { repeat, repeat } },
			{ { position.x - width / 2.f, position.y - height / 2.f, position.z - depth / 2.f }, { repeat, 0.0f } },
			{ { position.x + width / 2.f, position.y - height / 2.f, position.z - depth / 2.f }, { 0.0f, 0.0f } },
		};
	}
}

SceneBox::SceneBox( glm::vec3 position, float width, float height, float depth, std::string shader_name, unsigned int texture_id )
	: Renderer::Renderable( generate_box_vertices( position, width, height, depth, 4.f ), std::move( shader_name ), texture_id )
{
}
