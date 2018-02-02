#ifndef __QTLAVA_VULKANWINDOW__
#define __QTLAVA_VULKANWINDOW__

#include <lava/lava.h>
#include <qtLava/api.h>

#include "DefaultFramebuffer.h"
#include <QWindow>

namespace lava
{
  class QVulkanWindowRenderer
  {
  public:
		QTLAVA_API
		virtual ~QVulkanWindowRenderer( void );

		/**
		* Method called when creating renderer's resources
		*/
		QTLAVA_API
		virtual void initResources( void );
		QTLAVA_API
		virtual void initSwapChainResources( void );
		QTLAVA_API
		virtual void releaseSwapChainResources( void );
		/**
		* Method called when renderer's resources must be released
		*/
		QTLAVA_API
		virtual void releaseResources( void );
		/**
		* Method called when the draw calls for the next frame are to be added
		*   to the command buffer
		*/
		QTLAVA_API
		virtual void nextFrame( void ) = 0;

		//virtual void physicalDeviceLost( void );
		//virtual void logicalDeviceLost( void );
	};

	class QVulkanWindow : public QWindow
	{
        Q_OBJECT
	public:
        explicit QVulkanWindow( QWindow* parent = 0 );
		~QVulkanWindow( void );
		
		QTLAVA_API
	    QVulkanWindow( const QVulkanWindow& ) = delete;
		QTLAVA_API
	    QVulkanWindow( QVulkanWindow&& ) = delete;

		QTLAVA_API
	    QVulkanWindow& operator=( const QVulkanWindow& ) = delete;
		QTLAVA_API
	    QVulkanWindow& operator=( QVulkanWindow&& ) = delete;

		QTLAVA_API
		void beginFrame( void );
		QTLAVA_API
		void endFrame( void );
		QTLAVA_API
		std::shared_ptr<CommandBuffer> currentCommandBuffer( void ) const;
		QTLAVA_API
		std::shared_ptr< PhysicalDevice > physicalDevice( void ) const;
		QTLAVA_API
		const vk::PhysicalDeviceProperties physicalDeviceProperties( void ) const;
		QTLAVA_API
		std::shared_ptr< Device > device( void ) const;
		QTLAVA_API
		std::shared_ptr< Queue > gfxQueue( void ) const;
		QTLAVA_API
		std::shared_ptr< CommandPool > gfxCommandPool( void ) const;
		QTLAVA_API
		std::shared_ptr< RenderPass > defaultRenderPass( void ) const;

		QTLAVA_API
		vk::Format colorFormat( void ) const;
		QTLAVA_API
		vk::Format depthStencilFormat( void ) const;

		QTLAVA_API
		virtual QVulkanWindowRenderer* createRenderer( void );


		QTLAVA_API
		vk::SampleCountFlagBits sampleCountFlagBits( void ) const;
		QTLAVA_API
		void setSampleCountFlagBits( int sampleCount );
		QTLAVA_API
		std::vector<int> supportedSampleCounts( void );

		QTLAVA_API
		void init( void );

		QTLAVA_API
        void reset( void );
		
    QTLAVA_API
    void setVulkanInstance( const std::shared_ptr< Instance > instance );

    QTLAVA_API
    std::shared_ptr< Framebuffer > currentFramebuffer( void ) const
    {
      return _defaultFramebuffer->getFramebuffer( );
    }

   	QTLAVA_API
    vk::Offset2D swapChainImageSize( void ) const;
  protected:
		virtual bool setupRenderPass( void );

		virtual bool setupFramebuffer( void );

		virtual bool setupPipelineCache( void );

  protected:
		void exposeEvent( QExposeEvent* eev ) override;
		void resizeEvent( QResizeEvent* erv ) override;
		bool event( QEvent* ev ) override;

    QVulkanWindowRenderer* renderer = nullptr;
		std::vector<std::string > _requestedDeviceExts;

  protected:
		int numero = 0;
		std::shared_ptr< Instance > _instance;
		std::shared_ptr< PhysicalDevice > _physicalDevice;
		std::shared_ptr< Device > _device;
		std::shared_ptr< RenderPass > _renderPass;
		
		std::shared_ptr< Surface > _surface;

		std::unique_ptr< DefaultFramebuffer > _defaultFramebuffer;

		vk::SampleCountFlagBits sampleCount = vk::SampleCountFlagBits::e1;
		

		uint32_t _gfxQueueFamilyIdx;
		uint32_t _presQueueFamilyIdx;
		std::shared_ptr< Queue > _gfxQueue;
		std::shared_ptr< Queue > _presQueue;

		std::shared_ptr< Semaphore > _renderComplete;

		struct ImageResources
		{
		  std::shared_ptr<CommandBuffer> commandBuffer;
		} imageRes[ 2 ];
  public:
		std::shared_ptr< CommandPool > _cmdPool;
  protected:
		vk::Offset2D _swapChainImageSize;

		vk::Format _colorFormat;
		vk::ColorSpaceKHR _colorSpace;
		vk::Format _dsFormat;

		//vk::PresentModeKHR presentMode = vk::PresentMode::eFifoKHR;

		//glm::mat4 _clipCorrect;

		RenderAPICapabilities _caps;
		uint32_t _currentFrame;


		bool _framePending = false;
  private:
		std::shared_ptr< Surface > createSurfaceKHR( void )
		{
          std::shared_ptr< Surface > surface_;
          QPlatformNativeInterface *nativeInterface = qGuiApp->platformNativeInterface();
          // VkSurfaceKHR is non-dispatchable and maps to a pointer on x64 and a uint64 on x86.
          // Therefore a pointer is returned from the platform plugin, not the value itself.
          void *p = nativeInterface->nativeResourceForWindow(QByteArrayLiteral("vkSurface"), window);
          //return p ? *static_cast<VkSurfaceKHR *>(p) : 0;
          return surface_;
		}
  };
}

#endif /* __QTLAVA_VULKANWINDOW__ */
