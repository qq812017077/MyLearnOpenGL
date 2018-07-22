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
//模型引入
#include "Model.h"

/*Anti-Aliasing
	在学习渲染的旅途中，你可能会时不时遇到模型边缘有锯齿的情况。
	这些锯齿边缘(Jagged Edges)的产生和光栅器将顶点数据转化为片段的方式有关。

	走样(Aliasing)。
	有很多种抗锯齿（Anti-aliasing，也被称为反走样）的技术能够帮助我们缓解这种现象，从而产生更平滑的边缘。

	多重采样抗锯齿(Multisample Anti-aliasing, MSAA)。
		
		首先，我们需要明白OpenGL的光栅器的内部工作。
*/

/*光栅器/光栅化程序：
	光栅器：在最终处理过的顶点之后到片段程序之前的所有算法和过程的组合。
		将一个图元的所有顶点作为输入，并将它转换为一系列的片段。

		理论上顶点坐标可以有任意坐标，但是片段不可以，因为它们被屏幕的分辨率所约束。

		因此顶点坐标到片段从来不是一对一的映射。
		所以光栅器不得不以某种方式决定：
			每一个顶点最终所在的片段/屏幕坐标。

	我们知道屏幕是像素构成的，每个像素（方块）的中心都有一个采样点(Sample Point)，
	它会被用来决定这个图像是否遮盖了某个像素。
		图像的边缘总是有一部分遮住了某些像素，但是没有遮住采样点。因此就不会被受片段着色器影响
		由于屏幕像素总量的限制，有些边缘的像素能够被渲染出来，而有些则不会。
		结果就是我们使用了不光滑的边缘来渲染图元，导致之前讨论到的锯齿边缘。

	多重采样的功能:
		将单个的采样点变为多个采样点。我们不再使用像素中心的单一采样点，
		取而代之的是以特定图案排列的4个子采样点(Subsample)。
		我们将用这些子采样点来决定像素的遮盖度。当然，这也意味着颜色缓冲的大小会随着子采样点的增加而增加。

		采样点的数量可以是任意的，更多的采样点能带来更精确的遮盖率。

	多重采样（MSAA）真正的工作方式是：
		无论三角形遮盖了多少个子采样点，（每个图元中）每个像素只运行一次片段着色器。
		片段着色器所使用的顶点数据会插值到每个像素的中心，
		
					！所得到的结果颜色会被储存在每个被遮盖住的子采样点中。！ 注意是被遮盖的
		当颜色缓冲的子样本被图元的所有颜色填满时，所有的这些颜色将会在
		每个像素内部平均化。因为上图的4个采样点中只有2个被遮盖住了，
		这个像素的颜色将会是三角形颜色与其他两个采样点的颜色（在这里是无色）的平均值，最终形成一种淡蓝色。

	因此：
		一个像素如果能有更多的采样点被遮盖，则其颜色越接近团的颜色。
		而越少的子采样点被三角形所覆盖，那么它受到三角形的影响就越小！
*/

/*OpenGL中的MSAA
	如果我们想要在OpenGL中使用MSAA,我们需要使用一个颜色缓冲区,它能够为单个像素保存多个颜色值。
		（因为多重采样需要我们为每个采样点保存颜色）
	因此我们需要一种新类型的缓冲区，其能保存特定数量的多重采样样本。叫做：多重采样缓冲

	大多数的窗口系统提供给我们一个多重采样缓冲区,用来替代默认的缓冲区。
		GLFW也给了我们这个功能，
		我们需要做的就是告诉GLFW我们想要使用带N个采样点的多重采样缓冲区
		通过在创建窗口之前，调用glfwWindowHint来代替使用正常的颜色缓冲区。

			glfwWindowHint(GLFW_SAMPLES, 4);

	之后我们创建窗口，这时将使用每个屏幕坐标带有四个采样点颜色缓冲区。
	GLFW也自动的创建一个带四个子采样点的深度和模板缓冲区。
	（意味着所有缓冲区的大小都增大四倍）

		大多数时候，OpenGL驱动器会自动使能多重采样，但是我们最好手动处理一下：
			glEnable(GL_MULTISAMPLE);  

		实际的多重采样算法实现在了光栅中，我们需要做的就是使能！

*/

/*Off-screen MSAA 离屏MSAA
	由于GLFW负责了创建多重采样缓冲，启用MSAA非常简单。然而，
	如果我们想要使用我们自己的帧缓冲来进行离屏渲染，那么我们就必须要自己动手生成多重采样缓冲了。
	（简单来讲就是要和离屏渲染结合使用）

		PS：离屏渲染实际上是把画面渲染到了纹理上面。这个可以用来制作后视镜等

	有两种方法可以用来创建多重采样缓冲区

	多重采样纹理附件
		用作颜色缓冲区

	多重采样渲染缓冲对象
		用作深度和模板缓冲区

	工作方式：
		1.搭建一个多重采样的帧缓冲区（颜色+深度+模板： 注意用多重采样函数）
			多重颜色：
				glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex);
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGB, width, height, GL_TRUE);
				glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, tex, 0);
			多重渲染缓冲对象：
				glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, width, height);
		2.搭建一个中介帧缓冲区（普通的缓冲区，用于转换数据）
			// we only need a color buffer
				unsigned int screenTexture;
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenTexture, 0);	

				screenShader.setInt("screenTexture", 0);//
		3.将画面绘制入多重采样缓冲区
				glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject);
				draw...

		4.将多重采样缓冲区的颜色数据绘制入中介缓冲区：
			glBindFramebuffer(GL_READ_FRAMEBUFFER, multisampledFBO);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, intermediateFBO);
			glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		5.在屏幕输出的默认缓冲区中使用一个画布，将缓冲区的数据输出到画布上。
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glBindTexture(GL_TEXTURE_2D, screenTexture);
			glDrawArrays(GL_TRIANGLES, 0, 6);
*/

/*
	上面的方式有点遗憾：
		我们还是需要把数据放入一个普通的缓冲区（每个像素都是单采样点）

		GLSL提供了额外的选项，允许我们在着色器中获取所有的采样点采集结果。
			让我们能够对纹理图像的每个子样本进行采样。
			要想获取每个子样本的颜色值，你需要：
			
			将纹理uniform采样器设置为sampler2DMS，而不是平常使用的sampler2D：
					uniform sampler2DMS screenTextureMS;
			使用texelFetch函数就能够获取每个子样本的颜色值了：
				vec4 colorSample = texelFetch(screenTextureMS, TexCoords, 3);  // 第4个子样本
				
*/
unsigned int loadTexture(char const *path, unsigned int Tex_Wrap_Para);
unsigned int loadCubeMap(std::vector<std::string> texture_faces);
void framebuffer_size_callback(GLFWwindow * window, int width, int height) { glViewport(0, 0, width, height); return; }
void mouse_callback(GLFWwindow* window, double xPos, double yPos);
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
void ProcessInput(GLFWwindow * window, float deltaTime);

void Aliasing_Cube(GLFWwindow * window);
//off aliasing
void Off_Aliasing(GLFWwindow * window);

const int SCR_WIDTH = 800;
const int SCR_HEIGHT = 600;



glm::vec3 cameraPos(0.0f, 1.0f, 3.0f);
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

	//glfwWindowHint(GLFW_SAMPLES, 4);//创建

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
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);	//关闭鼠光标
	glfwSetCursorPosCallback(window, mouse_callback);				//打开鼠标回调
	glfwSetScrollCallback(window, scroll_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		system("pause");
		return -1;
	}

	Off_Aliasing(window);
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
		if (openEffect) { 
			openEffect = false; 
			glDisable(GL_MULTISAMPLE);
		}
		else {
			openEffect = true;
			glEnable(GL_MULTISAMPLE);
		}
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
GLfloat newCubeVertices[] = {
	// Positions       
	-0.5f, -0.5f, -0.5f,
	0.5f, -0.5f, -0.5f,
	0.5f,  0.5f, -0.5f,
	0.5f,  0.5f, -0.5f,
	-0.5f,  0.5f, -0.5f,
	-0.5f, -0.5f, -0.5f,

	-0.5f, -0.5f,  0.5f,
	0.5f, -0.5f,  0.5f,
	0.5f,  0.5f,  0.5f,
	0.5f,  0.5f,  0.5f,
	-0.5f,  0.5f,  0.5f,
	-0.5f, -0.5f,  0.5f,

	-0.5f,  0.5f,  0.5f,
	-0.5f,  0.5f, -0.5f,
	-0.5f, -0.5f, -0.5f,
	-0.5f, -0.5f, -0.5f,
	-0.5f, -0.5f,  0.5f,
	-0.5f,  0.5f,  0.5f,

	0.5f,  0.5f,  0.5f,
	0.5f,  0.5f, -0.5f,
	0.5f, -0.5f, -0.5f,
	0.5f, -0.5f, -0.5f,
	0.5f, -0.5f,  0.5f,
	0.5f,  0.5f,  0.5f,

	-0.5f, -0.5f, -0.5f,
	0.5f, -0.5f, -0.5f,
	0.5f, -0.5f,  0.5f,
	0.5f, -0.5f,  0.5f,
	-0.5f, -0.5f,  0.5f,
	-0.5f, -0.5f, -0.5f,

	-0.5f,  0.5f, -0.5f,
	0.5f,  0.5f, -0.5f,
	0.5f,  0.5f,  0.5f,
	0.5f,  0.5f,  0.5f,
	-0.5f,  0.5f,  0.5f,
	-0.5f,  0.5f, -0.5f
};
void Aliasing_Cube(GLFWwindow * window) {
	Shader shader("Shader4-11.vertex", "Shader4-11.fragment");
	//glEnable(GL_MULTISAMPLE);//enable 多重采样
	glDisable(GL_MULTISAMPLE);
	unsigned int VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1,&VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER,VBO);
	glBufferData(GL_ARRAY_BUFFER,sizeof(newCubeVertices),&newCubeVertices[0],GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0 , 3, GL_FLOAT, GL_FALSE, 3*sizeof(GL_FLOAT), (void*)0);
	glBindBuffer(GL_ARRAY_BUFFER,0);
	glBindVertexArray(0);

	float curTime{}, lastTime{};
	curTime = lastTime = glfwGetTime();

	while (!glfwWindowShouldClose(window)) {
		glEnable(GL_DEPTH_TEST);
		//ProcessInput
		curTime = glfwGetTime();
		ProcessInput(window, curTime - lastTime);
		lastTime = curTime;

		// render
		// ------
		glm::mat4 model;
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		//first pass
		glClearColor(0.1f, 0.1f, 0.1f, 0.1f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.use();
		model = glm::mat4();
		shader.setMartix("model", model);
		shader.setMartix("projection", projection);
		shader.setMartix("view", view);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES,0,36);
		glBindVertexArray(0);

		//swap and poll
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

void Off_Aliasing(GLFWwindow * window) {

	Shader shader("Shader4-11.vertex", "Shader4-11.fragment");
	Shader screenShader("Shader.vertex_canvas","Shader.fragment_canvas");
	//创建帧缓冲
	unsigned int frameBufferObject;
	glGenFramebuffers(1, &frameBufferObject);
	glBindFramebuffer(GL_FRAMEBUFFER,frameBufferObject);

	//颜色缓冲区
	int samples = 4;
	unsigned int multiSample_Buffer ;
	glGenTextures(1, &multiSample_Buffer);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, multiSample_Buffer);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples,GL_RGB, SCR_WIDTH , SCR_HEIGHT , GL_TRUE);
	/*
		参数1：目标
		参数2：设置我们希望贴图所有的样本数
		参数6：GL_TRUE：图像将会对每个纹素使用相同的样本位置以及相同数量的子采样点个数。
	*/
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//当前绑定的帧缓冲现在就有了一个纹理图像形式的多重采样颜色缓冲。
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 , GL_TEXTURE_2D_MULTISAMPLE , multiSample_Buffer , 0);
	
	//深度和模板缓冲区
	unsigned int renderBufferObject;
	glGenRenderbuffers(1, &renderBufferObject);
	glBindRenderbuffer(GL_RENDERBUFFER, renderBufferObject);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples ,GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBufferObject);
	//当我们为渲染缓冲对象分配了足够的内存之后，我们可以解绑这个渲染缓冲。
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER,0);

	// configure second post-processing framebuffer
	unsigned int intermediateFBO;
	glGenFramebuffers(1, &intermediateFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);
	// create a color attachment texture
	unsigned int screenTexture;
	glGenTextures(1, &screenTexture);
	glBindTexture(GL_TEXTURE_2D, screenTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenTexture, 0);	// we only need a color buffer

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Intermediate framebuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

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
		-1.0f, -1.0f, 0.0f,	0.0f, 0.0f,
		1.0f, -1.0f, 0.0f,	1.0f, 0.0f,

		//left top
		1.0f, 1.0f,  0.0f,	1.0f, 1.0f,
		-1.0f, 1.0f, 0.0f,	0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f
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
	screenShader.setInt("screenTexture",0);

	unsigned int cubeVAO, cubeVBO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);
	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(newCubeVertices), &newCubeVertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), (void*)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	float curTime{}, lastTime{};
	curTime = lastTime = glfwGetTime();
	
	/*将一个用4个屏幕空间坐标所定义的源区域复制到一个同样用4个屏幕空间坐标所定义的目标区域中。
		源区域  -> 目标区域
	*/
	while (!glfwWindowShouldClose(window)) {

		//ProcessInput
		curTime = glfwGetTime();
		ProcessInput(window, curTime - lastTime);
		lastTime = curTime;

		// render
		// ------
		glm::mat4 model;
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		
		//first pass
		
		glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject);
		glClearColor(0.1f, 0.1f, 0.1f, 0.1f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		
		//DrawScence
		shader.use();
		model = glm::mat4();
		shader.setMartix("projection", projection);
		shader.setMartix("view", view);
		shader.setMartix("model", model);
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

		//second pass
		glBindFramebuffer(GL_READ_FRAMEBUFFER, frameBufferObject);	//从自定义帧缓冲区读取
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, intermediateFBO);				//绘制如屏幕显示缓冲区
		glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		//third pass
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		screenShader.use();
		glBindVertexArray(canvasVAO);
		glDisable(GL_DEPTH_TEST);	//在绘制简单画布（四边形）的时候，我们不需要开启深度测试！
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, screenTexture);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		//swap and poll
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

}


inline void MyBindUniform(unsigned int shaderID, const char * uniformBlockName, unsigned int bindingPoint) {
	unsigned int UniformBlockIndex = glGetUniformBlockIndex(shaderID, uniformBlockName);
	glUniformBlockBinding(shaderID, UniformBlockIndex, bindingPoint);
	return;
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
