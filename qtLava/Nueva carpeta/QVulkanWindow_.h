/**
 * Copyright (c) 2017 - 2018, Lava
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

#ifndef __QTLAVA_VULKANWINDOW__
#define __QTLAVA_VULKANWINDOW__

#include <lava/lava.h>
#include <qtLava/api.h>

#include "DefaultFramebuffer.h"
#include <QtGui>
#include <QWindow>
#include <QVulkanInstance>
#include <5.10.0/QtGui/qpa/qplatformnativeinterface.h>

namespace lava
{
	class QVulkanWindow : public QWindow
	{
		//Q_OBJECT
	public:
		QTLAVA_API
		explicit QVulkanWindow( QWindow* parent = 0 );
		QTLAVA_API
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
		void frameReady( void );

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
		
		
		//QTLAVA_API
		//void setQVulkanInstance( const std::shared_ptr< Instance > instance );

		QTLAVA_API
		std::shared_ptr< Framebuffer > currentFramebuffer( void ) const
		{
			return _defaultFramebuffer->getFramebuffer( );
		}

		QTLAVA_API
		vk::Offset2D swapChainImageSize( void ) const;

		QTLAVA_API
		void setContinuousRendering( bool enabled ) { continuousAnimation = enabled; }

	protected:
		QTLAVA_API
		virtual bool setupRenderPass( void );
		QTLAVA_API
		virtual bool setupFramebuffer( void );
		QTLAVA_API
		virtual bool setupPipelineCache( void );

	protected:
		QTLAVA_API
		void exposeEvent( QExposeEvent* eev ) override;
		QTLAVA_API
		void resizeEvent( QResizeEvent* erv ) override;
		QTLAVA_API
		bool event( QEvent* ev ) override;

		QVulkanWindowRenderer* renderer = nullptr;
		std::vector<std::string > _requestedDeviceExts;

	protected:
		bool continuousAnimation = false;
		std::shared_ptr< Instance > _instance;
			QVulkanInstance* _qInstance;
		std::shared_ptr< PhysicalDevice > _physicalDevice;
		std::shared_ptr< Device > _device;
		std::shared_ptr< RenderPass > _renderPass;
		
		std::shared_ptr< Surface > _surface;

		std::unique_ptr< qt::DefaultFramebuffer > _defaultFramebuffer;

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


		bool initialized = false;

		bool _framePending = false;
	private:
		std::shared_ptr< Surface > createSurfaceKHR( void )
		{
			auto surf = vk::SurfaceKHR( vulkanInstance( )->surfaceForWindow( this ) );
			return std::make_shared<Surface>( _instance, surf, false );
			/*QPlatformNativeInterface *nativeInterface = qGuiApp->platformNativeInterface( );
			void *p = nativeInterface->nativeResourceForWindow(QByteArrayLiteral( "vkSurface" ), this );
			vk::SurfaceKHR surf( *static_cast< VkSurfaceKHR * >( p ) );
			return std::make_shared<Surface>( _instance, surf, false );*/
		}
	};
}

#endif /* __QTLAVA_VULKANWINDOW__ */
