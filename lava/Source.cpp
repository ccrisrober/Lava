#include "VulkanApp.h"
using namespace lava;

#include "routes.h"

class MyApp : public VulkanApp
{
public:
	struct Vertex
	{
		glm::vec3 pos;
		glm::vec3 normal;
		glm::vec2 texCoord;
	};

	MyApp(char const* title, uint32_t width, uint32_t height)
		: VulkanApp( title, width, height )
	{
		/*std::vector<DescriptorSetLayoutBinding> dslbs;
		dslbs.push_back(DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex));
		dslbs.push_back(DescriptorSetLayoutBinding(1, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment));
		std::shared_ptr<DescriptorSetLayout> descriptorSetLayout = _device->createDescriptorSetLayout(dslbs);

		std::shared_ptr<DescriptorPool> descriptorPool = _device->createDescriptorPool({}, 1, { { vk::DescriptorType::eUniformBuffer, 1 },{ vk::DescriptorType::eCombinedImageSampler, 1 } });
	
		std::shared_ptr<ShaderModule> vertexShaderModule = _device->createShaderModule(LAVA_EXAMPLES_RESOURCES_ROUTE + std::string("/vert.spv"));
		std::shared_ptr<ShaderModule> fragmentShaderModule = _device->createShaderModule(LAVA_EXAMPLES_RESOURCES_ROUTE + std::string("/frag.spv"));*/
	}
	void keyEvent(int key, int scancode, int action, int mods)
	{
		switch (key)
		{
		case GLFW_KEY_ESCAPE:
			switch (action)
			{
			case GLFW_PRESS:
				glfwSetWindowShouldClose(getWindow(), GLFW_TRUE);
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
		paint();
	}
};

int main( void )
{
	if (glfwInit())
	{
		MyApp window("MyApp", 800, 600);

		while (!glfwWindowShouldClose(window.getWindow()))
		{
			glfwWaitEvents();
			//window.paint();
		}

		glfwTerminate();
	}
	system( "PAUSE" );
	return 0;
}