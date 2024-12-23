#pragma once

#include <glm/glm.hpp>

#include <map>

#include "Renderer.h"
#include "RendererStructs.h"
#include "extras.h"

struct Model
{
	Model() = delete;
	Model(const Model&) = delete;
	Model& operator=(const Model&) = delete;
	Model(Model&&) = default;
	Model& operator=(Model&&) = default;

	Model(glm::vec3 _pos) : pos(_pos), faceDirection(glm::vec3(0.0f)), offset(glm::vec3(0.0f)) {}
	Model(glm::vec3 _pos, glm::vec3 _faceDirection) : pos(_pos), faceDirection(_faceDirection), offset(glm::vec3(0.0f)) {}
	Model(glm::vec3 _pos, glm::vec3 _faceDirection, glm::vec3 _offset) : pos(_pos), faceDirection(_faceDirection), offset(_offset) {}
	glm::vec3 pos;
	glm::vec3 faceDirection;
	glm::vec3 offset;
	std::map <std::string, std::multimap<std::string, Renderable, RenderableComp>::iterator> renderables;
	std::vector<RenderableInfo> renderableInfos;

	virtual void update(glm::vec3 pos = glm::vec3(0.0f), glm::vec3 faceDirection = glm::vec3(0.0f), glm::vec3 offset = glm::vec3(0.0f)) = 0;

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
	using Model::renderables;

	void update(glm::vec3 _pos = glm::vec3(0.0f), glm::vec3 _faceDirection = glm::vec3(0.0f), glm::vec3 _offset = glm::vec3(0.0f)) override
	{
		pos = _pos;
		faceDirection = glm::normalize(_faceDirection);
		offset = _offset;

		glm::mat4 translation = glm::translate(glm::mat4(1.0f), pos + offset);

		glm::quat rotation = glm::rotation(Constants::WORLD_UP, faceDirection);
		(*renderables["head"]).second.transformMatrix = translation * glm::toMat4(rotation);
	}

	VectorArrowModel(glm::vec3 _pos, glm::vec3 _faceDirection, glm::vec3 _offset) : Model(_pos, _faceDirection, _offset)
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

	void update(glm::vec3 pos = glm::vec3(0.0f), glm::vec3 faceDirection = glm::vec3(0.0f), glm::vec3 offset = glm::vec3(0.0f)) override
	{
		(*renderables["particleBody"]).second.transformMatrix = glm::translate(glm::mat4(1.0f), pos);
	}


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

struct MutableTrailModel : Model
{
	using Model::renderables;

	void update(glm::vec3 pos = glm::vec3(0.0f), glm::vec3 faceDirection = glm::vec3(0.0f), glm::vec3 offset = glm::vec3(0.0f)) override
	{
		(*renderables["trail"]).second.transformMatrix = glm::translate(glm::mat4(1.0f), pos);
	}

	MutableTrailModel(glm::vec3 _pos) : Model(_pos)
	{
		renderableInfos =
		{
			RenderableInfo{
				"trail",
				"line",
				"line",
				glm::translate(glm::mat4(1.0f), pos)
			}
		};
	}
};

struct AxesModel : Model
{
	using Model::renderables;

	void update(glm::vec3 pos = glm::vec3(0.0f), glm::vec3 faceDirection = glm::vec3(0.0f), glm::vec3 offset = glm::vec3(0.0f)) override
	{
		(*renderables["xAxis"]).second.transformMatrix = glm::translate(glm::mat4(1.0f), pos);
		(*renderables["yAxis"]).second.transformMatrix = glm::translate(glm::mat4(1.0f), pos);
		(*renderables["zAxis"]).second.transformMatrix = glm::translate(glm::mat4(1.0f), pos);
	}

	AxesModel(glm::vec3 _pos) : Model(_pos)
	{
		renderableInfos =
		{
			RenderableInfo{
				"xAxis",
				"xAxis",
				"line",
				glm::translate(glm::mat4(1.0f), pos)
			},
			RenderableInfo{
				"yAxis",
				"yAxis",
				"line",
				glm::translate(glm::mat4(1.0f), pos)
			},
			RenderableInfo{
				"zAxis",
				"zAxis",
				"line",
				glm::translate(glm::mat4(1.0f), pos)
			}
		};
	}
};