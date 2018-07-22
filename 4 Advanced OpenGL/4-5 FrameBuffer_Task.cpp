#include <glad\glad.h>
#include <glfw3.h>

//数学库
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <map>
#include "Shaders.h"
#include "Camera.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"



/*流程
	1.创建一个帧缓冲,注意绑定
	2.为帧缓冲创建附加物：
	颜色缓冲：一个纹理缓冲对象
	深度_模板缓冲：
	若要采集数据（读操作），使用纹理缓冲
	若不进行读操作，使用渲染缓冲
	3.将附加物附加到帧缓冲上
	4.检查帧缓冲是否完整
	5.解绑
*/

unsigned int loadTexture(char const *path, unsigned int Tex_Wrap_Para);
void framebuffer_size_callback(GLFWwindow * window, int width, int height) { glViewport(0, 0, width, height); return; }
void mouse_callback(GLFWwindow* window, double xPos, double yPos);
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
void ProcessInput(GLFWwindow * window, float deltaTime);

unsigned int loadTexture(char const * path);
unsigned int loadVertexToVBO(float const * data);

const int SCR_WIDTH = 800;
const int SCR_HEIGHT = 600;

void BackMirror_Task(GLFWwindow* window);


glm::vec3 cameraPos(0.0f, 2.0f, 2.0f);
glm::vec3 WorldUp(0.0f, 1.0f, 0.0f);

float yaw = -90.0f, pitch = 0.0f;
bool openEffect = true;
int lastStatus;

Camera camera(cameraPos, WorldUp, yaw, pitch);
int main(void) {
	if (!glfwInit()) {
		std::cout << "Initializing Failed" << std::endl;
	}

	//Set
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow * window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "3-3_Model", NULL, NULL);
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
	BackMirror_Task(window);
	glfwTerminate();

}

/*
鼠标向左移动，x值减小；向右移动，x值增大
鼠标向上移动，y值减小；向下移动，y值增大
*/
bool firstMouse = true;
static float lastX = 400.0f, lastY = 300.0f;
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
		if (openEffect) openEffect = false;
		else openEffect = true;
	}
	lastStatus = glfwGetKey(window, GLFW_KEY_SPACE);

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
	// positions          // texture Coords
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
	0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
	0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
};
float planeVertices[] = {
	// positions          // texture Coords (note we set these higher than 1 (together with GL_REPEAT as texture wrapping mode). this will cause the floor texture to repeat)
	5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
	-5.0f, -0.5f,  5.0f,  0.0f, 0.0f,
	-5.0f, -0.5f, -5.0f,  0.0f, 2.0f,

	5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
	-5.0f, -0.5f, -5.0f,  0.0f, 2.0f,
	5.0f, -0.5f, -5.0f,  2.0f, 2.0f
};


unsigned GenOneFrameBuffer() {
	unsigned int FBO;
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	//Create Color and depth and stencil
	//Texture
	unsigned int texColorBuffer;
	glGenTextures(1, &texColorBuffer);
	glBindTexture(GL_TEXTURE_2D, texColorBuffer);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	unsigned int renderBufferObject;
	glGenRenderbuffers(1, &renderBufferObject);
	glBindRenderbuffer(GL_RENDERBUFFER, renderBufferObject);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 600);
	//当我们为渲染缓冲对象分配了足够的内存之后，我们可以解绑这个渲染缓冲。
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBufferObject);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	//检验
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	}

	//解绑
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return FBO;
}

void BackMirror_Task(GLFWwindow* window) {
	//使能深度测试
	glEnable(GL_DEPTH_TEST);//深度测试

	Shader shader("Shader4-5.vertex_tasks", "Shader4-5.fragment_tasks");
	Shader shaderCanvas("Shader4-5.vertex_canvas", "Shader4-5.fragment_canvas");


	stbi_set_flip_vertically_on_load(true);//反转y轴，否则图片上下颠倒！
										   //Texture
	unsigned int cubeTexture = loadTexture(std::string("container.jpg").c_str(), GL_REPEAT);
	unsigned int floorTexture = loadTexture(std::string("wall.jpg").c_str(), GL_REPEAT);

	//***** frameBufferObject
	unsigned int frameBufferObject;
	glGenFramebuffers(1, &frameBufferObject);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject);
	//在绑定到GL_FRAMEBUFFER目标之后，所有的读取和写入帧缓冲的操作将会影响当前绑定的帧缓冲。
	//也可以通过绑定到GL_READ_FRAMEBUFFER来读取或者绑定到GL_DRAW_FRAMEBUFFER进行写入。

	//绑定到GL_READ_FRAMEBUFFER之后，所有的读操作，如glReadPixels，将使用该帧缓冲
	//绑定到GL_DRAW_FRAMEBUFFER之后，该缓冲区将被用于渲染、清空以及其他的写操作的目的地。
	//大部分时候，我们只需要绑定到GL_FRAMEBUFFER上面就好了。而不需要进行分别操作。

	//***** create a texture 
	unsigned int colorTextureBuffer, depth_stencilTextureBuffer;
	glGenTextures(1, &colorTextureBuffer);
	glBindTexture(GL_TEXTURE_2D, colorTextureBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 240, 180, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//attach it to currently bound framebuffer object.
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTextureBuffer, 0);
	/*参数如下：
	target：目标帧缓冲类型：DRAW、READ、BOTH
	attachment:我们要附加的附加物类型；这里我们附加了一个颜色附加物，
	注意最后的0意味着我们可以附加多个颜色附件。我们将在之后的教程中提到。
	textarget：你希望附加的纹理类型
	texture：实际附加的对象
	level：多级渐远纹理的级别。这里是0
	*/

	//绑定一个深度模板贴图附加物。
	glGenTextures(1, &depth_stencilTextureBuffer);
	glBindTexture(GL_TEXTURE_2D, depth_stencilTextureBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, 240, 180, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH24_STENCIL8, GL_TEXTURE_2D, depth_stencilTextureBuffer, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	//考虑创建一个渲染缓冲对象(我们不打算采集深度和模板的数据，因此我们考虑使用渲染缓冲)
	unsigned int renderBufferObject;
	glGenRenderbuffers(1, &renderBufferObject);
	glBindRenderbuffer(GL_RENDERBUFFER, renderBufferObject);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 240, 180);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBufferObject);
	//当我们为渲染缓冲对象分配了足够的内存之后，我们可以解绑这个渲染缓冲。
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);//解绑帧缓冲，保证我们不会不小心渲染到错误的帧缓冲


										 /*
										 将场景绘制到一个贴图上，我们需要以下步骤;
										 1.渲染场景到新的帧缓冲中，记得先绑定为当前激活的帧缓冲区。
										 2.绑定默认的帧缓冲
										 3.绘制一个横跨整个渲染屏幕的四边形，并使用新的帧缓冲区的颜色缓冲作为其的贴图。
										 */

	float canvasVertices[] = {
		/*
		注意纹理坐标和顶点坐标的区别，纹理坐标:(bottom-left: 0,0    top-right: 1,1)
		*/
		//right bottom       // texture 
		1.0f,  1.0f, 0.0f,	1.0f,1.0f,
		0.4f,  0.4f, 0.0f,	0.0f, 0.0f,
		1.0f,  0.4f, 0.0f,	1.0f, 0.0f,

		//left top
		1.0f, 1.0f,  0.0f,	1.0f, 1.0f,
		0.4f, 1.0f, 0.0f,	0.0f, 1.0f,
		0.4f, 0.4f, 0.0f, 0.0f, 0.0f
	};
	// quad VAO
	unsigned int canvasVAO, canvasVBO;
	glGenVertexArrays(1, &canvasVAO);
	glGenBuffers(1, &canvasVBO);
	glBindVertexArray(canvasVAO);
	glBindBuffer(GL_ARRAY_BUFFER, canvasVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(canvasVertices), canvasVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	// cube VAO
	unsigned int cubeVAO, cubeVBO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);
	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glBindVertexArray(0);
	// plane VAO
	unsigned int planeVAO, planeVBO;
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glBindVertexArray(0);

	/*
	1.激活贴图ID号
	2.将采集器绑定到当前的贴图ID上
	3.将贴图绑定到当前贴图ID
	NOTE:	2+3 将采集器和贴图链接了起来。
	*/
	shader.use();
	shader.setInt("texture1", 0);

	std::vector<glm::vec3> windows;
	windows.push_back(glm::vec3(-1.5f, 0.0f, -0.48f));
	windows.push_back(glm::vec3(1.5f, 0.0f, 0.51f));
	windows.push_back(glm::vec3(0.0f, 0.0f, 0.7f));
	windows.push_back(glm::vec3(-0.3f, 0.0f, -2.3f));
	windows.push_back(glm::vec3(0.5f, 0.0f, -0.6f));

	float curTime{}, lastTime{};
	curTime = lastTime = glfwGetTime();

	while (!glfwWindowShouldClose(window)) {

		//ProcessInput
		curTime = glfwGetTime();
		ProcessInput(window, curTime - lastTime);
		lastTime = curTime;

		// render
		// ------------------------------------
		//recording back mirror
		camera.Front = -camera.Front;
		glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		//DrawScence
		glm::mat4 model;
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		shader.use();
		shader.setMartix("projection", projection);
		shader.setMartix("view", view);

		//Plane
		shader.setBool("openEffect", openEffect);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, floorTexture);
		model = glm::mat4();
		shader.setMartix("model", model);
		glBindVertexArray(planeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		//cube
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, cubeTexture);

		//map会自动基于它的键排序它的值
		std::map<float, glm::vec3> sorted;
		for (unsigned int i = 0; i < windows.size(); i++)
		{
			float distance = glm::length(camera.Position - windows[i]);
			sorted[distance] = windows[i];
		}
		//距离从低到高储存了每个窗子的位置。
		shader.setMartix("projection", projection);
		shader.setMartix("view", view);
		//注意这里的迭代器是逆序的！！！！！
		for (std::map<float, glm::vec3>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it) {
			model = glm::mat4();
			model = glm::translate(model, it->second);
			shader.setMartix("model", model);
			glBindVertexArray(cubeVAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		camera.Front = -camera.Front;//调转回原来的方向
		// ------------------------------------
		// entire scene
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		view = camera.GetViewMatrix();
		shader.use();
		shader.setMartix("projection", projection);
		shader.setMartix("view", view);
		//Plane
		shader.setBool("openEffect", openEffect);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, floorTexture);
		model = glm::mat4();
		shader.setMartix("model", model);
		glBindVertexArray(planeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		//cube
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, cubeTexture);

		//map会自动基于它的键排序它的值
		sorted.clear();
		for (unsigned int i = 0; i < windows.size(); i++)
		{
			float distance = glm::length(camera.Position - windows[i]);
			sorted[distance] = windows[i];
		}
		//距离从低到高储存了每个窗子的位置。
		shader.setMartix("projection", projection);
		shader.setMartix("view", view);
		//注意这里的迭代器是逆序的！！！！！
		for (std::map<float, glm::vec3>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it) {
			model = glm::mat4();
			model = glm::translate(model, it->second);
			shader.setMartix("model", model);
			glBindVertexArray(cubeVAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		
		shaderCanvas.use();
		glBindVertexArray(canvasVAO);
		glDisable(GL_DEPTH_TEST);	//在绘制简单画布（四边形）的时候，我们不需要开启深度测试！
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, colorTextureBuffer);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glBindVertexArray(0);
		//swap and poll
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteVertexArrays(1, &planeVAO);
	glDeleteBuffers(1, &cubeVBO);
	glDeleteBuffers(1, &planeVBO);
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