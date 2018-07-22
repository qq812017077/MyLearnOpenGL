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

/* HDR
	当亮度和颜色值保存在一个帧缓冲中的时候，其值默认被限制在在0.0到1.0之间
	这个看似无辜的声明使得我们总是将光和颜色的值指定在这个范围以使其能适应场景。

	这并没有什么问题，但是如果我们遇上了一个指定的亮区，这里包含了大量的光源以至于总的亮度超过1.0，这时会发生什么？

	答案是这些片段中超过1.0的亮度或者颜色值会被约束在1.0，从而导致场景混成一片，难以分辨：

	由于大量的片段颜色之集中在1.0，在一个大的区域内，每一个每一个亮的片段都有着同样的白色，最终导致损失大量细节，使得看起来很假。

	解决方案：	
		1.减少光源的强度，确保场景中没有任何区域的片段会超出1.0. --这个方案不是很好
		2.允许颜色之暂时超出1.0，最终将他们转换到0.0-1.0的范围之内

	显示器限制了颜色范围只能在0.0-1.0之间，然而在光的方程中却没有这样的限制。

	通过允许片段的颜色超出1.0，我们有了一个更大的有效的颜色范围值，这被称为：
		高动态范围： High Dynamic Range.  HDR。
	使用HDR，亮的东西可以很亮，暗的也可以很暗。

	人眼：
	当光线很弱的啥时候，人眼会自动调整从而使过暗和过亮的部分变得更清晰，就像人眼有一个能自动根据场景亮度调整的自动曝光滑块。

	色调映射 Tone Mapping：
	HDR渲染和其很相似，我们允许用更大范围的颜色值渲染从而获取大范围的黑暗与明亮的场景细节，
	最后将所有HDR值转换成在[0.0, 1.0]范围的LDR(Low Dynamic Range,低动态范围)。
	转换HDR值到LDR值得过程叫做色调映射(Tone Mapping)，现在现存有很多的色调映射算法，
	这些算法致力于在转换过程中保留尽可能多的HDR细节。
	
		这些色调映射算法经常会包含一个曝光参数：其选择性得支持黑暗或者明亮区域。
	
		在实时渲染中，HDR允许我们不仅可以超出LDR范围以保留更多细节，也让我们有能力根据
	真实强度来指定光的强度。比如，光有着非常高的强度，比起其他像是手电筒之类的光源，
	所以为什么不这样设置呢？（比如说设置一个10.0的漫亮度）。
		
		这允许我们可以使用更真实地光照参数来更恰当地配置一个场景的光，而这些东西在LDR中式不可行的，
	因为他们直接被限制在了1.0.

	另外，因为显示器仅仅显示0.0-1.0范围的颜色，我们需要将当前的HDR颜色值转换到显示器的范围。
	但是简单的取平均值重新转换并不合适，因为明亮的区域会更加显著。
	我们可以使用不同的方程，或曲线将HDR转换到LDR，以让我们能完全的控制场景的颜色。
	这就是之前说的色调变换，也是HDR渲染的最终步骤。

*/

/* Floatint point framebuffers
	浮点帧缓冲区。
	要实现HDR渲染，我们需要避免在每个片段着色器运行后颜色值被限制。
	当帧缓冲使用标准化的顶点格式（像是GL_RGB）作为颜色缓冲区的内部格式。OpenGL将自动的
	将值限制在0.0，1.0之间（在保存在framebuffer之前），这个操作对大部分的帧缓冲格式成立，
	除了专门用来存放被拓展范围值的浮点格式。

	当颜色缓冲区的内置格式被设置为 :
		GL_RGB16F, GL_RGBA16F, GL_RGB32F 或者 GL_RGBA32F 
	帧缓冲区将被看作 浮点帧缓冲区。
	浮点帧缓冲可以存储超过0.0到1.0范围的浮点值，所以非常适合HDR渲染：

		glBindTexture(GL_TEXTURE_2D, colorBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL); 
		这里使用了GL_RGB16F。
	OpenGL的默认帧缓冲仅一个颜色分量接收8位使用每个颜色分量32位浮点数缓冲区（GL_RGB32F  或者  GL_RGBA32F）
	我们将使用4倍多的内存来保存颜色值。
	因为32位不是必要的，除非需要使用高精度，所以这里使用的是16bit。


	有了一个绑定有浮点颜色缓冲区的帧缓冲区后，我们可以渲染场景到这个帧缓冲区，并且颜色值不会被限制在0.0 - 1.0。


*/


/*
--函数------------------------------
*/
void framebuffer_size_callback(GLFWwindow * window, int width, int height) { glViewport(0, 0, width, height);	return; }
void mouse_callback(GLFWwindow* window, double xPos, double yPos);
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
void ProcessInput(GLFWwindow * window, float deltaTime);


unsigned int loadTexture(char const *path, unsigned int Tex_Wrap_Para = GL_REPEAT, bool gammaCorrection = false);
//HDR
void HDR_Test(GLFWwindow * window);

/*
--变量------------------------------
*/
const int SCR_WIDTH = 800;
const int SCR_HEIGHT = 600;

glm::vec3 cameraPos(0.0f, 0.0f, 5.0f);
glm::vec3 WorldUp(0.0f, 1.0f, 0.0f);

float yaw = 90.0f, pitch = 0.0f;
bool hdr = true;
bool hdrKeyPressed = false;
float exposure = 1.0f;

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
	HDR_Test(window);
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
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !hdrKeyPressed)
	{
		hdr = !hdr;
		hdrKeyPressed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
	{
		hdrKeyPressed = false;
	}

	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
	{
		if (exposure > 0.0f)
			exposure -= 0.001f;
		else
			exposure = 0.0f;
	}
	else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
	{
		exposure += 0.001f;
	}
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
	// positions         	//Normal				 // texture Coords	
	-0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,		 0.0f, 0.0f,
	0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,		1.0f, 0.0f,
	0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,		1.0f, 1.0f,
	0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,		1.0f, 1.0f,
	-0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,		 0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,		 0.0f, 0.0f,

	-0.5f, -0.5f,  0.5f,   0.0f,  0.0f, 1.0f,		 0.0f, 0.0f,
	0.5f, -0.5f,  0.5f,   0.0f,  0.0f, 1.0f,		1.0f, 0.0f,
	0.5f,  0.5f,  0.5f,   0.0f,  0.0f, 1.0f,		1.0f, 1.0f,
	0.5f,  0.5f,  0.5f,   0.0f,  0.0f, 1.0f,		1.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,   0.0f,  0.0f, 1.0f,		 0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,   0.0f,  0.0f, 1.0f,		 0.0f, 0.0f,

	-0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,		 1.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,		 1.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,		 0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,		 0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,		 0.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,		 1.0f, 0.0f,

	0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,		1.0f, 0.0f,
	0.5f,  0.5f, -0.5f,   1.0f,  0.0f,  0.0f,		1.0f, 1.0f,
	0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,		0.0f, 1.0f,
	0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,		0.0f, 1.0f,
	0.5f, -0.5f,  0.5f,   1.0f,  0.0f,  0.0f,		0.0f, 0.0f,
	0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,		1.0f, 0.0f,

	-0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,		 0.0f, 1.0f,
	0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,		1.0f, 1.0f,
	0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,		1.0f, 0.0f,
	0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,		1.0f, 0.0f,
	-0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,		 0.0f, 0.0f,
	-0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,		 0.0f, 1.0f,

	-0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,		 0.0f, 1.0f,
	0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,		1.0f, 1.0f,
	0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,		1.0f, 0.0f,
	0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,		1.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,		 0.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,		 0.0f, 1.0f
};

float planeVertices[] = {
	// x-z 平面
	// positions          // texture Coords		//Normal
	/*-4.0f, -1.0f, -4.0f,	0.0f, 5.0f,			0.0f,  1.0f, 0.0f,
	4.0f, -1.0f,  4.0f,		5.0f, 0.0f,			0.0f,  1.0f, 0.0f,
	4.0f, -1.0f, -4.0f,		5.0f, 5.0f,			0.0f,  1.0f, 0.0f,

	-4.0f, -1.0f, -4.0f,	0.0f, 5.0f,			0.0f,  1.0f, 0.0f,
	-4.0f, -1.0f,  4.0f,	0.0f, 0.0f,			0.0f,  1.0f, 0.0f,
	4.0f, -1.0f,  4.0f,		5.0f, 0.0f,			0.0f,  1.0f, 0.0f,
	*/
	// x-y 平面
	-2.0f,	-2.0f, -1.0f,	0.0f, 2.0f,			0.0f,  0.0f, 1.0f,
	2.0f,	 2.0f, -1.0f,	2.0f, 0.0f,			0.0f,  0.0f, 1.0f,
	2.0f,	-2.0f, -1.0f,	2.0f, 2.0f,			0.0f,  0.0f, 1.0f,

	-2.0f,	-2.0f, -1.0f,	0.0f, 2.0f,			0.0f,  0.0f, 1.0f,
	-2.0f,	 2.0f, -1.0f,	0.0f, 0.0f,			0.0f,  0.0f, 1.0f,
	2.0f,	 2.0f, -1.0f,	2.0f, 0.0f,			0.0f,  0.0f, 1.0f
};



void HDR_Test(GLFWwindow * window) {
	glEnable(GL_DEPTH_TEST);
	//Shader
	Shader light_Shader("Shader.vertex_lamp", "Shader.fragment_lamp");
	Shader shader("Shader5-6.vertex", "Shader5-6.fragment");
	Shader HDRshader("Shader5-6.vertex_hdr", "Shader5-6.fragment_hdr");
	//Texture
	unsigned int diffuseTexture =	loadTexture("wood.png",false);
	
	//FrameBuffer
	GLuint hdrFBO;
	glGenFramebuffers(1, &hdrFBO);
	
	// create color buffer
	GLuint colorBuffer;
	glGenTextures(1 , &colorBuffer);
	glBindTexture(GL_TEXTURE_2D , colorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);	//重点：使用16位的浮点缓冲区
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// create depth buffer (renderbuffer)
	GLuint depthBuffer;
	glGenRenderbuffers(1, &depthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);

	// attach 
	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffer, 0);	//
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
	//当我们为渲染缓冲对象分配了足够的内存之后，我们可以解绑这个渲染缓冲。
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	

	//VAO , VBO
	//Cube
	unsigned int cubeVAO, cubeVBO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);
	// fill buffer
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
	// link vertex attributes
	glBindVertexArray(cubeVAO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	float quadVertices[] = {
		// positions			// texture Coords
		-1.0f,  1.0f, 0.0f,		0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f,		0.0f, 0.0f,
		1.0f,  1.0f, 0.0f,		1.0f, 1.0f,
		1.0f, -1.0f, 0.0f,		1.0f, 0.0f,
	};
	// setup plane VAO
	unsigned int quadVAO, quadVBO;
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	// lighting info
	// -------------
	// positions
	std::vector<glm::vec3> lightPositions;
	lightPositions.push_back(glm::vec3(0.0f, 0.0f, 30.5f)); // back light
	lightPositions.push_back(glm::vec3(-0.4f, -0.9f, 15.0f));
	lightPositions.push_back(glm::vec3(0.0f, -0.8f, 12.0f));
	lightPositions.push_back(glm::vec3(0.8f, -0.7f, 16.0f));
	// colors
	std::vector<glm::vec3> lightColors;
	lightColors.push_back(glm::vec3(200.0f, 200.0f, 200.0f));
	lightColors.push_back(glm::vec3(0.1f, 0.0f, 0.0f));
	lightColors.push_back(glm::vec3(0.0f, 0.0f, 0.2f));
	lightColors.push_back(glm::vec3(0.0f, 0.1f, 0.0f));

	

	// shader configuration
	// --------------------
	shader.use();
	shader.setInt("diffuseTexture", 0);
	for (unsigned int i = 0; i < lightPositions.size(); i++)
	{
		shader.setVec3("lights[" + std::to_string(i) + "].Position", lightPositions[i]);
		shader.setVec3("lights[" + std::to_string(i) + "].Color", lightColors[i]);
	}
	
	HDRshader.use();
	HDRshader.setInt("hdrBuffer", 0);
	
	HDRshader.setInt("hdr", hdr);
	
	float curTime{}, lastTime{};
	curTime = lastTime = glfwGetTime();
	while (!glfwWindowShouldClose(window)) {
		//Process Input
		curTime = glfwGetTime();
		ProcessInput(window, curTime - lastTime);
		lastTime = curTime;


		//Draw

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 1. render scene into floating point framebuffer
		// -----------------------------------------------
		glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (GLfloat)SCR_WIDTH / (GLfloat)SCR_HEIGHT, 0.1f, 100.0f);
			glm::mat4 view = camera.GetViewMatrix();
			shader.use();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, diffuseTexture);
			shader.setMartix("projection" , projection);
			shader.setMartix("view" , view);
			shader.setVec3("viewerPos", camera.Position);
			// render tunnel
			glm::mat4 model = glm::mat4();
			model = glm::translate(model, glm::vec3(0.0f, 0.0f, 20.0));
			model = glm::scale(model, glm::vec3(3.0f, 3.0f, 27.5f));
			shader.setMartix("model", model);
			shader.setInt("inverse_normals", true);	//照亮立方体的内部！！
			// render Cube
			glBindVertexArray(cubeVAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			glBindVertexArray(0);

			//light
			light_Shader.use();
			light_Shader.setMartix("view", view);
			light_Shader.setMartix("projection", projection);
			for (unsigned int i = 0; i < lightPositions.size(); i++) {
				model = glm::mat4();
				model = glm::translate(model, lightPositions[i]);
				model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
				light_Shader.setMartix("model", model);
				glBindVertexArray(cubeVAO);
				glDrawArrays(GL_TRIANGLES, 0, 36);
			}
			
		// 2. now render floating point color buffer to 2D quad and tonemap HDR colors to 
		//		default framebuffer's (clamped) color range
		// ---------------------------------------------------------------------------------
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			HDRshader.use();
			HDRshader.setFloat1f("exposure", exposure);
			model = glm::mat4();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, colorBuffer);
			glBindVertexArray(quadVAO);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			glBindVertexArray(0);

		//swap and poll
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

unsigned int loadTexture(char const *path, unsigned int Tex_Wrap_Para, bool gammaCorrection)
{
	/*
	Gamma矫正的意义在于：
	这里获取到的颜色数据并不是实际所显示出来的数据，
	假设显示的颜色为Crgb,则这里保存的颜色应为 Crgb^(1/2.2)，
	若不进行任何处理：之后进行一次gamma矫正，成为 Crgb^(1/2.2)^2,在进行gamma则又回到了Crgb^(1/2.2)，所以会很亮
	若程序矫正到Crgb，则之后进行一次gamma矫正，成为 Crgb^(1/2.2),在进行gamma则又回到了Crgb，这样就保持了线性关系
	*/
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum internalFormat;
		GLenum dataFormat;
		if (nrComponents == 1) {
			internalFormat = dataFormat = GL_RED;
		}
		else if (nrComponents == 3) {
			internalFormat = gammaCorrection ? GL_SRGB : GL_RGB;
			dataFormat = GL_RGB;
		}
		else if (nrComponents == 4) {
			internalFormat = gammaCorrection ? GL_SRGB_ALPHA : GL_RGBA;
			dataFormat = GL_RGBA;
		}

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
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
