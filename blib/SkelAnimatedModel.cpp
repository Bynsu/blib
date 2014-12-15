#include "SkelAnimatedModel.h"
#include <blib/util/FileSystem.h>
#include <blib/json.h>
#include <blib/ResourceManager.h>
#include <blib/Renderer.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace blib
{
	static glm::mat4 jsonToMatrix(const blib::json::Value &v)
	{
		glm::mat4 mat;

		for (int y = 0; y < 4; y++)
			for (int x = 0; x < 4; x++)
				mat[y][x] = v[y][x];
		return mat;
	}
	


	SkelAnimatedModel::SkelAnimatedModel(const std::string &meshFile, const std::string &boneFile, blib::ResourceManager* resourceManager, blib::Renderer* renderer)
	{
		rootBone = NULL;

		loadModel(meshFile, resourceManager, renderer);
		loadSkeleton(boneFile);
	}

	void SkelAnimatedModel::loadModel(const std::string & modelFile, blib::ResourceManager* resourceManager, blib::Renderer* renderer)
	{
		const blib::json::Value modelData = blib::util::FileSystem::getJson(modelFile);
		std::string directory = "";
		if (modelFile.find("/") != std::string::npos)
			directory = modelFile.substr(0, modelFile.rfind("/"));

		blib::VertexP3T2N3B4B4 vertex;
		float* current = &vertex.position.x;
		float* end = (&vertex.boneWeights.w) + 1;

		int i = 0;
		for (float f : modelData["vertices"])
		{
			if (i >= 8 && i < 8 + 4)
				*((int*)current) = (int)f; //ewwie!
			else
				*current = f;
			current++;
			i++;
			if (current == end)
			{
				i = 0;
				vertices.push_back(vertex);
				current = &vertex.position.x;
			}
		}

		std::vector<unsigned short> indices;
		for (const json::Value& mesh : modelData["meshes"])
		{
			int start = indices.size();
			for (int i : mesh["faces"])
				indices.push_back(i);
			int end = indices.size();

			Mesh* newMesh = new Mesh();
			newMesh->material.alpha = mesh["material"]["alpha"];
			newMesh->material.diffuse = glm::vec3(mesh["material"]["diffuse"][0], mesh["material"]["diffuse"][1], mesh["material"]["diffuse"][2]);
			newMesh->material.ambient = glm::vec3(mesh["material"]["ambient"][0], mesh["material"]["ambient"][1], mesh["material"]["ambient"][2]);
			newMesh->material.specular = glm::vec3(mesh["material"]["specular"][0], mesh["material"]["specular"][1], mesh["material"]["specular"][2]);
			newMesh->material.shinyness = mesh["material"]["shinyness"];

			if (!mesh["material"].isMember("texture") || mesh["material"]["texture"].asString() == "")
				newMesh->material.texture = NULL;
			else
			{
				newMesh->material.texture = resourceManager->getResource<blib::Texture>(directory + "/" + mesh["material"]["texture"].asString());
				newMesh->material.texture->setTextureRepeat(true);
			}

			newMesh->begin = start;
			newMesh->count = end - start;
			meshes.push_back(newMesh);
		}

		vbo = resourceManager->getResource<blib::VBO>();
		vbo->setVertexFormat<blib::VertexP3T2N3B4B4>();
		renderer->setVbo(vbo, vertices);

		vio = resourceManager->getResource<blib::VIO>();
		vio->setElementType<unsigned short>();
		renderer->setVio(vio, indices);
	}
	void SkelAnimatedModel::loadSkeleton(const std::string & boneFile)
	{
		const blib::json::Value boneData = blib::util::FileSystem::getJson(boneFile);
		rootBone = new Bone(boneData);
		
		rootBone->foreach([this](Bone* b)
		{
			if (b->index < 0)
				return;
			if (b->index > (int)bones.size())
				bones.resize(b->index + 1);
			bones[b->index] = b;
		});
	}


	void SkelAnimatedModel::loadAnimation(const std::string &fileName)
	{
		const blib::json::Value animData = blib::util::FileSystem::getJson(fileName);
		Animation* animation = new Animation(animData, rootBone);
		animations[animation->name] = animation;
	}


	SkelAnimatedModel::Animation::Animation(const json::Value& data, Bone* rootBone)
	{
		name = data["name"];
		totalTime = data["length"];
		for (auto c : data["streams"])
			streams.push_back(Stream(c, rootBone));
	}

	SkelAnimatedModel::Animation::Stream* SkelAnimatedModel::Animation::getStream(const SkelAnimatedModel::Bone* bone)
	{
		for (size_t i = 0; i < streams.size(); i++)
			if (streams[i].bone == bone)
				return &streams[i];
		return NULL;
	}

	SkelAnimatedModel::Animation::Stream::Stream(const json::Value &data, Bone* rootBone)
	{
		this->bone = rootBone->get([&data](Bone* b){ return data["node"] == b->name; });
		for (auto v : data["positions"])
		{
			Frame<glm::vec3> pos;
			pos.time = v["time"];
			pos.value.x = v["pos"][0];
			pos.value.y = v["pos"][1];
			pos.value.z = v["pos"][2];
			this->positions.push_back(pos);
		}
		for (auto v : data["scales"])
		{
			Frame<glm::vec3> scale;
			scale.time = v["time"];
			scale.value.x = v["scale"][0];
			scale.value.y = v["scale"][1];
			scale.value.z = v["scale"][2];
			this->scales.push_back(scale);
		}
		for (auto v : data["rotations"])
		{
			Frame<glm::quat> rot;
			rot.time = v["time"];
			rot.value.x = v["rot"][0];
			rot.value.y = v["rot"][1];
			rot.value.z = v["rot"][2];
			rot.value.w = v["rot"][3];
			this->rotations.push_back(rot);
		}
	}

	
	
	
	
	
	
	SkelAnimatedModel::State* SkelAnimatedModel::getNewState()
	{
		State* newState = new State();
		newState->model = this;
		newState->time = 0;
		newState->currentAnimation = NULL;
		newState->boneMatrices.resize(bones.size());
		states.push_back(newState);
		return newState;
	}




	void SkelAnimatedModel::State::update(float elapsedTime)
	{
		if (!currentAnimation)
			currentAnimation = this->model->animations.begin()->second;
		time += elapsedTime;
		time = fmod(time, this->currentAnimation->totalTime);
		
		
		model->rootBone->update(this->boneMatrices, (float)time, currentAnimation);

	}

	void SkelAnimatedModel::State::draw(RenderState& renderState, Renderer* renderer, int materialUniform, int boneUniform)
	{
		renderState.activeVbo = model->vbo;
		renderState.activeVio = model->vio;

		for (size_t i = 0; i < boneMatrices.size(); i++)
			renderState.activeShader->setUniform(boneUniform, i, boneMatrices[i]);

		for (auto m : model->meshes)
		{
			renderState.activeShader->setUniformStruct(materialUniform, m->material);
			renderState.activeTexture[0] = m->material.texture;
			renderer->drawIndexedTriangles<VertexP3T2N3B4B4>(m->begin, m->count, renderState);
		}
	}






	SkelAnimatedModel::Bone::Bone(const json::Value& value)
	{
		index = -1;
		offset = NULL;
		parent = NULL;

		name = value["name"];
		matrix = jsonToMatrix(value["matrix"]);
		if (value.isMember("id"))
		{
			this->index = value["id"];
			this->offset = new glm::mat4(jsonToMatrix(value["offset"]));
		}
		if (value.isMember("children"))
			for (auto c : value["children"])
			{
				Bone* child = new Bone(c);
				child->parent = this;
				children.push_back(child);
			}

	}

	void SkelAnimatedModel::Bone::update(std::vector<glm::mat4> &boneMatrices, float time, Animation* animation, const glm::mat4& parentMatrix) const
	{
		glm::mat4 globalMatrix;
		if (index >= 0)
		{
			Animation::Stream* s = animation->getStream(this);
			assert(s);
			glm::vec3 pos;
			glm::vec3 scale;
			glm::quat rot;

			{
				unsigned int rotFrameIndex = 0;
				while (rotFrameIndex < s->rotations.size() - 1 && time > s->rotations[rotFrameIndex + 1].time)
					rotFrameIndex++;
				int next = (rotFrameIndex + 1) % s->rotations.size();
				float timeDiff = s->rotations[next].time - s->rotations[rotFrameIndex].time;
				float offset = time - s->rotations[rotFrameIndex].time;
				float fac = offset / timeDiff;
				if (timeDiff == 0)
					fac = 0;
				assert(fac >= 0 && fac <= 1);
				rot = glm::slerp(s->rotations[rotFrameIndex].value, s->rotations[next].value, fac);
			}

			{
				unsigned int posFrameIndex = 0;
				while (posFrameIndex < s->positions.size() - 1 && time > s->positions[posFrameIndex + 1].time)
					posFrameIndex++;
				int next = (posFrameIndex + 1) % s->positions.size();
				float timeDiff = s->positions[next].time - s->positions[posFrameIndex].time;
				float offset = time - s->positions[posFrameIndex].time;
				float fac = offset / timeDiff;
				if (timeDiff == 0)
					fac = 0;
				assert(fac >= 0 && fac <= 1);
				pos = glm::mix(s->positions[posFrameIndex].value, s->positions[next].value, fac);
			}

			{
				unsigned int scaleFrameIndex = 0;
				while (scaleFrameIndex < s->scales.size() - 1 && time > s->scales[scaleFrameIndex + 1].time)
					scaleFrameIndex++;
				int next = (scaleFrameIndex + 1) % s->scales.size();
				float timeDiff = s->scales[next].time - s->scales[scaleFrameIndex].time;
				float offset = time - s->scales[scaleFrameIndex].time;
				float fac = offset / timeDiff;
				if (timeDiff == 0)
					fac = 0;
				assert(fac >= 0 && fac <= 1);
				scale = glm::mix(s->scales[scaleFrameIndex].value, s->scales[next].value, fac);
			}

			glm::mat4 animMatrix;
			animMatrix = glm::translate(animMatrix, pos);
			animMatrix = animMatrix * glm::toMat4(rot);
			animMatrix = glm::scale(animMatrix, scale);

			globalMatrix = parentMatrix * animMatrix;
			boneMatrices[index] = globalMatrix * (*offset);
		}
		else
			globalMatrix = parentMatrix * matrix;

		for (auto c : children)
			c->update(boneMatrices, time, animation, globalMatrix);

	}

}