#include "Device.h"
#include "PhysicalDevice.h"

#include <assert.h>

namespace lava
{
  uint32_t findMemoryType( const vk::PhysicalDeviceMemoryProperties& memoryProperties,
    uint32_t requirementBits,
    vk::MemoryPropertyFlags wantedFlags )
  {
    for ( uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i )
    {
      if ( requirementBits & ( 1 << i ) )
      {
        if ( ( memoryProperties.memoryTypes[ i ].propertyFlags &
          wantedFlags ) == wantedFlags )
          return i;
      }
    }

    return -1;
  }
  DeviceRef Device::create( const std::shared_ptr<PhysicalDevice>& phyDev,
    const std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos,
    const std::vector<std::string>& enabledLayerNames,
    const std::vector<std::string>& enabledExtensionNames,
    const vk::PhysicalDeviceFeatures& enabledFeatures )
  {
    DeviceRef dev = std::make_shared<Device>( phyDev );
    dev->init(
      queueCreateInfos,
      enabledLayerNames, enabledExtensionNames, enabledFeatures );

    return dev;
  }
  Device::Device( const std::shared_ptr<PhysicalDevice>& phyDev )
    : _physicalDevice( phyDev )
  {

  }
  Device::~Device( void )
  {
    //_queues.clear();
    _device.destroy( );
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
    {},
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
          new Queue( shared_from_this( ), queue ) ) ) );
      }
      auto it = _queues.emplace( ci.queueFamilyIndex, std::move( queues ) );
      assert( it.second && "duplicate queueFamilyIndex" );
    }
    std::cout << "Device OK" << std::endl;
  }
  std::shared_ptr<Semaphore> Device::createSemaphore( void )
  {
    return std::make_shared<Semaphore>( shared_from_this( ) );
  }

  std::shared_ptr<RenderPass> Device::createRenderPass(
    vk::ArrayProxy<const vk::AttachmentDescription> attachments,
    vk::ArrayProxy<const vk::SubpassDescription> subpasses,
    vk::ArrayProxy<const vk::SubpassDependency> dependencies )
  {
    return std::make_shared<RenderPass>( shared_from_this( ), attachments,
      subpasses, dependencies );
  }

  std::shared_ptr<Queue> Device::getQueue( uint32_t familyIndex, uint32_t queueIndex )
  {
    auto it = _queues.find( familyIndex );
    assert( it != _queues.end( ) && "invalid queue family index" );
    assert( queueIndex < it->second.size( ) );

    return std::shared_ptr<Queue>( shared_from_this( ), it->second[ queueIndex ].get( ) );
  }

  std::shared_ptr<Swapchain> Device::createSwapchain( const std::shared_ptr<Surface>& surface,
    uint32_t numImageCount, vk::Format imageFormat, vk::ColorSpaceKHR colorSpace,
    const vk::Extent2D& imageExtent, uint32_t imageArrayLayers, vk::ImageUsageFlags imageUsage,
    vk::SharingMode imageSharingMode, const std::vector<uint32_t>& queueFamilyIndices,
    vk::SurfaceTransformFlagBitsKHR preTransform, vk::CompositeAlphaFlagBitsKHR compAlpha,
    vk::PresentModeKHR presentMode, bool clipped, const std::shared_ptr<Swapchain>& oldSwapchain )
  {
    return std::make_shared<Swapchain>( shared_from_this( ), surface, 
      numImageCount, imageFormat, colorSpace, imageExtent, imageArrayLayers, 
      imageUsage, imageSharingMode, queueFamilyIndices, preTransform,
      compAlpha, presentMode, clipped, oldSwapchain );
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
  std::shared_ptr<Framebuffer> Device::createFramebuffer( 
    const std::shared_ptr<RenderPass>& renderPass, 
    const std::vector<std::shared_ptr<ImageView>>& attachments, 
    const vk::Extent2D & extent, uint32_t layers )
  {
    return std::make_shared<Framebuffer>( shared_from_this( ), renderPass, 
      attachments, extent, layers );
  }
  std::shared_ptr<CommandPool> Device::createCommandPool( 
    vk::CommandPoolCreateFlags flags, uint32_t familyIdx )
  {
    return std::make_shared<CommandPool>( shared_from_this( ), flags, familyIdx );
  }
  std::shared_ptr<ShaderModule> Device::createShaderModule( 
    const std::string& filePath, vk::ShaderStageFlagBits type )
  {
    return std::make_shared<ShaderModule>( shared_from_this( ), filePath, type );
  }
  std::shared_ptr<ShaderModule> Device::createShaderModule( const std::string& filePath )
  {
    return std::make_shared<ShaderModule>( shared_from_this( ), filePath );
  }
  std::shared_ptr<ShaderModule> Device::createShaderModule( vk::ArrayProxy<const uint32_t> code )
  {
    return std::make_shared<ShaderModule>( shared_from_this( ), code );
  }
  std::shared_ptr<Fence> Device::createFence( bool signaled )
  {
    return std::make_shared<Fence>( shared_from_this( ), signaled );
  }
  std::shared_ptr<Sampler> Device::createSampler( const SamplerStateDesc & desc )
  {
    return std::make_shared<Sampler>( shared_from_this( ), desc );
  }
  std::shared_ptr<DescriptorSetLayout> Device::createDescriptorSetLayout(
    vk::ArrayProxy<const DescriptorSetLayoutBinding> bindings )
  {
    return std::make_shared<DescriptorSetLayout>( shared_from_this( ), bindings );
  }
  std::shared_ptr<DescriptorPool> Device::createDescriptorPool(
    vk::DescriptorPoolCreateFlags flags, uint32_t maxSets,
    vk::ArrayProxy<const vk::DescriptorPoolSize> poolSizes )
  {
    return std::make_shared<DescriptorPool>( shared_from_this( ), flags, maxSets, poolSizes );
  }
  LAVA_API std::shared_ptr<PipelineCache> Device::createPipelineCache( size_t initialSize, void const * initialData )
  {
    return std::make_shared<PipelineCache>( shared_from_this( ), vk::PipelineCacheCreateFlags( ), initialSize, initialData );
  }
  vk::DeviceMemory Device::allocateImageMemory( vk::Image image, vk::MemoryPropertyFlags flags )
  {
    vk::MemoryRequirements memReqs = _device.getImageMemoryRequirements( image );
    vk::DeviceMemory mem = allocateMemReqMemory( memReqs, flags );

    _device.bindImageMemory( image, mem, 0 );

    return mem;
  }
  vk::DeviceMemory Device::allocateBufferMemory( vk::Buffer buffer, vk::MemoryPropertyFlags flags )
  {
    vk::MemoryRequirements memReqs = _device.getBufferMemoryRequirements( buffer );
    vk::DeviceMemory mem = allocateMemReqMemory( memReqs, flags );

    _device.bindBufferMemory( buffer, mem, 0 );

    return mem;
  }
  vk::DeviceMemory Device::allocateMemReqMemory( const vk::MemoryRequirements & reqs, vk::MemoryPropertyFlags flags )
  {
    vk::MemoryAllocateInfo allocateInfo(
      reqs.size,
      findMemoryType( _physicalDevice->getMemoryProperties( ), reqs.memoryTypeBits, flags )
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

  std::shared_ptr<Pipeline> Device::createComputePipeline( 
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
}