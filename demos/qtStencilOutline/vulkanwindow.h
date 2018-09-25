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

#ifndef __POMPEII_QTSTENCILOUTLINE_VULKANNWINDOW_H__
#define __POMPEII_QTSTENCILOUTLINE_VULKANNWINDOW_H__

#include <pompeii/pompeii.h>
#include <pompeiiUtils/pompeiiUtils.h>
#include <qtPompeii/qtPompeii.h>
using namespace pompeii;

#include <routes.h>
#include <glm/gtc/matrix_transform.hpp>

#include <QKeyEvent>

class StencilOutlineRenderer : public pompeii::qt::VulkanWindowRenderer
{
private:
  pompeii::qt::VulkanWindow* _window;
public:
  StencilOutlineRenderer( pompeii::qt::VulkanWindow* window )
    : _window( window )
  {
    _window->setTitle( "(Perfect) Toon Shading with Stencil Outline" );
  }

  void initResources( void ) override
  {
    auto device = _window->device( );

    geometry = std::make_shared<pompeii::utility::Geometry>( device,
      POMPEII_EXAMPLES_MESHES_ROUTE + std::string( "ear_study3.obj_" ) );

    uMVP = device->createUniformBuffer( sizeof( uboVS ) );
    uBufferOutline = device->createUniformBuffer( sizeof( uboOutline ) );

    // Init descriptor and pipeline layouts
    std::vector<DescriptorSetLayoutBinding> dslbs =
    {
      DescriptorSetLayoutBinding( 
        0, vk::DescriptorType::eUniformBuffer,
        vk::ShaderStageFlagBits::eVertex
      )
    };
    descriptorSetLayouts.solid = device->createDescriptorSetLayout( dslbs );

    pipelineLayouts.solid = device->createPipelineLayout( descriptorSetLayouts.solid, nullptr );

    dslbs =
    {
      DescriptorSetLayoutBinding(
        0, vk::DescriptorType::eUniformBuffer,
        vk::ShaderStageFlagBits::eVertex
      ),
      DescriptorSetLayoutBinding( 
        1, vk::DescriptorType::eUniformBuffer,
        vk::ShaderStageFlagBits::eVertex
      )
    };
    descriptorSetLayouts.outline = device->createDescriptorSetLayout( dslbs );

    pipelineLayouts.outline = device->createPipelineLayout( descriptorSetLayouts.outline, nullptr );

    // init pipeline
    auto vertexStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "perfectToon_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex
    );
    auto fragmentStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "perfectToon_frag.spv" ),
      vk::ShaderStageFlagBits::eFragment
    );

    PipelineVertexInputStateCreateInfo vertexInput(
      vk::VertexInputBindingDescription( 0, sizeof( pompeii::utility::Vertex ),
        vk::VertexInputRate::eVertex ),
        {
          vk::VertexInputAttributeDescription( 0, 0, vk::Format::eR32G32B32Sfloat,
            offsetof( pompeii::utility::Vertex, position )
          ),
          vk::VertexInputAttributeDescription( 1, 0, vk::Format::eR32G32B32Sfloat,
            offsetof( pompeii::utility::Vertex, normal )
          )
        }
    );
    vk::PipelineInputAssemblyStateCreateInfo assembly( { },
      vk::PrimitiveTopology::eTriangleList, VK_FALSE );
    PipelineViewportStateCreateInfo viewport( 1, 1 );
    vk::PipelineRasterizationStateCreateInfo rasterization( { }, true,
      false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone,
      vk::FrontFace::eClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f );
    PipelineMultisampleStateCreateInfo multisample( vk::SampleCountFlagBits::e1,
      false, 0.0f, nullptr, false, false );
    vk::StencilOpState stencilOpState( vk::StencilOp::eKeep, vk::StencilOp::eKeep,
      vk::StencilOp::eKeep, vk::CompareOp::eAlways, 0, 0, 0 );
    vk::PipelineDepthStencilStateCreateInfo depthStencilState( { }, true, true,
      vk::CompareOp::eLessOrEqual, false, false, stencilOpState, stencilOpState,
      0.0f, 0.0f );
    vk::PipelineColorBlendAttachmentState colorBlendAttachment( false,
      vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
      vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
      vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA );
    PipelineColorBlendStateCreateInfo colorBlend( false, vk::LogicOp::eNoOp,
      colorBlendAttachment, { 1.0f, 1.0f, 1.0f, 1.0f } );
    PipelineDynamicStateCreateInfo dynamic( {
      vk::DynamicState::eViewport, vk::DynamicState::eScissor
    } );

    depthStencilState.stencilTestEnable = VK_TRUE;

    // glStencilFunc( GL_ALWAYS, 1, 0xFF );
    depthStencilState.back.compareOp = vk::CompareOp::eAlways;
    depthStencilState.back.compareMask = 0xff;
    depthStencilState.back.reference = 1;
    // glStencilMask(0xFF);
    depthStencilState.back.writeMask = 0xff;
    // glEnable( GL_DEPTH );
    depthStencilState.depthTestEnable = VK_TRUE;

    depthStencilState.back.failOp = vk::StencilOp::eKeep;
    depthStencilState.back.depthFailOp = vk::StencilOp::eKeep;
    depthStencilState.back.passOp = vk::StencilOp::eReplace;

    depthStencilState.front = depthStencilState.back;

    pipelines.solid = device->createGraphicsPipeline( _window->pipelineCache( ), { },
    { vertexStage, fragmentStage }, vertexInput, assembly, nullptr,
      viewport, rasterization, multisample, depthStencilState, colorBlend,
      dynamic, pipelineLayouts.solid, _window->renderPass( )
    );

    // Outline pass
    // glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    depthStencilState.back.compareOp = vk::CompareOp::eNotEqual;
    // depthStencilState.back.compareMask = 0xff;
    // depthStencilState.back.reference = 1;
    // glDisable( GL_DEPTH );
    depthStencilState.depthTestEnable = VK_FALSE;

    depthStencilState.front = depthStencilState.back;

    vertexStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "outline_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex
    );
    fragmentStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "outline_frag.spv" ),
      vk::ShaderStageFlagBits::eFragment
    );

    pipelines.outline = device->createGraphicsPipeline( _window->pipelineCache( ), { },
    { vertexStage, fragmentStage }, vertexInput, assembly, nullptr,
      viewport, rasterization, multisample, depthStencilState, colorBlend,
      dynamic, pipelineLayouts.outline, _window->renderPass( )
    );

    std::array<vk::DescriptorPoolSize, 1> poolSize =
    {
      vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 3 )
    };
    auto descriptorPool = device->createDescriptorPool( 2, poolSize );

    // Init descriptor set
    descriptorSets.solid = device->allocateDescriptorSet(
      descriptorPool, descriptorSetLayouts.solid
    );
    std::vector<WriteDescriptorSet> wdss =
    {
      WriteDescriptorSet( descriptorSets.solid, 0, 0,
        vk::DescriptorType::eUniformBuffer, 1, nullptr,
        DescriptorBufferInfo(
          uMVP, 0, sizeof( uboVS )
        )
      )
    };
    device->updateDescriptorSets( wdss, { } );

    descriptorSets.outline = device->allocateDescriptorSet(
      descriptorPool, descriptorSetLayouts.outline
    );
    wdss =
    {
      WriteDescriptorSet( descriptorSets.outline, 0, 0,
        vk::DescriptorType::eUniformBuffer, 1, nullptr,
        DescriptorBufferInfo(
          uMVP, 0, sizeof( uboVS )
        )
      ),
      WriteDescriptorSet( descriptorSets.outline, 1, 0,
        vk::DescriptorType::eUniformBuffer, 1, nullptr,
        DescriptorBufferInfo(
          uBufferOutline, 0, sizeof( uboOutline )
        )
      )
    };
    device->updateDescriptorSets( wdss, { } );
  }

  void releaseResources( void ) override
  {
    /*geometry.reset( );

    uMVP.reset( );
    uBufferOutline.reset( );

    descriptorSetLayouts.solid.reset( );
    pipelineLayouts.solid.reset( );

    descriptorSetLayouts.outline.reset( );
    pipelineLayouts.outline.reset( );
    
    pipelines.solid.reset( );
    pipelines.outline.reset( );

    descriptorSets.solid.reset( );
    descriptorSets.outline.reset( );*/
  }

  void updateUniforms( void )
  {
    auto size = _window->swapchainImageSize( );

    uint32_t width = size.width, height = size.height;

    static auto startTime = std::chrono::high_resolution_clock::now( );

    auto currentTime = std::chrono::high_resolution_clock::now( );
    float time = std::chrono::duration_cast< std::chrono::milliseconds >( currentTime - startTime ).count( ) / 1000.0f;

    uboVS.model = glm::mat4( 1.0f );
    uboVS.model = glm::rotate(
      uboVS.model, time * glm::radians( 90.0f ),
      glm::vec3( 0.0f, -1.0f, 0.0f )
    );
    glm::vec3 cameraPos = glm::vec3( 0.0f, 0.0f, 5.5f );
    glm::vec3 cameraFront = glm::vec3( 0.0f, 0.0f, -1.0f );
    glm::vec3 cameraUp = glm::vec3( 0.0f, 1.0f, 0.0f );

    uboVS.view = glm::lookAt( cameraPos, cameraPos + cameraFront, cameraUp );

    uboVS.proj = glm::perspective( glm::radians( 45.0f ),
      width / ( float ) height, 0.1f, 10.0f );
    uboVS.proj[ 1 ][ 1 ] *= -1;

    uMVP->set( &uboVS );

    uBufferOutline->set( &uboOutline );
  }

  void updateOutlineWidth( float inc )
  {
    if ( uboOutline.outlineWidth + inc > 0.01f )
    {
      uboOutline.outlineWidth += inc;
    }
  }

  void nextFrame( void ) override
  {
    updateUniforms( );

    std::array<vk::ClearValue, 2 > clearValues;
    std::array<float, 4> ccv = { 0.0f, 0.0f, 0.0f, 1.0f };
    clearValues[ 0 ].color = vk::ClearColorValue( ccv );
    clearValues[ 1 ].depthStencil = vk::ClearDepthStencilValue( 1.0f, 0 );

    auto cmd = _window->currentCommandBuffer( );
    vk::Rect2D rect;
    rect.extent = _window->swapchainImageSize( );
    cmd->beginRenderPass(
      _window->renderPass( ), _window->framebuffer( ),
      rect, clearValues, vk::SubpassContents::eInline
    );

    cmd->setViewportScissors( rect.extent );

    // First pass renders object (toon shaded) and fills stencil buffer
    cmd->bindGraphicsPipeline( pipelines.solid );
    cmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      pipelineLayouts.solid, 0, { descriptorSets.solid }, nullptr );

    geometry->render( cmd );

    // Second pass renders scaled object only where stencil was not set by first pass
    cmd->bindGraphicsPipeline( pipelines.outline );
    cmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      pipelineLayouts.outline, 0, { descriptorSets.outline }, nullptr );

    geometry->render( cmd );

    cmd->endRenderPass( );

    _window->frameReady( );
  }
private:
  struct
  {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
  } uboVS;

  struct
  {
    float outlineWidth = 0.05f;
  } uboOutline;

  std::shared_ptr<pompeii::UniformBuffer> uMVP;
  std::shared_ptr<pompeii::UniformBuffer> uBufferOutline;

  struct
  {
    std::shared_ptr<Pipeline> solid;
    std::shared_ptr<Pipeline> outline;
  } pipelines;

  struct
  {
    std::shared_ptr<PipelineLayout> solid;
    std::shared_ptr<PipelineLayout> outline;
  } pipelineLayouts;

  struct
  {
    std::shared_ptr<DescriptorSet> solid;
    std::shared_ptr<DescriptorSet> outline;
  } descriptorSets;

  struct
  {
    std::shared_ptr<DescriptorSetLayout> solid;
    std::shared_ptr<DescriptorSetLayout> outline;
  } descriptorSetLayouts;

  std::shared_ptr<pompeii::utility::Geometry> geometry;
};

class VulkanWindow : public pompeii::qt::VulkanWindow
{
  Q_OBJECT
private:
  StencilOutlineRenderer* _renderer;
public:
  VulkanWindow( QWindow* parent = nullptr )
    : pompeii::qt::VulkanWindow( parent )
  {
    setContinuousRendering( true );
  }
  virtual pompeii::qt::VulkanWindowRenderer* createRenderer( void )
  {
    _renderer = new StencilOutlineRenderer( this );
    return _renderer;
  }
protected:
  bool event( QEvent* ev ) override
  {
    if ( ev->type( ) == QEvent::KeyPress )
    {
      QKeyEvent* key = (QKeyEvent*)( ev );

      if ( key->key( ) == Qt::Key_Escape )
      {
        this->close( );
      }
      else if ( key->key( ) == Qt::Key_Z )
      {
        _renderer->updateOutlineWidth( -0.005f );
      }
      else if ( key->key( ) == Qt::Key_X )
      {
        _renderer->updateOutlineWidth( +0.005f );
      }
      else
      {
        return QtVulkanWindow::event( ev );
      }
      return true;
    }
    else
    {
      return QtVulkanWindow::event( ev );
    }
    return false;
  }
};

#endif /* __POMPEII_QTSTENCILOUTLINE_VULKANNWINDOW_H__ */