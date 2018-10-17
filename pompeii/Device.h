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

#ifndef __POMPEII_DEVICE__
#define __POMPEII_DEVICE__

#include <pompeii/api.h>

#include "includes.hpp"

#include <pompeii/noncopyable.hpp>
#include <memory>
#include <map>

namespace pompeii
{
  class Buffer;
  class BufferView;
  class CommandPool;
  class Event;
  class Fence;
  class Framebuffer;
  class Image;
  class ImageView;
  class PhysicalDevice;
  class Surface;
  class RenderPass;
  class Semaphore;
  class Sampler;
  class Swapchain;
  class Queue;
  class QueryPool;

  class UniformBuffer;
  class StorageBuffer;
  class StorageTexelBuffer;
  class UniformTexelBuffer;
  class UniformDynamicBuffer;
  class VertexBuffer;
  class IndexBuffer;
  class Texture2D;
  class Texture3D;
  class Texture2DArray;
  class TextureCubemap;

  class DescriptorSetLayout;
  struct DescriptorSetLayoutBinding;
  class DescriptorPool;
  class DescriptorSet;

  struct WriteDescriptorSet;
  struct CopyDescriptorSet;

  class Pipeline;
  class ComputePipeline;
  class PipelineLayout;

  class PipelineCache;
  struct PipelineShaderStageCreateInfo;
  struct PipelineVertexInputStateCreateInfo;
  struct PipelineViewportStateCreateInfo;
  struct PipelineMultisampleStateCreateInfo;
  struct PipelineColorBlendStateCreateInfo;
  struct PipelineDynamicStateCreateInfo;

  class ShaderModule;
  struct PipelineShaderStageCreateInfo;
  struct SpecializationInfo;
}

namespace pompeii
{
  uint32_t findMemoryType( const vk::PhysicalDeviceMemoryProperties& memProps,
    uint32_t reqBits, vk::MemoryPropertyFlags wantedFlags );
  uint32_t findMemoryTypeFromRequirementsWithFallback( 
    const vk::PhysicalDeviceMemoryProperties& memProps,
    uint32_t reqBits, vk::MemoryPropertyFlags wantedFlags );
  class Device :
    private NonCopyable<Device>,
    public std::enable_shared_from_this<Device>
  {
  public:
    POMPEII_API
    static std::shared_ptr<Device> create( 
      const std::shared_ptr<PhysicalDevice>& phyDev,
      const std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos,
      const std::vector<std::string>& enabledLayerNames,
      const std::vector<std::string>& enabledExtensionNames,
      const vk::PhysicalDeviceFeatures& enabledFeatures
    );
    POMPEII_API
    virtual ~Device( void );
    POMPEII_API
    vk::Result waitForFences( vk::ArrayProxy<const std::shared_ptr<Fence>> fences,
      bool waitAll, uint32_t timeout ) const;
    POMPEII_API
    inline operator vk::Device( void ) const
    {
      return _device;
    }
    POMPEII_API
    std::shared_ptr<PhysicalDevice> getPhysicalDevice( void ) const
    {
      return _physicalDevice;
    }

    POMPEII_API
    std::shared_ptr<Event> createEvent( void );

    POMPEII_API
    void waitIdle( void );

    POMPEII_API
    std::shared_ptr<Semaphore> createSemaphore( void );

    /**
    * Allocates memory for the provided image, and binds it to the image.
    * Returns null if it cannot find memory with the specified flags.
    */
    POMPEII_API
    vk::DeviceMemory allocateImageMemory( vk::Image image, 
      vk::MemoryPropertyFlags flags );

    /**
    * Allocates memory for the provided buffer, and binds it to the buffer.
    * Returns null if it cannot find memory with the specified flags.
    */
    POMPEII_API
    vk::DeviceMemory allocateBufferMemory( vk::Buffer buffer, 
      vk::MemoryPropertyFlags flags );

    /**
    * Allocates a block of memory according to the provided memory requirements.
    * Returns null if it cannot find memory with the specified flags.
    */
    POMPEII_API
    vk::DeviceMemory allocateMemReqMemory( const vk::MemoryRequirements& reqs,
      vk::MemoryPropertyFlags flags );

    // Frees a previously allocated block of memory.
    POMPEII_API
    void freeMemory( vk::DeviceMemory memory );

    POMPEII_API
    std::shared_ptr<Fence> createFence( bool signaled );
	POMPEII_API
	std::shared_ptr<Sampler> createSampler(vk::Filter magFilter,
		vk::Filter minFilter, vk::SamplerMipmapMode mipmapMode,
		vk::SamplerAddressMode addressModeU, vk::SamplerAddressMode addressModeV,
		vk::SamplerAddressMode addressModeW, float mipLodBias,
		bool anisotropyEnable, float maxAnisotropy, bool compareEnable,
		vk::CompareOp compareOp, float minLod, float maxLod,
		vk::BorderColor borderColor, bool unnormalizedCoordinates);
	POMPEII_API
	std::shared_ptr<Sampler> createSampler(const vk::SamplerCreateInfo& ci);

    POMPEII_API
    std::shared_ptr<CommandPool> createCommandPool( 
      vk::CommandPoolCreateFlags flags = { }, uint32_t familyIndex = 0 );

    POMPEII_API
    std::shared_ptr<DescriptorSetLayout> createDescriptorSetLayout(
      vk::ArrayProxy<const DescriptorSetLayoutBinding> bindings,
      vk::DescriptorSetLayoutCreateFlags flags = { } );
    POMPEII_API
    std::shared_ptr<DescriptorPool> createDescriptorPool(
      /* TODO: CHECK !vk::DescriptorPoolCreateFlags flags, */uint32_t maxSets,
      vk::ArrayProxy<const vk::DescriptorPoolSize> poolSizes );

    POMPEII_API
    std::shared_ptr<Image> createImage( vk::ImageCreateFlags createFlags, 
      vk::ImageType type, vk::Format format, const vk::Extent3D& extent, 
      uint32_t mipLevels, uint32_t arraySize, vk::SampleCountFlagBits samples, 
      vk::ImageTiling tiling, vk::ImageUsageFlags usageFlags, 
      vk::SharingMode sharingMode, const std::vector<uint32_t>& queueFamilyIdxs,
      vk::ImageLayout initialLayout, vk::MemoryPropertyFlags memoryPropFlags );


    POMPEII_API
    std::shared_ptr<Buffer> createBuffer( vk::BufferCreateFlags createFlags,
      vk::DeviceSize size, vk::BufferUsageFlags usageFlags =
      vk::BufferUsageFlagBits::eTransferDst,
      vk::SharingMode sharingMode = vk::SharingMode::eExclusive,
      vk::ArrayProxy<const uint32_t> queueFamilyIndices = nullptr,
      vk::MemoryPropertyFlags memoryPropertyFlags
      = vk::MemoryPropertyFlagBits::eDeviceLocal );
    POMPEII_API
    std::shared_ptr<Buffer> createBuffer( vk::DeviceSize size,
      vk::BufferUsageFlags usageFlags = vk::BufferUsageFlagBits::eTransferDst,
      vk::SharingMode sharingMode = vk::SharingMode::eExclusive,
      vk::ArrayProxy<const uint32_t> queueFamilyIndices = nullptr,
      vk::MemoryPropertyFlags memoryPropertyFlags
      = vk::MemoryPropertyFlagBits::eDeviceLocal );

    // Sharing Mode: Exclusive
    POMPEII_API
    std::shared_ptr<Buffer> createBuffer( vk::DeviceSize size,
      vk::BufferUsageFlags usageFlags, vk::MemoryPropertyFlags memPropFlags );
    // Sharing Mode: Concurrent
    POMPEII_API
    std::shared_ptr<Buffer> createBuffer( vk::DeviceSize size,
      vk::BufferUsageFlags usageFlags,
      vk::ArrayProxy<const uint32_t> queueFamilyIndices,
      vk::MemoryPropertyFlags memoryPropertyFlags );

    POMPEII_API
    std::shared_ptr<BufferView> createBufferView( std::shared_ptr<Buffer> buf,
      vk::Format format, vk::DeviceSize offset, vk::DeviceSize size );

    /*POMPEII_API
    std::shared_ptr<Swapchain> createSwapchain( 
      const std::shared_ptr<Surface>& surface, uint32_t numImageCount, 
      vk::Format imageFormat, vk::ColorSpaceKHR colorSpace,
      const vk::Extent2D& imageExtent, uint32_t imageArrayLayers,
      vk::ImageUsageFlags imageUsage, vk::SharingMode imageSharingMode,
      const std::vector<uint32_t>& queueFamilyIndices,
      vk::SurfaceTransformFlagBitsKHR preTransform,
      vk::CompositeAlphaFlagBitsKHR compositeAlpha,
      vk::PresentModeKHR presentMode, bool clipped,
      const std::shared_ptr<Swapchain>& oldSwapchain );*/

    POMPEII_API
    std::shared_ptr<DescriptorSet> allocateDescriptorSet(
      const std::shared_ptr<DescriptorPool>& pool,
      const std::shared_ptr<DescriptorSetLayout>& layout );
    POMPEII_API
    void updateDescriptorSets(
      std::vector<WriteDescriptorSet> descriptorWrites,
      std::vector<CopyDescriptorSet> descriptorCopies );
    POMPEII_API
    std::shared_ptr<Framebuffer> createFramebuffer( 
      const std::shared_ptr<RenderPass>& renderPass,
      vk::ArrayProxy<const std::shared_ptr<ImageView>> attachments,
      const vk::Extent2D& extent, uint32_t layers );

    POMPEII_API
    std::shared_ptr<RenderPass> createRenderPass(
      vk::ArrayProxy<const vk::AttachmentDescription> attachments,
      vk::ArrayProxy<const vk::SubpassDescription> subpasses,
      vk::ArrayProxy<const vk::SubpassDependency> dependencies );

    POMPEII_API
    size_t getQueueCount( uint32_t familyIndex ) const;
    POMPEII_API
    size_t getQueueFamilyCount( void ) const;
    POMPEII_API
    std::shared_ptr<Queue> getQueue( uint32_t familyIndex, uint32_t queueIdx );

    POMPEII_API
    std::shared_ptr<PipelineCache> createPipelineCache( size_t initialSize,
      void const* initialData );
    POMPEII_API
    std::shared_ptr<PipelineCache> createPipelineCache( 
      const std::string& filePath );

    POMPEII_API
    std::shared_ptr<Pipeline> createGraphicsPipeline(
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
      std::shared_ptr<RenderPass> const& renderPass, uint32_t subpass = 0,
      std::shared_ptr<Pipeline> const& basePipelineHandle = { },
      uint32_t basePipelineIndex = 0 );

    POMPEII_API
    std::shared_ptr<ComputePipeline> createComputePipeline(
      const std::shared_ptr<PipelineCache>& pipelineCache,
      vk::PipelineCreateFlags flags,
      const PipelineShaderStageCreateInfo& stage,
      const std::shared_ptr<PipelineLayout>& pipelineLayout,
      const std::shared_ptr<Pipeline>& basePipelineHandle = { },
      uint32_t basePipelineIndex = 0 );

    POMPEII_API
    std::shared_ptr<PipelineLayout> createPipelineLayout(
      vk::ArrayProxy<const std::shared_ptr<DescriptorSetLayout>> setLayouts,
      vk::ArrayProxy<const vk::PushConstantRange> pushConstantRanges = nullptr );


    POMPEII_API
    std::shared_ptr<ShaderModule> createShaderModule( 
      const std::string& filePath, vk::ShaderStageFlagBits type );
    POMPEII_API
    std::shared_ptr<ShaderModule> createShaderModule( 
      const std::string& filePath );
    POMPEII_API
    std::shared_ptr<ShaderModule> createShaderModule( 
      vk::ArrayProxy<const uint32_t> code );
    POMPEII_API
    const PipelineShaderStageCreateInfo createShaderPipelineShaderStage(
      const std::string& spvFile, vk::ShaderStageFlagBits stage,
      vk::Optional<const SpecializationInfo> specializationInfo = nullptr );

    POMPEII_API
    const PipelineShaderStageCreateInfo createShaderPipelineShaderStage(
      const std::shared_ptr<ShaderModule>& shaderModule, vk::ShaderStageFlagBits stage,
      vk::Optional<const SpecializationInfo> specializationInfo = nullptr );



#ifdef POMPEII_DEVICE_BUILDERS
    POMPEII_API
    const PipelineShaderStageCreateInfo createVertexShaderStage(
      const std::string& spvFile,
      vk::Optional<const SpecializationInfo> specializationInfo = nullptr );
    POMPEII_API
    const PipelineShaderStageCreateInfo createFragmentShaderStage(
      const std::string& spvFile,
      vk::Optional<const SpecializationInfo> specializationInfo = nullptr );
    POMPEII_API
    const PipelineShaderStageCreateInfo createTesselationControlShaderStage(
      const std::string& spvFile,
      vk::Optional<const SpecializationInfo> specializationInfo = nullptr );
    POMPEII_API
    const PipelineShaderStageCreateInfo createTesselationEvaluationShaderStage(
      const std::string& spvFile,
      vk::Optional<const SpecializationInfo> specializationInfo = nullptr );
    POMPEII_API
    const PipelineShaderStageCreateInfo createGeometryShaderStage(
      const std::string& spvFile,
      vk::Optional<const SpecializationInfo> specializationInfo = nullptr );
    POMPEII_API
    const PipelineShaderStageCreateInfo createComputeShaderStage(
      const std::string& spvFile,
      vk::Optional<const SpecializationInfo> specializationInfo = nullptr );
    POMPEII_API
    std::shared_ptr<UniformBuffer> createUniformBuffer( vk::DeviceSize size );
    POMPEII_API
    std::shared_ptr<StorageBuffer> createStorageBuffer( vk::DeviceSize size );
    POMPEII_API
    std::shared_ptr<StorageTexelBuffer> createStorageTexelBuffer( vk::DeviceSize size );
    POMPEII_API
    std::shared_ptr<UniformTexelBuffer> createUniformTexelBuffer( vk::DeviceSize size );
    POMPEII_API
    std::shared_ptr<VertexBuffer> createVertexBuffer( vk::DeviceSize size );
    POMPEII_API
    std::shared_ptr<IndexBuffer> createIndexBuffer( vk::IndexType type,
      vk::DeviceSize size );
    POMPEII_API
    std::shared_ptr<UniformDynamicBuffer> createUniformDynamicBuffer(
      vk::DeviceSize size, uint32_t count );

    POMPEII_API
    std::shared_ptr< Texture2D > createTexture2D( const std::string& textureSrc,
      std::shared_ptr<CommandPool> cmdPool, std::shared_ptr< Queue > queue,
        vk::Format format );
    POMPEII_API
    std::shared_ptr< Texture2DArray > createTexture2DArray(
    std::vector< std::string >& textureSrcs,
    std::shared_ptr<CommandPool> cmdPool, std::shared_ptr< Queue > queue,
    vk::Format format );
    POMPEII_API
    std::shared_ptr< TextureCubemap > createTextureCubemap(
      std::array< std::string, 6 >& cubeImages,
      std::shared_ptr<CommandPool> cmdPool, std::shared_ptr< Queue > queue,
      vk::Format format );
    POMPEII_API
    std::shared_ptr< Texture3D > createTexture3D( uint32_t width, uint32_t height, 
      uint32_t depth, const void* src, uint32_t size,
      const std::shared_ptr<CommandPool>& cmdPool,
      const std::shared_ptr<Queue>& queue, vk::Format format );
#endif
    POMPEII_API
    std::shared_ptr<QueryPool> createQuery( vk::QueryPoolCreateFlags flags, 
      vk::QueryType queryType, uint32_t entryCount, 
      vk::QueryPipelineStatisticFlags pipelineStatistics );

    POMPEII_API
    std::shared_ptr<QueryPool> createOcclusionQuery( uint32_t entryCount );

    POMPEII_API
    std::shared_ptr<QueryPool> createPipelineStatisticsQuery(
      uint32_t entryCount, vk::QueryPipelineStatisticFlags flags );
  protected:
    Device( const std::shared_ptr<PhysicalDevice>& phyDev );
    void init(
      const std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos,
      const std::vector<std::string>& enabledLayerNames,
      const std::vector<std::string>& enabledExtensionNames,
      const vk::PhysicalDeviceFeatures& enabledFeatures );
  protected:
    vk::Device _device;
    std::shared_ptr<PhysicalDevice> _physicalDevice;
    std::map<uint32_t, std::vector<std::unique_ptr<Queue>>> _queues;
  };
}

#endif /* __POMPEII_DEVICE__ */