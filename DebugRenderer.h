#ifndef _PHYSICS_DEBUG_RENDERER_H
#define _PHYSICS_DEBUG_RENDERER_H

#include <bullet/btBulletCollisionCommon.h>
#include <glm/vec3.hpp>

#include "Renderer.h"

namespace portal
{
	namespace physics
	{

		class DebugRenderer : public btIDebugDraw
		{
		public:
			DebugRenderer( Renderer& renderer )
				: mDebugMode( btIDebugDraw::DBG_DrawWireframe )
				, mRenderer( renderer )
			{}

			~DebugRenderer()
			{}

			virtual void drawContactPoint( const btVector3& /*PointOnB*/, const btVector3& /*normalOnB*/, btScalar /*distance*/, int /*lifeTime*/, const btVector3& /*color*/ )
			{}

			virtual void reportErrorWarning( const char* /*warningString*/ )
			{}

			virtual void draw3dText( const btVector3& /*location*/, const char* /*textString*/ )
			{}

			virtual void setDebugMode( int debugMode )
			{
				mDebugMode = debugMode;
			}

			virtual int getDebugMode() const
			{
				return mDebugMode;
			}

			virtual void drawLine(const btVector3& from1, const btVector3& to1, const btVector3& color1)
			{
				static const glm::vec4 color{ 1.f, 0.f, 0.f, 1.f };
				mVertices.push_back( 
					{ 
						{ from1.x(), from1.y(), from1.z() },
						color,
						{}
					} 
				);
				mVertices.push_back( 
					{ 
						{ to1.x(), to1.y(), to1.z() },
						color,
						{}
					} 
				);
			}

			virtual void flushLines()
			{
				auto renderable = std::make_unique<Renderer::Renderable>(
					std::move( mVertices ),
					Renderer::DEBUG_PHYSICS_SHADER,
					nullptr,
					Renderer::Renderable::DrawType::LINES
				);
				mVertices.clear();
				mRenderer.RenderOneoff( renderable.get() );
			}

		private:
			int mDebugMode;
			std::vector<Vertex> mVertices;
			Renderer& mRenderer;
		};
	}
}

#endif
