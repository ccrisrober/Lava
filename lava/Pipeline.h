#ifndef __LAVA_PIPELINE__
#define __LAVA_PIPELINE__

#include "includes.hpp"
#include "VulkanResource.h"

#include "noncopyable.hpp"
#include "Descriptor.h"

namespace lava
{
  class Device;

  class ShaderModule : public VulkanResource, private NonCopyable<ShaderModule>
  {
  public:
    ShaderModule( const DeviceRef& device, const std::string& filePath );
    ShaderModule( const DeviceRef& device, vk::ArrayProxy<const uint32_t> code );
    ~ShaderModule( );

    inline operator vk::ShaderModule( ) const { return _shaderModule; }

  private:
    vk::ShaderModule _shaderModule;

    const std::vector<uint32_t> readFile( const std::string& filename );
  };

  struct PipelineVertexInputStateCreateInfo
  {
    PipelineVertexInputStateCreateInfo(
      vk::ArrayProxy<const vk::VertexInputBindingDescription> vertexBindingDescriptions,
      vk::ArrayProxy<const vk::VertexInputAttributeDescription> vertexAttributeDesriptions );
    PipelineVertexInputStateCreateInfo( const PipelineVertexInputStateCreateInfo& rhs );
    PipelineVertexInputStateCreateInfo & operator=( const PipelineVertexInputStateCreateInfo& rhs );

    std::vector<vk::VertexInputBindingDescription> vertexBindingDescriptions;
    std::vector<vk::VertexInputAttributeDescription> vertexAttributeDesriptions;
  };

  struct PipelineDynamicStateCreateInfo
  {
    PipelineDynamicStateCreateInfo( vk::ArrayProxy<const vk::DynamicState> dynamicStates );
    PipelineDynamicStateCreateInfo( const PipelineDynamicStateCreateInfo& rhs );
    PipelineDynamicStateCreateInfo& operator=( const PipelineDynamicStateCreateInfo& rhs );

    std::vector<vk::DynamicState> dynamicStates;
  };

  struct PipelineViewportStateCreateInfo
  {
    PipelineViewportStateCreateInfo(
      vk::ArrayProxy<const vk::Viewport> viewports_,
      vk::ArrayProxy<const vk::Rect2D> scissors_ );
    PipelineViewportStateCreateInfo( const PipelineViewportStateCreateInfo& rhs );
    PipelineViewportStateCreateInfo & operator=( const PipelineViewportStateCreateInfo& rhs );

    std::vector<vk::Viewport> viewports;
    std::vector<vk::Rect2D> scissors;
  };

  class PipelineCache : private NonCopyable<PipelineCache>, public VulkanResource
  {
  public:
    PipelineCache( const DeviceRef& device, vk::PipelineCacheCreateFlags flags,
      size_t initialSize, void const* initialData );
    ~PipelineCache( );

    std::vector<uint8_t> getData( ) const;
    void merge( vk::ArrayProxy<const std::shared_ptr<PipelineCache>> srcCaches ) const;

    inline operator vk::PipelineCache( ) const
    {
      return _pipelineCache;
    }
  private:
    vk::PipelineCache _pipelineCache;
  };

  class PipelineLayout : public VulkanResource, private NonCopyable<PipelineLayout>
  {
  public:
    PipelineLayout( const std::shared_ptr<Device>& device, vk::ArrayProxy<const std::shared_ptr<DescriptorSetLayout>> setLayouts,
      vk::ArrayProxy<const vk::PushConstantRange> pushConstantRanges );
    ~PipelineLayout( );

    inline operator vk::PipelineLayout( ) const
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
    SpecializationInfo( vk::ArrayProxy<const vk::SpecializationMapEntry> mapEntries, vk::ArrayProxy<const uint8_t> data );
    SpecializationInfo( SpecializationInfo const& rhs );
    SpecializationInfo & operator=( SpecializationInfo const& rhs );

    std::vector<vk::SpecializationMapEntry> mapEntries;
    std::vector<uint8_t>                    data;
  };
  struct PipelineShaderStageCreateInfo
  {
    PipelineShaderStageCreateInfo( vk::ShaderStageFlagBits stage, std::shared_ptr<ShaderModule> const& module, std::string const& name,
      vk::Optional<const SpecializationInfo> specializationInfo = nullptr );
    PipelineShaderStageCreateInfo( PipelineShaderStageCreateInfo const& rhs );
    PipelineShaderStageCreateInfo & operator=( PipelineShaderStageCreateInfo const& rhs );

    vk::ShaderStageFlagBits             stage;
    std::shared_ptr<ShaderModule>       module;
    std::string                         name;
    std::unique_ptr<SpecializationInfo> specializationInfo;
  };
  class ComputePipeline : public Pipeline
  {
  public:
    ComputePipeline( const DeviceRef& device,
      const std::shared_ptr<PipelineCache>& pipelineCache,
      vk::PipelineCreateFlags flags,
      const PipelineShaderStageCreateInfo& stage,
      const std::shared_ptr<PipelineLayout>& layout,
      const std::shared_ptr<Pipeline>& basePipelineHandle,
      int32_t basePipelineIndex );
  };

  class GraphicsPipeline : public Pipeline
  {
  public:
    GraphicsPipeline( const DeviceRef& device,
      const std::shared_ptr<PipelineCache>& pipelineCache,
      vk::Optional<const PipelineVertexInputStateCreateInfo> vertexInputState,
      vk::Optional<const vk::PipelineInputAssemblyStateCreateInfo> inputAssemblyState,
      vk::Optional<const PipelineViewportStateCreateInfo> viewportState,
      vk::Optional<const PipelineDynamicStateCreateInfo> dynamicState,
      vk::Optional<const vk::PipelineRasterizationStateCreateInfo> rasterizationState,
      vk::Optional<const vk::PipelineMultisampleStateCreateInfo> multisampleState,
      vk::Optional<const vk::PipelineDepthStencilStateCreateInfo> depthStencilState );
  };
}

#endif /* __LAVA_PIPELINE__ */