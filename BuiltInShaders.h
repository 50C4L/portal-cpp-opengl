#ifndef _BUILT_IN_SHADERS_H
#define _BUILT_IN_SHADERS_H

#include <string>

namespace portal
{
	const std::string DEFAULT_VERTEX_SHADER = R"~~~(
		#version 330 core
		layout (location = 0) in vec3 in_pos;
		layout (location = 1) in vec4 in_color;
		layout (location = 2) in vec2 in_uv;
		layout (location = 3) in vec3 in_normal;

		out vec2 tex_coord;
		out vec4 color;
		out vec3 frag_pos;
		out vec3 normal;

		uniform mat4 model_mat;
		uniform mat4 view_mat;
		uniform mat4 projection_mat;
		
		void main()
		{
			gl_Position = projection_mat * view_mat * model_mat * vec4( in_pos, 1.0 );
			frag_pos = vec3( model_mat * vec4( in_pos, 1.0 ) );
			tex_coord = in_uv;
			color = in_color;
			vec4 temp_normal = model_mat * vec4( in_normal, 1.0 );
			normal = temp_normal.xyz;
		}
	)~~~";

	const std::string DEFAULT_FRAGMENT_SHADER = R"~~~(
		#version 330 core
		out vec4 frag_color;

		in vec2 tex_coord;
		in vec3 frag_pos;
		in vec3 normal;

		// texture
		uniform sampler2D color_texture;

		void main()
		{
			vec3 light_color = vec3( 1.0, 1.0, 1.0 );
			vec3 light_pos = vec3( 1000.0, 1000.0, 1000.0 );

			// Ambient
			vec3 ambient = 0.15 * light_color;

			// Diffuse
			vec3 n_normal = normalize( normal );
			vec3 light_dir = normalize( light_pos - frag_pos );
			float diff_f = max( dot( n_normal, light_dir ), 0.0 );
			vec3 diffuse = diff_f * light_color;
			
			// Final
			vec4 tex_color = texture( color_texture, tex_coord );
			vec3 result = ( ambient + diffuse ) * tex_color.rgb;
			frag_color = vec4( result, tex_color.a );
		} 
	)~~~";

	const std::string DEFAULT_SKYBOX_VERTEX_SHADER = R"~~~(
		#version 330 core
		layout (location = 0) in vec3 in_pos;
		
		out vec4 tex_coord;
		
		uniform mat4 model_mat;
		uniform mat4 view_mat;
		uniform mat4 projection_mat;
		
		void main()
		{
			tex_coord = model_mat * vec4( in_pos, 1.0 );
			gl_Position = projection_mat * view_mat * vec4( in_pos, 1.0 );
		} 
	)~~~";

	const std::string DEFAULT_SKYBOX_FRAGMENT_SHADER = R"~~~(
		#version 330 core
		out vec4 frag_color;
		
		in vec4 tex_coord;
		
		uniform samplerCube skybox;
		
		void main()
		{
			frag_color = texture( skybox, tex_coord.xyz );
		}
	)~~~";

	const std::string DEBUG_PHYSICS_FRAGMENT_SHADER = R"~~~(
		#version 330 core
		out vec4 frag_color;
		in vec4 color;
		
		void main()
		{
			frag_color = color;
		} 
	)~~~";

	const std::string PORTAL_HOLE_FRAGMENT_SHADER = R"~~~(
		#version 330 core
		out vec4 frag_color;
		in vec4 color;
		
		void main()
		{
			frag_color = color;
		} 
	)~~~";

	const std::string PORTAL_FRAME_FRAGMENT_SHADER = R"~~~(
		#version 330 core
		out vec4 frag_color;
		in vec2 tex_coord;

		uniform sampler2D color_texture;
		
		void main()
		{
			frag_color = texture( color_texture, tex_coord );
		} 
	)~~~";
}

#endif
