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

/*Shadow Mapping
	Shadow是因为遮挡导致没有光线的结果。当光源的光线因为受到一些物体的遮挡，
	而不能照向某处的时候，该处就处于了Shadow中。

	Shadow为一个光照场景增加了大量的现实，并且让观察者观察对象之间的空间关系变的更容易。
	它给了场景和对象以更好的深度感。

	目前还没有非常完美的shadow算法，有很多好的相近的算法，但都有一些自己的毛病需要被我们考虑。

	一个在大多数videogame中所使用的技术是（其有一个不错的效果，并且容易实现）：
		Shadow mapping
	
	Shadow mapping思想：
		从光源的试图位置进行渲染画面，所有看得到的一定是被点亮的，而所有看不到的则应该是阴暗的。
	假想从光源处沿着各个方向有一条射线可以获得，每条射线方向都找一个距离最近的点，那么比这个点更远的部分的点就不会被点亮了。
	（这是一个非常没有效率的方法，对于实时渲染不是特别有效）
	
	实际上我们要做的事情是类似的，但是不是通过射线，而是通过深度缓冲区。

		深度缓冲区中的值保存了摄像机视角下，一个片段的深度信息，那么
	如果我们要从光的视角渲染场景并将最终的深度信息保存在纹理中呢？ 这样我们就可以采集到
	从光的视角中最近的深度值信息。	最终，深度值就会显示从光源的透视图下见到的第一个片元了
	我们将这样的深度信息保存在一个叫做 depth map 或者 shadow map的纹理中。

	投影的基本思路不走如下：
		首先，我们需要在光源的透视试图下，基于光的位置和光的方向构建Model，View和Perspective矩阵(平行光的话应该是正交矩阵)；
	Perspective * View 即为从世界坐标转换到光源视角下坐标的变换矩阵，这里命名为T。
		对于要处理的片源P（假设在光源透视图下的深度信息为0.9）。
		1.首先将光源透视/正交图下的深度信息保存在Shadow_Map下，
				则该纹理图保存了离光最近的深度信息（P方向的深度信息值为0.4）。
		2.获取偏远P在T下的位置，以及该视图下的深度值。
		3.比较P的深度值（0.9）和对应方向下的深度信息值（0.4）
		4.发现P处于更远处，说明应在阴影下。
*/

/*阴影失真(Shadow Acne)
	在按照上面描述写完代码后运行的情况即惊喜后遗憾：
	阴影映射还是有点不真实：
		地板四边形渲染出很大一块交替黑线。
	阴影失真(Shadow Acne)：
		因为阴影贴图受限于分辨率，多个片元可能从深度贴图的同一个值中去采样。
		一个例子是：
			一个深度像素可能被其周围的多个片元所使用，也就是说在进行比较时，
		虽然理论上每个片元应该有着不同的深度值，但事实却被认为深度值相同。
		假设深度像素的值为10，则其周围可能9.8 ， 10.2 这样实际深度的片段，
		那么9.8的比较后就被点亮，而10.2的则不会。
		如此下来，就会形成交替黑线，

		解决的办法就是：
			对表面的深度/深度贴图的值应用一个偏移，即（容忍小于贴图深度某一定值的实际深度）
			float bias = 0.005;
			float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
			如上，即将实际得到的深度移动一段距离，让其能抵消掉失真的影响。

	!!!
		选用正确的偏移数值，在不同的场景中需要一些像这样的轻微调校，但大多情况下，实际上就是增加偏移量直到所有失真都被移除的问题。
		（如果便宜过大，将没有影子：实际深度偏移之后远小于最近深度。）
*/

/*悬浮:Peter panning
	使用阴影偏移的一个缺点是：
		你对物体的实际深度进行了偏移。
			偏移有可能足够大，以至于可以看出阴影相对实际物体位置的偏移
		这个阴影失真叫做悬浮，因为物体看起来轻轻悬浮在表面：或者说物体与影子轻微的分离。
	我们可以使用一点小把戏来解决大多数的悬浮问题:即在渲染深度纹理的时候使用正面剔除（front face culling）：
		我们之前使用过背面剔除来减少消耗。
		
		为了修复peter游移，我们要进行正面剔除，先必须开启GL_CULL_FACE：

		glCullFace(GL_FRONT);
		RenderSceneToDepthMap();
		glCullFace(GL_BACK); // 不要忘记设回原先的culling face
*/

/*Over sampling
	另一个视觉差异是在光的可见锥体外的某些区域一律被认为是处于阴影中，不管它真的处于阴影之中。
	(当前的程序中，地板的一个边缘角即是永远处于阴暗之中。)
	这种现象发生的原因是因为：
		在光椎体外的投影坐标高于1.0f，因此采集到的深度纹理超出了[0,1]的范围。
		基于纹理的环绕方式，我们将得到不正确的深度结果，它不是基于真实的来自光源的深度值
		目前我们使用的是GL_REPEAT。

		我们宁可让所有超出深度贴图的坐标的深度范围是1.0，这样超出的坐标将永远不在阴影之中：
		我们可以储存一个边框颜色，然后把深度贴图的纹理环绕选项设置为GL_CLAMP_TO_BORDER：

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
				GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
				glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
		这样的话，无论我们何时采集深度贴图坐标范围外的区域，纹理函数将返回1.0的深度。，阴影值为0.0。

	这样的话，shadow纹理超出光锥的范围z为1.0。到这里我们解决了纹理的问题，
	接下来是实际渲染时，该部分的深度值：
		实际上，超出光的椎体远平面的部分，其深度坐标z值大于1.0，这个时候，就会被认为在阴影中（比最近距离要远）


	到这里，所以异常的阴影就被解决了！！！

	现在：
		只有在深度贴图范围以内的被投影的fragment坐标才有阴影，所以任何超出范围的都将会没有阴影。
	由于在游戏中通常这只发生在远处，就会比我们之前的那个明显的黑色区域效果更真实。
*/

/*PCF
	目前的阴影仍有很大的不足：其分辨率明显不足，当放大阴影的时候可以明显的观察到锯齿。
	因为深度纹理有一个固定的分辨率，多个片元对应于一个纹理像素。
	
	因此多个片元会从深度贴图的同一个深度值进行采样，这几个片元便得到的是同一个阴影
	，这就会产生锯齿边缘。

	可以通过增加纹理分辨率来减少锯齿，或者尝试让光锥体尽可能接近场景。

	另一种解决方案即使：PCF ： percentage-closer filtering
		比例接近过滤：常用于柔化Shadow Map边缘,
			这是一种多种不同过滤方法的组合，用于产生柔化阴影。
	思想：
			从深度纹理中多次采样，每次采集的纹理坐标稍有改变。
			对于每个单独的采样都检查其是否在阴影中。
			将所有的结果组合，取平均值，我们就能得到一个好的阴影。

		一个简单的办法：
			简单的采集周边像素并取平均。
	我们用以对纹理坐标进行偏移，确保每个新样本，来自不同的深度值。
	使用更多的样本，更改texelSize变量，你就可以增加阴影的柔和程度。

	实际上PCF还有更多的内容，以及很多技术要点需要考虑以提升柔和阴影的效果，但处于本章内容长度考虑，我们将留在以后讨论。


*/

/*	Orthographic vs projection  正交Vs投影
	在使用正交或者投影矩阵渲染深度纹理的时候有一些区别：
		正交投影矩阵并不会将场景用透视图进行变形，所有视线/光线都是平行的，这对于定向光来讲是很好的投影矩阵。
		而透视投影矩阵会给予透视对顶点进行变形，这将给出不同的结果。

	透视矩阵对于存在实际位置的光源有着更实际的意义，
	因此透视投影通常被用于点光源或聚光灯，而正交投影被用于平行光。

	另一个细微(subtle)差别：透视投影矩阵，将深度缓冲视觉化经常会得到一个几乎全白的结果（即将深度纹理呈现在画布上）。
	这是因为，在透视矩阵下深度被转换为了非线性深度值，它在近平面有着更高的分辨率。
	为了可以像使用正交投影一样合适的观察到深度值，你必须先讲过非线性深度值转变为线性的，我们在深度测试教程中已经讨论过。

*/

/*
--函数------------------------------
*/
void framebuffer_size_callback(GLFWwindow * window, int width, int height) { glViewport(0, 0, width, height);	return; }
void mouse_callback(GLFWwindow* window, double xPos, double yPos);
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
void ProcessInput(GLFWwindow * window, float deltaTime);

void renderScene(const Shader &shader);
unsigned int loadTexture(char const *path, unsigned int Tex_Wrap_Para = GL_REPEAT);
//Depth_Map
void Depth_Map(GLFWwindow * window);
/*
--变量------------------------------
*/
const int SCR_WIDTH = 1280;
const int SCR_HEIGHT = 720;

glm::vec3 cameraPos(0.0f, 0.0f, 3.0f);
glm::vec3 WorldUp(0.0f, 1.0f, 0.0f);

float yaw = -90.0f, pitch = 0.0f;
bool openEffect = true;
bool functoinExchange = true;
int lastStatus;
int Exchange_lastStatus;
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
	Depth_Map(window);

	glfwTerminate();

}

/*
鼠标向左移动，x值减小；向右移动，x值增大
鼠标向上移动，y值减小；向下移动，y值增大
*/
bool firstMouse = true;
static float lastX = (float)SCR_WIDTH / 2.0 , lastY = (float)SCR_HEIGHT / 2.0;
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
			//glDisable(GL_MULTISAMPLE);
		}
		else {
			openEffect = true;
			//glEnable(GL_MULTISAMPLE);
		}
	}

	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE && Exchange_lastStatus == GLFW_PRESS) {
		//使用B来控制切换着色方案
		if (functoinExchange) {
			functoinExchange = false;
		}
		else {
			functoinExchange = true;
		}
	}
	lastStatus = glfwGetKey(window, GLFW_KEY_SPACE);
	Exchange_lastStatus = glfwGetKey(window, GLFW_KEY_B);
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

float planeVertices[] = {
	// x-z 平面
	// positions          // texture Coords		//Normal
	-8.0f, -0.5f, -8.0f,	0.0f, 8.0f,			0.0f,  1.0f, 0.0f,
	8.0f, -0.5f,  8.0f,		8.0f, 0.0f,			0.0f,  1.0f, 0.0f,
	8.0f, -0.5f, -8.0f,		8.0f, 8.0f,			0.0f,  1.0f, 0.0f,

	-8.0f, -0.5f, -8.0f,	0.0f, 8.0f,			0.0f,  1.0f, 0.0f,
	-8.0f, -0.5f,  8.0f,	0.0f, 0.0f,			0.0f,  1.0f, 0.0f,
	8.0f, -0.5f,  8.0f,		8.0f, 0.0f,			0.0f,  1.0f, 0.0f,

};

float canvasVertices[] = {
	// positions        
	-1.0f,	-1.0f,	0.0f,	0.0f, 1.0f,
	1.0f,	-1.0f,	0.0f,	1.0f, 0.0f,
	1.0f,	1.0f,	0.0f,	1.0f, 1.0f,
	1.0f,	1.0f,	0.0f,	0.0f, 1.0f,
	-1.0f,  1.0f, 	0.0f,	0.0f, 0.0f,
	-1.0f, -1.0f,  	0.0f,	1.0f, 0.0f
};							

unsigned int planeVAO, planeVBO;
unsigned int cubeVAO, cubeVBO;
unsigned int canvasVAO, canvasVBO;
unsigned int wallTexture;
unsigned int containerTexture;
void Depth_Map(GLFWwindow * window) {
	glEnable(GL_DEPTH_TEST);
	
	Shader shader("Shader5-3.vertex", "Shader5-3.fragment");
	Shader depthShader("Shader5-3.vertex_directLight" , "Shader5-3.fragment_directLight");
	Shader canvasShader("Shader5-3.vertex_canvas", "Shader5-3.fragment_canvas");

	wallTexture = loadTexture("wall.jpg", false);
	containerTexture = loadTexture("container.jpg", false);
	/*
	首先生成一个depth map， 深度映射是一个深度纹理，基于光源的透视试图渲染
	这里需要使用到frameBuffer
	*/
	unsigned int depthMapFBO;
	glGenFramebuffers(1,&depthMapFBO);
	//creating a 2D texture that we'll use as the framebuffer's depth buffer
	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

	unsigned int depthMapTexture;
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D , depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);//设置为深度附件
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//这样的环绕方式会导致在光源可视锥体之外的区域均在Shadow中，
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//我们选择GL_CLAMP_TO_BORDER选项，我们还需要指定一个边缘的颜色------纹理节中有说明
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	//glBindTexture(GL_TEXTURE_2D , 0);

	//生成的深度映射不应该太复杂，因为我们只想要深度信息！这里我们指定纹理格式为GL_DEPTH_COMPONENT。

	//接下来将贴图纹理附加到framebuffer之上。 注意不需要color buffer。
	//因为OpenGL下，一个帧缓冲区对象如果没有颜色缓冲区是不完整的，故这里显式指定不进行颜色数据的渲染。
	glBindFramebuffer(GL_FRAMEBUFFER , depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);//绑定深度信息
	glDrawBuffer(GL_NONE);//无绘制缓冲区
	glReadBuffer(GL_NONE);//无读取缓冲区
	glBindFramebuffer(GL_FRAMEBUFFER , 0);
	canvasShader.use();
	canvasShader.setInt("depthMap",0);

	//VAO , VBO
	//canvas

	glGenVertexArrays(1, &canvasVAO);
	glGenBuffers(1, &canvasVBO);
	glBindVertexArray(canvasVAO);
	glBindBuffer(GL_ARRAY_BUFFER, canvasVAO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(canvasVertices), &canvasVertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
	glBindVertexArray(0);

	//plane
	
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(5 * sizeof(float)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	//cube
	
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);
	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(5 * sizeof(float)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//light
	glm::vec3 lightPos(-2.0f, 4.0f, -1.0f);
	shader.use();
	shader.setVec3("directLight.direction", -lightPos);
	shader.setVec3("directLight.ambient", glm::vec3(0.2f, 0.2f, 0.2f));
	shader.setVec3("directLight.diffuse", glm::vec3(0.3f,0.3f,0.3f));
	shader.setVec3("directLight.specular", glm::vec3(0.1f,0.1f,0.1f));

	//material 即 纹理贴图
	shader.setInt("material.diffuse", 0);
	shader.setInt("material.specular", 1);
	shader.setInt("shadowMap", 2);
	shader.setFloat1f("material.shininess", 16.0f);

	float curTime{}, lastTime{};
	curTime = lastTime = glfwGetTime();
	while (!glfwWindowShouldClose(window)) {
		//Process Input
		curTime = glfwGetTime();
		ProcessInput(window, curTime - lastTime);
		lastTime = curTime;
		
		glm::mat4 model ;
		glm::mat4 view ;
		glm::mat4 projection ;
		glm::mat4 lightSpaceMatrix;

		//Draw

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		//first step --- 生成深度映射，光的视角
		// 1. render depth of scene to texture (from light's perspective)
		// --------------------------------------------------------------
		glCullFace(GL_FRONT);
		
		model = glm::translate(model , lightPos);	//光的位置
		view = glm::lookAt(lightPos,glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));	//光的视角
		float near_plane = 1.0f, far_plane = 7.5f;
		projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);//正交视图
		
		depthShader.use();
		depthShader.setMartix("view", view);
		depthShader.setMartix("projection", projection);
		lightSpaceMatrix = projection * view;

		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);//清空深度缓冲区，没有颜色缓冲区

		//RenderScene
		
		renderScene(depthShader);
		glCullFace(GL_BACK); // 不要忘记设回原先的culling face

		//second step --- 进行场景绘制：摄像机视角
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//ConfigureShaderAndMatrices()
		projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
		view = camera.GetViewMatrix();
		//Canvas 渲染
		//canvasShader.use();
		//canvasShader.setFloat1f("near_plane", near_plane);
		//canvasShader.setFloat1f("far_plane", far_plane);
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		////RenderScene
		//glBindVertexArray(canvasVAO);
		//glDrawArrays(GL_TRIANGLES,0,6);
		
		shader.use();
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		shader.setMartix("view", view);
		shader.setMartix("projection", projection);
		shader.setMartix("normalMatrix",glm::transpose(glm::inverse(model)));
		shader.setMartix("lightSpaceMatrix", lightSpaceMatrix);
		renderScene(shader);
		
		//swap and poll
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

void renderScene(const Shader &shader)
{

	// floor
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, wallTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, wallTexture);
	glm::mat4 model;
	shader.setMartix("model", model);
	glBindVertexArray(planeVAO);
	
	glDrawArrays(GL_TRIANGLES, 0, 6);
	// cubes
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, containerTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, containerTexture);
	glBindVertexArray(cubeVAO);
	model = glm::mat4();
	model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0));
	model = glm::scale(model, glm::vec3(0.5f));
	shader.setMartix("model", model);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	model = glm::mat4();
	model = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0));
	//model = glm::scale(model, glm::vec3(0.5f));
	shader.setMartix("model", model);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	model = glm::mat4();
	model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 2.0));
	model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
	model = glm::scale(model, glm::vec3(0.25));
	shader.setMartix("model", model);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
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
