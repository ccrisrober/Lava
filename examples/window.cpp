#include <lava/lava.h>
using namespace lava;

#include <routes.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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
	}
};

void errorCallback(int error, const char* description)
{
	fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

int main(void)
{
	try
	{
		//glfwSetErrorCallback(errorCallback);

		//if (glfwInit())
		//{
			MyApp window("MyApp", 800, 600);

			while (window.isRunning( ))
			{
				window.waitEvents();
				window.paint();
			}

			glfwTerminate();
		//}
	}
	catch (std::system_error systemError)
	{
		std::cout << "System Error: " << systemError.what() << std::endl;
	}
	system("PAUSE");
	return 0;
}