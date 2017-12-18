/**
 * Copyright (c) 2017, Lava
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

#include <lava/lava.h>
using namespace lava;

#include <routes.h>

class MyApp : public VulkanApp
{
public:
  std::shared_ptr<Pipeline> pipeline;
  std::shared_ptr<PipelineLayout> pipelineLayout;
  std::shared_ptr<DescriptorSet> descriptorSet;
  std::shared_ptr<Texture2D> tex;
  std::shared_ptr<CommandPool> commandPool;

  vk::Sampler sampler1, sampler2, sampler3, sampler4;

  MyApp(char const* title, uint32_t width, uint32_t height)
    : VulkanApp( title, width, height )
  {
    commandPool = _device->createCommandPool(
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _queueFamilyIndex );
    tex = std::make_shared<Texture2D>( _device, LAVA_EXAMPLES_IMAGES_ROUTE +
      std::string( "sample.png" ), commandPool, _graphicsQueue,
      vk::Format::eR8G8B8A8Unorm );

    // init descriptor and pipeline layouts
    std::vector<DescriptorSetLayoutBinding> dslbs = 
    {
      DescriptorSetLayoutBinding( 0, 
        vk::DescriptorType::eSampledImage, 
        vk::ShaderStageFlagBits::eFragment
      ),
      DescriptorSetLayoutBinding( 1, 
        vk::DescriptorType::eSampler, 
        vk::ShaderStageFlagBits::eFragment
      )
    };
    std::shared_ptr<DescriptorSetLayout> descriptorSetLayout = 
      _device->createDescriptorSetLayout( dslbs );

    pipelineLayout = _device->createPipelineLayout( 
      descriptorSetLayout, nullptr );


    std::shared_ptr<DescriptorPool> descriptorPool =
      _device->createDescriptorPool( { }, 1, { 
        { vk::DescriptorType::eSampledImage, 1 },
        { vk::DescriptorType::eSampler, 1 },
      } );

    // Init descriptor set
    descriptorSet = _device->allocateDescriptorSet( 
      descriptorPool, descriptorSetLayout );

    vk::Device device = static_cast< vk::Device >( *_device );
   
    {
      vk::SamplerCreateInfo sci;
      sci.setMagFilter( vk::Filter::eLinear );
      sci.setMinFilter( vk::Filter::eLinear );
      sci.setMipmapMode( vk::SamplerMipmapMode::eLinear );
      sci.setAddressModeU( vk::SamplerAddressMode::eRepeat );
      sci.setAddressModeV( vk::SamplerAddressMode::eRepeat );
      sci.setAddressModeW( vk::SamplerAddressMode::eRepeat );
      sci.setMipLodBias( 0.0f );
      sci.setCompareOp( vk::CompareOp::eNever );
      sci.setMinLod( 0.0f );
      sci.setMaxLod( /*useStaging ? mipLevels : 0.0f*/0.0f );
      sci.setMaxAnisotropy( 1.0f );
      sci.setAnisotropyEnable( VK_TRUE );
      sci.setBorderColor( vk::BorderColor::eFloatOpaqueWhite );

      sampler1 = device.createSampler( sci );

      sci.setAddressModeU( vk::SamplerAddressMode::eMirroredRepeat );
      sci.setAddressModeV( vk::SamplerAddressMode::eMirroredRepeat );
      sci.setAddressModeW( vk::SamplerAddressMode::eMirroredRepeat );
      sampler2 = device.createSampler( sci );

      sci.setAddressModeU( vk::SamplerAddressMode::eClampToEdge );
      sci.setAddressModeV( vk::SamplerAddressMode::eClampToEdge );
      sci.setAddressModeW( vk::SamplerAddressMode::eClampToEdge );
      sampler3 = device.createSampler( sci );

      sci.setAddressModeU( vk::SamplerAddressMode::eClampToBorder );
      sci.setAddressModeV( vk::SamplerAddressMode::eClampToBorder );
      sci.setAddressModeW( vk::SamplerAddressMode::eClampToBorder );
      sampler4 = device.createSampler( sci );
    }

    updateSamplerUniform( sampler1 );

    // init pipeline
    PipelineShaderStageCreateInfo vertexStage = _device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "fullquad_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex
    );
    PipelineShaderStageCreateInfo fragmentStage = _device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "fullquadSampler_frag.spv" ),
      vk::ShaderStageFlagBits::eFragment
    );
    PipelineVertexInputStateCreateInfo vertexInput( { }, { } );
    vk::PipelineInputAssemblyStateCreateInfo assembly( { }, 
      vk::PrimitiveTopology::eTriangleStrip, VK_FALSE );
    PipelineViewportStateCreateInfo viewport( 1, 1 );
    vk::PipelineRasterizationStateCreateInfo rasterization( { }, false, false, 
      vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, 
      vk::FrontFace::eClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f );
    PipelineMultisampleStateCreateInfo multisample( 
      vk::SampleCountFlagBits::e1, false, 0.0f, nullptr, false, false );
    vk::StencilOpState stencilOpState( vk::StencilOp::eKeep, 
      vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::CompareOp::eAlways, 
      0, 0, 0 );
    vk::PipelineDepthStencilStateCreateInfo depthStencil( { }, true, true, 
      vk::CompareOp::eLessOrEqual, false, false, stencilOpState, 
      stencilOpState, 0.0f, 0.0f );
    vk::PipelineColorBlendAttachmentState colorBlendAttachment( false, 
      vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd, 
      vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG 
      | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA );
    PipelineColorBlendStateCreateInfo colorBlend( false, vk::LogicOp::eNoOp, 
      colorBlendAttachment, { 1.0f, 1.0f, 1.0f, 1.0f } );
    PipelineDynamicStateCreateInfo dynamic( { vk::DynamicState::eViewport, 
      vk::DynamicState::eScissor } );


    pipeline = _device->createGraphicsPipeline( pipelineCache, { }, 
      { vertexStage, fragmentStage }, vertexInput, assembly, nullptr, 
      viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
      pipelineLayout, _renderPass );
  }
  void doPaint( void ) override
  {
    auto commandBuffer = commandPool->allocateCommandBuffer( );

    commandBuffer->beginSimple( );

    std::array<float, 4> ccv = { 0.2f, 0.3f, 0.3f, 1.0f };
    commandBuffer->beginRenderPass( _renderPass, 
      _defaultFramebuffer->getFramebuffer( ), vk::Rect2D( { 0, 0 }, 
        _defaultFramebuffer->getExtent( ) ),
    { vk::ClearValue( ccv ), vk::ClearValue( 
      vk::ClearDepthStencilValue( 1.0f, 0 ) ) }, vk::SubpassContents::eInline );
    commandBuffer->bindGraphicsPipeline( pipeline );
    commandBuffer->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      pipelineLayout, 0, { descriptorSet }, nullptr );

    commandBuffer->setViewportScissors( _defaultFramebuffer->getExtent( ) );

    commandBuffer->draw( 4, 1, 0, 0 );
    commandBuffer->endRenderPass( );

    commandBuffer->end( );

    _graphicsQueue->submit( SubmitInfo{
      { _defaultFramebuffer->getPresentSemaphore( ) },
      { vk::PipelineStageFlagBits::eColorAttachmentOutput },
      commandBuffer,
      _renderComplete
    } );
  }
  void updateSamplerUniform( vk::Sampler s )
  {
    DescriptorImageInfo descriptor;
    descriptor.imageLayout = tex->imageLayout;
    descriptor.imageView = std::make_shared< vk::ImageView>( tex->view );
    descriptor.sampler = VK_NULL_HANDLE;
    //descriptor.sampler = std::make_shared< vk::Sampler>( sampler );

    DescriptorImageInfo samplerInfo;
    samplerInfo.sampler = std::make_shared< vk::Sampler>( s );

    std::vector<WriteDescriptorSet> wdss =
    {
      WriteDescriptorSet( descriptorSet, 0, 0,
        vk::DescriptorType::eSampledImage, 1,
        descriptor, nullptr
      ),
      WriteDescriptorSet( descriptorSet, 1, 0,
        vk::DescriptorType::eSampler, 1,
        samplerInfo, nullptr
      )
    };

    _device->updateDescriptorSets( wdss, { } );
  }
  void keyEvent(int key, int scancode, int action, int mods)
  {
    switch (key)
    {
    case GLFW_KEY_ESCAPE:
      switch (action)
      {
      case GLFW_PRESS:
        getWindow( )->close( );
        break;
      default:
        break;
      }
      break;
    case GLFW_KEY_1:
      updateSamplerUniform( sampler1 );
      break;
    case GLFW_KEY_2:
      updateSamplerUniform( sampler2 );
      break;
    case GLFW_KEY_3:
      updateSamplerUniform( sampler3 );
      break;
    case GLFW_KEY_4:
      updateSamplerUniform( sampler4 );
      break;
    default:
      break;
    }
  }
};

void glfwErrorCallback(int error, const char* description)
{
  fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

int main( void )
{
  try
  {
    VulkanApp* app = new MyApp( "Sampler As Uniform", 512, 512 );

    app->getWindow( )->setErrorCallback( glfwErrorCallback );

    while ( app->isRunning( ) )
    {
      app->waitEvents( );
      app->paint( );
    }

    delete app;
  }
  catch ( std::system_error err )
  {
    std::cout << "System Error: " << err.what( ) << std::endl;
  }
  return 0;
}