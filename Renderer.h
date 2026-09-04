#ifndef _Renderer_H
#define _Renderer_H

#include <unordered_map>
#include <vector>
#include <memory>
#include <string>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace portal
{
	class Camera;

	struct Vertex
	{
		glm::vec3 pos;
		glm::vec4 color;
		glm::vec2 uv;
		glm::vec3 normal;
	};

	struct TextureInfo
	{
		unsigned int texture_id;
		int tex_type;
	};

	///
	/// Simple (crude) renderer
	/// Can only be used after an OpenGL context has been acquired
	/// 
	class Renderer
	{
	public:
		static const std::string DEFAULT_SHADER;
		static const std::string DEBUG_PHYSICS_SHADER;
		static const std::string DEFAULT_SKYBOX_SHADER;
		static const std::string PORTAL_HOLE_SHADER;
		static const std::string PORTAL_FRAME_SHADER;

		///
		/// Shader class
		/// 
		class Shader
		{
		public:
			///
			/// Default constructor, will compile the built-in shader
			/// 
			Shader();

			///
			/// Parameterized constructor
			/// 
			/// @param vertex_shader
			///		Vertex shader source
			/// 
			/// @param fragment_shader
			///		Fragment shader source
			/// 
			Shader( const std::string& vertex_shader, const std::string& fragment_shader );
			~Shader();

			///
			/// Check whether the shader compiled successfully
			/// 
			/// @return bool
			///		True means compile succeeded
			/// 
			bool IsValid() const;

			///
			/// Get the shader id
			/// 
			/// @return unsigned int
			///		OpenGL Shader Program id
			/// 
			unsigned int GetId() const;

			///
			/// Set the model matrix
			/// 
			/// @param matrix
			///		Const reference to matrix
			/// 
			void SetModelMatrix( const glm::mat4& matrix );

			///
			/// Set the view matrix
			/// 
			/// @param matrix
			///		Const reference to view matrix
			/// 
			void SetViewMatrix( const glm::mat4& matrix );

			///
			/// Set the projection matrix
			/// 
			/// @param matrix
			///		Const reference to projection matrix
			/// 
			void SetProjectionMatrix( const glm::mat4& matrix );

			///
			///	Set a matrix
			/// 
			/// @param location
			///		Uniform location in the shader, from glGetUniformLocation
			/// 
			/// @param matrix
			///		Const reference to the matrix you want
			/// 
			void SetMat4( int location, const glm::mat4& matrix );

		private:
			bool mIsValid;
			unsigned int mId;
			int mModelMatUniformLocation;
			int mViewMatUniformLocation;
			int mProjectionMatUniformLocation;
		};

		///
		/// A renderable object
		/// Includes the shader id, texture id, and buffer ids it uses
		/// 
		class Renderable
		{
		public:
			///
			/// Vertex list draw type
			/// 
			enum class DrawType
			{
				LINES,
				TRIANGLES,
				TRIANGLE_FANS
			};

			///
			/// Constructor
			/// 
			/// @param vertices
			///		Vertices
			/// 
			/// @param shader_name
			///		Shader used for this draw
			/// 
			/// @param texture_id
			///		Texture used for this draw
			/// 
			/// @param draw_type
			///		Draw type, triangles by default
			/// 
			Renderable( 
				std::vector<Vertex>&& vertices, 
				std::string shader_name, 
				TextureInfo* texture_id, 
				DrawType draw_type = DrawType::TRIANGLES );
			~Renderable();

			///
			/// Get the VAO id
			/// 
			/// @return unsigned int
			///		OpenGL VAO id
			/// 
			unsigned int GetVAO() const;

			///
			/// Get the vertex count
			/// 
			/// @return int
			///		Number of vertices
			/// 
			int GetNumberOfVertices() const;

			///
			/// Translate
			/// 
			/// @param offset
			///		Translation offset
			/// 
			void Translate( glm::vec3 offset );

			///
			/// Rotate
			/// 
			/// @param angle
			///		Rotation angle
			/// 
			/// @param axis
			///		Axis of rotation
			/// 
			void Rotate( float angle, glm::vec3 axis );

			std::string GetShaderName() const;
			TextureInfo* GetTexture() const;
			DrawType GetDrawType() const;

			void SetTransform( glm::mat4 trans );
			glm::mat4 GetTransform();

		private:
			unsigned int mVBO;
			unsigned int mVAO;
			int mNumberOfVertices;
			std::string mShader;
			TextureInfo* mTexture;
			DrawType mDrawType;
			glm::vec3 mTranslation;
			glm::vec3 mRotation;
			glm::mat4 mTransform;
			bool mIsDirty;
		};

		///
		/// Crude render resource manager
		/// Responsible for loading textures and shaders
		/// 
		class Resources
		{
		public:
			Resources();
			~Resources() = default;

			///
			/// Load a texture from file
			/// 
			/// @param path
			///		Relative path to the texture file
			/// 
			/// @return bool
			///		True means success
			/// 
			bool LoadTexture( const std::string& path );

			///
			/// Load cubemap files
			/// Make sure the file paths in `files` are ordered as
			/// 
			/// @param files
			///		Texture paths for the six faces, make sure the order is right
			/// 
			/// @param name
			///		Unique name for the cubemap, used to look it up after loading
			/// 
			/// @return bool
			///		True means success
			/// 
			bool LoadCubeMaps( std::vector<std::string> files, const std::string& name );

			///
			/// Get the id of an already loaded texture
			/// 
			/// @param path
			///		Path to the texture file
			/// 
			/// @return
			///		Const referecen to TextureInfo
			/// 
			TextureInfo* GetTextureInfo( const std::string& path );

			///
			/// Compile a shader. After a successful compile the program id lives in mCompiledShaders.
			/// The key is the name you provide
			/// 
			/// @param name
			///		The name stored as the map key
			/// 
			/// @param vertex_shader
			///		Vertex shader
			/// 
			/// @param fragment_shader
			///		Fragment shader
			/// 
			/// @return
			///		true on success, false on failure
			/// 
			bool CompileShader( const std::string& name, std::string vertex_shader, std::string fragment_shader );

			///
			/// Look up a compiled shader by name
			/// 
			/// @param name
			///		THE NAME
			/// 
			/// @return unsigned int
			///		Reference to Shader
			/// 
			Shader& GetShader( const std::string& name );

		private:
			std::unordered_map<std::string, TextureInfo> mLoadedTextures;
			std::unordered_map<std::string, Shader> mCompiledShaders;
		};

public:
		/// Renderer class
		Renderer();
		~Renderer();

		void ResizeViewport( glm::ivec2 size );
		glm::ivec2 GetViewportSize();

		///
		/// Render!!!
		/// 
		void RenderOneoff( Renderable* renderable_obj );

		///
		/// Use the given camera as the camera for subsequent renders
		/// 
		/// @param camera
		///		Raw pointer to Camera
		/// 
		void UseCameraMatrix( Camera* camera );

		void SetViewMatrix( glm::mat4 view );

		void SetProjectionMatrix( glm::mat4 projection );

		Resources& GetResources();

	private:
		glm::mat4 mProjectionMatrix;
		glm::mat4 mViewMatrix;

		std::unique_ptr<Resources> mResources;

		glm::ivec2 mViewportSize;
	};
}

#endif
