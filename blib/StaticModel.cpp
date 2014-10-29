#include "StaticModel.h"

#include <blib/util/FileSystem.h>
#include <blib/json.h>
#include <blib/VIO.h>
#include <blib/VBO.h>
#include <blib/gl/Vertex.h>
#include <blib/RenderState.h>
#include <blib/ResourceManager.h>
#include <blib/Renderer.h>

namespace blib
{
	StaticModel::StaticModel(const std::string &fileName, ResourceManager* resourceManager, Renderer* renderer)
	{
		json::Value modelData = blib::util::FileSystem::getJson(fileName);

		
		blib::VertexP3T2N3 vertex;
		float* current = &vertex.position.x;
		float* end = (&vertex.normal.z) + 1;

		std::vector<blib::VertexP3T2N3> vertices;
		for (float f : modelData["vertices"])
		{
			*current = f;
			current++;
			if (current == end)
			{
				vertices.push_back(vertex);
				current = &vertex.position.x;
			}
			
		}
		std::vector<unsigned short> indices;
		for (int i : modelData["meshes"][0]["faces"])
			indices.push_back(i);

		vbo = resourceManager->getResource<blib::VBO>();
		vbo->setVertexFormat<blib::VertexP3T2N3>();
		renderer->setVbo(vbo, vertices);

		vio = resourceManager->getResource<blib::VIO>();
		vio->setElementType<unsigned short>();
		renderer->setVio(vio, indices);

	}



	void StaticModel::draw(RenderState& renderState, Renderer* renderer)
	{

		renderState.activeVbo = vbo;
		renderState.activeVio = vio;

		renderer->drawIndexedTriangles<VertexP3T2N3>(0, vio->getLength(), renderState);

		renderState.activeVio = NULL;
		renderState.activeVbo = NULL;
	}


}