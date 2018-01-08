#include "RenderWindow.h"

#include "VulkanRenderAPI.h"

namespace lava
{
  RenderWindow::RenderWindow( VulkanRenderAPI& renderAPI )
    : _renderAPI( renderAPI )
  {
    // Surface KHR
    if ( glfwCreateWindowSurface(VkInstance(_renderAPI._getInstance( ))
		, _renderAPI.getWindow( ), nullptr, &_surface ) != VK_SUCCESS )
    {
      throw std::runtime_error( "failed to create window surface!" );
    }

    std::shared_ptr<VulkanDevice> presentDevice = _renderAPI._getPresentDevice( );
    vk::PhysicalDevice physicalDevice = presentDevice->getPhysical( );

    uint32_t presentQueueFamily = presentDevice->getQueueFamily( GPUT_GRAPHICS );

	vk::Bool32 supportsPresent = physicalDevice.getSurfaceSupportKHR(
		presentQueueFamily, _surface);
    if ( !supportsPresent )
    {
      // Note: Not supporting present only queues at the moment
      // Note: Also present device can only return one family of graphics queue, 
      // while there could be more (some of which support present)
      throw std::exception( R"(Cannot find a graphics queue that also 
      supports present operations.)" );
    }

	std::vector<vk::SurfaceFormatKHR> surfaceFormats = 
		physicalDevice.getSurfaceFormatsKHR(_surface);
	uint32_t numFormats = surfaceFormats.size();

    bool gamma = false;
    // If there is no preferred format, use standard RGBA
    if ( ( numFormats == 1 )
      && ( surfaceFormats[ 0 ].format == vk::Format::eUndefined) )
    {
      if ( gamma )
      {
        _colorFormat = vk::Format::eR8G8B8A8Srgb;
      }
      else
      {
        _colorFormat = vk::Format::eB8G8R8A8Unorm;
      }

      _colorSpace = surfaceFormats[ 0 ].colorSpace;
    }
    else
    {
      bool foundFormat = false;

      std::vector<vk::Format> wantedFormatsUNORM =
      {
		vk::Format::eR8G8B8A8Unorm,
		vk::Format::eB8G8R8A8Unorm,
		vk::Format::eA8B8G8R8UnormPack32,
		vk::Format::eA8B8G8R8UnormPack32,
		vk::Format::eR8G8B8Unorm,
		vk::Format::eB8G8R8Unorm
      };

      std::vector<vk::Format> wantedFormatsSRGB =
      {
		vk::Format::eR8G8B8A8Srgb,
		vk::Format::eB8G8R8A8Srgb,
		vk::Format::eA8B8G8R8SrgbPack32,
		vk::Format::eA8B8G8R8SrgbPack32,
		vk::Format::eR8G8B8Srgb,
		vk::Format::eB8G8R8Srgb
      };

      std::vector<vk::Format> wantedFormats;
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
    _depthFormat = vk::Format::eD24UnormS8Uint;

    _swapChain = std::make_shared<VulkanSwapChain>( );
    _swapChain->rebuild( _renderAPI._getPresentDevice( ), _surface, WIDTH,
      HEIGHT, true, _colorFormat, _colorSpace, true, _depthFormat );
  }


  RenderWindow::~RenderWindow( void )
  {
    _swapChain.reset( );
	_renderAPI._getInstance().destroySurfaceKHR(_surface);
  }

  void RenderWindow::resize( uint32_t width, uint32_t height )
  {
    _swapChain->rebuild( _renderAPI._getPresentDevice( ), _surface, width,
      height, true, _colorFormat, _colorSpace, true, _depthFormat );
  }
}