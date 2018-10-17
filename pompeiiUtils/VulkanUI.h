#pragma once

#include <pompeii/pompeii.h>

#include <glm/glm.hpp>

#include "../thirdparty/imgui/imgui.h"

#include <pompeiiUtils/api.h>

namespace pompeii
{
  namespace utils
  {
    struct UIOverlayCreateInfo
    {
      std::shared_ptr<Device> device;
      std::shared_ptr<Queue> copyQueue;
      std::shared_ptr<RenderPass> renderPass;
      std::vector<std::shared_ptr<Framebuffer>> framebuffers;
      vk::Format colorformat;
      vk::Format depthformat;
      uint32_t width;
      uint32_t height;
      std::vector<PipelineShaderStageCreateInfo> shaders;
      vk::SampleCountFlagBits rasterizationSamples = vk::SampleCountFlagBits::e1;
      uint32_t subpassCount = 1;
      std::vector<vk::ClearValue> clearValues = { };
      uint32_t attachmentCount = 1;
    };
    class UIOverlay
    {
    private:
      std::shared_ptr<Buffer> vertexBuffer, indexBuffer;
      uint32_t vertexCount = 0, indexCount = 0;

      std::shared_ptr<DescriptorPool> descriptorPool;
      std::shared_ptr<DescriptorSetLayout> descriptorSetLayout;
      std::shared_ptr<DescriptorSet> descriptorSet;
      std::shared_ptr<PipelineLayout> pipelineLayout;
      std::shared_ptr<PipelineCache> pipelineCache;
      std::shared_ptr<Pipeline> pipeline;
      std::shared_ptr<RenderPass> renderPass;
      std::shared_ptr<CommandPool> commandPool;
      std::shared_ptr<Fence> fence;

      std::shared_ptr<Image> fontImage = nullptr;
      std::shared_ptr<ImageView> fontView = nullptr;
      std::shared_ptr<Sampler> sampler;

      struct PushConstantBlock
      {
        glm::vec2 scale;
        glm::vec2 translate;
      } pushConstBlock;

      UIOverlayCreateInfo createInfo = { };

      void prepareResources( void );
      void preparePipeline( void );
      void prepareRenderPass( void );
      void updateCommandBuffers( void );
    public:
      bool visible = true;
      float scale = 1.0f;

      std::vector<std::shared_ptr<CommandBuffer>> cmdBuffers;

	  POMPEIIUTILS_API
      UIOverlay( UIOverlayCreateInfo createInfo );
	  POMPEIIUTILS_API
      ~UIOverlay( void );

	  POMPEIIUTILS_API
      void update( void );
	  POMPEIIUTILS_API
      void resize( uint32_t width, uint32_t height, 
        std::vector < std::shared_ptr< Framebuffer > > framebuffers );

	  POMPEIIUTILS_API
      void submit( std::shared_ptr<Queue> queue, uint32_t bufferindex, 
        SubmitInfo submitInfo );

	  POMPEIIUTILS_API
      bool header( const char* caption );
	  POMPEIIUTILS_API
      bool checkBox( const char* caption, bool* value );
	  POMPEIIUTILS_API
      bool checkBox( const char* caption, int32_t* value );
	  POMPEIIUTILS_API
      bool inputFloat( const char* caption, float* value, float step,
        uint32_t precision );
	  POMPEIIUTILS_API
      bool sliderFloat( const char* caption, float* value, float min, 
        float max );
	  POMPEIIUTILS_API
      bool sliderInt( const char* caption, int32_t* value, int32_t min, 
        int32_t max );
	  POMPEIIUTILS_API
      bool comboBox( const char* caption, int32_t* itemindex, 
        const std::vector<std::string>& items );
	  POMPEIIUTILS_API
      bool button( const char* caption );
	  POMPEIIUTILS_API
      void text( const char* formatstr, ... );
    };
  }
}