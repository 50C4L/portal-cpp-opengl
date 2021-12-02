#ifndef _UTILITY_H
#define _UTILITY_H

#include <glm/mat4x4.hpp>

namespace portal
{
	namespace utility
	{
		glm::vec3 extract_view_postion_from_matrix( const glm::mat4& view_matrix );

		bool is_vector_has_nan_value( const glm::vec3& vec );

		glm::vec3 round_vector_to_zero( glm::vec3 vec, float threashold = 0.00015f );
	}
}

#endif // !_UTILITY_H

