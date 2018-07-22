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
	GLSL中的函数：
		与C语言相似，需要函数名，返回值类型，且必须在调用前声明
		一个场景中的多光源方法：
			1.一个单独的color向量，代表像素的输出颜色：outputColor.
			2.将每个光的颜色贡献添加到输出颜色向量中(即outputColor)
		
		通常的原则：
			一个场景一般是
				1.一个方向光（一个太阳）
				2.多个点光源
				3.多个聚光源

		因此可以将每一种光的计算方式封装成函数，直接调用函数进行处理即可。

			vec3 outputColor;
				1. output += someFunctionToCalculateDirectionalLight();

				2. output += someFunctionToCalculatePointLight();

				3. output += someFunctionToCalculateSpotLight();

				FragColor = vec4(output,1.0f);
*/

void framebuffer_size_callback(GLFWwindow * window, int width, int height) { glViewport(0, 0, width, height); return; }
void mouse_callback(GLFWwindow* window, double xPos, double yPos);
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
void ProcessInput(GLFWwindow * window, float deltaTime);

unsigned int loadTexture(char const * path);
unsigned int loadVertexToVBO(float const * data);

const int SCR_WIDTH = 800;
const int SCR_HEIGHT = 600;

void MultipleLights(GLFWwindow* window);

glm::vec3 cameraPos(0.0f,0.0f,2.0f);
glm::vec3 WorldUp(0.0f,1.0f,0.0f);

float yaw = -90.0f, pitch = 0.0f; 

Camera camera(cameraPos,WorldUp,yaw,pitch);
int main(void) {
	if (!glfwInit()) {
		std::cout << "Initializing Failed" << std::endl;
	}

	//Set
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow * window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "2-6_MultipleLights",NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create the window" << std::endl;
		glfwTerminate();
		system("pause");
		return -1;
	}

	//注册环境
	glfwMakeContextCurrent(window);
	//初始化回调
	glfwSetFramebufferSizeCallback(window,framebuffer_size_callback);
	glfwSetInputMode(window,GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		system("pause");
		return -1;
	}

	//
	MultipleLights(window);
	glfwTerminate();

}

/*
鼠标向左移动，x值减小；向右移动，x值增大
鼠标向上移动，y值减小；向下移动，y值增大
*/
bool firstMouse = true;
static float lastX = 400.0f, lastY = 300.0f;
void mouse_callback(GLFWwindow* window, double xPos, double yPos) {
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
void ProcessInput(GLFWwindow * window , float deltaTime) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window,true);
	}
	
	//相机按键控制
	Camera_Movement curMovement = Camera_Movement::NONE;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		curMovement = Camera_Movement::FORWARD;
	}else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		curMovement = Camera_Movement::LEFT;
	}else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		curMovement = Camera_Movement::BACKWARD;
	}else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
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

void MultipleLights(GLFWwindow* window) {
	glEnable(GL_DEPTH_TEST);
	//Shader
	Shader objectShader("Shader2-6.vertex", "Shader2-6.fragment");
	Shader lightShader("lamp.vertex", "lamp.fragment");

	stbi_set_flip_vertically_on_load(true);//反转y轴，否则图片上下颠倒！
	unsigned int diffuseMap =  loadTexture("container2.png");
	unsigned int specularMap = loadTexture("container2_specular.png");


	unsigned int objectVAO;
	unsigned int dataVBO ;//= loadVertexToVBO(cubeVertices)
	glGenVertexArrays(1, &objectVAO);
	glGenBuffers(1,&dataVBO);
	glBindVertexArray(objectVAO);
	glBindBuffer(GL_ARRAY_BUFFER, dataVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	unsigned int lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);
	glBindBuffer(GL_ARRAY_BUFFER, dataVBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//定义物体材质和所有光照
	//物体材质
	objectShader.use();
	objectShader.setInt("material.diffuse", 0);
	objectShader.setInt("material.specular", 1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, diffuseMap);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, specularMap);
	objectShader.setFloat1f("material.shininess", 32.0f);
	//三种光照
		//Directional light
	glm::vec3 ambient(0.2f, 0.2f, 0.2f);
	glm::vec3 diffuse(0.5f, 0.5f, 0.5f);
	glm::vec3 specular(1.0f, 1.0f, 1.0f);
	glm::vec3 lightDir(-0.2f, -1.0f, -0.3f);
	objectShader.setVec3("directLight.direction", lightDir);
	objectShader.setVec3("directLight.ambient", ambient);
	objectShader.setVec3("directLight.diffuse", diffuse);
	objectShader.setVec3("directLight.specular", specular);
		//Point lights
	glm::vec3 pointLightPositions[] = {
		glm::vec3(0.7f,  0.2f,  2.0f),
		glm::vec3(2.3f, -3.3f, -4.0f),
		glm::vec3(-4.0f,  2.0f, -12.0f),
		glm::vec3(0.0f,  0.0f, -3.0f)
	};
	objectShader.setVec3("pointLight[0].position", pointLightPositions[0]);
	objectShader.setVec3("pointLight[1].position", pointLightPositions[1]);
	objectShader.setVec3("pointLight[2].position", pointLightPositions[2]);
	objectShader.setVec3("pointLight[3].position", pointLightPositions[3]);

	objectShader.setVec3("pointLight[0].ambient", ambient);
	objectShader.setVec3("pointLight[1].ambient", ambient);
	objectShader.setVec3("pointLight[2].ambient", ambient);
	objectShader.setVec3("pointLight[3].ambient", ambient);

	objectShader.setVec3("pointLight[0].diffuse", diffuse);
	objectShader.setVec3("pointLight[1].diffuse", diffuse);
	objectShader.setVec3("pointLight[2].diffuse", diffuse);
	objectShader.setVec3("pointLight[3].diffuse", diffuse);

	objectShader.setVec3("pointLight[0].specular", specular);
	objectShader.setVec3("pointLight[1].specular", specular);
	objectShader.setVec3("pointLight[2].specular", specular);
	objectShader.setVec3("pointLight[3].specular", specular);


	objectShader.setFloat1f("pointLight[0].constant", 1.0f);
	objectShader.setFloat1f("pointLight[1].constant", 1.0f);
	objectShader.setFloat1f("pointLight[2].constant", 1.0f);
	objectShader.setFloat1f("pointLight[3].constant", 1.0f);

	objectShader.setFloat1f("pointLight[0].linear", 0.09f);
	objectShader.setFloat1f("pointLight[1].linear", 0.09f);
	objectShader.setFloat1f("pointLight[2].linear", 0.09f);
	objectShader.setFloat1f("pointLight[3].linear", 0.09f);

	objectShader.setFloat1f("pointLight[0].quadratic", 0.032f);
	objectShader.setFloat1f("pointLight[1].quadratic", 0.032f);
	objectShader.setFloat1f("pointLight[2].quadratic", 0.032f);
	objectShader.setFloat1f("pointLight[3].quadratic", 0.032f);
		//Spot light
	objectShader.setVec3("spotLight.ambient", ambient);
	objectShader.setVec3("spotLight.diffuse", diffuse);
	objectShader.setVec3("spotLight.specular", specular);
	objectShader.setFloat1f("spotLight.constant", 1.0f);
	objectShader.setFloat1f("spotLight.linear", 0.09f);
	objectShader.setFloat1f("spotLight.quadratic", 0.032f);
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

	float curTime{}, lastTime= glfwGetTime();
	while (!glfwWindowShouldClose(window)) {
		//ProcessInput
		curTime = glfwGetTime();
		ProcessInput(window,curTime - lastTime);
		lastTime = curTime;

		//Color
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//render
		glm::mat4 model, view, projection;
		view = camera.GetViewMatrix();
		projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH/(float)SCR_HEIGHT , 0.1f , 100.0f );
		
		//渲染所有的点光源
		lightShader.use();
		lightShader.setMartix("view",view);
		lightShader.setMartix("projection",projection);
		for (int i = 0; i <4 ; i++) {
			model = glm::mat4();
			model = glm::translate(model, pointLightPositions[i] );
			model = glm::scale(model , glm::vec3(0.2f,0.2f,0.2f));
			lightShader.setMartix("model", model);

			glBindVertexArray(lightVAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			
		}
		
		//渲染所有的物体
		objectShader.use();
		objectShader.setVec3("spotLight.position", camera.Position);
		objectShader.setVec3("spotLight.direction", camera.Front);
		objectShader.setMartix("view",view);
		objectShader.setMartix("projection", projection);
		for (int i = 0; i < 10; i++) {
			model = glm::mat4();
			model = glm::translate(model, cubePositions[i]);
			model = glm::rotate(model, (float)glm::radians(i * 10.0f*glfwGetTime()), glm::vec3(1.0f, 0.3f, 0.5f));
			objectShader.setMartix("model", model);
			objectShader.setMartix("normalMatrix", glm::inverse(glm::transpose(model)));
			glBindVertexArray(objectVAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		//swap and poll
		glfwSwapBuffers(window);
		glfwPollEvents();

	}
	return;
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const * path)
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
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		}
	stbi_image_free(data);
	return textureID;
}

unsigned int loadVertexToVBO(float const * data) {
	unsigned int vertexBufferObjectID;
	glGenBuffers(1, &vertexBufferObjectID);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObjectID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(data), data , GL_STATIC_DRAW);
	return vertexBufferObjectID;
}

