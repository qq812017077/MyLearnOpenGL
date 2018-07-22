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

/*Instance
	实例化
	当要绘制一个包含大量相同模型位置不同的场景,比如草。
	（每一个草都是包含多个三角形的模型）
	大量的草地渲染，极度影响性能

	渲染大量对象代码类似代码如下：
		for(unsigned int i = 0; i < amount_of_models_to_draw; i++)
		{
			DoSomePreparations(); // bind VAO, bind textures, set uniforms etc.
			glDrawArrays(GL_TRIANGLES, 0, amount_of_vertices);
		}
		像这样绘制模型的大量实例(Instance)，你很快就会因为绘制调用过多而达到性能瓶颈：
			与绘制顶点本身相比，使用glDrawArrays或glDrawElements函数告诉GPU去绘制你的
			顶点数据会消耗更多的性能，因为OpenGL在绘制顶点数据之前需要做很多准备工作
			（比如告诉GPU该从哪个缓冲读取数据，从哪寻找顶点属性，而且这些都是在相对缓
			慢的CPU到GPU总线(CPU to GPU Bus)上进行的）。所以，即便渲染顶点非常快，命令GPU去渲染却未必。

	方法：
		实例化：只发送一次数据，告诉OpenGL绘制多次而只使用一次绘制调用。
		方法：代替使用 glDrawArrays 和 glDrawElements， 使用glDrawArraysInstanced 和 glDrawElementsInstanced

		新的函数需要额外的参数，来说明来设置想要渲染的实例数量
		将所需的数据发送给GPU一次，通过一个单独的调用来告诉GPU如何绘制所有的实例。

		NOTE:单独的调用这个函数的效果，同一个对象绘制多次，但是都在一个位置，视觉效果还是一个！

		解决：
			GLSL在顶点着色器中的内置变量：gl_InstanceID

			在使用实例化渲染调用时，gl_InstanceID会从0开始，在每个实例被渲染时递增1。
			eg：当在渲染第43个实例时， gl_InstanceID = 42

*/

/*Instanced Array
	NOTE:实例数组是一个顶点属性，或者说 顶点属性可以设置为实例数组！！
	实例数组
		一个需要注意问题是，上面我们在顶点着色器中设置了一个全局变量数组，大小为100.
		然而其设置存在上限：不过至少在1024(但是仍然可能不足！)
	所以存在另外一种替代方案：
		Instanced Array 实例数组，其被定义为顶点属性。这允许我们可以保存更多的数据。
		并且其仅在渲染新的实例时进行更新。
		
	使用顶点属性，每次运行顶点着色器都会使得GLSL提取一组对应的顶点属性，
		然而，当我们定义一个顶点属性作为实例数组时，顶点着色器仅更新每个实例的该顶点属性内容而不是每个顶点。
		
		这让我们可以使用标准顶点属性作为每个顶点的数据，
		并且使用实例数组作为每个实例的独一无二的存储数据。

*/

/*
	NOTE:
		实际上，我们声明的作为顶点属性的数据类型最大为vec4.
		但是我们仍然可以设置为mat4：
			此时，因为mat4是四个vec4.
			所以若mat4类型的属性布局在 location = 3 则实际上是 3 ， 4 ， 5 ， 6 四个位置

*/

unsigned int loadTexture(char const *path, unsigned int Tex_Wrap_Para);
unsigned int loadCubeMap(std::vector<std::string> texture_faces);
void framebuffer_size_callback(GLFWwindow * window, int width, int height) { glViewport(0, 0, width, height); return; }
void mouse_callback(GLFWwindow* window, double xPos, double yPos);
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
void ProcessInput(GLFWwindow * window, float deltaTime);

//借助gl_InstanceID的实例化
void Instanced_Quad(GLFWwindow * window);
//实例数组
void Instanced_Array_Quad(GLFWwindow * window);
//小行星带
void Asteroid_field_Uninstanced(GLFWwindow * window);
void Asteroid_field_Instanced(GLFWwindow * window);
const int SCR_WIDTH = 800;
const int SCR_HEIGHT = 600;



glm::vec3 cameraPos(0.0f, 1.0f, 55.0f);
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
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);	//关闭鼠光标
	glfwSetCursorPosCallback(window, mouse_callback);				//打开鼠标回调
	glfwSetScrollCallback(window, scroll_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		system("pause");
		return -1;
	}

	Asteroid_field_Instanced(window);
	
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

float quadVertices[] = {
	// positions     // colors
	-0.05f,  0.05f,  1.0f, 0.0f, 0.0f,
	0.05f, -0.05f,  0.0f, 1.0f, 0.0f,
	-0.05f, -0.05f,  0.0f, 0.0f, 1.0f,

	-0.05f,  0.05f,  1.0f, 0.0f, 0.0f,
	0.05f, -0.05f,  0.0f, 1.0f, 0.0f,
	0.05f,  0.05f,  0.0f, 1.0f, 1.0f
};
void Instanced_Quad(GLFWwindow * window) {
	
	Shader quad_shader("Shader4-10.vertex_quad","Shader4-10.fragment_quad");

	unsigned int VAO, VBO;
	glGenVertexArrays(1,&VAO);
	glGenBuffers(1,&VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER,VBO);
	glBufferData(GL_ARRAY_BUFFER,sizeof(quadVertices),&quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glm::vec2 translations[100];
	int index = 0;
	float offset = 0.1f;
	for (int y = -10; y < 10; y += 2)
	{
		for (int x = -10; x < 10; x += 2)
		{
			glm::vec2 translation;
			translation.x = (float)x / 10.0f + offset;
			translation.y = (float)y / 10.0f + offset;
			translations[index++] = translation;
		}
	}

	quad_shader.use();
	for (int i = 0; i < 100; ++i){
		std::stringstream ss;
		ss << "offsets[" << i << "]";
		std::cout << ss.str() << std::endl;
		quad_shader.setVec2( ss.str(), translations[i]);
	}

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
		glClearColor(0.1f, 0.1f, 0.1f, 0.1f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//points
		quad_shader.use();
		model = glm::mat4();
		quad_shader.setMartix("model", model);
		quad_shader.setMartix("view", view);
		quad_shader.setMartix("projection", projection);
		glBindVertexArray(VAO);
		glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 100);
		glBindVertexArray(0);


		//swap and poll
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

void Instanced_Array_Quad(GLFWwindow * window) {
	Shader array_shader("Shader4-10.vertex_array", "Shader4-10.fragment_array");

	glm::vec2 translations[100];
	int index = 0;
	float offset = 0.1f;
	for (int y = -10; y < 10; y += 2)
	{
		for (int x = -10; x < 10; x += 2)
		{
			glm::vec2 translation;
			translation.x = (float)x / 10.0f + offset;
			translation.y = (float)y / 10.0f + offset;
			translations[index++] = translation;
		}
	}

	unsigned int VAO, VBO , instanceVBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &instanceVBO);
	glBindVertexArray(VAO);
	//标准数据
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
	//实例数组
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 100, &translations[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glVertexAttribDivisor(2, 1);	//!!!
	/*glVertexAttribDivisor
			这个函数告诉OpenGL，什么时候更新顶点属性内容到下一个元素，
				第一个参数：需要的顶点属性。这里设置为2是指，位于位置值2的顶点属性是一个实例化数组
				第二个参数：属性除数（默认为0：即每次迭代着色器都更新顶点属性的内容）
				（设置成1，说明我们当我们开始渲染一个新的实例的时候，更新顶点属性的内容
				  而设置为2时，我们希望每2个实例更新一次属性，以此类推。）
	*/
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	

	array_shader.use();
	for (int i = 0; i < 100; ++i) {
		std::stringstream ss;
		ss << "offsets[" << i << "]";
		std::cout << ss.str() << std::endl;
		array_shader.setVec2(ss.str(), translations[i]);
	}

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
		glClearColor(0.1f, 0.1f, 0.1f, 0.1f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//points
		array_shader.use();
		model = glm::mat4();
		array_shader.setMartix("model", model);
		array_shader.setMartix("view", view);
		array_shader.setMartix("projection", projection);
		glBindVertexArray(VAO);
		glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 100);
		glBindVertexArray(0);


		//swap and poll
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

void Asteroid_field_Uninstanced(GLFWwindow * window) {
	unsigned int amount = 1000;
	glm::mat4 * modelMatrices;
	modelMatrices = new glm::mat4[amount];	//create a pointer pointing to a array of matrix

	srand(glfwGetTime()); // initialize random seed
	float radius = 50.0;
	float offset = 2.5f;

	for (unsigned int i = 0; i < amount; ++i) {
		
		//该行星带是基于xz平面的
		glm::mat4 model;
		// 1. translation: displace along circle with 'radius' in range [-offset, offset]
		float angle = (float)i / (float)amount * 360.0f; // to form a circle.
		float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset; 
		//notice: 取模运算的要求是分子分母为整数，因此放大一百倍意味着精度范围在0.01
		float x = sin(angle) * radius + displacement;
		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float y = displacement * 0.4f; // keep height of field smaller compared to width of x and z
		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float z = cos(angle) * radius + displacement;
		model = glm::translate(model, glm::vec3(x, y, z));

		// 2. scale: Scale between 0.05 and 0.25f
		float scale = (rand() % 20) / 100.0f + 0.05;
		model = glm::scale(model, glm::vec3(scale));

		// 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
		float rotAngle = (rand() % 360);
		model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

		// 4. not store to list of matrices
		modelMatrices[i] = model;
	}

	Shader shader("Shader4-10.vertex_asteroid","Shader4-10.fragment_asteroid");

	char planet_model_path[] = "E:/C++/OpenGL_Course/OpenGL_Course/Model/An_asteroid_field/planet/planet.obj";
	char rock_model_path[] = "E:/C++/OpenGL_Course/OpenGL_Course/Model/An_asteroid_field/rock/rock.obj";
	Model planetModel(planet_model_path);
	Model rockModel(rock_model_path);

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
		model = glm::translate(model, glm::vec3(0.0f, -3.0f, 0.0f));
		model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
		shader.setMartix("model", model);
		shader.setMartix("projection", projection);
		shader.setMartix("view", view);
		planetModel.Draw(shader);
		
		// draw meteorites
		for (unsigned int i = 0; i < amount; i++)
		{
			shader.setMartix("model", modelMatrices[i]);
			rockModel.Draw(shader);
		}
		

		//swap and poll
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

}

void Asteroid_field_Instanced(GLFWwindow * window) {
	unsigned int amount = 5000;
	glm::mat4 * modelMatrices;
	modelMatrices = new glm::mat4[amount];	//create a pointer pointing to a array of matrix

	srand(glfwGetTime()); // initialize random seed
	float radius = 50.0;
	float offset = 2.5f;

	for (unsigned int i = 0; i < amount; ++i) {

		//该行星带是基于xz平面的
		glm::mat4 model;
		// 1. translation: displace along circle with 'radius' in range [-offset, offset]
		float angle = (float)i / (float)amount * 360.0f; // to form a circle.
		float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		//notice: 取模运算的要求是分子分母为整数，因此放大一百倍意味着精度范围在0.01
		float x = sin(angle) * radius + displacement;
		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float y = displacement * 0.4f; // keep height of field smaller compared to width of x and z
		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float z = cos(angle) * radius + displacement;
		model = glm::translate(model, glm::vec3(x, y, z));

		// 2. scale: Scale between 0.05 and 0.25f
		float scale = (rand() % 20) / 100.0f + 0.05;
		model = glm::scale(model, glm::vec3(scale));

		// 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
		float rotAngle = (rand() % 360);
		model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

		// 4. not store to list of matrices
		modelMatrices[i] = model;
	}

	Shader shader("Shader4-10.vertex_asteroid", "Shader4-10.fragment_asteroid");
	Shader instanceShader("Shader4-10.vertex_asteroid_instance", "Shader4-10.fragment_asteroid");
	char planet_model_path[] = "E:/C++/OpenGL_Course/OpenGL_Course/Model/An_asteroid_field/planet/planet.obj";
	char rock_model_path[] = "E:/C++/OpenGL_Course/OpenGL_Course/Model/An_asteroid_field/rock/rock.obj";
	Model planetModel(planet_model_path);
	Model rockModel(rock_model_path);
	

	// vertex Buffer Object
	unsigned int buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);

	for (unsigned int i = 0; i < rockModel.GetMeshs().size(); i++)
	{
		unsigned int VAO = rockModel.GetMeshs()[i].GetVAO();
		glBindVertexArray(VAO);
		// vertex Attributes
		GLsizei vec4Size = sizeof(glm::vec4);
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(vec4Size));
		glEnableVertexAttribArray(7);
		glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(2 * vec4Size));
		glEnableVertexAttribArray(8);
		glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(3 * vec4Size));

		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);
		glVertexAttribDivisor(7, 1);
		glVertexAttribDivisor(8, 1);

		glBindVertexArray(0);
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);

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
		model = glm::translate(model, glm::vec3(0.0f, -3.0f, 0.0f));
		model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
		shader.setMartix("model", model);
		shader.setMartix("projection", projection);
		shader.setMartix("view", view);
		planetModel.Draw(shader);

		// draw meteorites
		instanceShader.use();
		instanceShader.setMartix("projection", projection);
		instanceShader.setMartix("view", view);
		for (unsigned int i = 0; i < rockModel.GetMeshs().size(); i++)
		{
			glBindVertexArray(rockModel.GetMeshs()[i].GetVAO());
			glDrawElementsInstanced(
				GL_TRIANGLES, rockModel.GetMeshs()[i].indices.size(), GL_UNSIGNED_INT, 0, amount
			);
		}


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
