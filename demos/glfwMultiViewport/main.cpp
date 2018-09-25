/**
* Copyright (c) 2017 - 2018, Pompeii
* All rights reserved.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
**/

#include <iostream>

#include <glfwPompeii/glfwPompeii.h>
using namespace pompeii;

#include <routes.h>
#include <glm/gtc/matrix_transform.hpp>

class MainWindowRenderer : public glfw::VulkanWindowRenderer
{
private:
	glfw::VulkanWindow* _window;
public:
	MainWindowRenderer(glfw::VulkanWindow* window)
		: _window(window)
	{
	}

	std::array<vk::Rect2D, 4> out_scissors;
	std::array<vk::Viewport, 4> out_viewports;

	struct Vertex
	{
		glm::vec3 pos;
		glm::vec2 uv;
	};

	const std::vector<Vertex> vertices =
	{
		{ { -0.5f, -0.5f,  0.5f },{ 0.0f, 0.0f } },
		{ { 0.5f, -0.5f,  0.5f },{ 1.0f, 0.0f } },
		{ { -0.5f,  0.5f,  0.5f },{ 0.0f, 1.0f } },
		{ { 0.5f,  0.5f,  0.5f },{ 1.0f, 1.0f } },

		{ { -0.5f, -0.5f, -0.5f },{ 0.0f, 0.0f } },
		{ { 0.5f, -0.5f, -0.5f },{ 1.0f, 0.0f } },
		{ { -0.5f,  0.5f, -0.5f },{ 0.0f, 1.0f } },
		{ { 0.5f,  0.5f, -0.5f },{ 1.0f, 1.0f } },

		{ { 0.5f, -0.5f, -0.5f },{ 0.0f, 0.0f } },
		{ { 0.5f, -0.5f,  0.5f },{ 1.0f, 0.0f } },
		{ { 0.5f,  0.5f, -0.5f },{ 0.0f, 1.0f } },
		{ { 0.5f,  0.5f,  0.5f },{ 1.0f, 1.0f } },

		{ { -0.5f, -0.5f, -0.5f },{ 0.0f, 0.0f } },
		{ { -0.5f, -0.5f,  0.5f },{ 1.0f, 0.0f } },
		{ { -0.5f,  0.5f, -0.5f },{ 0.0f, 1.0f } },
		{ { -0.5f,  0.5f,  0.5f },{ 1.0f, 1.0f } },

		{ { -0.5f,  0.5f, -0.5f },{ 0.0f, 0.0f } },
		{ { -0.5f,  0.5f,  0.5f },{ 1.0f, 0.0f } },
		{ { 0.5f,  0.5f, -0.5f },{ 0.0f, 1.0f } },
		{ { 0.5f,  0.5f,  0.5f },{ 1.0f, 1.0f } },

		{ { -0.5f, -0.5f, -0.5f },{ 0.0f, 0.0f } },
		{ { -0.5f, -0.5f,  0.5f },{ 1.0f, 0.0f } },
		{ { 0.5f, -0.5f, -0.5f },{ 0.0f, 1.0f } },
		{ { 0.5f, -0.5f,  0.5f },{ 1.0f, 1.0f } }
	};
	const std::vector<uint16_t> indices =
	{
		0,  1,  2,     1,  3,  2,
		4,  6,  5,     5,  6,  7,
		8, 10,  9,     9, 10, 11,
		12, 13, 14,    13, 15, 14,
		16, 17, 18,    17, 19, 18,
		20, 22, 21,    21, 22, 23,
	};

	virtual void initSwapChainResources(void) override
	{
		const auto size = _window->swapchainImageSize();

		uint32_t WINDOW_W = size.width;
		uint32_t WINDOW_H = size.height;

		{
			/* Top-left region */
			out_scissors[0].extent.height = WINDOW_H * 0.5f;
			out_scissors[0].extent.width = WINDOW_W * 0.5f;
			out_scissors[0].offset.x = 0;
			out_scissors[0].offset.y = 0;

			/* Top-right region */
			out_scissors[1] = out_scissors[0];
			out_scissors[1].offset.x = WINDOW_W * 0.5f;

			/* Bottom-left region */
			out_scissors[2] = out_scissors[0];
			out_scissors[2].offset.y = WINDOW_H * 0.5f;

			/* Bottom-right region */
			out_scissors[3] = out_scissors[2];
			out_scissors[3].offset.x = WINDOW_W * 0.5f;
		}

		{
			/* Top-left region */
			out_viewports[0].height = float(WINDOW_H * 0.5f);
			out_viewports[0].width = float(WINDOW_W * 0.5f);
			out_viewports[0].x = 0.0f;
			out_viewports[0].y = 0.0f;
			out_viewports[0].minDepth = 0.0f;
			out_viewports[0].maxDepth = 1.0f;

			/* Top-right region */
			out_viewports[1] = out_viewports[0];
			out_viewports[1].x = float(WINDOW_W * 0.5f);

			/* Bottom-left region */
			out_viewports[2] = out_viewports[0];
			out_viewports[2].y = float(WINDOW_H * 0.5f);

			/* Bottom-right region */
			out_viewports[3] = out_viewports[2];
			out_viewports[3].x = float(WINDOW_W * 0.5f);
		}
	}

	struct
	{
		glm::mat4 proj;
		glm::mat4 view;
		glm::mat4 model;
	} uboVS;

	virtual void initResources(void)
	{
		auto device = _window->device();

		auto cmd = _window->gfxCommandPool()->allocateCommandBuffer();
		cmd->begin();
		// Vertex buffer
		{
			uint32_t vertexBufferSize = vertices.size() * sizeof(Vertex);

			vertexBuffer = device->createBuffer(vertexBufferSize,
				vk::BufferUsageFlagBits::eVertexBuffer |
				vk::BufferUsageFlagBits::eTransferDst,
				vk::MemoryPropertyFlagBits::eDeviceLocal);
			vertexBuffer->update<Vertex>(cmd, 0, { uint32_t(vertices.size()),
				vertices.data() });
		}

		// Index buffer
		{
			uint32_t indexBufferSize = indices.size() * sizeof(uint32_t);

			indexBuffer = device->createBuffer(indexBufferSize,
				vk::BufferUsageFlagBits::eIndexBuffer |
				vk::BufferUsageFlagBits::eTransferDst,
				vk::MemoryPropertyFlagBits::eDeviceLocal);
			indexBuffer->update<uint16_t>(cmd, 0, { uint32_t(indices.size()),
				indices.data() });
		}
		cmd->end();
		_window->gfxQueue()->submitAndWait(cmd);

		mvpBuffer = device->createUniformBuffer(sizeof(uboVS));

		std::vector<std::string> images =
		{
			POMPEII_EXAMPLES_IMAGES_ROUTE + std::string("/matcap/Jade_Purple.png"),
			POMPEII_EXAMPLES_IMAGES_ROUTE + std::string("/matcap/silver.jpg"),
			POMPEII_EXAMPLES_IMAGES_ROUTE + std::string("/matcap/Outline.png"),
			POMPEII_EXAMPLES_IMAGES_ROUTE + std::string("/matcap/normals.jpg")
		};

		tex = device->createTexture2DArray(images,
			_window->gfxCommandPool(), _window->gfxQueue(),
			vk::Format::eR8G8B8A8Unorm);

		auto vertexStage = device->createShaderPipelineShaderStage(
			POMPEII_EXAMPLES_SPV_ROUTE + std::string("multiview2DArray_vert.spv"),
			vk::ShaderStageFlagBits::eVertex
		);
		auto geomStage = device->createShaderPipelineShaderStage(
			POMPEII_EXAMPLES_SPV_ROUTE + std::string("multiview2DArray_geom.spv"),
			vk::ShaderStageFlagBits::eGeometry
		);
		auto fragmentStage = device->createShaderPipelineShaderStage(
			POMPEII_EXAMPLES_SPV_ROUTE + std::string("multiview2DArray_frag.spv"),
			vk::ShaderStageFlagBits::eFragment
		);

		std::vector<DescriptorSetLayoutBinding> dslbs =
		{
			DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer,
			vk::ShaderStageFlagBits::eGeometry
			),
			DescriptorSetLayoutBinding(1, vk::DescriptorType::eCombinedImageSampler,
				vk::ShaderStageFlagBits::eFragment
			)
		};

		descriptorSetLayout = device->createDescriptorSetLayout(dslbs);

		pipelineLayout = device->createPipelineLayout(descriptorSetLayout, nullptr);

		PipelineVertexInputStateCreateInfo vertexInput(
			vk::VertexInputBindingDescription(0, sizeof(Vertex),
				vk::VertexInputRate::eVertex),
				{
					vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat,
					offsetof(Vertex, pos)
					),
			vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32Sfloat,
				offsetof(Vertex, uv)
			)
				}
		);

		vk::PipelineInputAssemblyStateCreateInfo assembly({},
			vk::PrimitiveTopology::eTriangleList, VK_FALSE);
		PipelineViewportStateCreateInfo viewport(1, 1);
		vk::PipelineRasterizationStateCreateInfo rasterization({}, true,
			false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone,
			vk::FrontFace::eCounterClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f);
		PipelineMultisampleStateCreateInfo multisample(vk::SampleCountFlagBits::e1,
			false, 0.0f, nullptr, false, false);
		vk::StencilOpState stencilOpState(vk::StencilOp::eKeep,
			vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::CompareOp::eAlways,
			0, 0, 0);
		vk::PipelineDepthStencilStateCreateInfo depthStencil({}, true, true,
			vk::CompareOp::eLessOrEqual, false, false, stencilOpState,
			stencilOpState, 0.0f, 0.0f);
		PipelineColorBlendStateCreateInfo colorBlend(false, vk::LogicOp::eNoOp,
			vk::PipelineColorBlendAttachmentState(
				false, vk::BlendFactor::eZero, vk::BlendFactor::eZero,
				vk::BlendOp::eAdd, vk::BlendFactor::eZero, vk::BlendFactor::eZero,
				vk::BlendOp::eAdd, vk::ColorComponentFlagBits::eR |
				vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
				vk::ColorComponentFlagBits::eA
			), { 1.0f, 1.0f, 1.0f, 1.0f }
		);
		PipelineDynamicStateCreateInfo dynamic({
			vk::DynamicState::eViewport, vk::DynamicState::eScissor
		});

		pipeline = device->createGraphicsPipeline(_window->pipelineCache(), {},
		{ vertexStage, geomStage, fragmentStage }, vertexInput, assembly, nullptr,
			viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
			pipelineLayout, _window->renderPass()
		);

		std::array<vk::DescriptorPoolSize, 2> poolSize =
		{
			vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, 1),
			vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, 1)
		};
		auto dspPool = device->createDescriptorPool(1, poolSize);

		// Init descriptor set
		descriptorSet = device->allocateDescriptorSet(dspPool, descriptorSetLayout);

		std::vector<WriteDescriptorSet> wdss =
		{
			WriteDescriptorSet(
				descriptorSet, 0, 0, vk::DescriptorType::eUniformBuffer,
				1, nullptr, DescriptorBufferInfo(mvpBuffer, 0, sizeof(uboVS))
			),
			WriteDescriptorSet(
				descriptorSet, 1, 0, vk::DescriptorType::eCombinedImageSampler, 1,
				tex->descriptor,
				nullptr
			)
		};
		device->updateDescriptorSets(wdss, {});
	}

	void updateMVP(void)
	{
		auto size = _window->swapchainImageSize();

		uint32_t width = size.width, height = size.height;

		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 1000.0f;

		uboVS.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		uboVS.model = glm::rotate(uboVS.model, time * glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		uboVS.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		uboVS.proj = glm::perspective(glm::radians(45.0f), width / (float)height, 0.1f, 10.0f);
		uboVS.proj[1][1] *= -1;

		mvpBuffer->set(&uboVS);
	}

	virtual void nextFrame(void)
	{
		auto cmd = _window->currentCommandBuffer();

		updateMVP();

		std::array<vk::ClearValue, 2 > clearValues;
		clearValues[0] = pompeii::utils::getClearValueColor(0.0f, 0.0f, 0.0f);
		clearValues[1] = pompeii::utils::getClearValueDepth();

		vk::Extent2D extent = _window->swapchainImageSize();

		cmd->beginRenderPass(_window->renderPass(),
			_window->framebuffer(),
			vk::Rect2D({ 0, 0 }, extent), clearValues,
			vk::SubpassContents::eInline);

		cmd->setScissor(0, out_scissors);
		cmd->setViewport(0, out_viewports);

		//cmd->setViewportScissors(extent);

		cmd->bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
			pipelineLayout, 0, descriptorSet, nullptr);
		cmd->bindGraphicsPipeline(pipeline);
		cmd->bindVertexBuffer(0, vertexBuffer, 0);
		cmd->bindIndexBuffer(indexBuffer, 0, vk::IndexType::eUint16);
		cmd->drawIndexed(indices.size( ));

		cmd->endRenderPass();

		_window->frameReady();
	}

	std::shared_ptr< pompeii::Texture2DArray > tex;
	std::shared_ptr< pompeii::UniformBuffer > mvpBuffer;
	std::shared_ptr< pompeii::Buffer > vertexBuffer;
	std::shared_ptr< pompeii::Buffer > indexBuffer;
	std::shared_ptr< pompeii::DescriptorSet > descriptorSet;
	std::shared_ptr< pompeii::DescriptorSetLayout > descriptorSetLayout;
	std::shared_ptr< pompeii::Pipeline > pipeline;
	std::shared_ptr< pompeii::PipelineLayout > pipelineLayout;
};

class VulkanWindow : public glfw::VulkanWindow
{
public:
	explicit VulkanWindow(int width, int height,
		const std::string& title, bool enableLayers)
		: glfw::VulkanWindow(width, height, title, enableLayers)
	{
	}
	// Enable physical device features required for this example				
	virtual void getEnabledFeatures( void ) override
	{
		// Multiple viewports must be supported
		if ( deviceFeatures.multiViewport )
		{
			enabledFeatures.multiViewport = VK_TRUE;
		}
		else
		{
			throw std::runtime_error( 
				"Selected GPU doesn't support multi viewport!" );
		}
	}
	virtual glfw::VulkanWindowRenderer* createRenderer(void) override
	{
		return new MainWindowRenderer(this);
	}
};


int main(int, char**)
{
	VulkanWindow app(500, 500, "GLFWMultiViewport", true);
	app.show();
	return EXIT_SUCCESS;
}