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

#include "Device.h"

#include <pompeii/Log.h>

#include <pompeii/Buffer.h>
#include <pompeii/CommandBuffer.h>
#include <pompeii/Descriptor.h>
#include <pompeii/Image.h>
#include <pompeii/Event.h>
#include <pompeii/Fence.h>
#include <pompeii/Framebuffer.h>
#include <pompeii/Pipeline.h>
#include <pompeii/PhysicalDevice.h>
#include <pompeii/Queue.h>
#include <pompeii/QueryPool.h>
#include <pompeii/RenderPass.h>
#include <pompeii/Semaphore.h>
#include <pompeii/Swapchain.h>
#include <pompeii/Texture1D.h>
#include <pompeii/Texture2D.h>
#include <pompeii/Texture2DArray.h>
#include <pompeii/TextureCubemap.h>
#include <pompeii/Texture3D.h>

namespace pompeii
{
  uint32_t findMemoryType( const vk::PhysicalDeviceMemoryProperties& memProps,
    uint32_t requirementBits, vk::MemoryPropertyFlags wantedFlags )
  {
    for ( uint32_t i = 0; i < memProps.memoryTypeCount; ++i )
    {
      if ( requirementBits & ( 1u << i ) )
      {
        if ( ( memProps.memoryTypes[ i ].propertyFlags &
          wantedFlags ) == wantedFlags )
          return i;
      }
    }
    return -1;
  }

  uint32_t findMemoryTypeFromRequirementsWithFallback(
    const vk::PhysicalDeviceMemoryProperties& memProps,
    uint32_t requirementBits, vk::MemoryPropertyFlags wantedFlags )
  {
    for ( uint32_t i = 0; i < memProps.memoryTypeCount; ++i )
    {
      if ( requirementBits & ( 1u << i ) )
      {
        if ( ( memProps.memoryTypes[ i ].propertyFlags &
          wantedFlags ) == wantedFlags )
          return i;
      }
    }

    // If we cannot find the particular memory type we're looking for,
    //    just pick the first one available.
    if ( wantedFlags != vk::MemoryPropertyFlagBits( ) )
    {
      return findMemoryType( memProps, requirementBits, { } );
    }
    else
    {
      abort( );
    }
  }

  std::shared_ptr<Device> Device::create(
    const std::shared_ptr<PhysicalDevice>& phyDev,
    const std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos,
    const std::vector<std::string>& enabledLayerNames,
    const std::vector<std::string>& enabledExtensionNames,
    const vk::PhysicalDeviceFeatures& enabledFeatures )
  {
    auto dev = std::shared_ptr<Device>( new Device( phyDev ) );
    dev->init(
      queueCreateInfos,
      enabledLayerNames, enabledExtensionNames, enabledFeatures );

    return dev;
  }

  Device::~Device( void )
  {
    //_queues.clear();
    _device.destroy( );
  }
  vk::Result Device::waitForFences(
    vk::ArrayProxy<const std::shared_ptr<Fence>> fences, bool waitAll,
    uint32_t timeout ) const
  {
    std::vector<vk::Fence> vfences;
    vfences.reserve( fences.size( ) );
    for ( auto const& f : fences )
    {
      vfences.push_back( *f );
    }
    return _device.waitForFences( vfences, waitAll, timeout );
  }
  
  std::shared_ptr<Event> Device::createEvent( void )
  {
    return std::make_shared< Event >( shared_from_this( ) );
  }

  void Device::waitIdle( void )
  {
    _device.waitIdle( );
  }

  std::shared_ptr<Semaphore> Device::createSemaphore( void )
  {
    return std::make_shared<Semaphore>( shared_from_this( ) );
  }

  vk::DeviceMemory Device::allocateImageMemory( vk::Image image, 
    vk::MemoryPropertyFlags flags )
  {
    auto memReqs = _device.getImageMemoryRequirements( image );
    auto mem = allocateMemReqMemory( memReqs, flags );

    _device.bindImageMemory( image, mem, 0 );

    return mem;
  }
  vk::DeviceMemory Device::allocateBufferMemory( 
    vk::Buffer buffer, vk::MemoryPropertyFlags flags )
  {
    auto memReqs = _device.getBufferMemoryRequirements( buffer );
    auto mem = allocateMemReqMemory( memReqs, flags );

    _device.bindBufferMemory( buffer, mem, 0 );

    return mem;
  }
  vk::DeviceMemory Device::allocateMemReqMemory( 
    const vk::MemoryRequirements & reqs, vk::MemoryPropertyFlags flags )
  {
    vk::MemoryAllocateInfo allocateInfo(
      reqs.size,
      findMemoryType( _physicalDevice->getMemoryProperties( ), 
        reqs.memoryTypeBits, flags )
    );

    if ( allocateInfo.memoryTypeIndex == uint32_t( -1 ) )
      throw; //return VK_NULL_HANDLE;

    vk::DeviceMemory memory = _device.allocateMemory( allocateInfo );

    return memory;
  }
  void Device::freeMemory( vk::DeviceMemory memory )
  {
    _device.freeMemory( memory );
  }
  std::shared_ptr<Fence> Device::createFence( bool signaled )
  {
    return std::make_shared<Fence>( shared_from_this( ), signaled );
  }
  std::shared_ptr<Sampler> Device::createSampler( vk::Filter magFilter,
    vk::Filter minFilter, vk::SamplerMipmapMode mipmapMode,
    vk::SamplerAddressMode addressModeU, vk::SamplerAddressMode addressModeV,
    vk::SamplerAddressMode addressModeW, float mipLodBias,
    bool anisotropyEnable, float maxAnisotropy, bool compareEnable,
    vk::CompareOp compareOp, float minLod, float maxLod,
    vk::BorderColor borderColor, bool unnormalizedCoordinates )
  {
    return std::make_shared<Sampler>( shared_from_this( ), magFilter, minFilter,
      mipmapMode, addressModeU, addressModeV, addressModeW, mipLodBias,
      anisotropyEnable, maxAnisotropy, compareEnable, compareOp, minLod, maxLod,
      borderColor, unnormalizedCoordinates );
  }
  std::shared_ptr<Sampler> Device::createSampler(const vk::SamplerCreateInfo& ci)
  {
	  return std::make_shared<Sampler>(shared_from_this(), ci);
  }
  std::shared_ptr<CommandPool> Device::createCommandPool(
    vk::CommandPoolCreateFlags flags, uint32_t famIdx )
  {
    return std::make_shared<CommandPool>( shared_from_this( ), flags, famIdx );
  }
  std::shared_ptr<DescriptorSetLayout> Device::createDescriptorSetLayout(
    vk::ArrayProxy<const DescriptorSetLayoutBinding> bindings,
    vk::DescriptorSetLayoutCreateFlags flags )
  {
    return std::make_shared<DescriptorSetLayout>( shared_from_this( ), 
      bindings, flags );
  }
  std::shared_ptr<DescriptorPool> Device::createDescriptorPool(
    /*vk::DescriptorPoolCreateFlags flags, */uint32_t maxSets,
    vk::ArrayProxy<const vk::DescriptorPoolSize> poolSizes )
  {
    return std::make_shared<DescriptorPool>( shared_from_this( ),
      vk::DescriptorPoolCreateFlags( ), maxSets, poolSizes );
  }
  std::shared_ptr<Image> Device::createImage( vk::ImageCreateFlags createFlags,
    vk::ImageType type, vk::Format format, const vk::Extent3D & extent,
    uint32_t mipLevels, uint32_t arraySize, vk::SampleCountFlagBits samples,
    vk::ImageTiling tiling, vk::ImageUsageFlags usageFlags,
    vk::SharingMode sharingMode, const std::vector<uint32_t>& queueFamilyIndices,
    vk::ImageLayout initialLayout, vk::MemoryPropertyFlags memoryPropertyFlags )
  {
    return std::make_shared<Image>( shared_from_this( ), createFlags, type,
      format, extent, mipLevels, arraySize, samples, tiling, usageFlags,
      sharingMode, queueFamilyIndices, initialLayout, memoryPropertyFlags );
  }
  std::shared_ptr<Buffer> Device::createBuffer(
    vk::BufferCreateFlags createFlags, vk::DeviceSize size,
    vk::BufferUsageFlags usageFlags, vk::SharingMode sharingMode,
    vk::ArrayProxy<const uint32_t> queueFamilyIndices,
    vk::MemoryPropertyFlags memoryPropertyFlags )
  {
    return std::make_shared<Buffer>( shared_from_this( ), createFlags, size,
      usageFlags, sharingMode, queueFamilyIndices, memoryPropertyFlags );
  }

  std::shared_ptr<Buffer> Device::createBuffer( vk::DeviceSize size,
    vk::BufferUsageFlags usageFlags, vk::SharingMode sharingMode,
    vk::ArrayProxy<const uint32_t> queueFamilyIndices,
    vk::MemoryPropertyFlags memoryPropertyFlags )
  {
    return std::make_shared<Buffer>( shared_from_this( ),
      vk::BufferCreateFlags( ), size, usageFlags, sharingMode,
      queueFamilyIndices, memoryPropertyFlags );
  }

  std::shared_ptr<Buffer> Device::createBuffer( vk::DeviceSize size,
    vk::BufferUsageFlags usageFlags, vk::MemoryPropertyFlags memPropFlags )
  {
    return createBuffer( size, usageFlags, vk::SharingMode::eExclusive,
      nullptr, memPropFlags );
  }

  std::shared_ptr<Buffer> Device::createBuffer( vk::DeviceSize size,
    vk::BufferUsageFlags usageFlags,
    vk::ArrayProxy<const uint32_t> queueFamilyIndices,
    vk::MemoryPropertyFlags memPropFlags )
  {
    return createBuffer( size, usageFlags, vk::SharingMode::eConcurrent,
      queueFamilyIndices, memPropFlags );
  }

  std::shared_ptr<BufferView> Device::createBufferView( 
    std::shared_ptr<Buffer> buffer, vk::Format format, vk::DeviceSize offset,
    vk::DeviceSize size )
  {
    return std::make_shared<BufferView>( buffer, format, offset, size );
  }

  /*std::shared_ptr<Swapchain> Device::createSwapchain( 
    const std::shared_ptr<Surface>& surface, uint32_t numImageCount, 
    vk::Format imageFormat, vk::ColorSpaceKHR colorSpace, 
    const vk::Extent2D& imageExtent, uint32_t imageArrayLayers, 
    vk::ImageUsageFlags imageUsage, vk::SharingMode imageSharingMode, 
    const std::vector<uint32_t>& queueFamilyIndices,
    vk::SurfaceTransformFlagBitsKHR preTransform, 
    vk::CompositeAlphaFlagBitsKHR compAlpha, vk::PresentModeKHR presentMode, 
    bool clipped, const std::shared_ptr<Swapchain>& oldSwapchain )
  {
    return std::make_shared<Swapchain>( shared_from_this( ), surface,
      numImageCount, imageFormat, colorSpace, imageExtent, imageArrayLayers,
      imageUsage, imageSharingMode, queueFamilyIndices, preTransform,
      compAlpha, presentMode, clipped, oldSwapchain );
  }*/
  std::shared_ptr<DescriptorSet> Device::allocateDescriptorSet(
    const std::shared_ptr<DescriptorPool>& pool,
    const std::shared_ptr<DescriptorSetLayout>& layout )
  {
    return std::make_shared<DescriptorSet>( shared_from_this( ), pool, layout );
  }

  void Device::updateDescriptorSets(
    std::vector<WriteDescriptorSet> descriptorWrites,
    std::vector<CopyDescriptorSet> descriptorCopies )
  {
    std::vector<std::unique_ptr<vk::DescriptorImageInfo>> diis;
    diis.reserve( descriptorWrites.size( ) );

    std::vector<std::unique_ptr<vk::DescriptorBufferInfo>> dbis;
    dbis.reserve( descriptorWrites.size( ) );

    std::vector<vk::WriteDescriptorSet> writes;
    writes.reserve( descriptorWrites.size( ) );
    for ( const auto& w : descriptorWrites )
    {
      diis.push_back( std::unique_ptr<vk::DescriptorImageInfo>(
        w.imageInfo ? new vk::DescriptorImageInfo(
          w.imageInfo->sampler ?
          static_cast<vk::Sampler>( *w.imageInfo->sampler ) : nullptr,
          w.imageInfo->imageView ?
          static_cast<vk::ImageView>( *w.imageInfo->imageView ) : nullptr,
          w.imageInfo->imageLayout ) : nullptr ) );
      dbis.push_back( std::unique_ptr<vk::DescriptorBufferInfo>(
        w.bufferInfo ? new vk::DescriptorBufferInfo( w.bufferInfo->buffer ?
          static_cast<vk::Buffer>( *w.bufferInfo->buffer ) : nullptr,
          w.bufferInfo->offset, w.bufferInfo->range ) : nullptr ) );
      vk::WriteDescriptorSet write(
        w.dstSet ? static_cast<vk::DescriptorSet>( *w.dstSet ) : nullptr,
        w.dstBinding,
        w.dstArrayElement,
        w.descriptorCount,
        w.descriptorType,
        diis.back( ).get( ),
        dbis.back( ).get( )
      );

      if ( w.texelBufferView )
      {
        auto bufferView = static_cast< vk::BufferView >( *w.texelBufferView );
        // TODO (LINUX FAILED) auto bb = static_cast< VkBufferView >( bufferView );
        write.setPTexelBufferView( &bufferView );
      }

      writes.push_back( std::move( write ) );
    }

    std::vector<vk::CopyDescriptorSet> copies;
    copies.reserve( descriptorCopies.size( ) );
    for ( auto const& c : descriptorCopies )
    {
      copies.push_back(
        vk::CopyDescriptorSet(
          c.srcSet ? static_cast<vk::DescriptorSet>( *c.srcSet ) : nullptr,
          c.srcBinding,
          c.srcArrayElement,
          c.dstSet ? static_cast<vk::DescriptorSet>( *c.dstSet ) : nullptr,
          c.dstBinding,
          c.dstArrayElement,
          c.descriptorCount
        )
      );
    }

    _device.updateDescriptorSets( writes, copies );
  }
  std::shared_ptr<Framebuffer> Device::createFramebuffer(
    const std::shared_ptr<RenderPass>& renderPass,
    vk::ArrayProxy<const std::shared_ptr<ImageView>> attachments,
    const vk::Extent2D & extent, uint32_t layers )
  {
    return std::make_shared<Framebuffer>( shared_from_this( ), renderPass,
      attachments, extent, layers );
  }
  std::shared_ptr<RenderPass> Device::createRenderPass(
    vk::ArrayProxy<const vk::AttachmentDescription> attachments,
    vk::ArrayProxy<const vk::SubpassDescription> subpasses,
    vk::ArrayProxy<const vk::SubpassDependency> dependencies )
  {
    return std::make_shared<RenderPass>( shared_from_this( ), attachments,
      subpasses, dependencies );
  }
  size_t Device::getQueueCount( uint32_t familyIndex ) const
  {
    auto it = _queues.find( familyIndex );
    return it != _queues.end( ) ? it->second.size( ) : 0;
  }
  size_t Device::getQueueFamilyCount( void ) const
  {
    return _queues.size( );
  }
  std::shared_ptr<Queue> Device::getQueue( uint32_t familyIndex, 
    uint32_t queueIndex )
  {
    auto it = _queues.find( familyIndex );
    assert( it != _queues.end( ) && "invalid queue family index" );
    assert( queueIndex < it->second.size( ) );

    return std::shared_ptr<Queue>( shared_from_this( ), 
      it->second[ queueIndex ].get( ) );
  }
  std::shared_ptr<PipelineCache> Device::createPipelineCache( size_t initialSize,
    void const * initialData )
  {
    return std::make_shared<PipelineCache>( shared_from_this( ),
      vk::PipelineCacheCreateFlags( ), initialSize, initialData );
  }
  std::shared_ptr<PipelineCache> Device::createPipelineCache( 
    const std::string& src )
  {
    return std::make_shared<PipelineCache>( shared_from_this( ), src );
  }
  std::shared_ptr<Pipeline> Device::createGraphicsPipeline(
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
    std::shared_ptr<RenderPass> const& renderPass, uint32_t subpass,
    std::shared_ptr<Pipeline> const& basePipelineHandle,
    uint32_t basePipelineIndex )
  {
    return std::make_shared<GraphicsPipeline>( shared_from_this( ), pipelineCache,
      flags, stages, vertexInputState, inputAssemblyState, tessellationState,
      viewportState, rasterizationState, multisampleState, depthStencilState,
      colorBlendState, dynamicState, pipelineLayout, renderPass, subpass,
      basePipelineHandle, basePipelineIndex );
  }

  std::shared_ptr<ComputePipeline> Device::createComputePipeline(
    const std::shared_ptr<PipelineCache>& pipelineCache,
    vk::PipelineCreateFlags flags,
    const PipelineShaderStageCreateInfo& stage,
    std::shared_ptr<PipelineLayout> const& pipelineLayout,
    std::shared_ptr<Pipeline> const& basePipelineHandle,
    uint32_t basePipelineIndex )
  {
    return std::make_shared<ComputePipeline>( shared_from_this( ), pipelineCache,
      flags, stage, pipelineLayout, basePipelineHandle, basePipelineIndex );
  }

  std::shared_ptr<PipelineLayout> Device::createPipelineLayout(
    vk::ArrayProxy<const std::shared_ptr<DescriptorSetLayout>> setLayouts,
    vk::ArrayProxy<const vk::PushConstantRange> pushConstantRanges )
  {
    return std::make_shared<PipelineLayout>( shared_from_this( ),
      setLayouts, pushConstantRanges );
  }

  std::shared_ptr<ShaderModule> Device::createShaderModule(
    const std::string& filePath, vk::ShaderStageFlagBits type )
  {
    return std::make_shared<ShaderModule>( shared_from_this( ), filePath, type );
  }
  std::shared_ptr<ShaderModule> Device::createShaderModule( 
    const std::string& filePath )
  {
    return std::make_shared<ShaderModule>( shared_from_this( ), filePath );
  }
  std::shared_ptr<ShaderModule> Device::createShaderModule( 
    vk::ArrayProxy<const uint32_t> code )
  {
    return std::make_shared<ShaderModule>( shared_from_this( ), code );
  }
  const PipelineShaderStageCreateInfo Device::createShaderPipelineShaderStage(
    const std::string& spvFile, vk::ShaderStageFlagBits stage,
    vk::Optional<const SpecializationInfo> specInfo )
  {
    auto shaderModule = createShaderModule( spvFile, stage );
    return PipelineShaderStageCreateInfo( stage, shaderModule, "main", specInfo );
  }
  const PipelineShaderStageCreateInfo Device::createShaderPipelineShaderStage(
    const std::shared_ptr<ShaderModule>& shaderModule, vk::ShaderStageFlagBits stage,
    vk::Optional<const SpecializationInfo> specInfo )
  {
    return PipelineShaderStageCreateInfo( stage, shaderModule, "main", specInfo );
  }
#ifdef POMPEII_DEVICE_BUILDERS
  const PipelineShaderStageCreateInfo Device::createVertexShaderStage( 
    const std::string & spvFile, vk::Optional<const SpecializationInfo> specInfo )
  {
    return createShaderPipelineShaderStage( spvFile, 
      vk::ShaderStageFlagBits::eVertex, specInfo );
  }
  const PipelineShaderStageCreateInfo Device::createFragmentShaderStage( 
    const std::string & spvFile, vk::Optional<const SpecializationInfo> specInfo )
  {
    return createShaderPipelineShaderStage( spvFile,
      vk::ShaderStageFlagBits::eFragment, specInfo );
  }
  const PipelineShaderStageCreateInfo Device::createTesselationControlShaderStage( 
    const std::string & spvFile, vk::Optional<const SpecializationInfo> specInfo )
  {
    return createShaderPipelineShaderStage( spvFile,
      vk::ShaderStageFlagBits::eTessellationControl, specInfo );
  }
  const PipelineShaderStageCreateInfo Device::createTesselationEvaluationShaderStage( 
    const std::string & spvFile, vk::Optional<const SpecializationInfo> specInfo )
  {
    return createShaderPipelineShaderStage( spvFile,
      vk::ShaderStageFlagBits::eTessellationEvaluation, specInfo );
  }
  const PipelineShaderStageCreateInfo Device::createGeometryShaderStage( 
    const std::string & spvFile, vk::Optional<const SpecializationInfo> specInfo )
  {
    return createShaderPipelineShaderStage( spvFile,
      vk::ShaderStageFlagBits::eGeometry, specInfo );
  }
  const PipelineShaderStageCreateInfo Device::createComputeShaderStage( 
    const std::string & spvFile, vk::Optional<const SpecializationInfo> specInfo )
  {
    return createShaderPipelineShaderStage( spvFile,
      vk::ShaderStageFlagBits::eCompute, specInfo );
  }
#endif
#ifdef POMPEII_DEVICE_BUILDERS
  std::shared_ptr<UniformBuffer> Device::createUniformBuffer( vk::DeviceSize size )
  {
    return std::make_shared<pompeii::UniformBuffer>( shared_from_this( ), size );
  }
  std::shared_ptr<StorageBuffer> Device::createStorageBuffer( vk::DeviceSize size )
  {
    return std::make_shared<StorageBuffer>( shared_from_this( ), size );
  }
  std::shared_ptr<StorageTexelBuffer> Device::Device::createStorageTexelBuffer( 
    vk::DeviceSize size )
  {
    return std::make_shared<StorageTexelBuffer>( shared_from_this( ), size );
  }
  std::shared_ptr<UniformTexelBuffer> Device::Device::createUniformTexelBuffer( 
    vk::DeviceSize size )
  {
    return std::make_shared<UniformTexelBuffer>( shared_from_this( ), size );
  }
  std::shared_ptr<VertexBuffer> Device::Device::createVertexBuffer( 
    vk::DeviceSize size )
  {
    return std::make_shared<VertexBuffer>( shared_from_this( ), size );
  }
  std::shared_ptr<IndexBuffer> Device::Device::createIndexBuffer( 
    vk::IndexType type, vk::DeviceSize size )
  {
    return std::make_shared<IndexBuffer>( shared_from_this( ), type, size );
  }
  std::shared_ptr<UniformDynamicBuffer> Device::createUniformDynamicBuffer( 
    vk::DeviceSize size, uint32_t count )
  {
    return std::make_shared<UniformDynamicBuffer>( shared_from_this( ), size, count );
  }
  std::shared_ptr<Texture2D> Device::createTexture2D(
    const std::string& textureSrc, std::shared_ptr<CommandPool> cmdPool,
    std::shared_ptr<Queue> queue, vk::Format format )
  {
    return std::make_shared<Texture2D>( shared_from_this( ), textureSrc,
      cmdPool, queue, format );
  }
  std::shared_ptr<Texture2DArray> Device::createTexture2DArray(
    std::vector<std::string>& textureSrcs,
    std::shared_ptr<CommandPool> cmdPool, std::shared_ptr<Queue> queue,
    vk::Format format )
  {
    return std::make_shared<Texture2DArray>( shared_from_this( ), textureSrcs,
      cmdPool, queue, format );
  }
  std::shared_ptr<TextureCubemap> Device::createTextureCubemap(
    std::array<std::string, 6>& cubeImages, std::shared_ptr<CommandPool> cmdPool,
    std::shared_ptr<Queue> queue, vk::Format format )
  {
    return std::make_shared<TextureCubemap>( shared_from_this( ), cubeImages,
      cmdPool, queue, format );
  }

  std::shared_ptr< Texture3D > Device::createTexture3D( 
    uint32_t width, uint32_t height, uint32_t depth, const void* src, uint32_t size,
    const std::shared_ptr<CommandPool>& cmdPool,
    const std::shared_ptr<Queue>& queue, vk::Format format )
  {
    return std::make_shared<Texture3D>( shared_from_this( ), width, height, 
      depth, src, size, cmdPool, queue, format );
  }
#endif
  std::shared_ptr<QueryPool> Device::createQuery( vk::QueryPoolCreateFlags flags,
    vk::QueryType queryType, uint32_t entryCount,
    vk::QueryPipelineStatisticFlags pipelineStatistics )
  {
    return std::make_shared<QueryPool>( shared_from_this( ),
      flags, queryType, entryCount, pipelineStatistics );
  }
  std::shared_ptr<QueryPool> Device::createOcclusionQuery( uint32_t entryCount )
  {
    return createQuery( vk::QueryPoolCreateFlags( ),
      vk::QueryType::eOcclusion, entryCount, 
      vk::QueryPipelineStatisticFlags( ) );
  }

  std::shared_ptr<QueryPool> Device::createPipelineStatisticsQuery(
    uint32_t entryCount, vk::QueryPipelineStatisticFlags flags )
  {
    return createQuery( vk::QueryPoolCreateFlags( ), 
      vk::QueryType::ePipelineStatistics, entryCount, flags );
  }

  Device::Device( const std::shared_ptr<PhysicalDevice>& phyDev )
    : _physicalDevice( phyDev )
  {
  }

  void Device::init(
    const std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos,
    const std::vector<std::string>& enabledLayerNames,
    const std::vector<std::string>& enabledExtensionNames,
    const vk::PhysicalDeviceFeatures& enabledFeatures )
  {
    std::vector<vk::DeviceQueueCreateInfo> queueCIs;
    queueCIs.reserve( queueCreateInfos.size( ) );
    for ( auto const& ci : queueCreateInfos )
    {
      queueCIs.push_back( ci );
    }

    std::vector<char const*> layers;
    layers.reserve( enabledLayerNames.size( ) );
    for ( auto const& s : enabledLayerNames )
    {
      layers.push_back( s.c_str( ) );
    }
    std::vector<const char*> extensions;
    extensions.reserve( enabledExtensionNames.size( ) );
    for ( auto const& s : enabledExtensionNames )
    {
      extensions.push_back( s.c_str( ) );
    }
    vk::DeviceCreateInfo dci(
    { },
      queueCIs.size( ),
      queueCIs.data( ),
      layers.size( ),
      layers.data( ),
      extensions.size( ),
      extensions.data( ),
      &enabledFeatures );
    _device = vk::PhysicalDevice( *_physicalDevice ).createDevice( dci );

    for ( auto const& ci : queueCreateInfos )
    {
      std::vector<std::unique_ptr<Queue>> queues;
      for ( uint32_t qi = 0; qi < ci.queueCount; ++qi )
      {
        vk::Queue queue = _device.getQueue( ci.queueFamilyIndex, qi );
        queues.push_back( std::move( std::unique_ptr<Queue>(
          new Queue( shared_from_this( ), queue, ci.queueFamilyIndex ) ) ) );
      }
      auto it = _queues.emplace( ci.queueFamilyIndex, std::move( queues ) );
      assert( it.second && "duplicate queueFamilyIndex" );
    }
    Log::info( "Device OK" );
  }
}
