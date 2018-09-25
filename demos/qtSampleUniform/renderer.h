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
public:
	Renderer( VulkanWindow* w );

  void changeSampler( int idx )
  {
    updateSamplerUniform( samplers.at( idx ) );
  }

  void initResources( void ) override
  {
    auto device = _window->device( );

    tex = device->createTexture2D( POMPEII_EXAMPLES_IMAGES_ROUTE +
      std::string( "sample.png" ), _window->gfxCommandPool( ), 
      _window->gfxQueue( ), vk::Format::eR8G8B8A8Unorm );

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

    descriptorSetLayout = device->createDescriptorSetLayout( dslbs );
    pipelineLayout = device->createPipelineLayout( descriptorSetLayout );

    std::shared_ptr<DescriptorPool> descriptorPool =
      device->createDescriptorPool( 1, {
        { vk::DescriptorType::eSampledImage, 1 },
        { vk::DescriptorType::eSampler, 1 },
      } );

    // Init descriptor set
    descriptorSet = device->allocateDescriptorSet( descriptorPool, descriptorSetLayout );


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

      samplers.push_back( device->createSampler( sci.magFilter, sci.minFilter, 
        sci.mipmapMode, sci.addressModeU, sci.addressModeV, sci.addressModeW, 
        sci.mipLodBias, sci.anisotropyEnable, sci.maxAnisotropy, 
        sci.compareEnable, sci.compareOp, sci.minLod, sci.maxLod, 
        sci.borderColor, sci.unnormalizedCoordinates ) );

      sci.setAddressModeU( vk::SamplerAddressMode::eMirroredRepeat );
      sci.setAddressModeV( vk::SamplerAddressMode::eMirroredRepeat );
      sci.setAddressModeW( vk::SamplerAddressMode::eMirroredRepeat );
      samplers.push_back( device->createSampler( sci.magFilter, sci.minFilter,
        sci.mipmapMode, sci.addressModeU, sci.addressModeV, sci.addressModeW,
        sci.mipLodBias, sci.anisotropyEnable, sci.maxAnisotropy,
        sci.compareEnable, sci.compareOp, sci.minLod, sci.maxLod,
        sci.borderColor, sci.unnormalizedCoordinates ) );

      sci.setAddressModeU( vk::SamplerAddressMode::eClampToEdge );
      sci.setAddressModeV( vk::SamplerAddressMode::eClampToEdge );
      sci.setAddressModeW( vk::SamplerAddressMode::eClampToEdge );
      samplers.push_back( device->createSampler( sci.magFilter, sci.minFilter,
        sci.mipmapMode, sci.addressModeU, sci.addressModeV, sci.addressModeW,
        sci.mipLodBias, sci.anisotropyEnable, sci.maxAnisotropy,
        sci.compareEnable, sci.compareOp, sci.minLod, sci.maxLod,
        sci.borderColor, sci.unnormalizedCoordinates ) );

      sci.setAddressModeU( vk::SamplerAddressMode::eClampToBorder );
      sci.setAddressModeV( vk::SamplerAddressMode::eClampToBorder );
      sci.setAddressModeW( vk::SamplerAddressMode::eClampToBorder );
      samplers.push_back( device->createSampler( sci.magFilter, sci.minFilter,
        sci.mipmapMode, sci.addressModeU, sci.addressModeV, sci.addressModeW,
        sci.mipLodBias, sci.anisotropyEnable, sci.maxAnisotropy,
        sci.compareEnable, sci.compareOp, sci.minLod, sci.maxLod,
        sci.borderColor, sci.unnormalizedCoordinates ) );
    }

    updateSamplerUniform( samplers.at( 0 ) );

    auto vertexStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "fullquad_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex
    );
    auto fragmentStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "fullquadSampler_frag.spv" ),
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

  void updateSamplerUniform( std::shared_ptr<Sampler> s )
  {
    auto device = _window->device( );

    device->waitIdle( );  // We can't update descriptor if device is busy

    DescriptorImageInfo descriptor;
    descriptor.imageLayout = tex->imageLayout;
    descriptor.imageView = tex->view;
    descriptor.sampler = VK_NULL_HANDLE;

    DescriptorImageInfo samplerInfo;
    samplerInfo.sampler = s;

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

    device->updateDescriptorSets( wdss, {} );
  }

  void nextFrame( void ) override
  {
    /*if ( Input::isKeyPressed( pompeii::Keyboard::Key::Num1 ) )
    {
      updateSamplerUniform( sampler1 );
    } else if ( Input::isKeyPressed( pompeii::Keyboard::Key::Num2 ) )
    {
      updateSamplerUniform( sampler2 );
    } else if ( Input::isKeyPressed( pompeii::Keyboard::Key::Num3 ) )
    {
      updateSamplerUniform( sampler3 );
    } else if ( Input::isKeyPressed( pompeii::Keyboard::Key::Num4 ) )
    {
      updateSamplerUniform( sampler4 );
    }*/

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
  std::shared_ptr< Texture2D > tex;

  std::shared_ptr<Sampler> sampler;

  std::vector<std::shared_ptr< pompeii::Sampler > > samplers;
};