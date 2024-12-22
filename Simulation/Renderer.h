#pragma once

#include <map>
#include <string>

#include "RendererStructs.h"

struct Model;

class Renderer
{
private: 
	inline static constexpr glm::vec3 VK_WORLD_UP = glm::vec3(0.0f, -1.0f, 0.0f);
public:
	inline static constexpr glm::vec3 WORLD_UP() { return VK_WORLD_UP; }

	virtual std::multimap<std::string, Renderable, RenderableComp>::iterator addRenderable(Renderable) = 0;
	virtual void addRenderables(Model*) = 0;
	virtual void removeRenderable(std::multimap<std::string, Renderable, RenderableComp>::iterator&) = 0;
};