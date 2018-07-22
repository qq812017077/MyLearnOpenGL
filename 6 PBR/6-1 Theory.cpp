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

/*Theory  理论
	PBR（或者更通俗得称为physically based rendering，基于物理渲染），是一个一定程度上基于相同的非常接近
	现实世界的基础理论的技术集合。	PBR目的是为了以更符合物理的方式来模拟光照。通常它看起来比原来的光照算法（冯氏光照、Blinn-Phong）
	更贴近现实。不仅看起来更好，而且更贴近实际物理。
	
	因此我们（尤其是美术师们）可以直接以物理参数为依据来编写表面材质，而不必依靠粗劣的修改与调整来让光照效果看上去正常。
	基于物理参数创作材质的一个巨大优势是这些材质看起来总是正常的而无论光照光照状况，而在非PBR的渲染管线当中有些东西就不会那么真实了。

	尽管如此(onetheless)PBR仍然只是贴近现实（基于物理管线）这就是为什么不被称为物理着色而是基于物理着色。
	
	
	一个PBR光照模型被认为是基于物理，其需要满足以下三个条件：
		1.基于微平面（microfacet）表面模型。
		2.能量守恒（energy conserving）
		3. 使用基于物理的BRDF


	该教程的PBR为最先由迪士尼(Disney)提出探讨并被Epic Games首先应用于实时渲染的PBR方案。
		他们基于金属工作流（metallic workflow）的方案有着完善的记录，并被广泛应用于流行引擎，效果非常惊人。
*/

/*The microfacet model 微平面模型
	所有的PBR技术都基于微平面理论，
		该理论描述了在微观尺度下，任何能被微小的完全反射镜面描述的平面称为微平面。
	
	根据平面粗糙程度的不同，这些细小镜面的取向排列可以相当不一致：
		一个平面越是粗糙，平面上的每个微平面的排列就越是混乱。这些微小镜面的排列效果即：
	当特别考虑镜面光照时，入射光线在粗糙平面上更可能朝着不同的方向发散，从而导致一个更宽广的镜面反射。
	相反，在一个更平滑的平面光线则更可能粗略朝着同一个方向发散，给我们一个更小，更尖锐的反射效果。

	在微观层面上，没有完全光滑的表面，然而由于这些微平面已经微小到无法逐像素的继续对其进行区分。
	因此我们只有假设一个粗糙度(Roughness)参数，然后用统计学的方法来概略的估算微平面的粗糙程度。

	根据一个表面的粗糙度，我们可以计算出某个向量的方向与微平面平均取向方向一致的概率。
	这个向量就是半程向量，位于光线向量l和视线向量v之间的中间向量
	我们曾经在之前的高级光照教程中谈到过中间向量，它的计算方法如下：
			h = ( l + v) / | l + v |  或者 h = ( normalize(l) + normalize(v) ) / 2
		
		微平面的取向方向与中间向量的方向越是一致，镜面反射的效果就越是强烈越是锐利。
	然后再加上一个介于0到1之间的粗糙度参数，这样我们就能概略的估算微平面的取向情况了：
	(较高的粗糙度值显示出来的镜面反射的轮廓要更大一些。与之相反地，较小的粗糙值显示出的镜面反射轮廓则更小更锐利。)

*/

/* 能量守恒 Energy Conservation
	微平面近似法使用了能量守恒的形式：
		发出的光线能量永远不会高于入射的光线能量（发光面除外）。
	随着粗糙度的上升镜面反射区域的会增加，但是镜面反射的亮度却会下降。
	如果在每个像素的镜面光照强度相同而不论镜面形状的大小，这样粗糙的平面会反射更多的能量，从而违背了
	能量守恒定律。这就是为什么我们看到在光滑平面的镜面反射更强，而粗糙平面的更弱
	
	为了遵守能量守恒定律，我们需要对漫反射和镜面光照做一个清晰的区分。在光照向平面的时候，它被分成了
	折射部分和反射部分。
		反射部分：即并进入平面内而被反射，这即是镜面光照
		折射部分：剩下的光进入平面而被吸收。这即是漫反射光。	？！！ 漫反射是折射光？

	这里有一些细微差别，因为折射光在接触大平面时不会被立刻吸收，
	通过物理学我们可以得知，光线实际上可以被认为是一束没有耗尽就不停向前运动的能量，
	而光束是通过碰撞的方式来消耗能量。每一种材质都包含了微小的颗粒，它们和光线碰撞。
	这些颗粒通过每次碰撞吸收部分或全部光能，他们被转换为热能。

	通常，不是所有的光照都会被吸收，光会在和其他微粒碰撞的区域以随机方向发散，直到能量耗尽（depleted）或者离开平面。
	而离开平面的光线将会协助形成该表面的观察（漫反射）颜色。在PBR中，我蛮进行了简化，我们假设对平面上的每一点
	所有的折射光都会被完全吸收并分散在一个很小的平面内，而忽视了那些离开平面一定距离的光线。

	一些次表面散射(Subsurface Scattering)的着色器技术将其考虑到了其中，这显著的提高了一些材质的视觉质量，如皮肤、大理石或蜡烛
	不过以性能作为代价。

	对于金属(Metallic)表面，当讨论到反射与折射的时候还有一个细节需要注意：
		金属表面对光的反应与非金属材料（也称为电介质(Dielectrics)）表面相比是不同的。
		金属材质遵循着同样的反射和折射原则，但是所有的折射光都被直接吸收而没有发散，仅留下反射或镜面光。
	金属表面不显示出漫反射。
		因为这个明显的区别，金属和电介质在PBR管道中被以不同的方式对待，

	反射和折射的这个区别给了我们另一条能量守恒的经验总结：它们是互相排斥的（mutually exclusive）
		无论何种光线，其被材质表面所反射的能量将无法再被材质吸收。
		因此，如折射光这样的剩下的进入表面之中的能量正好就是除反射以外的剩下的能量。

	我们按照能量守恒的关系，首先计算镜面反射部分，它的值等于入射光线被反射的能量所占的百分比。
	然后折射光部分就可以直接由镜面反射部分计算得出：
		float kS = calculateSpecularComponent(...); // 反射/镜面 部分
		float kD = 1.0 - ks;                        // 折射/漫反射 部分
	通过这种方式，我们就能获取入射光的反射部分和折射部分，并且遵循了能量守恒。
		这样镜面光和漫反射光的综合就不会超出1.0。
		
*/

/*The reflectance equation	反射方程
	在这里我们引入了一种被称为渲染方程(Render Equation)的东西。
		它是某些聪明绝顶人所构想出来的一个精妙（elaborate）的方程式，它是当前我们所用来模拟光照视觉的最好的模型。
	
	PBR坚持遵循一个渲染方程的专门版本，叫做反射方程（Reflectance equation）


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
void renderCube(Shader shader, std::vector<glm::vec3> cubePositions, std::vector<glm::vec3> cubeRotateAxis) {
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