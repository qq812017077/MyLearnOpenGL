#include <glad\glad.h>
#include <glfw3.h>

//数学库
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include "Shaders.h"
#include "Camera.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

/*
	！！！！！！！！！！！！！！！！！！！！！！！！！
	本章存在很多的困惑，关于点光源阴影的处理方面，着实让人困惑。
		1.一方面，几何着色器的处理，将一个三角形处理生成了6个视角下的三角形，

		2.关于从世界坐标系变换到光为原点的坐标系下，来获取深度信息时，变换矩阵的View矩阵的设置难以理解！
			为何Up向量的值必须如此特殊！！
			修改之后即会出现各式问题。。

	！！！！！！！！！！！！！！！！！！！！！！！！！
*/
/*
	Point Shadows
	本节我们的焦点是在各种方向生成动态阴影。这个技术可以适用于点光源，生成所有方向上的阴影。
	(上一节我们学习了定向的阴影映射，：directional shadow mapping , 在单一定向光源下生成的阴影。)

	该技术称为点阴影，或更正式的“全向阴影映射： omnidirectional shadow maps”
	
	该算法大体上和定向阴影映射相同：
		我们会从光的视角生成一个深度贴图，根据当前的fragment位置采集深度贴图，
	并用保存的深度值和每个fragment进行比较：看其是否在阴影中。
	
		主要的不同在于：深度贴图的使用。
		我们需要渲染的深度贴图，其场景来自于点光源的各个方向，一个普通的2D深度贴图将不能胜任这项工作。
		
		解决办法：
			立方体贴图可以储存6个面的环境数据，它可以将整个场景渲染到立方体贴图的
	每个面上，把它们当作点光源四周的深度值来采样。
	生成的深度立方贴图之后被传递给光的片段着色器，该着色器使用一个方向向量去采集立方体贴图，
	继而获取片段的深度（光的视角）。
		因此麻烦的地方在于深度立方贴图的生成。
*/

/*Generating the depth cubemap	生成深度立方贴图。
	要创建这样一个立方贴图，我们需要渲染场景六次，每次一个面。

	方法一：
		使用六个不同的view矩阵渲染场景6次。每次附加立方贴图的一个不同的面到framebuffer。

			for(unsigned int i = 0; i < 6; i++)
			{
				GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;		//不同的面
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, face, depthCubemap, 0);	将面绑定到framebuffer
				BindViewMatrix(lightViewMatrices[i]);					//绑定不同的view矩阵
				RenderScene();											//渲染场景
			}

		这种算法的开销比较昂贵：对于一个单个的深度贴图需要进行多次渲染调用。
	方法二：
		几何着色器允许我们使用一次渲染过程来建立深度立方体贴图。
		1.首先创建一个立方体贴图
			GLuint depthCubemap;
			glGenTextures(1, &depthCubemap);
		2.生成立方体贴图的每个面，将它们作为2D深度值纹理图像：
			const GLuint SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
			glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);	绑定立方贴图。
			for (GLuint i = 0; i < 6; ++i)
				 glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, 
						SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		3.为立方贴图设置纹理参数
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		4.使用 glFramebufferTexture 直接附加cubemap作为帧缓冲区的深度附加物。
		（正常情况下，我们需要附加cubemap纹理的每个面到真缓冲对象，然后渲染6次，每次切换帧缓冲区的
		深度缓冲目标到一个不同的cubemap面。这里我们打算使用几何着色器，故可以一次性处理完）
			代码：
				glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
				glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
				glDrawBuffer(GL_NONE);
				glReadBuffer(GL_NONE);
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				再次提示：我们只需要深度信息，故将颜色的缓冲区读和写都关闭了
		5.使用全向阴影映射：
			（1）生成深度贴图。
				glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
				glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
					glClear(GL_DEPTH_BUFFER_BIT);
					ConfigureShaderAndMatrices();					//帧缓冲的着色器和矩阵设置
					RenderScene();									//深度信息生成
				glBindFramebuffer(GL_FRAMEBUFFER, 0);

			（2）利用深度贴图为场景创建阴影。
				glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
				ConfigureShaderAndMatrices();						//默认缓冲的着色器和矩阵设置
				glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);	//场景渲染
				RenderScene();
			整体的代码思路基本和原先的一致，变化的是细节部分
		6.Light space transform
			我们需要一些方法来将场景的所有几何体变换到6个光的方向中相应的光空间。
			和上节的相似，我们会创建光的空间变换矩阵，但这次是每个面一个矩阵。

			每一个变换矩阵都包含一个projection和一个view矩阵。
			
			Projection：
				使用perspective projection 矩阵
			因为投影矩阵在每个方向上并不会改变，我们可以在6个变换矩阵中重复使用。
				float aspect = (float)SHADOW_WIDTH/(float)SHADOW_HEIGHT;
				float near = 1.0f;
				float far = 25.0f;
				glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspect, near, far);
				注意这里的视野范围设置为90度，这确保了视野是足够大以合适地填满立方体贴图的一个面
			这样所有的面都能够跟其他面对齐。

			View：
				我们需要六个不同的View矩阵
				我们需要使用glm::loogAt函数创建六个观察方向，每一个指向一个单独的方向：
					right, left, top, bottom, near and far.
					std::vector<glm::mat4> shadowTransforms;
					shadowTransforms.push_back(shadowProj *
					glm::lookAt(lightPos, lightPos + glm::vec3( 1.0, 0.0, 0.0), glm::vec3(0.0,-1.0, 0.0));
					shadowTransforms.push_back(shadowProj *
					glm::lookAt(lightPos, lightPos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0,-1.0, 0.0));
					shadowTransforms.push_back(shadowProj *
					glm::lookAt(lightPos, lightPos + glm::vec3( 0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0));
					shadowTransforms.push_back(shadowProj *
					glm::lookAt(lightPos, lightPos + glm::vec3( 0.0,-1.0, 0.0), glm::vec3(0.0, 0.0,-1.0));
					shadowTransforms.push_back(shadowProj *
					glm::lookAt(lightPos, lightPos + glm::vec3( 0.0, 0.0, 1.0), glm::vec3(0.0,-1.0, 0.0));
					shadowTransforms.push_back(shadowProj *
					glm::lookAt(lightPos, lightPos + glm::vec3( 0.0, 0.0,-1.0), glm::vec3(0.0,-1.0, 0.0));
			
			通过和projection组合即可以得到六个不同的光源空间变换矩阵。
			随后便可以将它们传给着色器使用。

		7.深度着色器
			要渲染深度值到一个深度立方贴图，我们将需要总共三个着色器：顶点着色器，片段着色器，和几何着色器。
			顶点着色器：简单的转换顶点到世界坐标，然后直接传递给几何着色器。
				
			几何着色器：负责转换所有的世界坐标顶点到6个不同的光空间。
				输入：3个三角形顶点，一个uniform数组：光的空间变换矩阵。
					Shader5-32.vertex_depth
				几何着色器有一个内置变量：gl_Layer,它指定发出的基本图形到达立方体贴图的哪个面。当不管它时（when left alone）
			几何着色器将像往常一样将其的基本图形发送到管道的下一阶段，但是当我们更新这个值的时候，我们能够控制将每个基础图形
			渲染到哪个立方贴图中。当然这只有当我们有了一个附加到激活的帧缓冲的立方体贴图纹理才有效.
				代码详见：
					Shader5-32.geometry_depth
				
			片段着色器：
				（这次我们将计算自己的深度，这个深度就是每个fragment位置和光源位置之间的线性距离。
				计算自己的深度值使得之后的阴影计算更加直观。）
				输入：FragPos(来自几何着色器)，光的位置向量以及锥体的远平面值。
				这里我们获取片段和光源的距离，根据锥体的远平面值，将其映射到[0,1]
				并将其作为片段的深度值。

			使用这些着色器渲染场景，立方体贴图附加的帧缓冲对象激活以后，你会得到一个完全填充的深度立方体贴图
		8.Omnidirectional shadow maps
			当所有东西准备就绪后，就可以渲染实际的全向阴影了。
				这个过程和定向阴影映射教程相似，尽管这次我们绑定的深度贴图是一个立方体贴图，
			而不是2D纹理，并且将光的投影的远平面发送给了着色器。
			代码如下：
				glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				shader.Use();
				// ... send uniforms to shader (including light's far_plane value)
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
				// ... bind other textures
				RenderScene();
			这里的RenderScene函数在一个大的cube里面渲染一些cube。它们散落在大立方体各处，光源在场景中央。
			
			顶点着色器和片段着色器和原来的阴影映射着色器大部分都一样：
				不同之处是在光空间中片段着色器不再需要一个fragment在光空间的位置，现在我们可以使用一个方向向量采样深度值。

		因为这个顶点着色器不再需要将他的位置向量变换到光空间，所以我们可以移除FragPosLightSpace变量：
			详见Shader5-3-2.vertex
		而片段着色器中的Blinn-Phong光代码和之前基本一致：变化的是阴影的计算代码：
			详见Shader5-3-2.fragment
			这里的代码中：
				阴影计算使用的的是Cube贴图，Cube贴图的采集需要方向向量。方向向量可以简单的通过光源位置和片段位置来得到
			并且注意，shadow部分影响了diffuse和specular组件。
*/

/*PCF
	因为全向阴影贴图基于和传统阴影贴图同样的原理，因此同样会有分辨率产生的锯齿边情况。
	Percentage-closer filtering 或者PCF允许我们通过对fragment位置周围过滤多个样本，并对结果平均化，继而平滑这些锯齿边，

	如果我们用和前面教程同样的那个简单的PCF过滤器，并加入第三个维度，就是这样的：
		看Shader5-3-2.fragment

*/

/*
--函数------------------------------
*/
void framebuffer_size_callback(GLFWwindow * window, int width, int height) { glViewport(0, 0, width, height);	return; }
void mouse_callback(GLFWwindow* window, double xPos, double yPos);
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
void ProcessInput(GLFWwindow * window, float deltaTime);

void renderScene(const Shader &shader);
unsigned int loadTexture(char const *path, unsigned int Tex_Wrap_Para = GL_REPEAT);
//Depth_Map
void Depth_Map_Point(GLFWwindow * window);
/*
--变量------------------------------
*/
const int SCR_WIDTH = 1280;
const int SCR_HEIGHT = 720;

glm::vec3 cameraPos(0.0f, 0.0f, 3.0f);
glm::vec3 WorldUp(0.0f, 1.0f, 0.0f);

float yaw = -90.0f, pitch = 0.0f;
bool openEffect = true;
bool functoinExchange = true;
int lastStatus;
int Exchange_lastStatus;
Camera camera(cameraPos, WorldUp, yaw, pitch);

int main(void) {
	if (!glfwInit()) {
		std::cout << "Initializing Failed" << std::endl;
	}

	//Set
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow * window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "5-1 Advanced Lighting", NULL, NULL);
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
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);	//关闭鼠光标
	glfwSetCursorPosCallback(window, mouse_callback);				//打开鼠标回调
	glfwSetScrollCallback(window, scroll_callback);

	//glad
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		system("pause");
		return -1;
	}
	Depth_Map_Point(window);

	glfwTerminate();

}

/*
鼠标向左移动，x值减小；向右移动，x值增大
鼠标向上移动，y值减小；向下移动，y值增大
*/
bool firstMouse = true;
static float lastX = (float)SCR_WIDTH / 2.0, lastY = (float)SCR_HEIGHT / 2.0;
void mouse_callback(GLFWwindow* window, double xPos, double yPos) {
	std::cout << xPos << "   " << yPos << std::endl;
	if (firstMouse) // this bool variable is initially set to true
	{
		//第一次调用时的xPos,yPos和lastX,lastY相差巨大，需要进行一次调整
		lastX = xPos;
		lastY = yPos;
		firstMouse = false;
	}
	//std::cout << "xPos:  " << xPos << "yPos:  " << yPos << std::endl;
	float sensitivity = 0.05f;	//敏感度
	float xOffset = xPos - lastX;	//向左移动为负，向右移动为正。
	float yOffset = lastY - yPos;	//向上移动为正，向下移动为负。

	lastX = xPos;
	lastY = yPos;

	camera.ProcessMouseMovement(xOffset, yOffset);
}
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset) {
	camera.ProcessMouseScroll(yOffset);
}
void ProcessInput(GLFWwindow * window, float deltaTime) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE && lastStatus == GLFW_PRESS) {
		//使用空格来开关镜面反射效果
		if (openEffect) {
			openEffect = false;
			//glDisable(GL_MULTISAMPLE);
		}
		else {
			openEffect = true;
			//glEnable(GL_MULTISAMPLE);
		}
	}

	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE && Exchange_lastStatus == GLFW_PRESS) {
		//使用B来控制切换着色方案
		if (functoinExchange) {
			functoinExchange = false;
		}
		else {
			functoinExchange = true;
		}
	}
	lastStatus = glfwGetKey(window, GLFW_KEY_SPACE);
	Exchange_lastStatus = glfwGetKey(window, GLFW_KEY_B);
	//相机按键控制
	Camera_Movement curMovement = Camera_Movement::NONE;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		curMovement = Camera_Movement::FORWARD;
	}
	else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		curMovement = Camera_Movement::LEFT;
	}
	else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		curMovement = Camera_Movement::BACKWARD;
	}
	else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		curMovement = Camera_Movement::RIGHT;
	}

	camera.ProcessKeyboard(curMovement, deltaTime, false);
}

float cubeVertices[] = {
	// positions          // texture Coords		//Normal
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,		  0.0f,  0.0f, -1.0f,
	0.5f, -0.5f, -0.5f,  1.0f, 0.0f,		 0.0f,  0.0f, -1.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,		 0.0f,  0.0f, -1.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,		 0.0f,  0.0f, -1.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,		  0.0f,  0.0f, -1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,		  0.0f,  0.0f, -1.0f,

	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,		  0.0f,  0.0f, 1.0f,
	0.5f, -0.5f,  0.5f,  1.0f, 0.0f,		 0.0f,  0.0f, 1.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 1.0f,		 0.0f,  0.0f, 1.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 1.0f,		 0.0f,  0.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,		  0.0f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,		  0.0f,  0.0f, 1.0f,

	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,		 -1.0f,  0.0f,  0.0f,
	-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,		 -1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,		 -1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,		 -1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,		 -1.0f,  0.0f,  0.0f,
	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,		 -1.0f,  0.0f,  0.0f,

	0.5f,  0.5f,  0.5f,  1.0f, 0.0f,		 1.0f,  0.0f,  0.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,		 1.0f,  0.0f,  0.0f,
	0.5f, -0.5f, -0.5f,  0.0f, 1.0f,		 1.0f,  0.0f,  0.0f,
	0.5f, -0.5f, -0.5f,  0.0f, 1.0f,		 1.0f,  0.0f,  0.0f,
	0.5f, -0.5f,  0.5f,  0.0f, 0.0f,		 1.0f,  0.0f,  0.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 0.0f,		 1.0f,  0.0f,  0.0f,

	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,		  0.0f, -1.0f,  0.0f,
	0.5f, -0.5f, -0.5f,  1.0f, 1.0f,		 0.0f, -1.0f,  0.0f,
	0.5f, -0.5f,  0.5f,  1.0f, 0.0f,		 0.0f, -1.0f,  0.0f,
	0.5f, -0.5f,  0.5f,  1.0f, 0.0f,		 0.0f, -1.0f,  0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,		  0.0f, -1.0f,  0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,		  0.0f, -1.0f,  0.0f,

	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,		  0.0f,  1.0f,  0.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,		 0.0f,  1.0f,  0.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 0.0f,		 0.0f,  1.0f,  0.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 0.0f,		 0.0f,  1.0f,  0.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,		  0.0f,  1.0f,  0.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,		  0.0f,  1.0f,  0.0f
};

float planeVertices[] = {
	// x-z 平面
	// positions          // texture Coords		//Normal
	-8.0f, -0.5f, -8.0f,	0.0f, 8.0f,			0.0f,  1.0f, 0.0f,
	8.0f, -0.5f,  8.0f,		8.0f, 0.0f,			0.0f,  1.0f, 0.0f,
	8.0f, -0.5f, -8.0f,		8.0f, 8.0f,			0.0f,  1.0f, 0.0f,

	-8.0f, -0.5f, -8.0f,	0.0f, 8.0f,			0.0f,  1.0f, 0.0f,
	-8.0f, -0.5f,  8.0f,	0.0f, 0.0f,			0.0f,  1.0f, 0.0f,
	8.0f, -0.5f,  8.0f,		8.0f, 0.0f,			0.0f,  1.0f, 0.0f,

};

float canvasVertices[] = {
	// positions        
	-1.0f,	-1.0f,	0.0f,	0.0f, 1.0f,
	1.0f,	-1.0f,	0.0f,	1.0f, 0.0f,
	1.0f,	1.0f,	0.0f,	1.0f, 1.0f,
	1.0f,	1.0f,	0.0f,	0.0f, 1.0f,
	-1.0f,  1.0f, 	0.0f,	0.0f, 0.0f,
	-1.0f, -1.0f,  	0.0f,	1.0f, 0.0f
};

unsigned int planeVAO, planeVBO;
unsigned int cubeVAO, cubeVBO;
unsigned int canvasVAO, canvasVBO;
unsigned int wallTexture;
unsigned int containerTexture;
void Depth_Map_Point(GLFWwindow * window) {
	/*
		流程
		1.首先创建一个立方体贴图
		2.生成立方体贴图的每个面，将它们作为2D深度值纹理图像：
		3.为立方贴图设置纹理参数
		4.使用 glFramebufferTexture 直接附加cubemap作为帧缓冲区的深度附加物。
		5.使用全向阴影映射：
		（1）生成深度贴图。
		（2）利用深度贴图为场景创建阴影。
		6.Light space transform
		7.深度着色器
		8.Omnidirectional shadow maps
	*/
	glEnable(GL_DEPTH_TEST);

	Shader shader("Shader5-3-2.vertex", "Shader5-3-2.fragment");
	Shader depthShader("Shader5-3-2.vertex_depth","Shader5-3-2.geometry_depth", "Shader5-3-2.fragment_depth");

	wallTexture = loadTexture("wall.jpg", false);
	containerTexture = loadTexture("container.jpg", false);
	/*
	首先生成一个depth map--cubeMap， 深度映射是一个深度纹理，基于光源的透视试图渲染
	这里需要使用到frameBuffer
	*/
	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);
	//creating a 2D texture that we'll use as the framebuffer's depth buffer
	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

	unsigned int depthCubemap;
	glGenTextures(1, &depthCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
	for (unsigned int i = 0; i < 6; ++i)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
			SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);//设置cube的每个面为2D的深度值纹理贴图
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	
	//生成的深度映射不应该太复杂，因为我们只想要深度信息！这里我们指定纹理格式为GL_DEPTH_COMPONENT。

	//接下来将贴图纹理附加到framebuffer之上。 注意不需要color buffer。
	//因为OpenGL下，一个帧缓冲区对象如果没有颜色缓冲区是不完整的，故这里显式指定不进行颜色数据的渲染。
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);//绑定深度信息
	glDrawBuffer(GL_NONE);//无绘制缓冲区
	glReadBuffer(GL_NONE);//无读取缓冲区
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	/*canvasShader.use();
	canvasShader.setInt("depthMap", 0);*/

	//VAO , VBO
	//canvas

	glGenVertexArrays(1, &canvasVAO);
	glGenBuffers(1, &canvasVBO);
	glBindVertexArray(canvasVAO);
	glBindBuffer(GL_ARRAY_BUFFER, canvasVAO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(canvasVertices), &canvasVertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
	glBindVertexArray(0);

	//plane

	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(5 * sizeof(float)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	//cube

	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);
	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(5 * sizeof(float)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//light
	glm::vec3 lightPos(0.0f, 1.0f, 0.0f);
	shader.use();
	shader.setVec3("pointLight.ambient", glm::vec3(0.2f, 0.2f, 0.2f));
	shader.setVec3("pointLight.diffuse", glm::vec3(1.0f, 1.0f, 1.0f));
	shader.setVec3("pointLight.specular", glm::vec3(0.4f, 0.4f, 0.4f));
	shader.setFloat1f("pointLight.constant", 1.0f);
	shader.setFloat1f("pointLight.linear", 0.09f);
	shader.setFloat1f("pointLight.quadratic", 0.032f);

	shader.setInt("depthMap", 2);
	shader.setFloat1f("far_plane",100.0f);
	//material 即 纹理贴图
	shader.setInt("material.diffuse", 0);
	shader.setInt("material.specular", 1);
	shader.setFloat1f("material.shininess", 16.0f);

	float curTime{}, lastTime{};
	curTime = lastTime = glfwGetTime();
	while (!glfwWindowShouldClose(window)) {
		//Process Input
		curTime = glfwGetTime();
		ProcessInput(window, curTime - lastTime);
		lastTime = curTime;

		lightPos.z = sin(glfwGetTime() * 0.5) * 3.0;
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 projection;
		glm::mat4 lightSpaceMatrix;

		//Draw

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		//first step --- 生成深度映射，光的视角
		// 1. render depth of scene to texture (from light's perspective)
		// --------------------------------------------------------------
		glCullFace(GL_FRONT);

		model = glm::translate(model, lightPos);	//光的位置
		view = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));	//光的视角
		float aspect = (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT;
		float near_plane = 1.0f;
		float far_plane = 25.0f;
		glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspect, near_plane, far_plane);
		std::vector<glm::mat4> shadowTransforms;
		//按照顺序保存
		/*这里的up向量y轴只能是-1， 换成1的话会出现问题！！
			原因在哪？
			！！！！！！！！！！！！！！！！！！！！！！
			！！！！！！！！！！！！！！！！！！！！！
			！！！！！！！！！！！！！！！！！！！
			！！！！！真的不懂啊！！！！！！！
			！！！！！！！！！！！！！！！
			！！！！！！！！！！！！！
			！！！！！！！！！！！
			！！！！！！！！！
			！！！！！！！
			！！！！！
			！！！
			！
		*/
		shadowTransforms.push_back(shadowProj *
			glm::lookAt(lightPos, lightPos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0,  -1.0, 0.0)));	//x轴正向
		shadowTransforms.push_back(shadowProj *
			glm::lookAt(lightPos, lightPos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));	//x轴负向
		shadowTransforms.push_back(shadowProj *
			glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));	//y轴正向
		shadowTransforms.push_back(shadowProj *
			glm::lookAt(lightPos, lightPos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));//y轴负向
		shadowTransforms.push_back(shadowProj *
			glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0,  -1.0, 0.0))); //z轴正向
		shadowTransforms.push_back(shadowProj *
			glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));	//z轴负向
		depthShader.use();
		depthShader.setVec3("lightPos", lightPos);
		depthShader.setFloat1f("far_plane", far_plane);//!!!! 注意这里和光的perspective 同步，不是和摄像机视野！！！
		for (int i = 0; i < 6; ++i) {
			std::stringstream ss;
			ss << "shadowMatrices[" << i << "]";
			//std::cout << ss.str() << std::endl;
			depthShader.setMartix(ss.str() , shadowTransforms[i]);
		}
		depthShader.setMartix("model" , model);


		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);//清空深度缓冲区，没有颜色缓冲区

		//RenderScene
		renderScene(depthShader);
		glCullFace(GL_BACK); // 不要忘记设回原先的culling face

		//second step --- 进行场景绘制：摄像机视角
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//ConfigureShaderAndMatrices()
		projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
		view = camera.GetViewMatrix();
		//projection = shadowProj;
		//view = glm::lookAt(lightPos, lightPos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
		//Canvas 渲染
		//canvasShader.use();
		//canvasShader.setFloat1f("near_plane", near_plane);
		//canvasShader.setFloat1f("far_plane", far_plane);
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		////RenderScene
		//glBindVertexArray(canvasVAO);
		//glDrawArrays(GL_TRIANGLES,0,6);

		shader.use();
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
		shader.setVec3("pointLight.position", lightPos);
		shader.setMartix("view", view);
		shader.setMartix("projection", projection);
		shader.setVec3("viewerPos", camera.Position);
		shader.setFloat1f("far_plane",25.0f);
		renderScene(shader);

		//swap and poll
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

void renderScene(const Shader &shader)
{

	// floor
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, wallTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, wallTexture);
	glm::mat4 model;
	shader.setMartix("model", model);
	shader.setMartix("normalMatrix", glm::transpose(glm::inverse(model)));
	glBindVertexArray(planeVAO);

	glDrawArrays(GL_TRIANGLES, 0, 6);
	// cubes
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, containerTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, containerTexture);
	glBindVertexArray(cubeVAO);
	model = glm::mat4();
	model = glm::translate(model, glm::vec3(1.0f, -0.25f, 0.0));
	model = glm::scale(model, glm::vec3(0.5f));
	shader.setMartix("model", model);
	shader.setMartix("normalMatrix", glm::transpose(glm::inverse(model)));
	glDrawArrays(GL_TRIANGLES, 0, 36);
	model = glm::mat4();
	model = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0));
	//model = glm::scale(model, glm::vec3(0.5f));
	shader.setMartix("model", model);
	shader.setMartix("normalMatrix", glm::transpose(glm::inverse(model)));
	glDrawArrays(GL_TRIANGLES, 0, 36);
	model = glm::mat4();
	model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 2.0));
	model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
	model = glm::scale(model, glm::vec3(0.25));
	shader.setMartix("model", model);
	shader.setMartix("normalMatrix", glm::transpose(glm::inverse(model)));
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}
unsigned int loadTexture(char const *path, unsigned int Tex_Wrap_Para)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, Tex_Wrap_Para);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, Tex_Wrap_Para);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}
