#ifndef __LAVA_PIPELINE__
#define __LAVA_PIPELINE__

#include "includes.hpp"
#include "VulkanResource.h"

#include "noncopyable.hpp"
#include "Descriptor.h"

#include <lava/api.h>

namespace lava
{
  class Device;
  class RenderPass;

  class ShaderModule : public VulkanResource, private NonCopyable<ShaderModule>
  {
  public:
    LAVA_API
    ShaderModule( const DeviceRef& device, const std::string& filePath, vk::ShaderStageFlagBits type );
    LAVA_API
    ShaderModule( const DeviceRef& device, const std::string& filePath );
    LAVA_API
    ShaderModule( const DeviceRef& device, vk::ArrayProxy<const uint32_t> code );
    LAVA_API
    ~ShaderModule( );

    inline operator vk::ShaderModule( ) const { return _shaderModule; }

  private:
    vk::ShaderModule _shaderModule;

    const std::vector<uint32_t> readFile( const std::string& filename );
  };

  struct PipelineVertexInputStateCreateInfo
  {
    LAVA_API
    PipelineVertexInputStateCreateInfo(
      vk::ArrayProxy<const vk::VertexInputBindingDescription> vertexBindingDescriptions,
      vk::ArrayProxy<const vk::VertexInputAttributeDescription> vertexAttributeDesriptions );
    LAVA_API
    PipelineVertexInputStateCreateInfo( const PipelineVertexInputStateCreateInfo& rhs );
    LAVA_API
    PipelineVertexInputStateCreateInfo & operator=( const PipelineVertexInputStateCreateInfo& rhs );

    std::vector<vk::VertexInputBindingDescription> vertexBindingDescriptions;
    std::vector<vk::VertexInputAttributeDescription> vertexAttributeDesriptions;
  };

  struct PipelineDynamicStateCreateInfo
  {
    LAVA_API
    PipelineDynamicStateCreateInfo( vk::ArrayProxy<const vk::DynamicState> dynamicStates );
    LAVA_API
    PipelineDynamicStateCreateInfo( const PipelineDynamicStateCreateInfo& rhs );
    LAVA_API
    PipelineDynamicStateCreateInfo& operator=( const PipelineDynamicStateCreateInfo& rhs );

    std::vector<vk::DynamicState> dynamicStates;
  };

  struct PipelineViewportStateCreateInfo
  {
    LAVA_API
    PipelineViewportStateCreateInfo(
      vk::ArrayProxy<const vk::Viewport> viewports_,
      vk::ArrayProxy<const vk::Rect2D> scissors_ );
    LAVA_API
    PipelineViewportStateCreateInfo( const PipelineViewportStateCreateInfo& rhs );
    LAVA_API
    PipelineViewportStateCreateInfo & operator=( const PipelineViewportStateCreateInfo& rhs );

    std::vector<vk::Viewport> viewports;
    std::vector<vk::Rect2D> scissors;
  };

  struct PipelineColorBlendStateCreateInfo
  {
    LAVA_API
    PipelineColorBlendStateCreateInfo( bool logicEnable, vk::LogicOp logicOp, 
      vk::ArrayProxy<const vk::PipelineColorBlendAttachmentState> attachments, 
      std::array<float, 4> const& blendConstants );
    LAVA_API
    PipelineColorBlendStateCreateInfo( PipelineColorBlendStateCreateInfo const& rhs );
    LAVA_API
    PipelineColorBlendStateCreateInfo & operator=( PipelineColorBlendStateCreateInfo const& rhs );

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
    PipelineMultisampleStateCreateInfo( PipelineMultisampleStateCreateInfo const &rhs );
    LAVA_API
    PipelineMultisampleStateCreateInfo & operator=( PipelineMultisampleStateCreateInfo const& rhs );

    vk::SampleCountFlagBits rasterizationSamples;
    bool sampleShadingEnable;
    float minSampleShading;
    std::vector<vk::SampleMask> sampleMasks;
    bool alphaToCoverageEnable;
    bool                        alphaToOneEnable;
  };
  class PipelineCache : private NonCopyable<PipelineCache>, public VulkanResource
  {
  public:
    LAVA_API
    PipelineCache( const DeviceRef& device, vk::PipelineCacheCreateFlags flags,
      size_t initialSize, void const* initialData );
    LAVA_API
    ~PipelineCache( );

    LAVA_API
    std::vector<uint8_t> getData( void ) const;
    LAVA_API
    void merge( vk::ArrayProxy<const std::shared_ptr<PipelineCache>> srcCaches ) const;

    LAVA_API
    void saveToFile( const char* filename );

    inline operator vk::PipelineCache( void ) const
    {
      return _pipelineCache;
    }
  private:
    vk::PipelineCache _pipelineCache;
  };

  class PipelineLayout : public VulkanResource, private NonCopyable<PipelineLayout>
  {
  public:
    LAVA_API
    PipelineLayout( const std::shared_ptr<Device>& device, 
      vk::ArrayProxy<const std::shared_ptr<DescriptorSetLayout>> setLayouts,
      vk::ArrayProxy<const vk::PushConstantRange> pushConstantRanges );
    LAVA_API
    ~PipelineLayout( );
    
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
    Pipeline( const DeviceRef& device );
    void setPipeline( const vk::Pipeline & pipeline );

  protected:
    vk::Pipeline _pipeline;
  };

  struct SpecializationInfo
  {
    LAVA_API
    SpecializationInfo( vk::ArrayProxy<const vk::SpecializationMapEntry> mapEntries, 
      vk::ArrayProxy<const uint8_t> data );
    LAVA_API
    SpecializationInfo( SpecializationInfo const& rhs );
    LAVA_API
    SpecializationInfo & operator=( SpecializationInfo const& rhs );

    std::vector<vk::SpecializationMapEntry> mapEntries;
    std::vector<uint8_t>                    data;
  };
  struct PipelineShaderStageCreateInfo
  {
    LAVA_API
    PipelineShaderStageCreateInfo( vk::ShaderStageFlagBits stage, 
      std::shared_ptr<ShaderModule> const& module, std::string const& name,
      vk::Optional<const SpecializationInfo> specializationInfo = nullptr );
    LAVA_API
    PipelineShaderStageCreateInfo( PipelineShaderStageCreateInfo const& rhs );
    LAVA_API
    PipelineShaderStageCreateInfo & operator=( PipelineShaderStageCreateInfo const& rhs );

    vk::ShaderStageFlagBits             stage;
    std::shared_ptr<ShaderModule>       module;
    std::string                         name;
    std::unique_ptr<SpecializationInfo> specializationInfo;
  };
  class ComputePipeline : public Pipeline
  {
  public:
    LAVA_API
    ComputePipeline( const DeviceRef& device,
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
      std::shared_ptr<PipelineLayout> const& pipelineLayout,
      std::shared_ptr<RenderPass> const& renderPass,
      uint32_t subpass, std::shared_ptr<Pipeline> const& basePipelineHandle,
      uint32_t basePipelineIndex );
  };
}

#endif /* __LAVA_PIPELINE__ */