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

#pragma once

#include <qtPompeii/qtPompeii.h>

#include "../utils/Camera.h"
#include <pompeiiUtils/pompeiiUtils.h>
using namespace pompeii;

#include <routes.h>
#include "vulkanwindow.h"

#include <unordered_map>

class Renderer: public pompeii::qt::VulkanWindowRenderer
{
private:
  struct PushConstant
  {
    float _MaskValue = 0.5f;
  } pushCtes;
public:
	Renderer( VulkanWindow* w );

  void changeImage( int idx )
  {
    updateCurrentImage( idx );
  }

  void updateCurrentImage( int i )
  {
    auto device = _window->device( );

    _MaskTex = device->createTexture2D( POMPEII_EXAMPLES_IMAGES_ROUTE +
      std::string( "masks/Mask" ) + std::to_string( i ) + std::string(".png" ), 
      _window->gfxCommandPool( ), _window->gfxQueue( ), vk::Format::eR8G8B8A8Unorm );

    device->waitIdle( );
    std::vector<WriteDescriptorSet> wdss =
    {
      WriteDescriptorSet( descriptorSet, 0, 0,
        vk::DescriptorType::eCombinedImageSampler, 1,
        _MainTex->descriptor, nullptr
      ),
      WriteDescriptorSet( descriptorSet, 1, 0,
        vk::DescriptorType::eCombinedImageSampler, 1,
        _MaskTex->descriptor, nullptr
      ),
      WriteDescriptorSet( descriptorSet, 2, 0,
        vk::DescriptorType::eCombinedImageSampler, 1,
        _TransTex->descriptor, nullptr
      )
    };
    device->updateDescriptorSets( wdss, { } );
  }

  void changeMaskLevel( float v )
  {
    pushCtes._MaskValue = v;
    std::cout << "Mask Level: " << v << std::endl;
  }

  void initResources( void ) override
  {
    auto device = _window->device( );

    _MainTex = device->createTexture2D( POMPEII_EXAMPLES_IMAGES_ROUTE +
      std::string( "sample.png" ), _window->gfxCommandPool( ), 
      _window->gfxQueue( ), vk::Format::eR8G8B8A8Unorm );

    _TransTex = device->createTexture2D( POMPEII_EXAMPLES_IMAGES_ROUTE +
      std::string( "glass.png" ), _window->gfxCommandPool( ),
      _window->gfxQueue( ), vk::Format::eR8G8B8A8Unorm );

    std::vector<DescriptorSetLayoutBinding> dslbs =
    {
      DescriptorSetLayoutBinding( 0,
        vk::DescriptorType::eCombinedImageSampler,
        vk::ShaderStageFlagBits::eFragment
      ),
      DescriptorSetLayoutBinding( 1,
        vk::DescriptorType::eCombinedImageSampler,
        vk::ShaderStageFlagBits::eFragment
      ),
      DescriptorSetLayoutBinding( 2,
        vk::DescriptorType::eCombinedImageSampler,
        vk::ShaderStageFlagBits::eFragment
      )
    };

    descriptorSetLayout = device->createDescriptorSetLayout( dslbs );
    pipelineLayout = device->createPipelineLayout( descriptorSetLayout,
      vk::PushConstantRange(
        vk::ShaderStageFlagBits::eFragment, 0, sizeof( pushCtes )
      ) );

    std::shared_ptr<DescriptorPool> descriptorPool =
      device->createDescriptorPool( 1, {
        { vk::DescriptorType::eCombinedImageSampler, 3 }
      } );

    // Init descriptor set
    descriptorSet = device->allocateDescriptorSet( descriptorPool, descriptorSetLayout );

    updateCurrentImage( 0 );

    auto vertexStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "fullquad_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex
    );
    auto fragmentStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "mask_frag.spv" ),
      vk::ShaderStageFlagBits::eFragment
    );

    PipelineVertexInputStateCreateInfo vertexInput( {}, {} );
    vk::PipelineInputAssemblyStateCreateInfo assembly( {},
      vk::PrimitiveTopology::eTriangleList, VK_FALSE );
    PipelineViewportStateCreateInfo viewport( 1, 1 );
    vk::PipelineRasterizationStateCreateInfo rasterization( {}, true,
      false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone,
      vk::FrontFace::eCounterClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f
    );
    PipelineMultisampleStateCreateInfo multisample(
      vk::SampleCountFlagBits::e1, false, 0.0f, nullptr, false, false
    );
    vk::StencilOpState stencilOpState( vk::StencilOp::eKeep,
      vk::StencilOp::eKeep, vk::StencilOp::eKeep,
      vk::CompareOp::eAlways, 0, 0, 0
    );
    vk::PipelineDepthStencilStateCreateInfo depthStencil( {},
      true, true, vk::CompareOp::eLessOrEqual, false, false,
      stencilOpState, stencilOpState, 0.0f, 0.0f
    );
    vk::PipelineColorBlendAttachmentState colorBlendAttachment( false,
      vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
      vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
      vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    );
    PipelineColorBlendStateCreateInfo colorBlend( false,
      vk::LogicOp::eNoOp, colorBlendAttachment, { 1.0f, 1.0f, 1.0f, 1.0f }
    );
    PipelineDynamicStateCreateInfo dynamic( {
      vk::DynamicState::eViewport, vk::DynamicState::eScissor
    } );

    pipeline = device->createGraphicsPipeline( _window->pipelineCache( ), {},
      { vertexStage, fragmentStage }, vertexInput, assembly, nullptr,
      viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
      pipelineLayout, _window->renderPass( ) );
  }

  void nextFrame( void ) override
  {
    std::array<vk::ClearValue, 2 > clearValues;
    std::array<float, 4> ccv = { 0.2f, 0.3f, 0.3f, 1.0f };
    clearValues[ 0 ].color = vk::ClearColorValue( ccv );
    clearValues[ 1 ].depthStencil = vk::ClearDepthStencilValue( 1.0f, 0 );

    auto cmd = _window->currentCommandBuffer( );
    vk::Rect2D rect;
    rect.extent = _window->swapchainImageSize( );
    cmd->beginRenderPass(
      _window->renderPass( ),
      _window->framebuffer( ),
      rect, clearValues, vk::SubpassContents::eInline
    );

    cmd->bindGraphicsPipeline( pipeline );
    cmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      pipelineLayout, 0, { descriptorSet }, nullptr );

    cmd->pushConstants<PushConstant>( pipelineLayout,
      vk::ShaderStageFlagBits::eFragment, 0, pushCtes );

    cmd->setViewportScissors( _window->swapchainImageSize( ) );
    cmd->draw( 4, 1, 0, 0 );

    cmd->endRenderPass( );

    _window->frameReady( );
  }
private:
  pompeii::qt::VulkanWindow* _window;

  std::shared_ptr< DescriptorSet > descriptorSet;
  std::shared_ptr< DescriptorSetLayout > descriptorSetLayout;
  std::shared_ptr< PipelineLayout > pipelineLayout;
  std::shared_ptr< Pipeline > pipeline;
  std::shared_ptr< Texture2D > _MainTex, _TransTex, _MaskTex;
};