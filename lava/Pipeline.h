/**
 * Copyright (c) 2017 - 2018, Lava
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

#ifndef __LAVA_PIPELINE__
#define __LAVA_PIPELINE__

#include "includes.hpp"
#include "VulkanResource.h"

#include "noncopyable.hpp"
#include "Descriptor.h"

#include <lava/api.h>

#include "RenderPass.h"

#include <iomanip>

namespace lava
{
  class ShaderModule
    : public VulkanResource
    , private NonCopyable<ShaderModule>
  {
  public:
    LAVA_API
    ShaderModule( const std::shared_ptr<Device>& device, 
      const std::string& filePath, vk::ShaderStageFlagBits type );
    LAVA_API
    ShaderModule( const std::shared_ptr<Device>& device, 
      const std::string& filePath );
    LAVA_API
    ShaderModule( const std::shared_ptr<Device>& device, 
      vk::ArrayProxy<const uint32_t> code );
    LAVA_API
    ~ShaderModule( void );

    inline operator vk::ShaderModule( void ) const { return _shaderModule; }

  private:
    vk::ShaderModule _shaderModule;

    const std::vector<uint32_t> readFile( const std::string& filename );
  };

  struct PipelineVertexInputStateCreateInfo
  {
    LAVA_API
    PipelineVertexInputStateCreateInfo(
      vk::ArrayProxy<const vk::VertexInputBindingDescription> vertexBindingDescriptions = { },
      vk::ArrayProxy<const vk::VertexInputAttributeDescription> vertexAttrirDescriptions = { } );
    LAVA_API
    PipelineVertexInputStateCreateInfo( const PipelineVertexInputStateCreateInfo& rhs );
    LAVA_API
    PipelineVertexInputStateCreateInfo & operator=( 
      const PipelineVertexInputStateCreateInfo& rhs );

    std::vector<vk::VertexInputBindingDescription> vertexBindingDescriptions;
    std::vector<vk::VertexInputAttributeDescription> vertexAttrirDescriptions;
  };

  struct PipelineDynamicStateCreateInfo
  {
    LAVA_API
    PipelineDynamicStateCreateInfo( 
      vk::ArrayProxy<const vk::DynamicState> dynamicStates );
    LAVA_API
    PipelineDynamicStateCreateInfo( const PipelineDynamicStateCreateInfo& rhs );
    LAVA_API
    PipelineDynamicStateCreateInfo& operator=( 
      const PipelineDynamicStateCreateInfo& rhs );

    std::vector<vk::DynamicState> dynamicStates;
  };

  struct ScissorsViewportDynamicPipelineState : public PipelineDynamicStateCreateInfo
  {
    LAVA_API
    ScissorsViewportDynamicPipelineState( void );
  };

  struct PipelineViewportStateCreateInfo
  {
    LAVA_API
    PipelineViewportStateCreateInfo( uint32_t dummyViews, uint32_t dummySci );
    LAVA_API
    PipelineViewportStateCreateInfo(
      vk::ArrayProxy<const vk::Viewport> viewports_,
      vk::ArrayProxy<const vk::Rect2D> scissors_ );
    LAVA_API
    PipelineViewportStateCreateInfo( const PipelineViewportStateCreateInfo& rhs );
    LAVA_API
    PipelineViewportStateCreateInfo & operator=( 
      const PipelineViewportStateCreateInfo& rhs );

    std::vector<vk::Viewport> viewports;
    std::vector<vk::Rect2D> scissors;
  };

  struct PipelineColorBlendStateCreateInfo
  {
    LAVA_API
    PipelineColorBlendStateCreateInfo( bool logicEnable, vk::LogicOp logicOp, 
      vk::ArrayProxy<const vk::PipelineColorBlendAttachmentState> attachments, 
      const std::array<float, 4>& blendConstants );
    LAVA_API
    PipelineColorBlendStateCreateInfo( const PipelineColorBlendStateCreateInfo& rhs );
    LAVA_API
    PipelineColorBlendStateCreateInfo& operator=( 
      const PipelineColorBlendStateCreateInfo& rhs );

    bool logicEnable;
    vk::LogicOp logicOp;
    std::vector<vk::PipelineColorBlendAttachmentState> attachments;
    std::array<float, 4> blendConstants;
  };
  struct PipelineMultisampleStateCreateInfo
  {
    // TODO: find a way to keep rasterizationSamples and sampleMasks in sync!!
    LAVA_API
    PipelineMultisampleStateCreateInfo( vk::SampleCountFlagBits rasterizationSamples, 
      bool sampleShadingEnable, float minSampleShading, 
      vk::ArrayProxy<const vk::SampleMask> sampleMasks,
      bool alphaToCoverageEnable, bool alphaToOneEnable );
    LAVA_API
    PipelineMultisampleStateCreateInfo( const PipelineMultisampleStateCreateInfo& rhs );
    LAVA_API
    PipelineMultisampleStateCreateInfo & operator=( 
      const PipelineMultisampleStateCreateInfo& rhs );

    vk::SampleCountFlagBits rasterizationSamples;
    bool sampleShadingEnable;
    float minSampleShading;
    std::vector<vk::SampleMask> sampleMasks;
    bool alphaToCoverageEnable;
    bool alphaToOneEnable;
  };
  class PipelineCache
    : private NonCopyable<PipelineCache>
    , public VulkanResource
  {
  public:
    LAVA_API
    PipelineCache( const std::shared_ptr<Device>& device, 
      const std::string& filename );
    LAVA_API
    PipelineCache( const std::shared_ptr<Device>& device, 
      vk::PipelineCacheCreateFlags flags, size_t initialSize, 
      void const* initialData );
    LAVA_API
    ~PipelineCache( void );

    LAVA_API
    void loadFromFile( const std::string& filename );
    LAVA_API
    void loadFromFile( vk::PipelineCacheCreateFlags flags, size_t initialSize,
      void const* initialData );

    LAVA_API
    std::vector<uint8_t> getData( void ) const;
    LAVA_API
    void merge( vk::ArrayProxy<const std::shared_ptr<PipelineCache>> srcCaches ) const;

    LAVA_API
    void saveToFile( const char* filename );
    LAVA_API
    void saveToFile( const std::string& filename );

    inline operator vk::PipelineCache( void ) const
    {
      return _pipelineCache;
    }
  private:
    vk::PipelineCache _pipelineCache;

    void print_UUID( uint8_t *pipelineCacheUUID )
    {
      for ( int j = 0; j < VK_UUID_SIZE; ++j )
      {
        std::cout << std::setw( 2 ) << ( uint32_t ) pipelineCacheUUID[ j ];
        if ( j == 3 || j == 5 || j == 7 || j == 9 )
        {
          std::cout << '-';
        }
      }
    }
  };

  class PipelineLayout 
    : public VulkanResource
    , private NonCopyable<PipelineLayout>
  {
  public:
    LAVA_API
    PipelineLayout( const std::shared_ptr<Device>& device, 
      vk::ArrayProxy<const std::shared_ptr<DescriptorSetLayout>> setLayouts,
      vk::ArrayProxy<const vk::PushConstantRange> pushConstantRanges );
    LAVA_API
    ~PipelineLayout( void );
    
    inline operator vk::PipelineLayout( void ) const
    {
      return _pipelineLayout;
    }

  private:
    vk::PipelineLayout _pipelineLayout;
    std::vector<std::shared_ptr<DescriptorSetLayout>> _setLayouts;
  };
  class Pipeline : public VulkanResource, private NonCopyable<Pipeline>
  {
  public:
    ~Pipeline( );

    inline operator vk::Pipeline( ) const
    {
      return _pipeline;
    }

  protected:
    Pipeline( const std::shared_ptr<Device>& device );
    void setPipeline( const vk::Pipeline & pipeline );

  protected:
    vk::Pipeline _pipeline;
  };

  struct SpecializationInfo
  {
    LAVA_API
    SpecializationInfo( vk::ArrayProxy<const vk::SpecializationMapEntry> mapEntries, 
      const void* data );
    LAVA_API
    SpecializationInfo( const SpecializationInfo& rhs );
    LAVA_API
    SpecializationInfo & operator=( const SpecializationInfo& rhs );

    std::vector<vk::SpecializationMapEntry> mapEntries;
    const void*                             data;
  };
  
  struct PipelineShaderStageCreateInfo
  {
    LAVA_API
    PipelineShaderStageCreateInfo( vk::ShaderStageFlagBits stage, 
      const std::shared_ptr<ShaderModule>& module, const std::string& name = "main",
      vk::Optional<const SpecializationInfo> specializationInfo = nullptr );
    LAVA_API
    PipelineShaderStageCreateInfo( const PipelineShaderStageCreateInfo& rhs );
    LAVA_API
    PipelineShaderStageCreateInfo & operator=( const PipelineShaderStageCreateInfo& rhs );

    vk::ShaderStageFlagBits             stage;
    std::shared_ptr<ShaderModule>       module;
    std::string                         name;
    std::unique_ptr<SpecializationInfo> specializationInfo;
  };

  class ComputePipeline : public Pipeline
  {
  public:
    LAVA_API
    ComputePipeline( const std::shared_ptr<Device>& device,
      const std::shared_ptr<PipelineCache>& pipelineCache,
      vk::PipelineCreateFlags flags,
      const PipelineShaderStageCreateInfo& stage,
      const std::shared_ptr<PipelineLayout>& layout,
      const std::shared_ptr<Pipeline>& basePipelineHandle,
      uint32_t basePipelineIndex );
  };

  class GraphicsPipeline : public Pipeline
  {
  public:
    LAVA_API
    GraphicsPipeline(
      const std::shared_ptr<Device>& device,
      const std::shared_ptr<PipelineCache>& pipelineCache,
      vk::PipelineCreateFlags flags,
      vk::ArrayProxy<const PipelineShaderStageCreateInfo> stages,
      vk::Optional<const PipelineVertexInputStateCreateInfo> vertexInputState,
      vk::Optional<const vk::PipelineInputAssemblyStateCreateInfo> inputAssemblyState,
      vk::Optional<const vk::PipelineTessellationStateCreateInfo> tessellationState,
      vk::Optional<const PipelineViewportStateCreateInfo> viewportState,
      vk::Optional<const vk::PipelineRasterizationStateCreateInfo> rasterizationState,
      vk::Optional<const PipelineMultisampleStateCreateInfo> multisampleState,
      vk::Optional<const vk::PipelineDepthStencilStateCreateInfo> depthStencilState,
      vk::Optional<const PipelineColorBlendStateCreateInfo> colorBlendState,
      vk::Optional<const PipelineDynamicStateCreateInfo> dynamicState,
      const std::shared_ptr<PipelineLayout>& pipelineLayout,
      const std::shared_ptr<RenderPass>& renderPass,
      uint32_t subpass, const std::shared_ptr<Pipeline>& basePipelineHandle,
      uint32_t basePipelineIndex );
    LAVA_API
    inline bool isBlendingEnabled( void ) const
    {
      return _blendingEnabled;
    }
  protected:
    bool _blendingEnabled = false;
  };
}

#endif /* __LAVA_PIPELINE__ */