#pragma once

#include "Models.h"
#include "Renderer.h"

class Node
{
public:
	Node(const Node&) = delete;
	Node& operator=(const Node&) = delete;
	Node(Node&&) = default;
	Node& operator=(Node&&) = default;
	Node() = default;

	static void setRenderer(Renderer* renderer)
	{
		rendererHandle = renderer;
	}

	void cleanup()
	{
		for (auto& [name, model] : m_models)
		{
			model->cleanup(rendererHandle);
		}
	}

    void uploadModel(const std::string& name, std::unique_ptr<Model> model) 
    {
		rendererHandle->addRenderables(model.get());
		m_models.insert({ name, std::move(model) });
    }
	
	Model* getModel(const std::string& name)
	{
		return m_models[name].get();
	}

protected:
	std::map<std::string, std::unique_ptr<Model>> m_models;
private:
	inline static Renderer* rendererHandle = nullptr;
};

