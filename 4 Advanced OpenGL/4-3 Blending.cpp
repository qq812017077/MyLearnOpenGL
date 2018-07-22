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
/*
	Blending:
		Blending因为用于实现透明技术而闻名。
		透明实际上是指物体拥有的其本身以及其后方的物体颜色的混合色。而非一种纯色
		一个有色玻璃窗就是一种透明物体，玻璃有自身的颜色，但是最终的颜色包含了所有玻璃后面的颜色
		透明允许完全透明（让所有颜色通过）以及部分透明（让部分颜色通过以及部分自身颜色）。
			
	透明度：
		由alpha决定
		0.0的透明度意味着物体完全透明。
		0.5的透明度意味着50%的玻璃颜色和50%的后面物体的颜色。

	我们之前的贴图都包含3个颜色组分：红、绿、蓝。
		对于png的照片类型也包含了alpha通道，它为每个纹理像素(Texel)包含着一个alpha值。
				这个alpha值告诉我们贴图的哪些部分是透明的以及透明度。

*/

/*
	Discarding fragments
		丢弃：用于产生完全的透明效果

	set the texture wrapping method to GL_CLAMP_TO_EDGE whenever you use alpha textures:
	在使用透明贴图的时候，注意贴图的参数设置里使用：GL_CLAMP_TO_EDGE
		之前我们一直使用的repeat，repeat在到达边界之后会重复贴图绘制（如到达顶部之后会从底部重新绘制）。
		又因为我们使用的线性插值，所以会出现一些非透明情况：透明顶部和非透明底部导致产生的有色边缘

*/

/*	
	Blending:混合
		丢弃片段不能产生半透明的图像，
		
		开启混合后，我们还需要告诉OpenGL它该如何混合：
			OpenGL中的混合公式：

			Cresult =  Csource * Fsource  + Cdestination * Fdestination
			
				C¯source ：源颜色向量。这是来自纹理的本来的颜色向量
				C¯destination ： 目标颜色向量，当前保存在颜色缓冲区中的颜色向量
				F¯source：	源因子。设置了对源颜色的alpha值影响
				F¯destionation：目标因子，设置对目标颜色的alpha值的影响。
		
		在片段着色器运行并且所有测试通过之后，混合方程才被用于片段的颜色输出以及当前颜色缓冲区中的内容（当前缓冲区中保存的是之前的片段颜色的值）

			源和目标颜色由OpenGL自动设置，但源和目标因子则是由我们自行设置的。下面是一个例子：
				
			举个例子：
				有两个方块
					(1.0,0.0,0.0,1.0)	----Red
					(0.0,1.0,0.0,0.6)	----Green
				根据alpha也可以看出，我们想要绘制Green在Red上面。
				这里红色应该是目标值。
					
				问题：因子该如何设置：
					我们至少想要有 绿色方块和其alpha值相乘，
						这里我们将Fsource设置为0.6，即alpha值。
						而让目标方块，也就是红块的Fdestionation设置为0.4 ---- 1-0.6
			
			问题：如何告诉OpenGL这些因子的值？
					void glBlendFunc(GLenum sfactor, GLenum dfactor)
				颜色常数向量C¯constant可以用glBlendColor函数分开来设置
				可选参数：
						Option									Value
						GL_ZERO								Factor = 0
						GL_ONE						   		Factor = 1
					 GL_SRC_COLOR						Factor = C¯source
				GL_ONE_MINUS_SRC_COLOR					Factor = 1 - C¯source
					GL_DST_COLOR						Factor = C¯destination
				GL_ONE_MINUS_DST_COLOR					Factor = 1 - C¯destination
					GL_SRC_ALPHA						Factor = C¯source.alpha
				GL_ONE_MINUS_SRC_ALPHA					Factor = 1 - C¯source.alpha
					GL_DST_ALPHA						Factor = C¯destination.alpha
				GL_ONE_MINUS_DST_ALPHA					Factor = 1 - C¯destination.alpha
				  GL_CONSTANT_COLOR						Factor = C¯constant
			  GL_ONE_MINUS_CONSTANT_COLOR				Factor = 1 - C¯constant
				 GL_CONSTANT_ALPHA						Factor = C¯constant.alpha
			  GL_ONE_MINUS_CONSTANT_ALPHA				Factor = 1 - C¯constant.alpha

	glBlendFuncSeparate-----单独的设置RGB 和 alpha 通道
			glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

	我们甚至可以改变方程：
		void glBlendEquation(GLenum mode)

		GL_FUNC_ADD:				默认	C¯result = Src + Dst
		GL_FUNC_SUBTRACT:					C¯result = Src - Dst
		GL_FUNC_REVERSE_SUBTRACT：			C¯result = Dst - Src


*/

/*	一个棘手的问题：
	我们在绘制的时候，同时开启了深度测试和混合。
		混合意味着，在渲染的时候会考虑其后方的内容，
		深度呢：渲染的时候，丢弃其后方的内容，节省时间！

		因此当我们先绘制前方的透明物体时，等到要绘制后面的部分的时候，
		深度测试发现其在已经渲染东西的后面（深度测试可不会在意什么透明度）。
			就会直接丢弃掉（小聪明！）

		解决的办法：
			1.先绘制不透明的物体
			2.透明的物体按从远朝近进行排序
			3.按顺序绘制透明的物体，从远到近

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

void Blending_Test(GLFWwindow* window);


glm::vec3 cameraPos(0.0f, 2.0f, 2.0f);
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
	Blending_Test(window);
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

void DrawTwoCube();
void DrawScaleUpTwoCube();
void Blending_Test(GLFWwindow* window) {
	//使能深度测试
	glEnable(GL_DEPTH_TEST);//深度测试

	//混合
	glEnable(GL_BLEND);	//混合
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	Shader shader("Shader4-3.vertex", "Shader4-3.fragment_semiwindow");

	stbi_set_flip_vertically_on_load(true);//反转y轴，否则图片上下颠倒！
	//Texture
	unsigned int windowsTexture = loadTexture(std::string("blending_transparent_window.png").c_str(), GL_CLAMP_TO_EDGE);
	unsigned int floorTexture = loadTexture(std::string("wall.jpg").c_str(), GL_REPEAT);

	// cube VAO
	unsigned int windowVAO, windowVBO;
	glGenVertexArrays(1, &windowVAO);
	glGenBuffers(1, &windowVBO);
	glBindVertexArray(windowVAO);
	glBindBuffer(GL_ARRAY_BUFFER, windowVBO);
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
		// ------

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		glm::mat4 model;
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();

		//Plane
		shader.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, floorTexture);
		model = glm::mat4();
		shader.setMartix("projection",projection);
		shader.setMartix("view", view);
		shader.setMartix("model", model);
		glBindVertexArray(planeVAO);
		glDrawArrays(GL_TRIANGLES,0,6);

		//windows
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, windowsTexture);
		
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
		for (std::map<float, glm::vec3>::reverse_iterator it = sorted.rbegin() ; it !=  sorted.rend();++it) {
			model = glm::mat4();
			model = glm::translate(model , it->second);
			shader.setMartix("model", model);
			glBindVertexArray(windowVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
		glBindVertexArray(0);
		//swap and poll
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &windowVAO);
	glDeleteVertexArrays(1, &planeVAO);
	glDeleteBuffers(1, &windowVBO);
	glDeleteBuffers(1, &planeVBO);
}


unsigned int loadTexture(char const *path , unsigned int Tex_Wrap_Para)
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