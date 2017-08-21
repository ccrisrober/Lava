#include "RenderWindow.h"

#include "VulkanRenderAPI.h"

namespace lava
{
  RenderWindow::RenderWindow( VulkanRenderAPI& renderAPI )
    : _renderAPI( renderAPI )
  {
    // Surface KHR
    if ( glfwCreateWindowSurface( _renderAPI._getInstance( ), _renderAPI.getWindow( ),
      nullptr, &_surface ) != VK_SUCCESS )
    {
      throw std::runtime_error( "failed to create window surface!" );
    }

    std::shared_ptr<VulkanDevice> presentDevice = _renderAPI._getPresentDevice( );
    VkPhysicalDevice physicalDevice = presentDevice->getPhysical( );

    uint32_t presentQueueFamily = presentDevice->getQueueFamily( GPUT_GRAPHICS );

    VkBool32 supportsPresent = false;
    vkGetPhysicalDeviceSurfaceSupportKHR( physicalDevice, presentQueueFamily,
      _surface, &supportsPresent );
    if ( !supportsPresent )
    {
      // Note: Not supporting present only queues at the moment
      // Note: Also present device can only return one family of graphics queue, 
      // while there could be more (some of which support present)
      throw std::exception( R"(Cannot find a graphics queue that also 
      supports present operations.)" );
    }

    uint32_t numFormats;
    VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR( physicalDevice,
      _surface, &numFormats, nullptr );
    assert( result == VK_SUCCESS );
    assert( numFormats > 0 );

    std::vector<VkSurfaceFormatKHR> surfaceFormats( numFormats );
    result = vkGetPhysicalDeviceSurfaceFormatsKHR( physicalDevice, _surface,
      &numFormats, surfaceFormats.data( ) );
    assert( result == VK_SUCCESS );

    bool gamma = false;
    // If there is no preferred format, use standard RGBA
    if ( ( numFormats == 1 )
      && ( surfaceFormats[ 0 ].format == VK_FORMAT_UNDEFINED ) )
    {
      if ( gamma )
      {
        _colorFormat = VK_FORMAT_R8G8B8A8_SRGB;
      }
      else
      {
        _colorFormat = VK_FORMAT_B8G8R8A8_UNORM;
      }

      _colorSpace = surfaceFormats[ 0 ].colorSpace;
    }
    else
    {
      bool foundFormat = false;

      std::vector<VkFormat> wantedFormatsUNORM =
      {
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_FORMAT_B8G8R8A8_UNORM,
        VK_FORMAT_A8B8G8R8_UNORM_PACK32,
        VK_FORMAT_A8B8G8R8_UNORM_PACK32,
        VK_FORMAT_R8G8B8_UNORM,
        VK_FORMAT_B8G8R8_UNORM
      };

      std::vector<VkFormat> wantedFormatsSRGB =
      {
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_FORMAT_B8G8R8A8_SRGB,
        VK_FORMAT_A8B8G8R8_SRGB_PACK32,
        VK_FORMAT_A8B8G8R8_SRGB_PACK32,
        VK_FORMAT_R8G8B8_SRGB,
        VK_FORMAT_B8G8R8_SRGB
      };

      std::vector<VkFormat> wantedFormats;
      if ( gamma )
      {
        wantedFormats = wantedFormatsSRGB;
      }
      else
      {
        wantedFormats = wantedFormatsUNORM;
      }

      for ( const auto& wantedFormat : wantedFormats )
      {
        for ( const auto& surfFormat : surfaceFormats )
        {
          if ( surfFormat.format == wantedFormat )
          {
            _colorFormat = surfFormat.format;
            _colorSpace = surfFormat.colorSpace;

            foundFormat = true;
            break;
          }
        }
        if ( foundFormat )
          break;
      }

      wantedFormatsSRGB.clear( );
      wantedFormatsUNORM.clear( );
      wantedFormats.clear( );

      // If we haven't found anything, fall back to first available
      if ( !foundFormat )
      {
        _colorFormat = surfaceFormats[ 0 ].format;
        _colorSpace = surfaceFormats[ 0 ].colorSpace;

        if ( gamma )
          throw new std::exception( R"(Cannot find a valid sRGB format for a 
          render window surface, falling back to a default format.)" );
      }
    }
    _depthFormat = VK_FORMAT_D24_UNORM_S8_UINT;

    _swapChain = std::make_shared<VulkanSwapChain>( );
    _swapChain->rebuild( _renderAPI._getPresentDevice( ), _surface, WIDTH,
      HEIGHT, true, _colorFormat, _colorSpace, true, _depthFormat );
  }


  RenderWindow::~RenderWindow( void )
  {
    _swapChain.reset( );
    vkDestroySurfaceKHR( _renderAPI._getInstance( ), _surface, nullptr );
  }

  void RenderWindow::resize( uint32_t width, uint32_t height )
  {
    _swapChain->rebuild( _renderAPI._getPresentDevice( ), _surface, width,
      height, true, _colorFormat, _colorSpace, true, _depthFormat );
  }
}