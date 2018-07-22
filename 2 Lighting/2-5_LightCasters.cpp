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


/*light caster. 
	投光物

	Directional Light（定向光）
		类似太阳，该光照一般保持强度不变，并且所有的物体乃至顶点都被同一个方向的光照射，
		因为光是平行的。

	Point light（点光）
		点光源的最大特点即：
			存在衰弱现象：Attenuation
			离的近的被照亮和远的则不会。
				一方面我们可以简单的使用线性衰弱，但这不符合实际情况
				（光线的衰弱速度会随着距离的拉长而逐渐下降）
				公式如下：
						Fatt = 1.0 / (Kc + Kl * d + Kq * d2) 
					其中：
						d：即距离
						d2:即距离的平方
						Kc、Kl、Kq 分别表示常量、线性和平方的系数。
					Kc：常量系数通常为1.0.其目的是确保最后的结果不会小于1.0（小于1表示随着距离增加亮度增加）

					Kl：表示以线性的方式减少亮度

					Kq：二次项在距离比较近的时候相比一次项会比一次项更小，但是当距离更远的时候比一次项更大
			现在的问题是：
				我们应该如何设置这3个系数的值？
					这依赖于很多因素：the environment, the distance you want a light to cover, the type of light
						大多数场合，这是经验的问题，也要适度调整。
				下面是一个表格：

						Distance	Constant	  Linear	Quadratic
							7			1.0			0.7			1.8
							13			1.0			0.35		0.44
							20			1.0			0.22		0.20
							32			1.0			0.14		0.07
							50			1.0			0.09		0.032
							65			1.0			0.07		0.017
							100			1.0			0.045		0.0075
							160			1.0			0.027		0.0028
							200			1.0			0.022		0.0019
							325			1.0			0.014		0.0007
							600			1.0			0.007		0.0002
							3250		1.0			0.0014		0.000007
						Kc永远是1
						Kl会随着 distance的增大而越来越小，但应始终大于Quadraic
						Kq则会非常小


	Spotlights(聚光)：聚光形成了一个圆锥光体
		注意聚光灯也需要考虑光的衰弱现象
		在OpenGL中，要表示一个聚光需要以下参数：
			 a world-space position：	一个世界空间坐标
			 a direction :				方向
			 a cutoff angle：			切光角：说明了聚光半径
			 我们计算的每个片段，如果片段在聚光的切光方向之间
			LightDir：从片段指向光源的向量。
			SpotDir：聚光所指向的方向。
			PhiΦ：定义聚光半径的切光角。每个落在这个角度之外的，聚光都不会照亮。
			Thetaθ：LightDir向量和SpotDir向量之间的角度。θ值应该比Φ值小，这样才会在聚光内。	
	
	Smooth/Soft edges:平滑/软化边缘
		一个真实的聚光的光会在它的边界处平滑减弱的。
			因此我们需要构建一个参数，可以在一定范围内将漫反射和镜面反射进行不断的弱化，知道最后仅剩环境光。
		这里的一个方案是采用了两个半径：
			内圆锥半径
			外圆锥半径
				在两个半径之间进行光的弱化。
*/
void framebuffer_size_callback(GLFWwindow * window, int width, int height) { glViewport(0, 0, width, height); }
void mouse_callback(GLFWwindow * window, double xPos, double yPos);
void scroll_callback(GLFWwindow * window, double xOffset, double yOffset);
void ProcessInput(GLFWwindow * window, float deltaTime);
const int SCR_WIDTH = 800;
const int SCR_HEIGHT = 600;

void LightCasters_Directional(GLFWwindow * window);
void LightCasters_Point(GLFWwindow * window);
void LightCasters_Spot(GLFWwindow * window);
glm::vec3 cameraPos(0.0f, 0.0f, 3.0f);
glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);
float yaw = -90.0f, pitch = 0.0f;
Camera camera(cameraPos, cameraUp, yaw, pitch);

bool openSpecular = true;
int lastStatus;

int main(void) {
	if (!glfwInit()) {
		std::cout << "Initializing Failed" << std::endl;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow * window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "2-3_materials_tasks", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create the window" << std::endl;
		glfwTerminate();
		system("pause");
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		system("pause");
		return -1;
	}

	//LightCasters_Point(window);
	LightCasters_Spot(window);
	glfwTerminate();
}


/*
鼠标向左移动，x值减小；向右移动，x值增大
鼠标向上移动，y值减小；向下移动，y值增大
*/
bool firstMouse = true;
static float lastX = 400.0f, lastY = 300.0f;
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
	std::cout << camera.Zoom << std::endl;
	camera.ProcessMouseScroll(yOffset);
}
void ProcessInput(GLFWwindow * window, float deltaTime) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE && lastStatus == GLFW_PRESS) {
		//使用空格来开关镜面反射效果
		if (openSpecular) openSpecular = false;
		else openSpecular = true;
	}
	lastStatus = glfwGetKey(window, GLFW_KEY_SPACE);

	//按键的处理
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
/*包括了每个顶点及其法向量,以及漫反射贴图
*/
float cubeVertices[] = {
	// positions          // normals           // texture coords
	-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
	0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
	0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
	0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
	-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

	-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
	0.5f, -0.5f,  0.5f,   0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
	0.5f,  0.5f,  0.5f,   0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
	0.5f,  0.5f,  0.5f,   0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

	-0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

	0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
	0.5f,  0.5f, -0.5f,   1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
	0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
	0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
	0.5f, -0.5f,  0.5f,   1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
	0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

	-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
	0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
	0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
	0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

	-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
	0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
	0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
	0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
};

/*平行光
*/
void LightCasters_Directional(GLFWwindow * window) {
	glEnable(GL_DEPTH_TEST);
	//Shader
	Shader objectShader("Shader2-5.vertex", "Shader2-5.fragment");
	Shader lightShader("lamp.vertex", "lamp.fragment");

	//Texture
	unsigned int diffuseMap, specularMap;
	glGenTextures(1, &diffuseMap);	glGenTextures(1, &specularMap);
	glBindTexture(GL_TEXTURE_2D, diffuseMap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_set_flip_vertically_on_load(true);//反转y轴，否则图片上下颠倒！
	int width, height, nrChannels;//宽、高、 number of color channels（颜色通道数）、
	unsigned char * data = stbi_load("container2.png", &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	stbi_image_free(data);

	glBindTexture(GL_TEXTURE_2D, specularMap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	data = stbi_load("container2_specular.png", &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	stbi_image_free(data);
	glBindTexture(GL_TEXTURE_2D, 0);
	//VAO,VBO
	unsigned objectVAO, lightVAO, VBO;
	glGenVertexArrays(1, &objectVAO);
	glGenVertexArrays(1, &lightVAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(objectVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glBindVertexArray(lightVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//之前已经传过数据了，这里就不用重复操作
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glm::vec3 lightPos(1.2f, 1.0f, 3.0f);
	objectShader.use();

	//这是物体的颜色
	objectShader.setInt("material.diffuse", 0);
	objectShader.setInt("material.specular", 1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, diffuseMap);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, specularMap);
	objectShader.setVec3("material.specular", silver.specular);
	objectShader.setFloat1f("material.shininess", 32.0f);

	//光的颜色
	glm::vec3 ambient(0.8f, 0.8f, 0.8f);
	glm::vec3 diffuse(0.9f, 0.9f, 0.9f);
	glm::vec3 specular(1.0f, 1.0f, 1.0f);
	glm::vec3 lightDir(-0.2f, -1.0f, -0.3f);
	objectShader.setVec3("directLight.direction", lightDir);
	objectShader.setVec3("directLight.ambient", ambient);
	objectShader.setVec3("directLight.diffuse", diffuse);
	objectShader.setVec3("directLight.specular", specular);


	glm::vec3 cubePositions[] = {
		glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(2.0f,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f,  2.0f, -2.5f),
		glm::vec3(1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	};
	float curTime{}, lastTime{};
	while (!glfwWindowShouldClose(window)) {

		//ProcessInput
		curTime = glfwGetTime();
		ProcessInput(window, curTime - lastTime);
		lastTime = curTime;

		//Color
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		//Render
		glm::mat4 model, view, projection;
		view = camera.GetViewMatrix();
		projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

		lightShader.use();
		lightShader.setMartix("view", view);
		lightShader.setMartix("projection", projection);
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.2f));
		lightShader.setMartix("model", model);

		glBindVertexArray(lightVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		objectShader.use();
		objectShader.setBool("openSpecular", openSpecular);
		objectShader.setVec3("viewerPos", camera.Position);
		objectShader.setMartix("view", view);
		objectShader.setMartix("projection", projection);
		for (int i = 0; i < 10; ++i) {
			model = glm::mat4();
			model = glm::translate(model,cubePositions[i]);
			model = glm::rotate(model, (float)glm::radians( i * 10.0f*glfwGetTime()), glm::vec3(1.0f, 0.3f, 0.5f));
			objectShader.setMartix("model", model);
			objectShader.setMartix("normalMatrix", glm::inverse(glm::transpose(model)));
			glBindVertexArray(objectVAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		

		//Swap and Poll
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}
/*点光源
*/
void LightCasters_Point(GLFWwindow * window) {
	glEnable(GL_DEPTH_TEST);
	//Shader
	Shader objectShader("Shader2-5.vertex", "Shader2-5.fragment_point");
	Shader lightShader("lamp.vertex", "lamp.fragment");

	//Texture
	unsigned int diffuseMap, specularMap;
	glGenTextures(1, &diffuseMap);	glGenTextures(1, &specularMap);
	glBindTexture(GL_TEXTURE_2D, diffuseMap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_set_flip_vertically_on_load(true);//反转y轴，否则图片上下颠倒！
	int width, height, nrChannels;//宽、高、 number of color channels（颜色通道数）、
	unsigned char * data = stbi_load("container2.png", &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	stbi_image_free(data);

	glBindTexture(GL_TEXTURE_2D, specularMap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	data = stbi_load("container2_specular.png", &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	stbi_image_free(data);
	glBindTexture(GL_TEXTURE_2D, 0);
	//VAO,VBO
	unsigned objectVAO, lightVAO, VBO;
	glGenVertexArrays(1, &objectVAO);
	glGenVertexArrays(1, &lightVAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(objectVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glBindVertexArray(lightVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//之前已经传过数据了，这里就不用重复操作
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glm::vec3 lightPos(1.2f, 1.0f, 3.0f);
	objectShader.use();

	//这是物体的颜色
	objectShader.setInt("material.diffuse", 0);
	objectShader.setInt("material.specular", 1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, diffuseMap);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, specularMap);
	objectShader.setVec3("material.specular", silver.specular);
	objectShader.setFloat1f("material.shininess", 32.0f);

	//光的颜色
	glm::vec3 ambient(0.8f, 0.8f, 0.8f);
	glm::vec3 diffuse(0.9f, 0.9f, 0.9f);
	glm::vec3 specular(1.0f, 1.0f, 1.0f);
	glm::vec3 lightDir(-0.2f, -1.0f, -0.3f);
	objectShader.setVec3("pointLight.position", lightPos);

	objectShader.setVec3("pointLight.ambient", ambient);
	objectShader.setVec3("pointLight.diffuse", diffuse);
	objectShader.setVec3("pointLight.specular", specular);

	objectShader.setFloat1f("pointLight.constant", 1.0f);
	objectShader.setFloat1f("pointLight.linear", 0.045f);
	objectShader.setFloat1f("pointLight.quadratic", 0.0075f);

	glm::vec3 cubePositions[] = {
		glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(2.0f,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f,  2.0f, -2.5f),
		glm::vec3(1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	};
	float curTime{}, lastTime{};
	while (!glfwWindowShouldClose(window)) {

		//ProcessInput
		curTime = glfwGetTime();
		ProcessInput(window, curTime - lastTime);
		lastTime = curTime;

		//Color
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		//Render
		glm::mat4 model, view, projection;
		view = camera.GetViewMatrix();
		projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

		lightShader.use();
		lightShader.setMartix("view", view);
		lightShader.setMartix("projection", projection);
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.2f));
		lightShader.setMartix("model", model);

		glBindVertexArray(lightVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		objectShader.use();
		objectShader.setBool("openSpecular", openSpecular);
		objectShader.setVec3("viewerPos", camera.Position);
		objectShader.setMartix("view", view);
		objectShader.setMartix("projection", projection);
		for (int i = 0; i < 10; ++i) {
			model = glm::mat4();
			model = glm::translate(model, cubePositions[i]);
			model = glm::rotate(model, (float)glm::radians(i * 10.0f*glfwGetTime()), glm::vec3(1.0f, 0.3f, 0.5f));
			objectShader.setMartix("model", model);
			objectShader.setMartix("normalMatrix", glm::inverse(glm::transpose(model)));
			glBindVertexArray(objectVAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}


		//Swap and Poll
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}
/*聚光灯
*/
void LightCasters_Spot(GLFWwindow * window) {
	glEnable(GL_DEPTH_TEST);
	//Shader
	Shader objectShader("Shader2-5.vertex", "Shader2-5.fragment_spot");
	Shader lightShader("lamp.vertex", "lamp.fragment");

	//Texture
	unsigned int diffuseMap, specularMap;
	glGenTextures(1, &diffuseMap);	glGenTextures(1, &specularMap);
	glBindTexture(GL_TEXTURE_2D, diffuseMap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_set_flip_vertically_on_load(true);//反转y轴，否则图片上下颠倒！
	int width, height, nrChannels;//宽、高、 number of color channels（颜色通道数）、
	unsigned char * data = stbi_load("container2.png", &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	stbi_image_free(data);

	glBindTexture(GL_TEXTURE_2D, specularMap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	data = stbi_load("container2_specular.png", &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	stbi_image_free(data);
	glBindTexture(GL_TEXTURE_2D, 0);
	//VAO,VBO
	unsigned objectVAO, lightVAO, VBO;
	glGenVertexArrays(1, &objectVAO);
	glGenVertexArrays(1, &lightVAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(objectVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glBindVertexArray(lightVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//之前已经传过数据了，这里就不用重复操作
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glm::vec3 lightPos(1.2f, 1.0f, 3.0f);
	objectShader.use();

	//这是物体的颜色
	objectShader.setInt("material.diffuse", 0);
	objectShader.setInt("material.specular", 1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, diffuseMap);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, specularMap);
	objectShader.setVec3("material.specular", silver.specular);
	objectShader.setFloat1f("material.shininess", 32.0f);

	//光的颜色
	glm::vec3 ambient(0.8f, 0.8f, 0.8f);
	glm::vec3 diffuse(0.9f, 0.9f, 0.9f);
	glm::vec3 specular(1.0f, 1.0f, 1.0f);
	glm::vec3 lightDir(-0.2f, -1.0f, -0.3f);

	objectShader.setVec3("spotLight.ambient", ambient);
	objectShader.setVec3("spotLight.diffuse", diffuse);
	objectShader.setVec3("spotLight.specular", specular);
	objectShader.setFloat1f("spotLight.innerCutOff", glm::cos(glm::radians(12.5f)));
	objectShader.setFloat1f("spotLight.outerCutOff", glm::cos(glm::radians(17.5f)));

	glm::vec3 cubePositions[] = {
		glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(2.0f,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f,  2.0f, -2.5f),
		glm::vec3(1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	};
	float curTime{}, lastTime{};
	while (!glfwWindowShouldClose(window)) {

		//ProcessInput
		curTime = glfwGetTime();
		ProcessInput(window, curTime - lastTime);
		lastTime = curTime;

		//Color
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		//Render
		glm::mat4 model, view, projection;
		view = camera.GetViewMatrix();
		projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

		lightShader.use();
		lightShader.setMartix("view", view);
		lightShader.setMartix("projection", projection);
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.2f));
		lightShader.setMartix("model", model);

		glBindVertexArray(lightVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		objectShader.use();
		objectShader.setVec3("spotLight.position", camera.Position);
		objectShader.setVec3("spotLight.direction", camera.Front);
		objectShader.setBool("openSpecular", openSpecular);
		objectShader.setVec3("viewerPos", camera.Position);
		objectShader.setMartix("view", view);
		objectShader.setMartix("projection", projection);
		for (int i = 0; i < 10; ++i) {
			model = glm::mat4();
			model = glm::translate(model, cubePositions[i]);
			model = glm::rotate(model, (float)glm::radians(i * 10.0f*glfwGetTime()), glm::vec3(1.0f, 0.3f, 0.5f));
			objectShader.setMartix("model", model);
			objectShader.setMartix("normalMatrix", glm::inverse(glm::transpose(model)));
			glBindVertexArray(objectVAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}


		//Swap and Poll
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}