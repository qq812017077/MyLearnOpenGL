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

// 视差贴图造成了一种平移表面的错觉。

/*
	视差贴图是提升场景细节非常好的技术，但是使用的时候还是要考虑到它会带来一点不自然。
	大多数时候视差贴图用在地面和墙壁表面，这种情况下查明表面的轮廓并不容易，同时观察角度往往趋向于垂直于表面。
	这样视差贴图的不自然也就很难能被注意到了，对于提升物体的细节可以祈祷难以置信的效果。
*/

/*
	Parallax Mapping	视差贴图
	视差贴图是一种和法线贴图相似的技术，但是基于不同的原理。
	和法线贴图相似视差贴图能够极大提升表面细节，使之具有深度感。
	同时也是一个错觉，视差贴图在表达深度感上更有优势，其和法线贴图共同创作令人难以置信的效果。

	视差贴图和光照无关，
	视差贴图属于位移贴图(Displacement Mapping)技术的一种，其根据储存在纹理中的几何信息对顶点进行位移或偏移
	一种实现的方式是比如有1000个顶点，根据纹理中的数据对平面特定区域的顶点的高度进行位移。
	这样的一张纹理其包含了每个像素的高度值，称为视差贴图。	

	当平面上的每个顶点基于高度贴图采样的高度值进行位移，一个平坦的平面就根据材质的几何属性
	变换成凹凸不平的表面。


	displacing顶点的一个问题是：一个平面需要包含大量的三角形以获得一个真实的位移感。否则变换看起来是很死板。
	如果每一个平面都都包含超过1000个顶点那么这会迅速的变得不可计算。那么我们要如何使用有限的点来实现相似的效果。
	视差贴图可以帮我们实现这样的效果。

	思想：
		视差贴图是通过修改纹理坐标，使一个fragment的表面看起来比实际的更高或者更低。
		   _____________B____
		  /			   /	 \
		 /			  /		  \
		/___________A__________\
	如图，
		这里的突出的面代表的是heightmap的值，即高度图中的值，作为砖表面的几何表示，
		
		下面的平面则表示实际的平面。
		假设存在BA这样的一个观察视角
		也就是说，
			如果存在实际的偏移，则观察者应该看到的是B点。
			然而，我们的平面实际上没有偏移，所以我们看到的是A点。
	
	因此，视差贴图的目的是：
		对纹理坐标进行偏移，
		使得在A位置上的fragment不再使用点A的纹理坐标，而是使用点B的。
	随后我们用点B的纹理坐标采样，观察者就像看到了点B一样。

	问题：
		如何从点A获得点B的？
	解决：
		视差贴图尝试解决这个问题，通过缩放沿frag到view方向的向量V到片段A的高。
	所以这个向量的长度即等于heightmap在片段点A的采集值（也即A的高度）：
		
		这里另缩放后的向量为P，所以 P 和 V 同向
	
		随后我们获取P与平面对齐的坐标作为纹理坐标的偏移量，这是可行的因为向量P是通过使用helghtMap
	的高度值来计算的，所以片段的高度越高，它位移的效果越明显。

	需要注意的是：这里的P点是粗略获取的，并不是完全对应的B点（即使在上面的case中，都不在一处）
	注意我们只能想办法获取相近的平面坐标来获取纹理坐标，而不能反过来！

	问题1：
		这个方法在大多数时候工作良好，但是如果表面存在一个快速的改变。结果可能看起来不那么真实。
	问题2：
		另一个问题是，当表面被任意旋转以后很难指出从P获取哪一个坐标。

	因此这里使用的视差贴图仍然是在切线空间下使用的，

	通过将V，也即fragment-view方向向量转换到切线空间下，
	经变换的P¯向量的x和y元素将于表面的切线和副切线向量对齐。
	由于切线和副切线向量与表面纹理坐标的方向相同，
	我们可以用P¯的x和y元素作为纹理坐标的偏移量，这样就不用考虑表面的方向了。


	在实际应用中，我们可是使用深度贴图，而非高度贴图，
	因为使用反色高度贴图（也叫深度贴图）去模拟深度比模拟高度更容易。即图片中存储的是深度不是高度，注意深度是越小越近（高）
	该处理有两种方法：
		1.	通过在着色器中用1.0减去采样得到的高度贴图中的值来取得深度值，而不再是高度值，
		2.	在图片编辑软件中把这个纹理进行反色操作，就像这里的那个深度贴图所做的一样。

	视差贴图在片段着色器中应用，因为视差效果对于每一个三角形表面都不同。
	在片段着色器中，我们需要
		计算片段到视点的向量，fragment-view向量：我们需要片段和视点在切线空间中的坐标。

	到目前为止的运行效果还不错，但是仍然存在问题：
	当从一个角度看过去的时候，会有一些问题产生（和法线贴图相似），陡峭的地方会产生不正确的结果，

	陡峭视差贴图可以减轻这个问题
*/

/*Steep Parallax Mapping	陡峭视差贴图
	陡峭视差贴图是对视差贴图的扩展，它们采用同样的原理，但是不是使用一个样本而是多个样本来确定向量P到B。
	即使在陡峭的高度变化下，多采样也可以提高技术的精确度。

	基本思想：
		它将总深度范围分布到同一个深度/高度的多个层中。对于每一层我们都沿着方向P采集偏移的纹理坐标。
	直到我们发现一个采集值低于当前层的深度值。

		我们从顶部向下遍历深度层，对于每一层我们比较我们比较它的深度值和保存在深度图中的深度值，
	若
		层的深度值  < 深度图的深度值：意味着向量P在这一层的部分在表面的上面。
	我们将继续搜寻，知道我们找到
		一个层的深度值 < 该处深度层中的值（P在该层的部分低于了表面）：
		这意味着偏移后的深度值在该层和前一层之间。
	然后就可以选择该层的P作为偏移值。
		算法见fragment着色器中的 Steep_ParallaxMapping函数

	继续优化：
		当垂直看一个表面的时候纹理时位移比以一定角度看时的小。
	我们可以在垂直看时使用更少的样本，以一定角度看时增加样本数量：

	const float minLayers = 8.0;
	const float maxLayers = 32.0;
	float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));

	这里我们获取观察方向和z轴正向的点积，使用它的结果根据我们看向表面的角度调整样本数量
	（注意正z方向等于切线空间中的表面的法线）。如果我们所看的方向平行于表面，我们就是用32层。
	
	陡峭视差贴图的问题在于：
		因为其基于有限数量的采样，我们会获得锯齿样效果，图层之间会有明显的断层。
		（通过增加采样可以减少这个问题，但是同时会增加消耗）

	有一些方法可以处理这个问题，不使用低于表面的第一个位置，而是对两个最近的深度层的位置进行线性插值以获取更匹配的位置。

	有两个比较流行的方法称为： Relief Parallax Mapping 和 Parallax Occlusion Mapping。
	Relief Parallax Mapping的精度更高，但是Parallax Occlusion Mapping的性能消耗更少。

	相对来讲，Parallax Occlusion Mapping的效率更好，故更常用。
*/

/*Parallax Occlusion Mapping	视差遮蔽映射
		视差遮蔽映射和陡峭视差映射的原理相同，但是其不是使用在触碰后的第一个深度值，
	而是选择对触碰前后的深度值进行线性插值。
		线性插值的权重设置基于表面高度到这些层的深度层值距离。

	其精度相对陡峭更高，但是仍然是近视值。

*/

/*
--函数------------------------------
*/
void framebuffer_size_callback(GLFWwindow * window, int width, int height) { glViewport(0, 0, width, height);	return; }
void mouse_callback(GLFWwindow* window, double xPos, double yPos);
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
void ProcessInput(GLFWwindow * window, float deltaTime);


unsigned int loadTexture(char const *path, unsigned int Tex_Wrap_Para = GL_REPEAT);
//Brick Wall Normal Test
void ParallaxTest(GLFWwindow * window);

void TangentSpace(GLFWwindow * window);
/*
--变量------------------------------
*/
const int SCR_WIDTH = 800;
const int SCR_HEIGHT = 600;

glm::vec3 cameraPos(-1.0f, 0.0f, 2.0f);
glm::vec3 WorldUp(0.0f, 1.0f, 0.0f);

float yaw = -90.0f, pitch = 0.0f;
float height_scale = 0.075f;
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
	ParallaxTest(window);
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
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
	{
		if (height_scale > 0.0f)
			height_scale -= 0.0005f;
		else
			height_scale = 0.0f;
	}
	else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
	{
		if (height_scale < 1.0f)
			height_scale += 0.0005f;
		else
			height_scale = 1.0f;
	}
	std::cout << "height_scale:" << height_scale << std::endl;
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



void ParallaxTest(GLFWwindow * window) {
	glEnable(GL_DEPTH_TEST);
	//Shader
	Shader light_Shader("Shader.vertex_lamp", "Shader.fragment_lamp");
	Shader shader("Shader5-5.vertex", "Shader5-5.fragment");
	//Texture
	/*unsigned int diffuseTexture =	loadTexture("wood.png");
	unsigned int normalTexture =	loadTexture("toy_box_normal.png");
	unsigned int depthTexture =		loadTexture("toy_box_disp.png");*/
	unsigned int diffuseTexture = loadTexture("bricks2.jpg");
	unsigned int normalTexture = loadTexture("bricks2_normal.jpg");
	unsigned int depthTexture = loadTexture("bricks2_disp.jpg");

	//切线空间的处理
	// positions----平面的四个顶点				   1(0,1)__________4(1,1)
	glm::vec3 pos1(-2.0, 2.0, 0.0);		//左上		|		   |
	glm::vec3 pos2(-2.0, -2.0, 0.0);		//左下		|		   |
	glm::vec3 pos3(2.0, -2.0, 0.0);		//右下		|		   |
	glm::vec3 pos4(2.0, 2.0, 0.0);		//右上	    |__________|
										// texture coordinates								2(0,0)	   3(1,0)
	glm::vec2 uv1(0.0, 2.0);
	glm::vec2 uv2(0.0, 0.0);
	glm::vec2 uv3(2.0, 0.0);
	glm::vec2 uv4(2.0, 2.0);
	// normal vector
	glm::vec3 normal(0.0, 0.0, 1.0);

	//step1:第一个三角形的边 和 ΔUV
	glm::vec3 edge1 = pos2 - pos1;	//1 -> 2
	glm::vec3 edge2 = pos3 - pos1;	//1 -> 3
	glm::vec2 deltaUV1 = uv2 - uv1;
	glm::vec2 deltaUV2 = uv3 - uv1;
	//注意这里的边是有方向的，但是方向如何关系不大，只要Delta与之对应就好了。
	/*公式：
	| Tx  Ty  Tz | =	| ΔU1 ΔV1 |-1	| E1x  E1y  E1z |
	| Bx  By  Bz |		| ΔU2 ΔV2 |	| E2x  E2y  E2z |
	*/
	//计算方法1：

	float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
	glm::vec3 tangent1;
	tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
	tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
	tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
	tangent1 = glm::normalize(tangent1);

	glm::vec3 bitangent1;
	bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
	bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
	bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
	bitangent1 = glm::normalize(bitangent1);

	//计算方法2
	/*glm::mat2x2 uv(deltaUV1, deltaUV2);
	glm::mat2x3 edge(edge1, edge2);
	glm::mat2x3 result = edge * glm::inverse(uv) ;
	tangent = result[0];
	bitangent = result[1];*/

	//step2:第二个三角形的边 和 ΔUV ---实际上，这里的不进行计算也行，因为整个平面的切线和
	edge1 = pos3 - pos1;	//1 -> 3
	edge2 = pos1 - pos4;	//4 -> 1
	deltaUV1 = uv3 - uv1;
	deltaUV2 = uv1 - uv4;

	f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
	glm::vec3 tangent2;
	tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
	tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
	tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
	tangent2 = glm::normalize(tangent1);

	glm::vec3 bitangent2;
	bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
	bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
	bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
	bitangent2 = glm::normalize(bitangent1);

	GLfloat quadVertices[] = {
		// Positions            // normal						// TexCoords  // Tangent                         // Bitangent
		pos1.x, pos1.y, pos1.z, normal.x, normal.y, normal.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
		pos2.x, pos2.y, pos2.z, normal.x, normal.y, normal.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
		pos3.x, pos3.y, pos3.z, normal.x, normal.y, normal.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

		pos1.x, pos1.y, pos1.z, normal.x, normal.y, normal.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
		pos3.x, pos3.y, pos3.z, normal.x, normal.y, normal.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
		pos4.x, pos4.y, pos4.z, normal.x, normal.y, normal.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
	};
	//VAO , VBO
	//plane
	unsigned int planeVAO, planeVBO;
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, 14 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void *)(6 * sizeof(float)));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void *)(8 * sizeof(float)));
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void *)(11 * sizeof(float)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);


	//light
	glm::vec3 lightPosition(0.0f, 0.0f, 3.0f);

	unsigned int lightVAO, lightVBO;
	glGenVertexArrays(1, &lightVAO);
	glGenBuffers(1, &lightVBO);
	glBindVertexArray(lightVAO);
	glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, 8 * sizeof(float), (void *)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//Shader参数设置
	shader.use();

	shader.setVec3("light.ambient", glm::vec3(0.2f, 0.2f, 0.2f));
	shader.setVec3("light.diffuse", glm::vec3(0.8f, 0.8f, 0.8f));
	shader.setVec3("light.specular", glm::vec3(0.6f, 0.6f, 0.6f));
	shader.setFloat1f("light.constant", 1.0f);
	shader.setFloat1f("light.linear", 0.09f);
	shader.setFloat1f("light.quadratic", 0.032f);
	shader.setInt("material.diffuse", 0);
	shader.setInt("material.specular", 1);
	shader.setInt("material.normalMap", 2);
	shader.setInt("material.depthMap", 3);
	shader.setFloat1f("material.shininess", 16.0f);
	
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, diffuseTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, diffuseTexture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, normalTexture);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	float curTime{}, lastTime{};
	curTime = lastTime = glfwGetTime();
	while (!glfwWindowShouldClose(window)) {
		//Process Input
		curTime = glfwGetTime();
		ProcessInput(window, curTime - lastTime);
		lastTime = curTime;
		//Draw
		//lightPosition.z = (sin(glfwGetTime()) + 1.0f) / 2.0 + 0.5f;
		glm::mat4 model;
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//plane
		shader.use();
		model = glm::mat4();
		//model = glm::rotate(model, glm::radians(60.0f * (float)sin(glfwGetTime())), glm::vec3(1.0f, 0.0f, 0.0f));
		//model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));
		shader.setVec3("light.position", lightPosition);
		shader.setMartix("model", model);
		shader.setMartix("normalMatrix", glm::inverse(glm::transpose(model)));
		shader.setMartix("view", view);
		shader.setMartix("projection", projection);
		shader.setVec3("viewerPos", camera.Position);
		shader.setBool("openEffect", openEffect);
		shader.setFloat1f("height_scale", height_scale);
		glBindVertexArray(planeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		//light
		light_Shader.use();
		model = glm::mat4();
		model = glm::translate(model, lightPosition);
		model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
		light_Shader.setMartix("model", model);
		light_Shader.setMartix("view", view);
		light_Shader.setMartix("projection", projection);
		glBindVertexArray(lightVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		//swap and poll
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
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
