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

/*Geometry Shader 几何着色器
	在顶点和片段着色器之间有一个可选的几何着色器(Geometry Shader)。

	几何着色器：
		输入：a set of vertices (形成单个基元)
			转换：可以在发送给下一个着色器之前进行转换，只要看着合适。
			它能够将（这一组）顶点变换为完全不同的图元，并且还能生成比原来更多的顶点。

	
	代码实例：
			#version 330 core
			layout (points) in;
			layout (line_strip, max_vertices = 2) out;

			void main() {    
				gl_Position = gl_in[0].gl_Position + vec4(-0.1, 0.0, 0.0, 0.0); 
				EmitVertex();

				gl_Position = gl_in[0].gl_Position + vec4( 0.1, 0.0, 0.0, 0.0);
				EmitVertex();
    
				EndPrimitive();
			}  
	输入类型：
		在开头，我们需要声明从顶点着色器接收的基元的输入类型，
			我们通过在in之前声明一个layout修饰符来达到目的，该修饰符接收下面的基元值：
			（括号内的数字表示的是一个图元所包含的最小顶点数。）
				points：				绘制GL_POINTS图元（1）
				lines：					绘制GL_LINES或GL_LINE_STRIP（2）
				lines_adjacency：		GL_LINES_ADJACENCY或GL_LINE_STRIP_ADJACENCY（4）
				triangles：				GL_TRIANGLES、GL_TRIANGLE_STRIP或GL_TRIANGLE_FAN（3）
				triangles_adjacency：	GL_TRIANGLES_ADJACENCY或GL_TRIANGLE_STRIP_ADJACENCY（6）
			以上就是所有的可提供给渲染函数（如glDrawArrays）调用的的渲染基元了。
				如果我们想要将顶点绘制为GL_TRIANGLES，我们就要将输入修饰符设置为triangles。
	输出类型：
		除了输入，我们还需要指定输出类型；类似于输入，输出也有以下几种值：
			points：
			line_strip：
			triangle_strip：
		有了这三种输出修饰符，我们几乎可以创造任意的形状。
			要生成一个三角形的话，我们将输出定义为triangle_strip，并输出3个顶点。
		
		几何着色器同时希望我们设置一个它最大能够输出的顶点数量（可以在输出修饰符中描述），
		当超出这个值的时候，openGL将不会绘制多出的部分。

	输入变量：
		在GLSL中是一个内置的接口块变量：
		in gl_Vertex{
			vec4 gl_Position;
			float gl_PointSize;
			float gl_ClipDistance[];
		} gl_in[];
		注意其被声明为数组，因为输入是一个图元的所有顶点。
	
	数据处理阶段：
		使用2个几何着色器函数，EmitVertex和EndPrimitive，来生成新的数据。
			gl_Position = gl_in[0].gl_Position + vec4(-0.1, 0.0, 0.0, 0.0); 
			EmitVertex();

			gl_Position = gl_in[0].gl_Position + vec4( 0.1, 0.0, 0.0, 0.0);
			EmitVertex();

			EndPrimitive();
		每当调用EmitVertex()的时候，gl_Position中的向量就会被添加到图元中，

		当调用EndPrimitive()后，所有发射(Emit)的顶点都会被合成指定的输出渲染图元，

		在一个或多个EmitVertex调用之后重复调用EndPrimitive可以生成多个图元。

*/

/*Using Gemometry shaders
	没什么好说的。。
	在shader类中加入一个带有几何着色器的构造函数就好了。
	然后渲染就是：
		顶点->几何->片段

*/

/*爆破
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
//点、线、面
void Point2Line_Test(GLFWwindow* window);
//爆破
void Explode_Test(GLFWwindow * window);
//可视化法向量
void Visualizing_Normal_Test(GLFWwindow * window);
glm::vec3 cameraPos(0.0f, 0.0f, 2.0f);
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

	//
	//FragCoord_Test(window);
	//Point2Line_Test(window);
	//Explode_Test(window);
	Visualizing_Normal_Test(window);
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


float points[] = {
	//position					//color
	-0.5f,  0.5f, 0.0f,			1.0f , 0.0f, 0.0f,		// top-left
	0.5f,  0.5f, 0.0f,			0.0f , 1.0f, 0.0f,		// top-right
	0.5f, -0.5f, 0.0f,			0.0f , 0.0f, 1.0f,		// bottom-right
	-0.5f, -0.5f,0.0f,			1.0f , 1.0f, 0.0f		// bottom-left
};

void Point2Line_Test(GLFWwindow* window) {
	//使能深度测试
	glEnable(GL_DEPTH_TEST);//深度测试

	Shader shader("Shader4-9.vertex_p2l","Shader4-9.geometry_p2house" ,"Shader4-9.fragment_p2l");
	//Shader shader("Shader4-9.vertex_p2l" , "Shader4-9.fragment_p2l");
	camera.MovementSpeed = 1.5f;

	// cube VAO
	unsigned int pointsVAO, pointsVBO;
	glGenVertexArrays(1, &pointsVAO);
	glGenBuffers(1, &pointsVBO);
	glBindVertexArray(pointsVAO);
	glBindBuffer(GL_ARRAY_BUFFER, pointsVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), &points, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
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
		//DrawScence
		glm::mat4 model;
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();

		//first pass
		glClearColor(0.1f, 0.1f, 0.1f, 0.1f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//points
		shader.use();
		model = glm::mat4();
		shader.setMartix("model", model);
		shader.setMartix("view", view);
		shader.setMartix("projection", projection);
		glBindVertexArray(pointsVAO);
		glDrawArrays(GL_POINTS, 0, 4);
		glBindVertexArray(0);

		//swap and poll
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

}

void Explode_Test(GLFWwindow * window) {
	//使能深度测试
	glEnable(GL_DEPTH_TEST);//深度测试

	Shader shader("Shader4-9.vertex_explode", "Shader4-9.geometry_explode", "Shader4-9.fragment_explode");
	
	char model_path[] = "E:/C++/OpenGL_Course/OpenGL_Course/Model/nanosuit_reflrection/nanosuit.obj";
	Model ourModel(model_path);

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
		shader.use();
		model = glm::mat4();
		model = glm::scale(model , glm::vec3(0.1f,0.1f,0.1f));
		shader.setMartix("model", model);
		shader.setMartix("view", view);
		shader.setMartix("projection", projection);
		shader.setMartix("normalMatrix", glm::inverse(glm::transpose(model)));
		shader.setFloat1f("time",glfwGetTime());
		ourModel.Draw(shader);
		glBindVertexArray(0);

		//swap and poll
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

void Visualizing_Normal_Test(GLFWwindow * window) {
	//使能深度测试
	glEnable(GL_DEPTH_TEST);//深度测试

	Shader shader("Shader4-9.vertex", "Shader4-9.fragment");
	Shader noraml_display_shader("Shader4-9.vertex_normal", "Shader4-9.geometry_normal", "Shader4-9.fragment_normal");

	char model_path[] = "E:/C++/OpenGL_Course/OpenGL_Course/Model/nanosuit_reflrection/nanosuit.obj";
	Model ourModel(model_path);

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
		shader.use();
		model = glm::mat4();
		model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
		shader.setMartix("model", model);
		shader.setMartix("view", view);
		shader.setMartix("projection", projection);
		shader.setMartix("normalMatrix", glm::inverse(glm::transpose(model)));
		ourModel.Draw(shader);

		noraml_display_shader.use();
		noraml_display_shader.setMartix("model", model);
		noraml_display_shader.setMartix("view", view);
		noraml_display_shader.setMartix("projection", projection);
		noraml_display_shader.setMartix("normalMatrix", glm::inverse(glm::transpose(model)));
		ourModel.Draw(noraml_display_shader);
		glBindVertexArray(0);

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

unsigned int loadCubeMap(std::vector<std::string> texture_faces) {
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

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
		}
		else {
			std::cout << "Cubemap texture failed to load at path: " << texture_faces[i] << std::endl;
			stbi_image_free(data);
		}

	}
	return textureID;
}
