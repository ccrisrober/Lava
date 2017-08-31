#include "Swapchain.h"
#include "Device.h"
#include "Surface.h"

namespace lava
{
  Swapchain::Swapchain( const DeviceRef& device,
    const std::shared_ptr<Surface>& surface, uint32_t numImageCount,
    vk::Format imageFormat, vk::ColorSpaceKHR colorSpace, 
    const vk::Extent2D& imageExtent,
    uint32_t imageArrayLayers, vk::ImageUsageFlags imageUsage,
    vk::SharingMode imageSharingMode,
    const std::vector<uint32_t>& queueFamilyIndices,
    vk::SurfaceTransformFlagBitsKHR preTransform,
    vk::CompositeAlphaFlagBitsKHR compositeAlpha,
    vk::PresentModeKHR presentMode, bool clipped,
    const std::shared_ptr<Swapchain>& oldSwapchain )
    : VulkanResource( device )
  {
    vk::SwapchainCreateInfoKHR sci( {}, static_cast< vk::SurfaceKHR >( *surface ),
      numImageCount, imageFormat, colorSpace, imageExtent, imageArrayLayers, 
      imageUsage, imageSharingMode, queueFamilyIndices.size( ),
      queueFamilyIndices.data( ), preTransform, compositeAlpha, presentMode, clipped,
      oldSwapchain ? static_cast< vk::SwapchainKHR >( *oldSwapchain ) : nullptr );

    _swapchain = static_cast< vk::Device >( *_device ).createSwapchainKHR( sci );

    std::vector<vk::Image> images =
      static_cast< vk::Device >( *_device ).getSwapchainImagesKHR( _swapchain );
    _images.reserve( images.size( ) );
    _presentCompleteSemaphores.reserve( images.size( ) + 1 );
    for ( size_t i = 0; i < images.size( ); ++i )
    {
      _images.push_back( std::make_shared<Image>( _device, images[ i ] ) );
      _presentCompleteSemaphores.push_back( _device->createSemaphore( ) );
    }
    _freeSemaphore = _device->createSemaphore( );
  }
  Swapchain::~Swapchain( )
  {
    _images.clear( );
    static_cast< vk::Device >( *_device ).destroySwapchainKHR( _swapchain );
  }
  std::vector<std::shared_ptr<Image>> const& Swapchain::getImages( void ) const
  {
    return _images;
  }

  uint32_t Swapchain::acquireNextImage( uint64_t timeout,
    const std::shared_ptr<Fence>& fence )
  {
    vk::ResultValue<uint32_t> result = static_cast< vk::Device >( *_device )
      .acquireNextImageKHR( _swapchain, timeout, *_freeSemaphore,
        fence ? static_cast< vk::Fence >( *fence ) : nullptr );
    assert( result.result == vk::Result::eSuccess );  // need to handle timeout, not ready, and suboptimal
                            // put the semaphore at the correct index and use the semaphore from the new index as next free semaphore
    std::swap( _freeSemaphore, _presentCompleteSemaphores[ result.value ] );
    return result.value;

  }
}