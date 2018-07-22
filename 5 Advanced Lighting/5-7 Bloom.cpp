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
	这个教程我们只是用了一个相对简单的高斯模糊过滤器，它在每个方向上只有5个样本。
	通过沿着更大的半径或重复更多次数的模糊，进行采样我们就可以提升模糊的效果。因为模糊的质量
	与泛光效果的质量正相关，提升模糊效果就能够提升泛光效果。有些提升将模糊过滤器与不同大小的模
	糊kernel或采用多个高斯曲线来选择性地结合权重结合起来使用。来自Kalogirou和EpicGames的附加资
	源讨论了如何通过提升高斯模糊来显著提升泛光效果。
*/


/*Bloom	: 泛光
	明亮的光源和明亮区域通常是难以传达给观察者的，因为一个显示器的强度范围是有限的。
	一种在显示器上区分明亮光源的方式是生成光晕，光晕的光在光源周围。
	这能够有效的给观察者一种这些光源或者光亮区域是非常亮的错觉。

	这种通过后期效果生成的光渗(light bleeding)或者光晕效果被称为Bloom，泛光

	Bloom使得明亮的物体给人以明显的视觉感，因为Bloom趋向于给人一种物体非常亮的错觉。
	当在一种微妙的时尚方法进行的时候，Bloom将显著的提升场景的光照并产生大范围的戏剧效果。

	Bloom可以和HDR完美的结合使用。一个常见的误解是：
		HDR等同于Bloom，因为许多人交替的使用这些术语。然而这是两种完全不同的技术，并且用于不同的目的。
	可以在默认的8Bit精度的帧缓冲区下实现Bloom，正如可以在没有Bloom的效果下使用HDR。
	
	不过，HDR使得Bloom更有效的实现。

	要实现Bloom，我们像往常一样，渲染一个光照场景，然后提取出场景的HDR颜色缓冲区和仅包含可见光亮区的场景图像。
	将提取出来的明亮的图片进行模糊，结果被添加到HDR场景上面。
		（
			1.渲染一个光照场景
			2.提取出HDR颜色缓冲区A
			3.提取中可见光亮区的图像B
			4.对B进行模糊处理
			5.将处理的结果添加到A中
		）
	现在让我们来一步步的解释这个过程：
	
	1.	本实验中渲染了四个亮的立方体光源，他们的亮度值在 1.5 到 15.0 之间。
	
		记得应用HDR，否则会爆掉，
	
	2.	我们需要从渲染的场景中提取出两张图片，我们可以渲染场景两次，然后使用不同的着色器，
	分别保存在不同的帧缓冲区中。不过这里我们会使用一些小技巧：多重渲染目标(Muliple Render Target MRT)
		MRT允许我们可以指定多于两个的片段着色器输出，这让我们可以在一次渲染中提取出两个图像。

	在像素着色器的输出前，我们指定一个布局location标识符，这样我们便可控制一个片段着色器写入到哪个颜色缓冲：
		
		layout (location = 0) out vec4 FragColor;
		layout (location = 1) out vec4 BrightColor;
	然而这个只有在我们有多个位置可以写入的时候才有用。
		为了使用多个片段着色器的输出，我们需要绑定多个颜色缓冲区到帧缓冲对象中。而这时可以的：
	
	我们之前总是将颜色缓冲区绑定在GL_COLOR_ATTACHMENT0上，来进行绑定，现在我们可以使用GL_COLOR_ATTACHMENT1，来绑定多个
			for (unsigned int i = 0; i < 2; i++)
			{
				[......]
				// attach texture to framebuffer
				glFramebufferTexture2D(
					GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0
				);
			}  
		代码看起来是这个样子
	
	现在我们将不得不显式的告诉OpenGL我们要通过glDrawBuffers渲染到多个缓冲区中，
		否则openGL默认只渲染到帧缓冲的第一个颜色附件上，
		unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, attachments);

	3.	高斯模糊
		高斯模糊是基于高斯曲线
		高斯曲线在它的中间处的面积最大，使用它的值作为权重使得近处的样本拥有最大的优先权。
		比如我们对片段从32*32的区域面积内采样，随着离片段的距离增加，我们采用的权重越小。
		因此，我们需要一个二维的权重四方形，从这个二维高斯曲线方程中去获取它。
		然而问题是，如果每个点进行采样的话 将采样 32*32 = 1024次！这明显是不可能的

		幸运的是，高斯方程有个非常巧妙的特性，它允许我们把二维方程分解为两个更小的方程：一个描述水平权重，另一个描述垂直权重。

		我们首先在整个纹理上使用水平权重进行水平模糊，然后在处理后的纹理上应用垂直模糊。
		利用这个特性，结果是一样的，但是可以节省难以置信的性能，
		因为我们现在只需做32+32次采样，不再是1024了！这叫做两步高斯模糊( two-pass Gaussian Blur)。

		也就是说，我们需要对同一个图像至少模糊两次，最好使用帧缓冲对象做这件事。
		我们将实现一个Ping-Pong 缓冲区，这是一对缓冲区：
		这里我们把另一个帧缓冲的颜色缓冲放进当前的帧缓冲的颜色缓冲中指定次数，并同时使用不同的着色效果渲染。
		基本上就是不断地切换帧缓冲和纹理去绘制。
		这样我们先在场景纹理的第一个缓冲中进行模糊，
		然后在把第一个帧缓冲的颜色缓冲放进第二个帧缓冲进行模糊，
		接着，将第二个帧缓冲的颜色缓冲放进第一个模糊，循环往复。

	着色器：
		在Shader5-7.fragment_Gaussian_Blur中可以看到，我们分别在水平和垂直进行采样，这里进行了5次采样，
		采样围绕当前的片段进行，注意水平和垂直采样是分离的。
			（我们通过一个horizontal 的bool类型，将水平和垂直分开来的。）
		通过用1.0除以纹理的大小（从textureSize得到一个vec2）得到一个纹理像素的实际大小，以此作为偏移距离的根据。
	缓冲区：
		我们需要创建两个基本缓冲区，每个都只包含颜色缓冲区：
			GLuint PingPongFBO[2];
			GLuint PingPongColorBuffer[2];
			glGenFramebuffers(2 , PingPongFBO);
			glGenTextures(2 , PingPongColorBuffer);
			for (int i = 0; i < 2; ++i) {
				glBindFramebuffer(GL_FRAMEBUFFER , PingPongFBO[i]);
				glBindTexture(GL_TEXTURE_2D, PingPongColorBuffer[i]);
		
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);	//重点：使用16位的浮点缓冲区
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 , GL_TEXTURE_2D, PingPongColorBuffer[i], 0);

			}
	得到一个HDR纹理后，我们用提取出来的亮区纹理填充一个帧缓冲，然后对其模糊处理10次（5次垂直5次水平）：
	
	最后将模糊的结果和原场景混合即可！
*/

/*
--函数------------------------------
*/
void framebuffer_size_callback(GLFWwindow * window, int width, int height) { glViewport(0, 0, width, height);	return; }
void mouse_callback(GLFWwindow* window, double xPos, double yPos);
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
void ProcessInput(GLFWwindow * window, float deltaTime);

void renderPlane(Shader shader);
void renderCube(Shader shader, std::vector<glm::vec3> cubePositions, std::vector<glm::vec3> cubeRotateAxis);
void renderLight(Shader shader, std::vector<glm::vec3> lightPositions, std::vector<glm::vec3> lightColors);
void RenderQuad(Shader shader);

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

float yaw = -90.0f, pitch = 0.0f;
bool hdr = true;
bool hdrKeyPressed = false;
float exposure = 1.0f;
int extract = 0;
Camera camera(cameraPos, WorldUp, yaw, pitch);
int amount = 10;
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
		extract = 1;
		hdrKeyPressed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
	{
		extract = 0;
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
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		if (amount > 2)
			amount -= 2;
	}
	else if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		if (amount < 20)
			amount += 2;
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
	std::cout << "extract : " << extract << std::endl;
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
	// positions         	//Normal			 // texture Coords	
	-4.0f, -1.0f, -4.0f,	0.0f,  1.0f, 0.0f,		0.0f, 5.0f,
	4.0f, -1.0f,  4.0f,		0.0f,  1.0f, 0.0f,		5.0f, 0.0f,
	4.0f, -1.0f, -4.0f,		0.0f,  1.0f, 0.0f,		5.0f, 5.0f,

	-4.0f, -1.0f, -4.0f,	0.0f,  1.0f, 0.0f,		0.0f, 5.0f,
	-4.0f, -1.0f,  4.0f,	0.0f,  1.0f, 0.0f,		0.0f, 0.0f,
	4.0f, -1.0f,  4.0f,		0.0f,  1.0f, 0.0f,		5.0f, 0.0f,
	
	// x-y 平面
	/*-2.0f,	-2.0f, -1.0f,	0.0f, 2.0f,			0.0f,  0.0f, 1.0f,
	2.0f,	 2.0f, -1.0f,	2.0f, 0.0f,			0.0f,  0.0f, 1.0f,
	2.0f,	-2.0f, -1.0f,	2.0f, 2.0f,			0.0f,  0.0f, 1.0f,

	-2.0f,	-2.0f, -1.0f,	0.0f, 2.0f,			0.0f,  0.0f, 1.0f,
	-2.0f,	 2.0f, -1.0f,	0.0f, 0.0f,			0.0f,  0.0f, 1.0f,
	2.0f,	 2.0f, -1.0f,	2.0f, 0.0f,			0.0f,  0.0f, 1.0f*/
};



void HDR_Test(GLFWwindow * window) {
	glEnable(GL_DEPTH_TEST);
	//Shader
	Shader light_Shader("Shader.vertex_lamp", "Shader.fragment_lamp");
	Shader shader("Shader5-7.vertex", "Shader5-7.fragment");
	Shader Blend_HDRshader("Shader5-7.vertex_Blend", "Shader5-7.fragment_Blend");
	Shader blurShader("Shader5-7.vertex_Blend","Shader5-7.fragment_Gaussian_Blur");

	//Texture
	unsigned int planeTexture = loadTexture("wood.png", false);
	unsigned int cubeTexture = loadTexture("container2.png", false);

	//create 2 basic framebuffers	--	PingPong缓冲区
	GLuint PingPongFBO[2];
	GLuint PingPongColorBuffer[2];
	glGenFramebuffers(2 , PingPongFBO);
	glGenTextures(2 , PingPongColorBuffer);
	for (int i = 0; i < 2; ++i) {
		glBindFramebuffer(GL_FRAMEBUFFER , PingPongFBO[i]);
		glBindTexture(GL_TEXTURE_2D, PingPongColorBuffer[i]);
		
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);	//重点：使用16位的浮点缓冲区
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 , GL_TEXTURE_2D, PingPongColorBuffer[i], 0);

	}
	
	//FrameBuffer
	GLuint hdrFBO;
	glGenFramebuffers(1, &hdrFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
	// create color buffer 渲染两个缓冲区，
	GLuint colorBuffers[2];
	glGenTextures(2, colorBuffers); 
	for (int i = 0; i < 2	; ++i) {
		glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);	//重点：使用16位的浮点缓冲区
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);	//
	}
	//告诉OpenGL渲染到这两个缓冲区中，
	unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);
	// create depth buffer (renderbuffer)
	GLuint depthBuffer;
	glGenRenderbuffers(1, &depthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);

	// attach 
	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
	//当我们为渲染缓冲对象分配了足够的内存之后，我们可以解绑这个渲染缓冲。
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	// shader configuration
	// --------------------
	shader.use();
	shader.setInt("diffuseTexture", 0);
	Blend_HDRshader.use();
	Blend_HDRshader.setInt("hdr_scene", 0);
	Blend_HDRshader.setInt("bloomBlur", 1);
	Blend_HDRshader.setInt("hdr", hdr);
	blurShader.use();
	blurShader.setInt("image",0);
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

		//cube info
		std::vector<glm::vec3> cubePositions;
		cubePositions.push_back(glm::vec3(0.0f, -0.5f, 0.0f));
		cubePositions.push_back(glm::vec3(2.0f, 0.0f, 0.0f));
		cubePositions.push_back(glm::vec3(-2.0f, -0.5f, 1.0f));
		cubePositions.push_back(glm::vec3(2.0f, 2.0f, -1.0f));
		cubePositions.push_back(glm::vec3(-1.0f, 1.0f, -1.0f));
		cubePositions.push_back(glm::vec3(1.0f, 2.0f, -3.0f));

		std::vector<glm::vec3> cubeRotateAxis;
		cubeRotateAxis.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
		cubeRotateAxis.push_back(glm::vec3(1.0f, 1.0f, 0.0f));
		cubeRotateAxis.push_back(glm::vec3(1.0f, 1.0f, 1.0f));
		cubeRotateAxis.push_back(glm::vec3(-1.0f, 0.0f, 0.0f));
		cubeRotateAxis.push_back(glm::vec3(-1.0f, -1.0f, 0.0f));
		cubeRotateAxis.push_back(glm::vec3(-1.0f, -1.0f, -1.0f));
		// lighting info
		// -------------
		// positions
		std::vector<glm::vec3> lightPositions;
		std::vector<glm::vec3> lightColors;
		lightPositions.push_back(glm::vec3(0.0f, 3.0f, 0.0f)); // back light
		lightPositions.push_back(glm::vec3(-0.4f, 0.9f, 3.0f));
		lightPositions.push_back(glm::vec3(0.0f, 0.8f, 2.0f));
		lightPositions.push_back(glm::vec3(0.8f, 0.7f, -3.0f));
		// colors
		lightColors.push_back(glm::vec3(15.0f, 0.0f, 0.0f));
		lightColors.push_back(glm::vec3(0.0f, 10.0f, 0.0f));
		lightColors.push_back(glm::vec3(0.0f, 0.0f, 7.0f));
		lightColors.push_back(glm::vec3(5.0f, 5.0f, 5.0f));

		// 1. render scene into floating point framebuffer
		// -----------------------------------------------
		glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (GLfloat)SCR_WIDTH / (GLfloat)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();

		//渲染Cube
		shader.use();
		shader.setMartix("projection", projection);
		shader.setMartix("view", view);
		shader.setVec3("viewerPos", camera.Position);
		shader.setInt("inverse_normals", false);//控制照亮内部还是外部
		for (unsigned int i = 0; i < lightPositions.size(); i++)
		{
			shader.setVec3("lights[" + std::to_string(i) + "].Position", lightPositions[i]);
			shader.setVec3("lights[" + std::to_string(i) + "].Color", lightColors[i]);
		}
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, planeTexture);
		renderPlane(shader);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, cubeTexture);
		renderCube(shader , cubePositions , cubeRotateAxis);

		//渲染Light
		light_Shader.use();
		light_Shader.setMartix("view", view);
		light_Shader.setMartix("projection", projection);
		renderLight(light_Shader, lightPositions ,lightColors);

		// 2. 通过上面的步骤我们得到了一个HDR纹理和一个亮度纹理
		//	现在，我们将渲染得到的亮度纹理填充入一个缓冲区，然后对其进行模糊处理
		// ---------------------------------------------------------------------------------
		GLboolean horizontal = true, first_iteration = true;
		//int amount = 10;	//十次模糊处理：5次水平，5次垂直
		blurShader.use();
		for (int i = 0; i < amount; ++i) {
			glBindFramebuffer(GL_FRAMEBUFFER, PingPongFBO[horizontal]);
			blurShader.setInt("horizontal", horizontal);
			glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : PingPongColorBuffer[!horizontal]);
			RenderQuad(blurShader);
			horizontal = !horizontal;
			if (first_iteration)
				first_iteration = false;	//注意在第一个的时候使用亮度缓冲区,之后就不断切换PingPong的缓冲区进行模糊了
		}

		// 2. now render floating point color buffer to 2D quad and tonemap HDR colors to 
		//		default framebuffer's (clamped) color range
		// ---------------------------------------------------------------------------------
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		Blend_HDRshader.use();
		Blend_HDRshader.setFloat1f("exposure", exposure);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, PingPongColorBuffer[!horizontal]);
		RenderQuad(Blend_HDRshader);

		//swap and poll
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

static unsigned int cubeVAO, cubeVBO;
static unsigned int planeVAO, planeVBO;
static unsigned int quadVAO, quadVBO;
static bool CubeisGenerated = false;
static bool PlaneisGenerated = false;
static bool QuadisGenerated = false;

void renderPlane(Shader shader) {
	if (!PlaneisGenerated) {
		//VAO , VBO
		//Cube
		glGenVertexArrays(1, &planeVAO);
		glGenBuffers(1, &planeVBO);
		// fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
		// link vertex attributes
		glBindVertexArray(planeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);		//顶点数据
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));	//法向量数据
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));	//纹理坐标数据
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		PlaneisGenerated = true;
	}
	shader.use();
	glBindVertexArray(planeVAO);
	glm::mat4 model = glm::mat4();
	model = glm::scale(model , glm::vec3(3.0f,1.0f,3.0f));
	shader.setMartix("model", model);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

}
void renderCube(Shader shader , std::vector<glm::vec3> cubePositions , std::vector<glm::vec3> cubeRotateAxis) {
	if (!CubeisGenerated) {	//若数据没有生成，则先生成数据
							//VAO , VBO
							//Cube
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		// fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
		// link vertex attributes
		glBindVertexArray(cubeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);		//顶点数据
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));	//法向量数据
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));	//纹理坐标数据
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		CubeisGenerated = true;
	}
	shader.use();
	glBindVertexArray(cubeVAO);

	for (int i = 0; i < cubePositions.size(); ++i) {
		glm::mat4 model = glm::mat4();
		model = glm::translate(model, cubePositions[i]);
		model = glm::rotate(model,glm::radians(i * 15.0f) , cubeRotateAxis[i]);
		shader.setMartix("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
	glBindVertexArray(0);

}

void renderLight(Shader shader , std::vector<glm::vec3> lightPositions , std::vector<glm::vec3> lightColors) {
	if (!CubeisGenerated) {	//若数据没有生成，则先生成数据
		//VAO , VBO
		//Cube
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		// fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
		// link vertex attributes
		glBindVertexArray(cubeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);		//顶点数据
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));	//法向量数据
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));	//纹理坐标数据
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		CubeisGenerated = true;
	}
	//light
	for (unsigned int i = 0; i < lightPositions.size(); i++) {
		glm::mat4 model = glm::mat4();
		model = glm::translate(model, lightPositions[i]);
		model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));
		shader.setMartix("model", model);
		shader.setVec3("cubeColor" , lightColors[i]);
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
}

void RenderQuad(Shader shader) {
	if (!QuadisGenerated) {
		float quadVertices[] = {
			// positions			// texture Coords
			-1.0f,  1.0f, 0.0f,		0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f,		0.0f, 0.0f,
			1.0f,  1.0f, 0.0f,		1.0f, 1.0f,
			1.0f, -1.0f, 0.0f,		1.0f, 0.0f,
		};
		// setup plane VAO
		
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		glBindVertexArray(0);

		QuadisGenerated = true;
	}

	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
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
