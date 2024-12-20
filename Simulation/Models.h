#pragma once

#include <glm/glm.hpp>

#include <map>

#include "Renderer.h"
#include "RendererStructs.h"
#include "extras.h"

struct Model
{
	Model() : pos(glm::vec3(0.0f)) {}
	Model(glm::vec3 _pos) : pos(_pos) {}
	glm::vec3 pos;
	std::map <std::string, std::multimap<std::string, Renderable, RenderableComp>::iterator> renderables;
	std::vector<RenderableInfo> renderableInfos;

	void cleanup(Renderer* rendererHandle)
	{
		for (auto& [name, renderableIt] : renderables)
		{
			rendererHandle->removeRenderable(renderableIt);
		}
		renderables.clear();
	}
};

inline void printVec3(glm::vec3& vec, const std::string& name = "")
{
	std::cout << name << vec.x << " " << vec.y << " " << vec.z << std::endl;
}

struct VectorArrowModel : Model
{
	inline static const glm::vec3 FACING_DEFAULT = glm::vec3(0.0f, -1.0f, 0.0f);

	using Model::renderables;

	glm::vec3 force = glm::vec3(1.0f, 0.0f, 0.0f);


	void update(glm::vec3 _pos, glm::vec3 _force)
	{
		pos = _pos;
		force = _force;
		glm::vec3 forceNorm = glm::normalize(force);

		glm::mat4 translation = glm::translate(glm::mat4(1.0f), pos + force);

		glm::quat rotation = glm::rotation(FACING_DEFAULT, forceNorm);
		(*renderables["head"]).second.transformMatrix = translation * glm::toMat4(rotation);
	}

	VectorArrowModel(glm::vec3 _pos, glm::vec3 _force) : Model(_pos), force(_force)
	{

		renderableInfos =
		{
			RenderableInfo{
				"head",
				"pyramid",
				"cube",
				glm::translate(glm::mat4(1.0f), pos) // * glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.3f, 0.3f, 0.3f))
			}
			/*
			RenderableInfo{
				"shaft",
				"line",
				"line",
				glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, -9.0f, 0.5f)) * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)) //* glm::scale(glm::mat4(1.0f), glm::vec3(0.3f, 0.0f, 0.3f))
			}
			*/
		};
	}
};

struct ParticleModel : Model
{
	using Model::renderables;
	ParticleModel(glm::vec3 _pos) : Model(_pos)
	{
		renderableInfos =
		{
			RenderableInfo{
				"particleBody",
				"cube",
				"cube",
				glm::translate(glm::mat4(1.0f), pos)
			}
		};
	}
};