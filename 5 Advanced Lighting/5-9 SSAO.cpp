#include <glad\glad.h>
#include <glfw3.h>

//数学库
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <random>
#include <iostream>
#include "Shaders.h"
#include "Camera.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Model.h"

/*
	困惑， 这里SSAO构建中，为了减少样本量，对生成的kernel进行了随机的旋转。
	旋转是基于kernel的，教程中说：因为采样核心在切线空间内沿着正z轴旋转，因此旋转的向量是一个xy平面（切线空间内）的向量。

	然而在着色器中，该旋转向量是和位于观察空间内的法向量结合使用的！ 其直接被拿来和该法向量构建正交基，
	即切线空间和观察空间的变换矩阵。
	有点搞不懂原因！
	作者说这里不需要逐顶点构建切线空间，也即此切线空间并不完全准确。

	一点想法是：
	任意两个不在同一直线上的向量都可以构建一个正交基，这里只要保证正交基的z轴沿着法向量也就足够了！
	所以这里主要是保证创建的旋转向量不合法向量重线就够了。

*/

/*	SSAO
	我们在之前的基础光照教程中已经进行了简单的介绍：Ambient light 。环境光照。
	环境光照是我们加入场景总体光照中的一个固定光照常量，它被用来模拟光的散射(Scattering)。
	在现实世界里，光在不同方向的散射有着不同的强度。所以间接被照到的那部分场景也应该有变化的强度。

	一种间接光照的模拟叫做：ambient occlusion - 环境光遮蔽。
	它的原理是通过将褶皱、孔洞和非常靠近的墙面变暗的方法近似模拟出间接光照。
	这些区域很大程度上是被周围的几何体遮蔽的，光线会很难逃离，因此相应区域会变暗。

	环境光遮蔽这一技术会带来很大的性能开销，因为他们不得不考虑周围的集合体。我们可以对空间中每一点
	发射大量光线来确定其遮蔽量，但是这会在实时处理中迅速变得不可计算。

	SSAO：screen-space ambient occlusion 屏幕空间环境遮蔽 是2007年发布的一款技术，
		该技术使用了屏幕空间的场景深度信息去判断遮蔽量而不是使用真实的几何数据。
		这一做法相对于真正的环境光遮蔽不但速度快，而且还能获得很好的效果，使得它成为近似实时环境光遮蔽的标准。

	SSAO的原理是很简单的：
		对于在一个铺屏四边形(Screen-filled Quad)上的每个片段我们基于该片段周围的深度信息来计算出遮蔽值。
	这个遮蔽因子之后会被用来减少或者抵消片段的环境光照分量。
		遮蔽因子是通过采集片段周围球型核心(Kernel)的多个深度样本，并和当前片段深度值对比而得到的。
	高于片段深度值样本的个数就是我们想要的遮蔽因子。
		
		几何体内样本个数越多，片段获得的环境光照也就越少。因此效果的质量和精度是与我们采集量直接相关的。
		如果样本数量太低，渲染的精度会急剧减少，我们会得到一种叫做波纹(Banding)的效果；而如果采样太多对性能影响会很重
		
		我们可以通过引入随机性到采样核心(Sample Kernel)的采样中从而减少样本的数目。通过随机旋转采样核心，
	我们能够使用更少的样本数量得到一个高质量的结果。这需要付出一些代价（does come at a price）因为随机会导致生成
	一个明显的噪声模式。我们需要通过模糊来进行修正。
		
	因为使用的采样内核是一个球体，它会让平坦的墙壁看起来呈现灰色因为一般的内科采样最终会进入周围的集合体中。
	
	出于这样的原因，我们打算不用球体的采样内核，而是采样沿着表面法向量的一个半球采样内核

		通过在法向半球体(Normal-oriented Hemisphere)周围采样，我们将不会考虑到片段底部的几何体.它消除了环境
	光遮蔽灰蒙蒙的感觉，从而产生更真实的结果。

*/

/*	Sample buffers
	SSAO需要几何信息，因为我们需要一些方式来判定一个片段的遮蔽因子。对于每一个片段，我们都需要如下信息：
		位置向量：
		法向量：
		反射颜色
		一个采样内核
		一个随机的旋转向量：被用于旋转采样内核

	通过使用逐片段的观察空间位置，我们可以将一个采样半球内核沿着片段的观察空间下的表面法向量。然后使用这个内核
	以不同的偏移量去采集位置缓冲区纹理。对于每一个逐片段内核采集，我们会对其的深度与位置缓冲区中的深度进行比较，
	以判断遮蔽量。遮蔽因子的结果被用于限制最终的环境光组分。另外通过包含一个逐片段的旋转向量，我们可以显著的减少
	所需要的采样量，我们随后会看到

	因为SSAO是一个屏幕空间技术，我们在一个铺满平面的2D四方形上为每一个片段计算其效果。但是这个意味着我们没有场景的几何信息。
	我们要做的是逐片段渲染几何数据到屏幕空间纹理中，我们随后会发送这个纹理到SSAO着色器，所以我们可以使用每一个片段的几何数据
	
	这和延迟渲染很相似。这也就是说SSAO和延迟渲染能完美地兼容，因为我们已经存位置和法线向量到G缓冲中了。
	这里会使用一个简化的延迟渲染。

	由于我们应该已经从G缓冲区获得了每片段位置和普通数据，因此几何阶段的片段着色器非常简单：
			Shader5-9.fragment_SSAO_gbuffer
	因为SSAO是一个屏幕空间技术，这里遮蔽的计算是基于可见视图。因此在观察视图中实现算法是有意义的。
	因此，几何阶段的顶点着色器所提供的FragPos被转换到观察空间中。所有随后的计算也都是在观察空间中进行，所以
	应确保G-buffer的位置和法线都处于观察空间中（通过乘以View 矩阵）
		
		通过一些小技巧来通过深度值重构实际位置值是可能的，Matt Pettineo在他的博客里提到了这一技巧。
	这一技巧需要在着色器里进行一些计算，但是省了我们在G缓冲中存储位置数据，从而省了很多内存。为了
	示例的简单，我们将不会使用这些优化技巧，你可以自行探究。

	gPosition颜色缓冲区纹理配置如下：
		glGenTextures(1, &gPosition);
		glBindTexture(GL_TEXTURE_2D, gPosition);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);  
	这里给出了一个位置纹理，我们可以用于获取每个内核采集的深度值。注意我们将位置信息保存为浮点数据格式。
	这种方式下，位置值不会被限制在[0.0 , 1.0], 同时也注意纹理的环绕方式为GL_CLAMP_TO_EDGE，这保证了我们不会意外的
	采样在屏幕空间中纹理默认坐标区域外的位置/深度值。
		
	接下来我们需要实际的半球采集内核，以及一些随机渲染的方法
*/

/*Normal-oriented hemisphere 法向半球
	我们需要沿着表面法线方向生成大量的样本。就像我们在这个教程的开始介绍的那样，我们想要生成形成半球形的样本。
	由于对每个表面法线方向生成采样核心非常困难，也不合实际，我们将在切线空间(Tangent Space)内生成采样核心，
	法向量将指向正z方向。

	假设我们有一个单位半球，我们可以获得一个拥有最大64样本值的采样核心：
		std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between 0.0 - 1.0
		std::default_random_engine generator;
		std::vector<glm::vec3> ssaoKernel;
		for (unsigned int i = 0; i < 64; ++i)	//64个采样
		{
			glm::vec3 sample(
				randomFloats(generator) * 2.0 - 1.0,	//-1.0 - 1.0
				randomFloats(generator) * 2.0 - 1.0,	//-1.0 - 1.0
				randomFloats(generator)					//0.0 - 1.0
			);
			sample  = glm::normalize(sample);
			sample *= randomFloats(generator);
			float scale = (float)i / 64.0; 
			ssaoKernel.push_back(sample);  
		}
		我们在切线空间中以-1.0到1.0为范围变换x和y方向，并以0.0和1.0为范围变换样本的z方向（如果我们在-1.0到1.0之间变换，
	则我们会得到一个球状采集内核）。因为采集内核朝向表面法向量，所得的样本矢量将会在半球里。
		
		当前，所有的采样都随机分布在采样内核，但是我们更愿意对靠近实际片段的遮蔽赋予更大权重，也就是将核心样本靠近原点分布。
	我们可以用一个加速插值函数实现它：
			...[接上函数]
		   scale = lerp(0.1f, 1.0f, scale * scale);
		   sample *= scale;
		   ssaoKernel.push_back(sample);  
		}

		lerp定义如下：
			float lerp(float a, float b, float f)
			{
				return a + f * (b - a);
			}  
	这就给了我们一个大部分样本靠近原点的核心分布。
		
		每一个内核采集都将被用于去偏移观察空间的片段位置以采集周围的几何。为了获得真实的效果，我们需要在观察空间中获得大量
	的采样，但是这对性能的影响太大。
		然而，如果我们在每个片段的基础上引用一些半随机处理，我们可以显著减少大量的采样需求。

*/


/*Random kernel rotations 随机内核旋转
	通过引入一些随机性到采样核心上，我们可以大大减少获得不错结果所需的样本数量。
	我们可以对场景中每一个片段创建一个随机旋转向量，但这会很快将内存耗尽。
	所以，更好的方法是创建一个小的随机旋转向量纹理平铺在屏幕上。

	我们创建一个4*4的朝向切线空间表面法向量的随机旋转向量组：
		std::vector<glm::vec3> ssaoNoise;
		for (unsigned int i = 0; i < 16; i++)
		{
			glm::vec3 noise(
				randomFloats(generator) * 2.0 - 1.0, 
				randomFloats(generator) * 2.0 - 1.0, 
				0.0f); 
			ssaoNoise.push_back(noise);
		} 
	因为采样内核朝向了切线空间的正z方向，我们设定z分量为0.0，从而围绕z轴旋转。

	然后创建4*4的纹理，用来容纳随机旋转向量，确保将其环绕方式设置为GL_REPEAT，从而保证它合适地平铺在屏幕上。
		GLuint noiseTexture;
		glGenTextures(1, &noiseTexture);
		glBindTexture(GL_TEXTURE_2D, noiseTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	现在我们有了所有的相关输入数据，接下来我们需要实现SSAO。

*/

/*	SSAO着色器	The SSAO shader
	SSAO着色器在2D的铺屏四边形上运行，它对于每一个生成的片段计算遮蔽值(为了在最终的光照着色器中使用)。
	由于我们需要存储SSAO阶段的结果，我们还需要在创建另外一个帧缓冲对象：

		unsigned int ssaoFBO;
		glGenFramebuffers(1, &ssaoFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);

		unsigned int ssaoColorBuffer;
		glGenTextures(1, &ssaoColorBuffer);
		glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);
	因为环境遮蔽的结果是一个单纯的灰度值，我们仅需要一个纹理的红色组件，因此这里设置颜色缓冲区的内置格式为为GL_RED。

	渲染SSAO的完整过程看起来应该是这个样子：
		// geometry pass: render stuff into G-buffer
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
			[...]
		glBindFramebuffer(GL_FRAMEBUFFER, 0);  
  
		// use G-buffer to render SSAO texture
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
			glClear(GL_COLOR_BUFFER_BIT);    
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, gPosition);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, gNormal);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, noiseTexture);
			shaderSSAO.use();
			SendKernelSamplesToShader();
			shaderSSAO.setMat4("projection", projection);
			RenderQuad();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
		// lighting pass: render scene lighting
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shaderLightingPass.use();
		[...]
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
		[...]
		RenderQuad();
	
	shaderSSAO着色器接收相应的G-buffer纹理，噪声纹理以及法向半球内核采样作为输入，
		其代码详见 Shader5-9.fragment_SSAO。

		其基本思想即：
			随机获取对应位置下，以其法向量为方向的半球空间内的点（此案例中逐步获取64个采样点），
		通过对比随机采样的点的深度和该位置的深度关系，这里认为若采样点的深度大于及对应的实际深度，则增加遮蔽因子的值
*/


/*
--函数------------------------------
*/
void framebuffer_size_callback(GLFWwindow * window, int width, int height) { glViewport(0, 0, width, height);	return; }
void mouse_callback(GLFWwindow* window, double xPos, double yPos);
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
void ProcessInput(GLFWwindow * window, float deltaTime);

void renderPlane(Shader shader);
void renderCube(Shader shader, std::vector<glm::vec3> cubePositions, std::vector<glm::vec3> cubeRotateAxis = std::vector<glm::vec3>());
void renderLight(Shader shader, std::vector<glm::vec3> lightPositions, std::vector<glm::vec3> lightColors = std::vector<glm::vec3>());
void RenderQuad(Shader shader);

unsigned int loadTexture(char const *path, unsigned int Tex_Wrap_Para = GL_REPEAT, bool gammaCorrection = false);

void SSAO_Test(GLFWwindow * window);
void SSAO_Gbuff_Test(GLFWwindow * window);

/*
--变量------------------------------
*/
const int SCR_WIDTH = 1280;
const int SCR_HEIGHT = 720;

glm::vec3 cameraPos(0.0f, 0.0f, 5.0f);
glm::vec3 WorldUp(0.0f, 1.0f, 0.0f);

float yaw = -90.0f, pitch = 0.0f;
bool hdr = true;
bool hdrKeyPressed = false;
float exposure = 1.0f;

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
	SSAO_Test(window);
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
	//std::cout << "extract : " << extract << std::endl;
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

void SSAO_Gbuff_Test(GLFWwindow * window) {
	glEnable(GL_DEPTH_TEST);
	//Shader
	Shader geometryPass_Shader("Shader5-9.vertex_gbuffer", "Shader5-9.fragment_gbuffer");	//将几何数据传输入G-buffer
	Shader shaderSSAO("Shader5-9.vertex_SSAO", "Shader5-9.fragment_SSAO");
	Shader lightPass_shader("Shader5-9.vertex_light", "Shader5-9.fragment_light");

	//纳米机器人
	char model_path[] = "E:/C++/OpenGL_Course/OpenGL_Course/Model/nanosuit/nanosuit.obj";
	Model nanosuitModel(model_path);

	//G-buffer的配置
	GLuint gBuffer;
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	GLuint gPosition, gNormal, gAlbedo; //三个缓冲区

	//0 - Position buffer
	glGenTextures(1, &gPosition);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);	//NULL表示空的无数据
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);	//Position - 0

																								//1 - Normal buffer
	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);	//NULL表示空的无数据
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);	//Normal - 1

																								// color + specular color buffer
	glGenTextures(1, &gAlbedo);
	glBindTexture(GL_TEXTURE_2D, gAlbedo);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedo, 0);

	unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);

	//3 - Create and attach Depth buffer
	GLuint depthBuffer;
	glGenRenderbuffers(1, &depthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// lighting info
	// -------------
	glm::vec3 lightPos = glm::vec3(2.0, 4.0, -2.0);
	glm::vec3 lightColor = glm::vec3(0.2, 0.2, 0.7);

	lightPass_shader.use();
	lightPass_shader.setInt("gPosition", 0);
	lightPass_shader.setInt("gNormal", 1);
	lightPass_shader.setInt("gAlbedo", 2);

	float curTime{}, lastTime{};
	curTime = lastTime = glfwGetTime();
	while (!glfwWindowShouldClose(window)) {
		glEnable(GL_DEPTH_TEST);
		//Process Input
		curTime = glfwGetTime();
		ProcessInput(window, curTime - lastTime);
		lastTime = curTime;

		std::vector<glm::vec3> cubePosition;
		cubePosition.push_back(glm::vec3(0.0f, 7.0f, 0.0f));
		//Draw
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glm::mat4 projection = glm::perspective(camera.Zoom, (GLfloat)SCR_WIDTH / (GLfloat)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 model;
		// 1.geometry pass: render stuff into G-buffer
		//glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// room cube  -- 一个大的立方体
		geometryPass_Shader.setMartix("projection", projection);
		geometryPass_Shader.setMartix("view", view);
		geometryPass_Shader.setInt("invertedNormals", 1); // invert normals as we're inside the cube
		renderCube(geometryPass_Shader, cubePosition);
		geometryPass_Shader.setInt("invertedNormals", 0);

		// nanosuit model on the floor	--躺在地板上的模型
		model = glm::mat4();
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 5.0));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));
		model = glm::scale(model, glm::vec3(0.5f));
		geometryPass_Shader.setMartix("model", model);
		nanosuitModel.Draw(geometryPass_Shader);

		// 2. lighting pass:
		// -----------------------------------------------------------------------------------------------------
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//lightPass_shader.use();
		//// send light relevant uniforms
		//glm::vec3 lightPosView = glm::vec3(camera.GetViewMatrix() * glm::vec4(lightPos, 1.0));
		//lightPass_shader.setVec3("light.Position", lightPosView);
		//lightPass_shader.setVec3("light.Color", lightColor);
		//// Update attenuation parameters
		//const float constant = 1.0; // note that we don't send this to the shader, we assume it is always 1.0 (in our case)
		//const float linear = 0.09;
		//const float quadratic = 0.032;
		//lightPass_shader.setFloat1f("light.Linear", linear);
		//lightPass_shader.setFloat1f("light.Quadratic", quadratic);
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, gPosition);
		//glActiveTexture(GL_TEXTURE1);
		//glBindTexture(GL_TEXTURE_2D, gNormal);
		//glActiveTexture(GL_TEXTURE2);
		//glBindTexture(GL_TEXTURE_2D, gAlbedo);
		//RenderQuad(lightPass_shader);

		//
		//swap and poll
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}
//延迟渲染
void SSAO_Test(GLFWwindow * window) {
	glEnable(GL_DEPTH_TEST);
	//Shader
	Shader geometryPass_Shader("Shader5-9.vertex_gbuffer", "Shader5-9.fragment_gbuffer");	//将几何数据传输入G-buffer
	Shader shaderSSAO("Shader5-9.vertex_SSAO", "Shader5-9.fragment_SSAO");
	Shader lightPass_shader("Shader5-9.vertex_light", "Shader5-9.fragment_light");

	//纳米机器人
	char model_path[] = "E:/C++/OpenGL_Course/OpenGL_Course/Model/nanosuit/nanosuit.obj";
	Model nanosuitModel(model_path);

	//G-buffer的配置
	GLuint gBuffer;
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	GLuint gPosition, gNormal, gAlbedo; //三个缓冲区

	//0 - Position buffer
	glGenTextures(1, &gPosition);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);	//NULL表示空的无数据
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);	//Position - 0

	//1 - Normal buffer
	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);	//NULL表示空的无数据
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);	//Normal - 1

	// color + specular color buffer
	glGenTextures(1, &gAlbedo);
	glBindTexture(GL_TEXTURE_2D, gAlbedo);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedo, 0);

	unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);

	//3 - Create and attach Depth buffer
	GLuint depthBuffer;
	glGenRenderbuffers(1, &depthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//SSAO 帧缓冲 以及 SSAO 模糊帧缓冲
	unsigned int ssaoFBO, ssaoBlurFBO;
	glGenFramebuffers(1, &ssaoFBO);					glGenFramebuffers(1, &ssaoBlurFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);	
	unsigned int ssaoColorBuffer, ssaoColorBufferBlur;
	glGenTextures(1, &ssaoColorBuffer);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "SSAO Framebuffer not complete!" << std::endl;
	// and blur stage
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
	glGenTextures(1, &ssaoColorBufferBlur);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBufferBlur, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "SSAO Blur Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//SSAO采样内核
	std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between 0.0 - 1.0
	std::default_random_engine generator;
	std::vector<glm::vec3> ssaoKernel;
	for (unsigned int i = 0; i < 64; ++i)
	{
		//生成了64个sample向量，随机分布在朝向z轴正向的半球体内！
		glm::vec3 sample(
			randomFloats(generator) * 2.0 - 1.0,	//-1.0 ～ 1.0
			randomFloats(generator) * 2.0 - 1.0,	//-1.0 ～ 1.0
			randomFloats(generator)					// 0.0 ～ 1.0
		);
		sample = glm::normalize(sample);	//单位向量
		sample *= randomFloats(generator);	//
		float scale = (float)i / 64.0;		

		//给了我们一个大部分样本靠近原点的核心分布。
		/*
			lerp:线性插值
			float lerp(float a , float b , scale){
				return a + scale * (b - a);
			}
		*/
		float lerp = 0.1f + scale * scale * (1.0f - 0.1f);
		scale = lerp;
		sample *= scale;
		ssaoKernel.push_back(sample);
	}

	std::vector<glm::vec3> ssaoNoise;	//用于转动采样kernal
	for (GLuint i = 0; i < 16; i++)
	{
		glm::vec3 noise(	
			randomFloats(generator) * 2.0 - 1.0,		//-1.0 - 1.0
			randomFloats(generator) * 2.0 - 1.0,		//-1.0 - 1.0
			0.0f);	//噪声向量							// 0.0		z轴为0：即 其为 xy平面的向量。
		ssaoNoise.push_back(noise);
	}
	unsigned int noiseTexture; glGenTextures(1, &noiseTexture);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);	//噪声纹理，填充了随机噪声向量
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// lighting info
	// -------------
	glm::vec3 lightPos = glm::vec3(2.0, 4.0, -2.0);
	glm::vec3 lightColor = glm::vec3(0.2, 0.2, 0.7);

	lightPass_shader.use();
	lightPass_shader.setInt("gPosition", 0);
	lightPass_shader.setInt("gNormal", 1);
	lightPass_shader.setInt("gAlbedo", 2);
	lightPass_shader.setInt("ssao", 3);
	shaderSSAO.use();
	shaderSSAO.setInt("gPosition", 0);
	shaderSSAO.setInt("gNormal", 1);
	shaderSSAO.setInt("texNoise", 2);
	//shaderSSAOBlur.use();
	//shaderSSAOBlur.setInt("ssaoInput", 0);


	float curTime{}, lastTime{};
	curTime = lastTime = glfwGetTime();
	while (!glfwWindowShouldClose(window)) {
		glEnable(GL_DEPTH_TEST);
		//Process Input
		curTime = glfwGetTime();
		ProcessInput(window, curTime - lastTime);
		lastTime = curTime;

		std::vector<glm::vec3> cubePosition;
		cubePosition.push_back(glm::vec3(0.0f, 3.0f, 0.0f));
		//Draw
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glm::mat4 projection = glm::perspective(camera.Zoom, (GLfloat)SCR_WIDTH / (GLfloat)SCR_HEIGHT, 0.1f, 50.0f);
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 model;
		// 1.geometry pass: render stuff into G-buffer, 包括 位置、法向量和反射率
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		geometryPass_Shader.use();
		geometryPass_Shader.setMartix("projection", projection);
		geometryPass_Shader.setMartix("view", view);

		// room cube  -- 一个大的立方体
		geometryPass_Shader.setInt("invertedNormals", 1); // invert normals as we're inside the cube
		renderCube(geometryPass_Shader, cubePosition);
		geometryPass_Shader.setInt("invertedNormals", 0);
		// nanosuit model on the floor	--躺在地板上的模型
		model = glm::mat4();
		model = glm::translate(model, glm::vec3(0.0f, -1.5f, 0.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));
		model = glm::scale(model, glm::vec3(0.3f));
		geometryPass_Shader.setMartix("model", model);
		nanosuitModel.Draw(geometryPass_Shader);

		// 2. generate SSAO texture
		//glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			shaderSSAO.use();
			shaderSSAO.setMartix("projection", projection);
			// Send kernel + rotation 
			for (unsigned int i = 0; i < 64; ++i)
				shaderSSAO.setVec3("samples[" + std::to_string(i) + "]", ssaoKernel[i]);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, gPosition);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, gNormal);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, noiseTexture);	//noiseTexture
			RenderQuad(shaderSSAO);
		
		// 3. blur SSAO texture to remove noise
		// ------------------------------------
		/*glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
		glClear(GL_COLOR_BUFFER_BIT);
		shaderSSAOBlur.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
		renderQuad();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);*/

		// 4. lighting pass: traditional deferred Blinn-Phong lighting with added screen-space ambient occlusion
		// -----------------------------------------------------------------------------------------------------
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		lightPass_shader.use();
		// send light relevant uniforms
		glm::vec3 lightPosView = glm::vec3(camera.GetViewMatrix() * glm::vec4(lightPos, 1.0));
		lightPass_shader.setVec3("light.Position", lightPosView);
		lightPass_shader.setVec3("light.Color", lightColor);
		// Update attenuation parameters
		const float constant = 1.0; // note that we don't send this to the shader, we assume it is always 1.0 (in our case)
		const float linear = 0.09;
		const float quadratic = 0.032;
		lightPass_shader.setFloat1f("light.Linear", linear);
		lightPass_shader.setFloat1f("light.Quadratic", quadratic);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gPosition);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gNormal);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gAlbedo);
		glActiveTexture(GL_TEXTURE3); // add extra SSAO texture to lighting pass
		glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
		RenderQuad(lightPass_shader);
		
		//
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
	model = glm::scale(model, glm::vec3(3.0f, 1.0f, 3.0f));
	shader.setMartix("model", model);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

}
void renderCube(Shader shader, std::vector<glm::vec3> cubePositions, std::vector<glm::vec3> cubeRotateAxis ) {
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
		if (!cubeRotateAxis.empty())
			model = glm::rotate(model, glm::radians(i * 15.0f), cubeRotateAxis[i]);
		model = glm::scale(model, glm::vec3(10.0f, 10.0f, 10.0f));
		shader.setMartix("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
	glBindVertexArray(0);

}

void renderLight(Shader shader, std::vector<glm::vec3> lightPositions, std::vector<glm::vec3> lightColors) {
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
		if (!lightColors.empty())
			shader.setVec3("cubeColor", lightColors[i]);
		else
			shader.setVec3("cubeColor", glm::vec3(1.0f, 1.0f, 1.0f));
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
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

void Model_Test(GLFWwindow * window) {
	glEnable(GL_DEPTH_TEST);
	Shader geometryPass_Shader("Shader5-8.vertex_gbuffer", "Shader5-9.fragment_SSAO_gbuffer");
	//纳米机器人
	char model_path[] = "E:/C++/OpenGL_Course/OpenGL_Course/Model/nanosuit/nanosuit.obj";
	Model nanosuitModel(model_path);


	std::vector<glm::vec3> objectPositions;
	objectPositions.push_back(glm::vec3(-3.0, -3.0, -3.0));
	objectPositions.push_back(glm::vec3(0.0, -3.0, -3.0));
	objectPositions.push_back(glm::vec3(3.0, -3.0, -3.0));
	objectPositions.push_back(glm::vec3(-3.0, -3.0, 0.0));
	objectPositions.push_back(glm::vec3(0.0, -3.0, 0.0));
	objectPositions.push_back(glm::vec3(3.0, -3.0, 0.0));
	objectPositions.push_back(glm::vec3(-3.0, -3.0, 3.0));
	objectPositions.push_back(glm::vec3(0.0, -3.0, 3.0));
	objectPositions.push_back(glm::vec3(3.0, -3.0, 3.0));

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

		glm::mat4 projection = glm::perspective(camera.Zoom, (GLfloat)SCR_WIDTH / (GLfloat)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 model;
		geometryPass_Shader.use();
		geometryPass_Shader.setMartix("projection", projection);
		geometryPass_Shader.setMartix("view", view);
		geometryPass_Shader.setBool("inverse_normals", false);
		for (GLuint i = 0; i < objectPositions.size(); i++) {
			model = glm::mat4();
			model = glm::translate(model, objectPositions[i]);
			model = glm::scale(model, glm::vec3(0.25f));
			geometryPass_Shader.setMartix("model", model);
			nanosuitModel.Draw(geometryPass_Shader);
		}
		//swap and poll
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}