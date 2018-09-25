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

#include <random>

#include <routes.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <ctime>
#include <memory>
#include <list>
#include <random>

class ParticleSystem
{
	enum class ClassifyPoint
	{
		FRONT,
		BACK,
		ONPLANE
	};
	enum class CollisionResult
	{
		BOUNCE,
		STICK,
		RECYCLE
	};
public:
	struct ParticleVertex
	{
		glm::vec3 pos;
		glm::vec3 color;
	};
	struct Particle
	{
		glm::vec3 pos;
		glm::vec3 velocity;
		glm::vec3 color;
		float startTime;
	};
	struct Plane
	{
		glm::vec3 normal;
		glm::vec3 point;
		float bounceFactor;
		CollisionResult collisionResult;
	};
public:
	ParticleSystem(void)
	{
		rng.seed(static_cast<std::mt19937::result_type>(clock()));
	}
	void setCollisionPlane(const glm::vec3& planeNormal, const glm::vec3& point,
		float bounceFactor = 1, CollisionResult collisionResult = CollisionResult::BOUNCE )
	{
		Plane plane;
		plane.normal = planeNormal;
		plane.point = point;
		plane.bounceFactor = bounceFactor;
		plane.collisionResult = collisionResult;
		planes.push_back(plane);
	}

	void setMaxParticles(uint32_t maxParticles) { this->maxParticles = maxParticles; }
	void setNumToRelease(uint32_t numToRelease) { this->numToRelease = numToRelease; }
	void setReleaseInterval(float releaseInterval) { this->releaseInterval = releaseInterval; }
	void setLifeCycle(float lifeCycle) { this->lifeCycle = lifeCycle; }
	void setPosition(const glm::vec3& position) { this->position = position; }
	void setVelocity(const glm::vec3& velocity) { this->velocity = velocity; }
	void setGravity(const glm::vec3& gravity) { this->gravity = gravity; }
	void setWind(const glm::vec3& wind) { this->wind = wind; }
	void setAirResistence(bool airResistence) { this->airResistence = airResistence; }
	void setVelocityScale(float scale) { this->velocityScale = scale; }

	void initialize(std::shared_ptr<pompeii::Device> device)
	{
		vertexBuffer = device->createVertexBuffer(maxParticles * sizeof(ParticleVertex));
		drawParams.reset(new pompeii::IndirectBuffer(device));
	}

	void update(float dt)
	{
		currentTime += dt;

		std::cout << "dt " << dt << std::endl;

		for (auto it = activeList.begin(); it != activeList.end();)
		{
			Particle& particle = *it;
			// Calculate new position
			float timePassed = currentTime - particle.startTime;
			if (timePassed >= lifeCycle)
			{   // Move particle to free list
				freeList.splice(freeList.end(), activeList, it);
				it = activeList.end();
			}
			else
			{   // Update velocity with respect to Gravity (Constant Accelaration)
				particle.velocity += gravity * dt;
				// Update velocity with respect to Wind (Accelaration based on difference of vectors)
				if (airResistence)
					particle.velocity += (wind - particle.velocity) * dt;
				// Finally, update position with respect to velocity
				const glm::vec3 oldPosition = particle.pos;
				particle.pos += particle.velocity * dt;
				// Checking the particle against each plane that was set up
				for (auto& plane : planes)
				{
					ClassifyPoint result = classifyPoint(particle.pos, plane);
					if (result == ClassifyPoint::BACK)
					{
						if (plane.collisionResult == CollisionResult::BOUNCE)
						{
							particle.pos = oldPosition;
							const float Kr = plane.bounceFactor;
							const glm::vec3 vn = glm::dot(plane.normal, particle.velocity) * plane.normal;
							const glm::vec3 vt = particle.velocity - vn;
							const glm::vec3 vp = vt - Kr * vn;
							particle.velocity = vp;
						}
						else if (plane.collisionResult == CollisionResult::RECYCLE)
						{
							particle.startTime -= lifeCycle;
						}
						else if (plane.collisionResult == CollisionResult::STICK)
						{
							particle.pos = oldPosition;
							particle.velocity = glm::vec3(0.0f);
						}
					}
				}
				++it;
			}
		}

		if (currentTime - lastUpdate > releaseInterval)
		{
			// Reset update timing...
			lastUpdate = currentTime;
			// Emit new particles at specified flow rate...
			for (uint32_t i = 0; i < numToRelease; ++i)
			{   // Do we have any free particles to put back to work?
				if (!freeList.empty())
				{   // If so, hand over the first free one to be reused.
					activeList.splice(activeList.end(), freeList, std::prev(freeList.end()));
				}
				else if (activeList.size() < maxParticles)
				{   // There are no free particles to recycle...
					// We'll have to create a new one from scratch!
					activeList.push_back(Particle());
				}
				if (activeList.size() < maxParticles)
				{   // Set the attributes for our new particle...
					Particle& particle = activeList.back();
					particle.velocity = velocity;
					if (velocityScale != 0.f)
					{
						const glm::vec3 randomVec = randomVector();
						particle.velocity += randomVec * velocityScale;
					}
					particle.startTime = currentTime;
					particle.pos = position;
					particle.color = randomColor();
				}
			}
		}

		// Update vertex buffer
		if (!activeList.empty())
		{
			if (ParticleVertex *pv = (ParticleVertex *)vertexBuffer->map(0, VK_WHOLE_SIZE))
			{
				for (const auto& particle : activeList)
				{
					pv->pos = particle.pos;
					//particle.pos = pv->pos;
					pv->color = particle.color;
					++pv;
				}
				vertexBuffer->unmap();
			}
			drawParams->writeDrawCommand(static_cast<uint32_t>(activeList.size()), 0);
		}
	}

	void reset()
	{
		freeList.splice(freeList.end(), activeList);
	}

	void draw(std::shared_ptr<pompeii::CommandBuffer> cmdBuffer)
	{
		cmdBuffer->bindVertexBuffer(0, vertexBuffer);
		cmdBuffer->drawIndirect(drawParams, 0, 1, 0);
	}

private:
	float randomScalar(float min, float max)
	{
		float f = rng() / (float)rng.max();
		return min + (max - min) * f;
	}

	glm::vec3 randomVector()
	{
		glm::vec3 v;
		v.z = randomScalar(-1.0f, 1.0f);
		float radius = sqrtf(1.f - v.z * v.z); // Get radius of this circle
		float t = randomScalar(-3.14156f, 3.14156f); // Pick a random point on a circle
		v.x = (float)cosf(t) * radius; // Compute matching X and Y for our Z
		v.y = (float)sinf(t) * radius;
		return v;
	}

	glm::vec3 randomColor()
	{
		glm::vec3 color;
		color.x = rng() / (float)rng.max();
		color.y = rng() / (float)rng.max();
		color.z = rng() / (float)rng.max();
		return color;
	}

	inline ClassifyPoint classifyPoint(const glm::vec3& point, const Plane& plane)
	{
		const glm::vec3 direction = plane.point - point;
		const float dp = glm::dot( direction, plane.normal );
		if (dp < -0.001f)
			return ClassifyPoint::FRONT;
		if (dp > 0.001f)
			return ClassifyPoint::BACK;
		return ClassifyPoint::ONPLANE;
	}


private:
	std::list<Particle> activeList, freeList;
	std::list<Plane> planes;
	std::mt19937 rng;
	float currentTime = 0.f;
	float lastUpdate = 0.f;

	uint32_t maxParticles = 1;
	uint32_t numToRelease = 1;
	float releaseInterval = 1.f;
	float lifeCycle = 1.f;

	glm::vec3 position, velocity, gravity, wind;
	bool airResistence = true;
	float velocityScale = 1.f;

	std::shared_ptr<pompeii::VertexBuffer> vertexBuffer;
	std::shared_ptr<pompeii::IndirectBuffer> drawParams;
};

class MainWindowRenderer : public glfw::VulkanWindowRenderer
{
private:
  glfw::VulkanWindow* _window;
public:
  MainWindowRenderer( glfw::VulkanWindow* window )
    : _window( window )
  {
  }

  struct PushConstants
  {
	  float width;
	  float height;
	  float h;
	  float pointSize;
  };

  struct
  {
    glm::mat4 modelViewProjection;
  } ubo;

  const float fov = glm::radians(60.f);

  std::unique_ptr<ParticleSystem> particles;
  std::shared_ptr<pompeii::UniformBuffer> uniformBuffer;
  std::shared_ptr<pompeii::DescriptorPool> descriptorPool;
  std::shared_ptr<pompeii::DescriptorSetLayout> descriptorSetLayout;
  std::shared_ptr<pompeii::DescriptorSet> descriptorSet;
  std::shared_ptr<pompeii::PipelineLayout> pipelineLayout;
  std::shared_ptr<pompeii::/*Graphics*/Pipeline> pipeline;

  void initParticleSystem(void)
  {
	  auto device = _window->device();

	  particles.reset(new ParticleSystem());
	  particles->setMaxParticles(200);
	  particles->setNumToRelease(10);
	  particles->setReleaseInterval(0.05f);
	  particles->setLifeCycle(5.0f);
	  particles->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
	  particles->setVelocity(glm::vec3(0.0f, 0.0f, 0.0f));
	  particles->setGravity(glm::vec3(0.0f, -9.8f, 0.0f));
	  particles->setWind(glm::vec3(0.0f, 0.0f, 0.0f));
	  particles->setVelocityScale(20.0f);
	  particles->setCollisionPlane(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	  particles->initialize(device);
  }

  void setupView(void)
  {
	  vk::Extent2D extent = _window->swapchainImageSize();

	  const glm::vec3 eye(0.f, 3.f, 30.f);
	  const glm::vec3 center(0.f, 0.f, 0.f);
	  const glm::vec3 up(0.f, 1.f, 0.f);
	  const float aspect = extent.width / (float)extent.height;
	  const float zn = 1.f, zf = 100.f;
	  const glm::mat4 view = glm::lookAtRH(eye, center, up);
	  const glm::mat4 proj = glm::perspectiveRH(fov, aspect, zn, zf);
	  ubo.modelViewProjection = view * proj;
  }

  void createUniformBuffer(void)
  {
	  auto device = _window->device();

	  uniformBuffer = device->createUniformBuffer(sizeof(ubo));
  }

  void setupDescriptorSet(void)
  {
	  auto device = _window->device();

	  descriptorPool = device->createDescriptorPool(1, {
		  descriptors::UniformBuffer(1)
	  });

	  std::vector<DescriptorSetLayoutBinding> dslbs =
	  {
		  DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer,
			vk::ShaderStageFlagBits::eVertex
		  )
	  };

	  descriptorSetLayout = device->createDescriptorSetLayout(dslbs);

	  const vk::PushConstantRange pushConstantRange(
		  vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
		  0, sizeof(PushConstants));

	  pipelineLayout = device->createPipelineLayout(
		  descriptorSetLayout, pushConstantRange);

	  auto descriptorPool = device->createDescriptorPool(1, {
		  vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, 2)
	  });

	  descriptorSet = device->allocateDescriptorSet(
		  descriptorPool, descriptorSetLayout);

	  std::vector<WriteDescriptorSet> wdss =
	  {
		  WriteDescriptorSet(descriptorSet, 0, 0,
		  vk::DescriptorType::eUniformBuffer, 1, nullptr,
			  DescriptorBufferInfo(
				  uniformBuffer, 0, sizeof(ubo)
			  )
		  )
	  };

	  device->updateDescriptorSets(wdss, {});
  }

  void setupPipeline(void)
  {
	  auto device = _window->device();

	  auto vertexStage = device->createShaderPipelineShaderStage(
		  POMPEII_EXAMPLES_SPV_ROUTE + std::string("particles2_vert.spv"),
		  vk::ShaderStageFlagBits::eVertex
	  );
	  auto fragmentStage = device->createShaderPipelineShaderStage(
		  POMPEII_EXAMPLES_SPV_ROUTE + std::string("particles2_frag.spv"),
		  vk::ShaderStageFlagBits::eFragment
	  );

	  PipelineVertexInputStateCreateInfo vertexInput({
		  vk::VertexInputBindingDescription(0,
		  sizeof(ParticleSystem::ParticleVertex), vk::VertexInputRate::eVertex
		  )
	  }, {
		  // Position attribute
		  vk::VertexInputAttributeDescription(0, 0,
		  vk::Format::eR32G32B32Sfloat, offsetof(ParticleSystem::ParticleVertex, pos)
		  ),
		  // Color attribute
		  vk::VertexInputAttributeDescription(1, 0,
			  vk::Format::eR32G32B32Sfloat, offsetof(ParticleSystem::ParticleVertex, color)
		  )
	  });
	  vk::PipelineInputAssemblyStateCreateInfo assembly({},
		  vk::PrimitiveTopology::ePointList, VK_FALSE);
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
	  vk::PipelineColorBlendAttachmentState colorBlendAttachment(true,
		  vk::BlendFactor::eSrcAlpha, vk::BlendFactor::eOneMinusSrcAlpha, vk::BlendOp::eAdd,
		  vk::BlendFactor::eOne, vk::BlendFactor::eOneMinusSrcAlpha, vk::BlendOp::eAdd,
		  vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
		  vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
	  );
	  PipelineColorBlendStateCreateInfo colorBlend(false, vk::LogicOp::eNoOp,
		  colorBlendAttachment, { 1.0f, 1.0f, 1.0f, 1.0f }
	  );
	  PipelineDynamicStateCreateInfo dynamic({
		  vk::DynamicState::eViewport, vk::DynamicState::eScissor
	  });

	  pipeline = device->createGraphicsPipeline(_window->pipelineCache(), {},
	  { vertexStage, fragmentStage }, vertexInput, assembly, nullptr,
		  viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
		  pipelineLayout, _window->renderPass()
	  );
  }

  virtual void initResources( void ) override
  {
	initParticleSystem();
	setupView();
	createUniformBuffer();
	setupDescriptorSet();
	setupPipeline();
  }

  float lastTime = 0.0f;

  virtual void nextFrame( void )
  {
	  static auto startTime = std::chrono::high_resolution_clock::now();

	  auto currentTime = std::chrono::high_resolution_clock::now();
	  
	  float time = std::chrono::duration_cast<std::chrono::milliseconds>(
		  currentTime - startTime).count() / 1000.0f;
	  float dt = time - lastTime;
	  particles->update(dt);

	  lastTime = time;

    auto cmd = _window->currentCommandBuffer( );

	vk::Extent2D extent = _window->swapchainImageSize();

    std::array<vk::ClearValue, 2 > clearValues;
    clearValues[ 0 ] = pompeii::utils::getClearValueColor( 0.0f, 0.0f, 0.0f );
    clearValues[ 1 ] = pompeii::utils::getClearValueDepth( );

    cmd->beginRenderPass( _window->renderPass( ),
      _window->framebuffer( ),
      vk::Rect2D( { 0, 0 }, extent ), clearValues,
      vk::SubpassContents::eInline );

    cmd->bindGraphicsPipeline( pipeline );
    cmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics, pipelineLayout,
      0, { descriptorSet }, { } );

	PushConstants pc;
	pc.width = extent.width;
	pc.height = extent.height;
	pc.h = (float)extent.height / (2.0f * std::tanf(fov * 0.5f)); // Scale with distance
	pc.pointSize = 0.5f;

    cmd->setViewportScissors( extent );
	cmd->pushConstants<PushConstants>(pipelineLayout, 
		vk::ShaderStageFlagBits::eVertex | 
		vk::ShaderStageFlagBits::eFragment, 0, pc);

	particles->draw(cmd);

    cmd->endRenderPass( );

    _window->frameReady( );
  }
};

class VulkanWindow : public glfw::VulkanWindow
{
public:
  explicit VulkanWindow( int width, int height,
    const std::string& title, bool enableLayers )
    : glfw::VulkanWindow( width, height, title, enableLayers )
  {
  }
  virtual glfw::VulkanWindowRenderer* createRenderer( void ) override
  {
    return new MainWindowRenderer( this );
  }
};


int main( int, char** )
{
  VulkanWindow app( 500, 500, "GLFWRenderer", true );
  app.show( );
  return EXIT_SUCCESS;
}