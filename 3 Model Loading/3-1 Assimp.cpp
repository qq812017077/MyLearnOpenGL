#include <glad\glad.h>
#include <glfw3.h>

//数学库
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//图像库
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>
#include "Shaders.h"
#include "Camera.h"
#include "materialLibrary.h"

/*
	Assimp   模型加载库
	模型结构对象：
		Scene：包含了场景中的所有数据，如所有的材质和网格信息。以及场景根节点的引用。

		Root Node： 场景的根节点包含了一系列子节点，以及在场景对象的mMeshes[]中的一系列索引（指向网格数据）
				根节点的mMeshes[]包含了实际的Mesh对象；
				而 每个节点的mMeshes[]的值仅是根节点mMeshes[]的索引
		Mesh 对象：
				其包含了所有渲染所需的相关信息，如 顶点位置，法向量 贴图坐标， 对象的材质。
			一个Mesh也包含了一些Face。 
					一个 Face 代表了对象的一个渲染基本单位（点、线、三角面片、矩形面片）
					一个 Face 包含了形成一个圆形的顶点的索引，
					（通过这个索引，可以在mMeshes[]中寻找到对应的顶点位置数据。）
					（顶点和索引是分离的，这使得通过一个顶点数组渲染是比较容易的，通过VAO、VBO、NBO、TBO、IBO）
			一个Mesh也包含了一个材质对象，用于指定物体的一些材质属性
				如颜色、纹理贴图（漫反射贴图、高光贴图等）

		即，我们做的第一件事：
			加载一个对象到Scene对象，遍历每个节点获取对应的Mesh对象（递归查询每一个节点）。
				处理每一个Mesh对象以获取顶点数据，索引及其材质属性。
			最终我们得到一个包含我们需要的数据的Mesh集合，其位于单个Model对象中。

		NOTE：
			需要注意的是，设计师通常不会使用单个性状来构建以整个模型，每个模型通常包含多个子模型和形状。
			
					构成一个模型的每个性状都被称为一个Mesh！
			（例如一个人物模型通常是由头、四肢、衣服、武器多个组建组合而成）

				单个Mesh是OpenGL中可绘制的最小单位（包含顶点数据，索引，材质属性）。一个模型包含多个Mesh。
*/

void framebuffer_size_callback(GLFWwindow * window, int width, int height) { glViewport(0, 0, width, height); return; }
void mouse_callback(GLFWwindow* window, double xPos, double yPos);
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
void ProcessInput(GLFWwindow * window, float deltaTime);

unsigned int loadTexture(char const * path);
unsigned int loadVertexToVBO(float const * data);

const int SCR_WIDTH = 800;
const int SCR_HEIGHT = 600;

void MultipleLights_Task_Desert(GLFWwindow* window);
void MultipleLights_Task_Factory(GLFWwindow* window);
void MultipleLights_Task_Horror(GLFWwindow* window);
void MultipleLights_Task_Lab(GLFWwindow* window);

glm::vec3 cameraPos(0.0f, 0.0f, 2.0f);
glm::vec3 WorldUp(0.0f, 1.0f, 0.0f);

float yaw = -90.0f, pitch = 0.0f;

Camera camera(cameraPos, WorldUp, yaw, pitch);
int main(void) {
	if (!glfwInit()) {
		std::cout << "Initializing Failed" << std::endl;
	}

	//Set
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow * window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "2-6_MultipleLights", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create the window" << std::endl;
		glfwTerminate();
		system("pause");
		return -1;
	}

	//注册环境
	glfwMakeContextCurrent(window);
	//初始化回调
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		system("pause");
		return -1;
	}

	//
	MultipleLights_Task_Lab(window);
	glfwTerminate();

}