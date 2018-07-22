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

/*Basic Light
	OpenGL的光照仅仅使用了简化的模型并基于对现实的估计来进行模拟，这样处理起来会更容易一些

	light model：光照模型
		Phong lighting model：
			冯氏光照模型(Phong Lighting Model)
			该模型包含3个元素：
				Ambient:环境
					即使在黑暗的情况下，世界上也仍然有一些光亮(月亮、一个来自远处的光)，
					所以物体永远不会是完全黑暗的。我们使用环境光照来模拟这种情况，也就是无论如何永远都给物体一些颜色。
				Diffuse:漫反射
					模拟一个发光物对物体的方向性影响(Directional Impact)。它是冯氏光照模型最显著的组成部分。面向光源的一面比其他面会更亮。
				Specular:镜面
					模拟有光泽物体上面出现的亮点，镜面光照的颜色，相比于物体的颜色更倾向于光的颜色。
*/

/*Ambient lighting
	光通常不会仅来自一个光源。
		光的一个属性是：
				它可以沿不同方向进行分散和反弹，并到达一些不能直接照射到的点。
			光能够像这样反射(Reflect)到其他表面，一个物体的光照可能受到来自一个非直射的光源影响。
		考虑到这种情况的算法叫做全局照明(Global Illumination)算法，但是这种算法既开销高昂又极其复杂。
	
	全局照明的简化版本：
		ambient lighting 环境照明
		使用一个(数值)很小的常量(光)颜色添加到物体片段的最终颜色里(片段，Fragment，指当前讨论的光线在物体上的照射点)
		这看起来就像即使没有直射光源也始终存在着一些发散的光。
*/

/*Diffuse lighting
	漫反射光照(Diffuse Lighting)
	漫反射光使物体上与光线排布越近的片段越能从光源处获得更多的亮度。

	我们需要测量这个光线与它所接触片段之间的角度。如果光线垂直于物体表面，这束光对物体的影响会最大化
		因此要计算漫反射，我们需要：
					1.法向量(Normal Vector)：垂直于顶点表面的向量
					2.定向的光线：作为光的位置和片段的位置之间的向量差的方向向量。通过光源位置和顶点位置获得。
	
	Cube的坐标转换成了空间坐标，且FragDirection也是转换后的点到光源的方向，但是
		法向量是从来没有变化过的（因为是固定的）。所以有光的点也不会出现多少变化。
	如果要解决这个问题，就需要对法向量进行更新，使之能跟随点的变化进行相应的变化（一般来讲是旋转的影响）。
		实际上:
			这不是简单地把它乘以一个模型矩阵就能搞定的
			首先，法向量只是一个方向向量，不能表达空间中的特定位置。
			同时，法向量没有齐次坐标(顶点位置中的w分量)。这意味着，平移不应该影响到法向量。
			因此，如果我们打算把法向量乘以一个模型矩阵，我们就要把模型矩阵的平移部分移除，即仅仅使用该矩阵的左上角的3*3部分。
			(注意，我们也可以把法向量的w分量设置为0，再乘以4×4矩阵；同样可以移除平移)。
			对于法向量，我们只能对它应用缩放(Scale)和旋转(Rotation)变换。
		
		而当进行了不等比缩放的时候，法向量很可能被破坏掉！！！即使用模型矩阵转换后的法向量将不再垂直于表面！
			因此，实际上，我们可以为法向量单独设置一个模型矩阵用于转换，该矩阵被称为Normal matrix:
				即为model的逆的转置，或转置的逆也一样ok！ （转置和逆的可交换性）
				Normal = mat3(transpose(inverse(model))) * normal;
			
			证明如下：
				令	N为	normal vector	法向量
					T为 tangent vector	即正切向量	有 NT = TN = 0	这里是指点积

					另T的变换矩阵为M	N的变换矩阵为G
					变换为为 T'				N'
					即：
						T'N' = TN = 0 = NT = N'T'
					其中	N' = G*N		T' = M*T	
					N'T' = transpose(T') * T'		transpose即表示转置，这里是描述了点积和矩阵的关系	
					故有：
						N'T' = transpose(M * T) * (G * N) = transpose(T) * transpose(M) * G * N		这里使用了转置的交换

						又因为N'T' = 0 = T * N = transpose(T) * N
						即transpose(M) * G = I 即单位矩阵
							故 G = inverse(transpose(M))	即model矩阵的转置的逆，或者逆的转置（因为可交换）。
						
						该公式适用于所有的法向量变换！！

			
*/

/*Specular Lighting	镜面反射
	
*/

void framebuffer_size_callback(GLFWwindow * window, int width, int height) { glViewport(0, 0, width, height);	return; }
void mouse_callback(GLFWwindow * window, double xPos, double yPos);
void scroll_callback(GLFWwindow * window, double xOffset, double yOffset);
void ProcessInput(GLFWwindow * window);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

static float lastX = 400.0f, lastY = 300.0f;
float pitch, yaw = -90.0f;
float aspect = 45.0f;

float deltaTime, curTime, lastTime;
glm::vec3 cameraPos = glm::vec3(0.0f, 3.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

Camera camera(cameraPos, cameraUp, yaw, -45.0f);//定义了一个相机, front 由yaw 和 pitch确定



void Light(GLFWwindow * window);
int main(void) {
	if (!glfwInit()) {
		std::cout << "Failed to Initialization" << std::endl;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow * window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "2-2 BasicLight", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create the window" << std::endl;
		glfwTerminate();
		system("pause");
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);	//光标回调函数
	glfwSetScrollCallback(window, scroll_callback);		//鼠标滚轮监测
														//loading GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		system("pause");
		return -1;
	}

	Light(window);
	glfwTerminate();
	return 0;
}


void ProcessInput(GLFWwindow * window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	Camera_Movement camera_Movement;


	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera_Movement = Camera_Movement::FORWARD;
	else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera_Movement = Camera_Movement::BACKWARD;
	else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera_Movement = Camera_Movement::LEFT;
	else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera_Movement = Camera_Movement::RIGHT;
	else
		camera_Movement = Camera_Movement::NONE;
	//std::cout << deltaTime << std::endl;
	camera.ProcessKeyboard(camera_Movement, deltaTime, false);
	return;
}


/*包括了每个顶点及其法向量
*/
float cubeVertices[] = {
	//每个面6个顶点，
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 

    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,

    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
};

void Light(GLFWwindow * window) {
	glEnable(GL_DEPTH_TEST);
	//Shader shader("shader2-1.vertex", "shader2-1.fragment");
	Shader lightShader("shader2-2.vertex", "shader2-2.fragment");
	Shader lampShader("shader2-2.vertex", "lamp.fragment");
	//Vertices
	unsigned int VAO, VBO, lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glBindVertexArray(lightVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//之前已经传过数据了，这里就不用重复操作
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//while
	glm::vec3 lightPos(1.2f, 1.0f, -2.0f);
	lightShader.use();
	glm::vec3 objectColor(1.0f, 0.5f, 0.31f);
	glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
	lightShader.setVec3("objectColor", objectColor);
	lightShader.setVec3("lightColor", lightColor);
	lightShader.setVec3("lightPos", lightPos);

	while (!glfwWindowShouldClose(window)) {
		//Process Input
		ProcessInput(window);

		curTime = glfwGetTime();
		deltaTime = curTime - lastTime;
		lastTime = curTime;

		//render
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		lightShader.setVec3("viewerPos", camera.Position);

		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 projection;
		view = camera.GetViewMatrix();	//直接获取相机
		projection = glm::perspective(glm::radians(aspect), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		lampShader.setMartix("projection", projection);

		//渲染lamp
		lampShader.use();
		lampShader.setMartix("view", view);
		lampShader.setMartix("projection", projection);
		model = glm::mat4();
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.2f));	//缩放
		lampShader.setMartix("model", model);
		glBindVertexArray(lightVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		//渲染其他物体.
		lightShader.use();
		lightShader.setMartix("view", view);
		lightShader.setMartix("projection", projection);
		model = glm::mat4();
		model = glm::rotate(model,(float)glm::radians(10.0f * glfwGetTime()),glm::vec3(0.0f,1.0f,0.0f));
		lightShader.setMartix("model", model);//使用标准model,不进行任何操作!
		lightShader.setMartix("normalMatrix",glm::inverse(glm::transpose(model)));	//适应于法向量的变换矩阵
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		//swap & Polly
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	return;
}

bool firstMouse = true;
/*
鼠标向左移动，x值减小；向右移动，x值增大
鼠标向上移动，y值减小；向下移动，y值增大
*/
void mouse_callback(GLFWwindow * window, double xPos, double yPos) {
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

void scroll_callback(GLFWwindow * window, double xOffset, double yOffset) {
	camera.ProcessMouseScroll(xOffset);
}