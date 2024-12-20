#pragma once

#include "Models.h"
#include "Renderer.h"

class Node
{
public:
	static void setRenderer(Renderer* renderer)
	{
		rendererHandle = renderer;
	}

	void cleanup()
	{
		for (auto& [name, model] : m_models)
		{
			model.cleanup(rendererHandle);
		}
	}

	void uploadModel(const std::string& name, Model&& model) 
	{
		m_models.emplace(name, std::move(model));
		rendererHandle->addRenderables(m_models[name]);
	}

protected:
	std::map<std::string, Model> m_models;
private:
	inline static Renderer* rendererHandle = nullptr;
};

