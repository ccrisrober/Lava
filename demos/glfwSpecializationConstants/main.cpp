#include <iostream>

#include <glfwPompeii/glfwPompeii.h>
#include <pompeiiUtils/pompeiiUtils.h>
using namespace pompeii;

#include <routes.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class MainWindowRenderer : public glfw::VulkanWindowRenderer
{
private:
  glfw::VulkanWindow* _window;
public:
  MainWindowRenderer( glfw::VulkanWindow* window )
    : _window( window )
  {
  }

  struct
  {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec3 viewPos;
  } uboVS;

  void initResources( void ) override
  {
    auto device = _window->device( );

    geometry = std::make_shared<pompeii::utility::Geometry>( device,
      POMPEII_EXAMPLES_MESHES_ROUTE + std::string( "teapot.ply" ) );

    // MVP buffer
    {
      mvpBuffer = device->createUniformBuffer( sizeof( uboVS ) );
    }

    std::vector<DescriptorSetLayoutBinding> dslbs =
    {
      DescriptorSetLayoutBinding( 0, vk::DescriptorType::eUniformBuffer,
      vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment
      )
    };

    descriptorSetLayout = device->createDescriptorSetLayout( dslbs );

    pipelineLayout = device->createPipelineLayout( descriptorSetLayout, nullptr );

    auto vertexStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "mesh_ctes_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex
    );

    PipelineVertexInputStateCreateInfo vertexInput(
      vk::VertexInputBindingDescription( 0, sizeof( pompeii::utility::Vertex ),
        vk::VertexInputRate::eVertex ),
        {
          vk::VertexInputAttributeDescription(
            0, 0, vk::Format::eR32G32B32Sfloat,
            offsetof( pompeii::utility::Vertex, position )
          ),
      vk::VertexInputAttributeDescription(
        1, 0, vk::Format::eR32G32B32Sfloat,
        offsetof( pompeii::utility::Vertex, normal )
      ),
      vk::VertexInputAttributeDescription(
        2, 0, vk::Format::eR32G32Sfloat,
        offsetof( pompeii::utility::Vertex, texCoord )
      )
        }
    );

    vk::PipelineInputAssemblyStateCreateInfo assembly( { },
      vk::PrimitiveTopology::eTriangleList, VK_FALSE );
    PipelineViewportStateCreateInfo viewport( 1, 1 );
    vk::PipelineRasterizationStateCreateInfo rasterization( { }, true,
      false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack,
      vk::FrontFace::eCounterClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f );
    PipelineMultisampleStateCreateInfo multisample( vk::SampleCountFlagBits::e1,
      false, 0.0f, nullptr, false, false );
    vk::StencilOpState stencilOpState( vk::StencilOp::eKeep,
      vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::CompareOp::eAlways,
      0, 0, 0 );
    vk::PipelineDepthStencilStateCreateInfo depthStencil( { }, true, true,
      vk::CompareOp::eLessOrEqual, false, false, stencilOpState,
      stencilOpState, 0.0f, 0.0f );
    ;
    PipelineColorBlendStateCreateInfo colorBlend( false, vk::LogicOp::eNoOp,
      vk::PipelineColorBlendAttachmentState(
        false, vk::BlendFactor::eZero, vk::BlendFactor::eZero,
        vk::BlendOp::eAdd, vk::BlendFactor::eZero, vk::BlendFactor::eZero,
        vk::BlendOp::eAdd, vk::ColorComponentFlagBits::eR |
        vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
        vk::ColorComponentFlagBits::eA
      ), { 1.0f, 1.0f, 1.0f, 1.0f }
    );
    PipelineDynamicStateCreateInfo dynamic( {
      vk::DynamicState::eViewport, vk::DynamicState::eScissor
    } );

    struct
    {
      int mode;
      float shininess;
    } specData;

    std::vector<vk::SpecializationMapEntry> specMapEntries =
    {
      vk::SpecializationMapEntry( 0, 0, sizeof( specData.mode ) ),
      vk::SpecializationMapEntry( 1, 0, sizeof( specData.shininess ) ),
    };

    // Left pipeline
    {
      // Emerald color
      specData.mode = 1;
      specData.shininess = 0.6f;

      SpecializationInfo specInfo( specMapEntries, &specData );

      auto fragmentStage = device->createShaderPipelineShaderStage(
        POMPEII_EXAMPLES_SPV_ROUTE + std::string( "mesh_ctes_frag.spv" ),
        vk::ShaderStageFlagBits::eFragment, specInfo
      );

      pipelines.left = device->createGraphicsPipeline( _window->pipelineCache( ),
        // Specify that we will be creating a derivative of this pipeline.
        vk::PipelineCreateFlagBits::eAllowDerivatives,
        { vertexStage, fragmentStage }, vertexInput, assembly, nullptr,
        viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
        pipelineLayout, _window->renderPass( )
      );
    }
    // Right pipeline
    {
      // Gold color
      specData.mode = 2;
      specData.shininess = 0.4f;

      SpecializationInfo specInfo( specMapEntries, &specData );

      auto fragmentStage = device->createShaderPipelineShaderStage(
        POMPEII_EXAMPLES_SPV_ROUTE + std::string( "mesh_ctes_frag.spv" ),
        vk::ShaderStageFlagBits::eFragment, specInfo
      );

      rasterization = vk::PipelineRasterizationStateCreateInfo( { }, true,
        false, vk::PolygonMode::eLine, vk::CullModeFlagBits::eNone,
        vk::FrontFace::eCounterClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f );

      pipelines.right = device->createGraphicsPipeline( _window->pipelineCache( ),
        // Modify pipeline info to reflect derivation
        vk::PipelineCreateFlagBits::eDerivative,
        { vertexStage, fragmentStage }, vertexInput, assembly, nullptr,
        viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
        pipelineLayout, _window->renderPass( ), 0, pipelines.left, -1
      );
    }


    std::array<vk::DescriptorPoolSize, 2> poolSize =
    {
      vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 1 ),
      vk::DescriptorPoolSize( vk::DescriptorType::eCombinedImageSampler, 1 )
    };
    auto dspPool = device->createDescriptorPool( 1, poolSize );

    // Init descriptor set
    descriptorSet = device->allocateDescriptorSet( dspPool, descriptorSetLayout );

    std::vector<WriteDescriptorSet> wdss =
    {
      WriteDescriptorSet(
        descriptorSet, 0, 0, vk::DescriptorType::eUniformBuffer,
        1, nullptr, DescriptorBufferInfo( mvpBuffer, 0, sizeof( uboVS ) )
      )
    };
    device->updateDescriptorSets( wdss, { } );
  }

  void updateMVP( void )
  {
    auto size = _window->swapchainImageSize( );

    uint32_t width = size.width, height = size.height;

    static auto startTime = std::chrono::high_resolution_clock::now( );

    auto currentTime = std::chrono::high_resolution_clock::now( );
    float time = std::chrono::duration_cast<std::chrono::milliseconds>( 
      currentTime - startTime ).count( ) / 1000.0f;

    uboVS.viewPos = glm::vec3( 2.0f, 2.0f, 2.0f );

    uboVS.model = glm::mat4( 1.0f );
    uboVS.model = glm::scale( uboVS.model, glm::vec3( 0.25f ) );

    uboVS.model = glm::rotate( uboVS.model, 
      time * glm::radians( 90.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
    uboVS.model = glm::rotate( uboVS.model, 
      time * glm::radians( 45.0f ), glm::vec3( 0.0f, 1.0f, 0.0f ) );

    uboVS.view = glm::lookAt( uboVS.viewPos, 
      glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
    uboVS.proj = glm::perspective( glm::radians( 45.0f ), 
      width / ( float ) height, 0.1f, 10.0f );
    uboVS.proj[ 1 ][ 1 ] *= -1;


    mvpBuffer->writeData( 0, sizeof( uboVS ), &uboVS );
  }

  void nextFrame( void ) override
  {
    updateMVP( );

    std::array<vk::ClearValue, 2 > clearValues;
    std::array<float, 4> ccv = { 0.2f, 0.3f, 0.3f, 1.0f };
    clearValues[ 0 ].color = vk::ClearColorValue( ccv );
    clearValues[ 1 ].depthStencil = vk::ClearDepthStencilValue( 1.0f, 0 );

    const auto size = _window->swapchainImageSize( );
    auto cmd = _window->currentCommandBuffer( );
    vk::Rect2D rect;
    rect.extent = size;
    cmd->beginRenderPass(
      _window->renderPass( ),
      _window->framebuffer( ),
      rect, clearValues, vk::SubpassContents::eInline
    );

    cmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      pipelineLayout, 0, { descriptorSet }, nullptr );

    vk::Viewport viewport( 0.0f, 0.0f, size.width, size.height, 0.0f, 1.0f );

    cmd->setScissor( 0, vk::Rect2D( { 0, 0 }, size ) );

    // Left
    viewport.width = size.width * 0.5f;
    cmd->setViewport( 0, viewport );
    cmd->bindGraphicsPipeline( pipelines.left );
    geometry->render( cmd );

    // Right
    viewport.x = size.width * 0.5f;
    cmd->setViewport( 0, viewport );
    cmd->bindGraphicsPipeline( pipelines.right );
    geometry->render( cmd );

    cmd->endRenderPass( );

    _window->frameReady( );
  }
private:
  std::shared_ptr< DescriptorSetLayout > descriptorSetLayout;
  std::shared_ptr< DescriptorSet > descriptorSet;
  std::shared_ptr< PipelineLayout > pipelineLayout;

  struct
  {
    std::shared_ptr< Pipeline > left;
    std::shared_ptr< Pipeline > right;
  } pipelines;

  std::shared_ptr< pompeii::utility::Geometry > geometry;
  std::shared_ptr< UniformBuffer > mvpBuffer;
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
  VulkanWindow app( 1024, 768, "Pipeline Derivation with SpecializationInfo", true );
  app.show( );
  return EXIT_SUCCESS;
}