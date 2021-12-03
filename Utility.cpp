#include "Utility.h"

#define _USE_MATH_DEFINES
#include <math.h>

const float portal::utility::RAD_TO_DEG = 180.f / static_cast<float>( M_PI );
const float portal::utility::DEG_TO_RAD = static_cast<float>( M_PI ) / 180.f;

glm::vec3 
portal::utility::extract_view_postion_from_matrix( const glm::mat4& view_matrix )
{
	glm::mat4 model_view_t = glm::transpose( view_matrix );
  
	// Get plane normals 
	glm::vec3 n1( model_view_t[0] );
	glm::vec3 n2( model_view_t[1] );
	glm::vec3 n3( model_view_t[2] );

	// Get plane distances
	float d1( model_view_t[0].w );
	float d2( model_view_t[1].w );
	float d3( model_view_t[2].w );

	// Get the intersection of these 3 planes
	glm::vec3 n2n3 = cross( n2, n3 );
	glm::vec3 n3n1 = cross( n3, n1 );
	glm::vec3 n1n2 = cross( n1, n2 );

	glm::vec3 top = ( n2n3 * d1 ) + ( n3n1 * d2 ) + ( n1n2 * d3 );
	float denom = dot( n1, n2n3 );

	return top / -denom;
}

bool 
portal::utility::is_vector_has_nan_value( const glm::vec3& vec )
{
	return glm::any( glm::isnan( vec ) );
}

glm::vec3 
portal::utility::round_vector_to_zero( glm::vec3 vec, float threashold )
{
	if( abs( vec.x ) < threashold )
		vec.x = 0.f;
	if( abs( vec.y ) < threashold )
		vec.y = 0.f;
	if( abs( vec.z ) < threashold )
		vec.z = 0.f;
	return vec;
}
