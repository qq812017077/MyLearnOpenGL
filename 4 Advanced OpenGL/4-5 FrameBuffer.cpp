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

/*目前已经使用过的缓冲区：
	颜色缓冲区：保存每一帧的颜色信息
	深度缓冲区：每一帧中保存最近的深度信息。
	模板缓冲区：允许我们根据一些条件丢弃特定片段，

		上诉缓冲区保存的是一个二维图像画面。

	这些缓冲区的组合就被称为一个帧缓冲。它们被保存在内存中的某个地方。

	OpenGL允许我们定义我们自己的帧缓冲，也就是说我们能够
		
			自己定义我们自己的颜色缓冲，甚至是深度缓冲和模板缓冲。
	之前的渲染运算都是基于默认的帧缓冲所进行的渲染缓冲。
		当我们创建窗口时，GLFW为我们创建并配置了默认的FrameBuffer

	通过渲染场景到不同的帧缓冲，我们可以创建出镜子，等不同的后期效果。
*/

/*	
	帧缓冲的使用步骤：
		1.创建帧缓冲，并绑定
		2.附加到其上至少一个缓冲区（颜色、深度、或者模板）
		3.至少有一个颜色附件(Attachment)。
		4.所有的附件都需要是完整的（有相应的存储内存）。
		5.每个缓冲都应该有相同的样本数(samples)。

	在我们完成上面的操作之后，我们可以检查我们的帧缓冲区是否完整：
		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
	
	之后所有的渲染操作将会渲染到当前绑定帧缓冲的附件中。
	但是需要注意的是，因为我们使用的非默认缓冲区，渲染结果不会影响窗口输出，我们称之为：
			Off-screen rendering	离屏渲染

	要保证所有的渲染操作在主窗口中有视觉效果，我们需要再次激活默认帧缓冲，将它绑定到0。
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
	当然，不要忘记删除掉这个不再使用的frameBufferObject：
		glDeleteFramebuffer(1, &frameBufferObject);


*/

/*Attachment
	附件：在检查完整性之前，我们需要添加几个附件给帧缓冲去。

	帧缓冲区是一个内存位置，可以作为帧缓冲的一个缓冲区执行。
	他我们创建一个Attachment的时候，要做两件事：
		textures	 or	   render buffer object
		贴图		或者	渲染缓冲区对象
	
	Texture Attachment
		当附加一个Texture到帧缓冲区的时候，
			所有的渲染命令将写到这个贴图中，仿佛这是一个正常的颜色或者深度或者模板的缓冲区！
			好处：
				渲染运算的结果被保存在一个贴图图像中，因此我们可以轻易的使用在shader中。
		为帧缓冲区创建一个贴图差不多和创建一个正常的贴图一样：
			unsigned int texture;
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
  
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);//最后的data被设置为了空而已！！

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		区别在于：我们将贴图的大小设置为了窗口的大小，然后将最后的data参数设置为了NULL。
			注意这里我们仅仅为其分配的内存，而没有填充数据。
					填充数据将在我们渲染帧缓冲区的时候发生！
		并且注意，我们不关心其的环绕方式，以及多级纹理MipMap。 因为我们不会用到。

		如果我们想将屏幕渲染到更大或者更小的贴图上，我们应该在渲染之前修改屏幕窗口的大小(glViewPort)，使用纹理的新维度作为参数。
			否则将只有一部分画面会被绘制在贴图上。

	在创建好一张贴图之后，我们需要附加到framebuffer中，
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);


	除了颜色附加之外，还可以附加深度和模板贴图到帧缓冲对象。
		附加深度：
			GL_DEPTH_ATTACHMENT，注意纹理的格式和内置类型应该为GL_DEPTH_COMPONENT

		附加模板：
			GL_STENCIL_ATTACHMENT ， 贴图格式为GL_STENCIL_INDEX

	另外我们可以将深度缓冲和模板缓冲附加为一个单独的纹理
		贴图的每32位包含了24位的深度信息以及8位的模板信息。
			这时，可以使用GL_DEPTH_STENCIL_ATTACHMENT,
					且需要配置贴图格式包含深度和模板值。
			例如：
				glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, 800, 600, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, texture, 0);

*/

/*
	Renderbuffer object attachment
		在过去贴图是唯一可用的附加件。
		Renderbuffer则是在Texture之后被引入到OpenGL中的，作为一种可用的类型。

		就像一个纹理图像，一个渲染缓冲对象是一个实际的缓冲区（如一组字节，整数，像素或者其他）
		一个渲染缓冲区对象有着额外的优势：它会将数据储存为OpenGL原生的渲染格式
				他是为了离屏渲染而进行过优化的。

	渲染缓冲区对象直接将所有的渲染数据保存在缓冲区中，而不会做任何针对纹理格式的转换，
		这让他们成为一个更快的可写入存储介质。
		
	然而，渲染缓冲区通常是只写的。因此不能从其中读取（像贴图访问）。
		我们仍然可以从中读取数据（通过glReadPixels函数，尽管它从当前的绑定帧缓冲中返回指定的像素区域），只是不能直接从附加物本身中获取。

	因为数据已经是原生格式了，所以写入数据或者拷贝数据到其他缓冲区是非常快的。
		当使用渲染缓冲对象的时候，像是交换缓冲区这样的操作是非常快的。
		（我们一直使用的glfwSwapBuffers，是可以用于RenderBuffer对象的：
				只需要写入一个渲染缓冲图像，然后在最后交换到另一个渲染缓冲。
		）

		创建渲染缓冲的代码：
			glGenRenderbuffers(1,&RBO);//RBO--> render buffer object
		绑定：
			glBindRenderbuffer(GL_RENDERBUFFER,RBO);
			绑定这个渲染缓冲对象，让之后所有的渲染缓冲操作影响当前的RBO.
		
		因为渲染缓冲对象是只写的，所以他们常被用作深度和模板的附加物。
			因为大部分时间我们都不需要从深度和模板缓冲中读取值，只关心深度和模板测试。
			我们需要深度和模板值用于测试，但不需要对它们进行采样，所以渲染缓冲对象非常适合它们。
		当我们不从缓冲区采集的时候，渲染缓冲区通常是更合适的，因为它被优化了。

		创建一个深度和模板缓冲区对象：
			glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH24_STENCIL8,800,600);
			（创建一个渲染缓冲对象和纹理对象类似，不同的是这个对象是专门被设计作为图像使用的，
					而不是纹理那样的通用数据缓冲(General Purpose Data Buffer)。
			）
		附加这个渲染缓冲对象：
			glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_STENCIL_ATTACHMENT,GL_RENDERBUFFER,RBO);

	NOTE:
		搞清楚什么时候使用渲染缓冲对象，什么时候使用纹理是非常重要的。
		一般规则：
			如果不需要采集数据：使用渲染缓冲对象，因为其会提供相应的优化。
			如果需要从中采集数据，比如颜色或者深度，则应该使用纹理。
*/

/*Rendering to a texture.
	sort of ：稍微，
	由于我们知道了（差不多）帧缓冲是如何工作，那么是时候使用了。
		
		我们想要渲染场景到一个我们创建好的帧缓冲上的颜色贴图附加物。
				之后将在一个横跨整个屏幕的四边形上绘制这个纹理。
		
		视图输出将和没有使用帧缓冲时一模一样，但是确实打印在了一个四边形上！
*/

/*流程：
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

void FrameBuffer_Test(GLFWwindow* window);


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
	FrameBuffer_Test(window);
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
	glGenFramebuffers(1,&FBO);
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
	glBindFramebuffer(GL_FRAMEBUFFER , 0);
	return FBO;
}

void FrameBuffer_Test(GLFWwindow* window) {
	//使能深度测试
	glEnable(GL_DEPTH_TEST);//深度测试

	Shader shader("Shader4-5.vertex", "Shader4-5.fragment");
	Shader shaderCanvas("Shader4-5.vertex_canvas", "Shader4-5.fragment_canvas");


	stbi_set_flip_vertically_on_load(true);//反转y轴，否则图片上下颠倒！
										   //Texture
	unsigned int cubeTexture = loadTexture(std::string("container.jpg").c_str(), GL_REPEAT);
	unsigned int floorTexture = loadTexture(std::string("wall.jpg").c_str(), GL_REPEAT);

	//***** frameBufferObject
	unsigned int frameBufferObject;
	glGenFramebuffers(1,&frameBufferObject);
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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER , GL_LINEAR);
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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, 800, 600, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH24_STENCIL8, GL_TEXTURE_2D, depth_stencilTextureBuffer, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	//考虑创建一个渲染缓冲对象(我们不打算采集深度和模板的数据，因此我们考虑使用渲染缓冲)
	unsigned int renderBufferObject;
	glGenRenderbuffers(1, &renderBufferObject);
	glBindRenderbuffer(GL_RENDERBUFFER, renderBufferObject);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 600);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_STENCIL_ATTACHMENT,GL_RENDERBUFFER,renderBufferObject);
	//当我们为渲染缓冲对象分配了足够的内存之后，我们可以解绑这个渲染缓冲。
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER,0);//解绑帧缓冲，保证我们不会不小心渲染到错误的帧缓冲


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
	glGenBuffers(1,&canvasVBO);
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
		// ------
		//first pass
		glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		//DrawScence
		glm::mat4 model;
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();

		//Plane
		shader.use();
		shader.setBool("openEffect", openEffect);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, floorTexture);
		model = glm::mat4();
		shader.setMartix("projection", projection);
		shader.setMartix("view", view);
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

		//second pass
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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