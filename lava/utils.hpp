#ifndef __LAVA_UTILS__
#define __LAVA_UTILS__

#include "includes.hpp"

#include <sstream>
#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace lava
{
	class utils
	{
    static unsigned char* loadImageTexture( const std::string& fileName,
      uint32_t& width, uint32_t& height, uint32_t& numChannels )
    {
      int w, h, c;
      stbi_uc* pixels = stbi_load( fileName.c_str( ),
        &w, &h, &c, STBI_rgb_alpha );

      if ( !pixels )
      {
        throw std::runtime_error( "failed to load texture image!" );
      }

      width = w;
      height = h;
      numChannels = c;

      return pixels;
    }
    static std::vector<char> readBinaryile( const std::string& fileName )
    {
      std::ifstream file( fileName, std::ios::ate | std::ios::binary );

      if ( !file.is_open( ) )
      {
        throw std::runtime_error( "failed to open file!" );
      }

      size_t fileSize = ( size_t ) file.tellg( );
      std::vector<char> buffer( fileSize );

      file.seekg( 0 );
      file.read( buffer.data( ), fileSize );

      file.close( );

      return buffer;
    }
		static const std::string translateVulkanResult( vk::Result res )
		{
      VkResult result = VkResult( res );
      switch ( result ) {
        // Success codes
      case VK_SUCCESS:
        return "Command successfully completed.";
      case VK_NOT_READY:
        return "A fence or query has not yet completed.";
      case VK_TIMEOUT:
        return "A wait operation has not completed in the specified time.";
      case VK_EVENT_SET:
        return "An event is signaled.";
      case VK_EVENT_RESET:
        return "An event is unsignaled.";
      case VK_INCOMPLETE:
        return "A return array was too small for the result.";
      case VK_SUBOPTIMAL_KHR:
        return "A swapchain no longer matches the surface properties exactly, but can still be used to present to the surface successfully.";

        // Error codes
      case VK_ERROR_OUT_OF_HOST_MEMORY:
        return "A host memory allocation has failed.";
      case VK_ERROR_OUT_OF_DEVICE_MEMORY:
        return "A device memory allocation has failed.";
      case VK_ERROR_INITIALIZATION_FAILED:
        return "Initialization of an object could not be completed for implementation-specific reasons.";
      case VK_ERROR_DEVICE_LOST:
        return "The logical or physical device has been lost.";
      case VK_ERROR_MEMORY_MAP_FAILED:
        return "Mapping of a memory object has failed.";
      case VK_ERROR_LAYER_NOT_PRESENT:
        return "A requested layer is not present or could not be loaded.";
      case VK_ERROR_EXTENSION_NOT_PRESENT:
        return "A requested extension is not supported.";
      case VK_ERROR_FEATURE_NOT_PRESENT:
        return "A requested feature is not supported.";
      case VK_ERROR_INCOMPATIBLE_DRIVER:
        return "The requested version of Vulkan is not supported by the driver or is otherwise incompatible for implementation-specific reasons.";
      case VK_ERROR_TOO_MANY_OBJECTS:
        return "Too many objects of the type have already been created.";
      case VK_ERROR_FORMAT_NOT_SUPPORTED:
        return "A requested format is not supported on this device.";
      case VK_ERROR_SURFACE_LOST_KHR:
        return "A surface is no longer available.";
      case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
        return "The requested window is already connected to a VkSurfaceKHR, or to some other non-Vulkan API.";
      case VK_ERROR_OUT_OF_DATE_KHR:
        return "A surface has changed in such a way that it is no longer compatible with the swapchain, and further presentation requests using the swapchain will fail. Applications must query the new surface properties and recreate their swapchain if they wish to continue presenting to the surface.";
      case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
        return "The display used by a swapchain does not use the same presentable image layout, or is incompatible in a way that prevents sharing an image.";
      case VK_ERROR_VALIDATION_FAILED_EXT:
        return "A validation layer found an error.";
      default:
        std::stringstream ss;
        ss << "Unknown [" << result << "]";
        return ss.str( );
      }
		}
	};
}

#endif /* __LAVA_UTILS__ */