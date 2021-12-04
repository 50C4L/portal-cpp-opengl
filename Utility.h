#ifndef _UTILITY_H
#define _UTILITY_H

#include <glm/mat4x4.hpp>
#include <vector>
#include "Renderer.h"

namespace portal
{
	namespace utility
	{
		extern const float RAD_TO_DEG;
		extern const float DEG_TO_RAD;

		glm::vec3 extract_view_postion_from_matrix( const glm::mat4& view_matrix );

		bool is_vector_has_nan_value( const glm::vec3& vec );

		glm::vec3 round_vector_to_zero( glm::vec3 vec, float threashold = 0.00015f );

		std::vector<Vertex> generate_box_vertices( glm::vec3 position, float width, float height, float depth, float repeat );
	}
}

#endif // !_UTILITY_H

