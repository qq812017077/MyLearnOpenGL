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
#include "Model.h"  //Offical_
/*
	天空盒！！！
	CubeMaps
	立方体贴图：
		立方体贴图实际上是包含了6个独立2D贴图的一个贴图，每一张贴图都在一个立方体的一面。
		实质上是一个贴图立方体。

	what's the point？ 意义在哪？
		立方体贴图有着非常有用的属性：使用方向向量进行采集和索引。
			假想：一个单位立方体，方向向量的起点在立方体的中心。
		***
			方向向量的大小无关紧要。一旦提供了方向，
			OpenGL就会获取方向向量触碰到立方体表面上的相应的纹理像素（texel），
			这样就返回了正确的纹理采样值。
		***
	
	假设我们有一个立方体，其上附加了一个立方体贴图。
		则采集的方向向量和立方体的顶点位置会非常相似。
		当立方体位于原点的时候，我们可以直接使用立方体顶点的实际位置向量作为方向向量，即对纹理进行采样。
		然后我们就可以得到所以纹理坐标，就和立方体上的顶点位置一样，结果即是纹理坐标。
		继而获取正确的立方贴图各个面的纹理信息。

	立方体贴图有6个面，OpenGL就提供了6个不同的纹理目标，来应对立方体贴图的各个面：
		纹理目标（Texture target）			   方位
		GL_TEXTURE_CUBE_MAP_POSITIVE_X			右
		GL_TEXTURE_CUBE_MAP_NEGATIVE_X			左
		GL_TEXTURE_CUBE_MAP_POSITIVE_Y			上
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Y			下
		GL_TEXTURE_CUBE_MAP_POSITIVE_Z			后
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Z			前
		和很多OpenGL其他枚举一样，对应的int值都是连续增加的：
		GL_TEXTURE_CUBE_MAP_POSITIVE_X + 1 -> GL_TEXTURE_CUBE_MAP_NEGATIVE_X

*/

/*Skybox
	天空盒(Skybox)是一个包裹整个场景的立方体，它由6个图像构成一个环绕的环境，
	给玩家一种他所在的场景比实际的要大得多的幻觉。
	希尔
	天空盒的两种使用方法：
		1.在每次渲染的时候，先关闭深度值写入，然后首先渲染天空盒，
				这样会绘制出盒子，但是没有相关的深度信息，
				然后我们打开深度值写入再绘制盒子（非背景的物体），
				这样子盒子一定会写入到深度缓冲，所以保证了盒子的绘制。
			（没有比较，便没有丢弃的说法！）
		2.第一种方法存在资源浪费，
			我们希望后面绘制天空盒子，
			（这样子的话，深度缓冲就会填充满所有物体的深度值了，我们只需要
			在提前深度测试通过的地方渲染天空盒的片段就可以了，很大程度上减
			少了片段着色器的调用）
			但是因为盒子只有 1x1x1,非常小，因此我们需要欺骗深度缓冲区相信，盒子有着最大的深度值。
*/

/*Environment mapping 
	环境映射
	通过使用环境的立方体贴图，我们可以给物体反射和折射的属性。
	这样使用环境立方体贴图的技术叫做环境映射(Environment Mapping)，其中最流行的两个是反射(Reflection)和折射(Refraction)。

	Reflection：
		反射表现为一个物体的某个部分反射了其周围的环境。
			即根据观察者的视角，物体的颜色或多或少等于它的环境。
	Refraction
		折射表现为：
			光线通过了介质，但是传播的角度发生了偏移，直观来讲即
			通过介质的光线不再为一条直线。
		折射率：refractive index
			Material	Refractive index
				Air			1.00
			   Water		1.33
				Ice			1.309
				Glass		1.52
			   Diamond		2.42

*/

unsigned int loadTexture(char const *path, unsigned int Tex_Wrap_Para);
unsigned int loadCubeMap(std::vector<std::string> texture_faces);
void framebuffer_size_callback(GLFWwindow * window, int width, int height) { glViewport(0, 0, width, height); return; }
void mouse_callback(GLFWwindow* window, double xPos, double yPos);
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
void ProcessInput(GLFWwindow * window, float deltaTime);

unsigned int loadTexture(char const * path);
unsigned int loadVertexToVBO(float const * data);

const int SCR_WIDTH = 800;
const int SCR_HEIGHT = 600;

void CubeMap_Test(GLFWwindow* window);


glm::vec3 cameraPos(0.0f, 0.0f, 0.0f);
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
	CubeMap_Test(window);
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
	// positions          // texture Coords (note we set these higher than 1 (together with GL_REPEAT as texture wrapping mode). this will cause the floor texture to repeat)
	5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
	-5.0f, -0.5f,  5.0f,  0.0f, 0.0f,
	-5.0f, -0.5f, -5.0f,  0.0f, 2.0f,

	5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
	-5.0f, -0.5f, -5.0f,  0.0f, 2.0f,
	5.0f, -0.5f, -5.0f,  2.0f, 2.0f
};
float skyboxVertices[] = {
	//背面
	// positions          
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	//左侧面
	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,
	//右侧面
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	//正面
	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,
	//顶面
	-1.0f,  1.0f, -1.0f,
	1.0f,  1.0f, -1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,
	//底面
	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	1.0f, -1.0f,  1.0f
};

void CubeMap_Test(GLFWwindow* window) {
	//使能深度测试
	glEnable(GL_DEPTH_TEST);//深度测试

	Shader shader("Shader4-6.vertex", "Shader4-6.fragment");
	Shader skyboxShader("Shader4-6.vertex_skybox", "Shader4-6.fragment_skybox");

	//加载模型
	char model_path[] = "E:/C++/OpenGL_Course/OpenGL_Course/Model/nanosuit_reflrection/nanosuit.obj";
	Model ourModel(model_path);
	std::cout << "Model加载完成，总共有"<< ourModel.GetMeshs().size()<<"个Mesh" << std::endl;
	std::vector<Mesh> Meshes = ourModel.GetMeshs();
	std::vector<Mesh>::iterator it = Meshes.begin();
	int i = 0; int j = 0;
	for (std::vector<Mesh>::iterator it = Meshes.begin(); it != Meshes.end(); it++)
	{
		std::cout << "第" << i++<< "个Mesh" << std::endl;
		std::vector<Texture>::iterator texIt =  it->textures.begin();
		for (std::vector<Texture>::iterator texIt = it->textures.begin(); texIt != it->textures.end(); texIt++)
		{
			std::cout << j <<":  " << texIt->path << "   " << texIt->type<< std::endl;
			
		}
	}

										   //Texture
	unsigned int cubeTexture = loadTexture(std::string("container.jpg").c_str(), GL_REPEAT);
	unsigned int floorTexture = loadTexture(std::string("wall.jpg").c_str(), GL_REPEAT);
	
	//textures_faces 包含了所有的贴图位置。注意加载顺序：按照循环中循环中的枚举顺序进行。
	std::vector<std::string> textures_faces;
	textures_faces.push_back("SkyBox/right.jpg");
	textures_faces.push_back("SkyBox/left.jpg");
	textures_faces.push_back("SkyBox/top.jpg");
	textures_faces.push_back("SkyBox/bottom.jpg");
	textures_faces.push_back("SkyBox/front.jpg");
	textures_faces.push_back("SkyBox/back.jpg");
	
	//cubeMap
	unsigned int skyTexture = loadCubeMap(textures_faces);

	// skybox VAO VBO
	unsigned int skyVAO, skyVBO; 
	glGenVertexArrays(1,&skyVAO);
	glGenBuffers(1,&skyVBO);
	glBindVertexArray(skyVAO);
	glBindBuffer(GL_ARRAY_BUFFER,skyVBO);
	glBufferData(GL_ARRAY_BUFFER , sizeof(skyboxVertices),skyboxVertices,GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// cube VAO
	unsigned int cubeVAO, cubeVBO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);
	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
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
	shader.setInt("cubemap", 3);
	std::vector<glm::vec3> windows;
	windows.push_back(glm::vec3(-1.5f, 0.0f, -0.48f));
	windows.push_back(glm::vec3(1.5f, 0.0f, 0.51f));
	windows.push_back(glm::vec3(0.0f, 0.0f, 0.7f));
	windows.push_back(glm::vec3(-0.3f, 0.0f, -2.3f));
	windows.push_back(glm::vec3(0.5f, 0.0f, -0.6f));

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
		//DrawScence
		glm::mat4 model;
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();

		//first pass
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//cube
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyTexture);
		shader.use();
		shader.setVec3("cameraPos",camera.Position);
		model = glm::mat4();
		model = glm::scale(model,glm::vec3(0.2f,0.2f,0.2f));
		shader.setMartix("model", model);
		shader.setMartix("normalMatrix",glm::transpose(glm::inverse(model)));
		shader.setMartix("view", view);
		shader.setMartix("projection", projection);
		/*glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);*/
		ourModel.Draw(shader);

		//skybox---注意关闭深度写入，这样子才能成为背景。
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyTexture);
		glDepthFunc(GL_LEQUAL);//刷新后的缓冲区深度值为1，因此这里需要设置为当深度值小于等于1的时候都能通过。，否则背景还是写不上，
		skyboxShader.use();
		skyboxShader.setMartix("projection", projection);
		glm::mat4 skyview = glm::mat4(glm::mat3(camera.GetViewMatrix()));
		skyboxShader.setMartix("view", skyview);
		glBindVertexArray(skyVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS);
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

unsigned int loadCubeMap(std::vector<std::string> texture_faces) {
	unsigned int textureID;
	glGenTextures(1,&textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP,textureID);

	//仍然和普通的纹理一样，我们需要指定其环绕和过滤方法。
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	/*
	注意我们将三个维度的环绕方式都设置为了GL_CLAMP_TO_EDGE，是因为
	在两个面之间的纹理坐标可能并不能触及哪个面（由于硬件限制），所以通过使用GL_CLAMP_TO_EDGE
	每当我们采集边缘的时候，OpenGL将总是返回他们的边缘值。

	这里的GL_TEXTURE_WRAP_R设置了纹理的R坐标（对应了纹理的第三个维度），

	*/

	int width, height, nrChannels;
	unsigned char *data;
	for (GLuint i = 0; i < texture_faces.size(); i++)
	{
		data = stbi_load(texture_faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data) {
			glTexImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);
			stbi_image_free(data);
		}else{
			std::cout << "Cubemap texture failed to load at path: " << texture_faces[i] << std::endl;
			stbi_image_free(data);
		}
		
	}
	return textureID;
}
