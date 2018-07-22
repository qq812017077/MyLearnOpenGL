#include <glad\glad.h>
#include <glfw3.h>

//��ѧ��
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <map>
#include "Shaders.h"
#include "Camera.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
//ģ������
#include "Model.h"

/*Geometry Shader ������ɫ��
	�ڶ����Ƭ����ɫ��֮����һ����ѡ�ļ�����ɫ��(Geometry Shader)��

	������ɫ����
		���룺a set of vertices (�γɵ�����Ԫ)
			ת���������ڷ��͸���һ����ɫ��֮ǰ����ת����ֻҪ���ź��ʡ�
			���ܹ�������һ�飩����任Ϊ��ȫ��ͬ��ͼԪ�����һ������ɱ�ԭ������Ķ��㡣

	
	����ʵ����
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
	�������ͣ�
		�ڿ�ͷ��������Ҫ�����Ӷ�����ɫ�����յĻ�Ԫ���������ͣ�
			����ͨ����in֮ǰ����һ��layout���η����ﵽĿ�ģ������η���������Ļ�Ԫֵ��
			�������ڵ����ֱ�ʾ����һ��ͼԪ����������С����������
				points��				����GL_POINTSͼԪ��1��
				lines��					����GL_LINES��GL_LINE_STRIP��2��
				lines_adjacency��		GL_LINES_ADJACENCY��GL_LINE_STRIP_ADJACENCY��4��
				triangles��				GL_TRIANGLES��GL_TRIANGLE_STRIP��GL_TRIANGLE_FAN��3��
				triangles_adjacency��	GL_TRIANGLES_ADJACENCY��GL_TRIANGLE_STRIP_ADJACENCY��6��
			���Ͼ������еĿ��ṩ����Ⱦ��������glDrawArrays�����õĵ���Ⱦ��Ԫ�ˡ�
				���������Ҫ���������ΪGL_TRIANGLES�����Ǿ�Ҫ���������η�����Ϊtriangles��
	������ͣ�
		�������룬���ǻ���Ҫָ��������ͣ����������룬���Ҳ�����¼���ֵ��
			points��
			line_strip��
			triangle_strip��
		����������������η������Ǽ������Դ����������״��
			Ҫ����һ�������εĻ������ǽ��������Ϊtriangle_strip�������3�����㡣
		
		������ɫ��ͬʱϣ����������һ��������ܹ�����Ķ���������������������η�����������
		���������ֵ��ʱ��openGL��������ƶ���Ĳ��֡�

	���������
		��GLSL����һ�����õĽӿڿ������
		in gl_Vertex{
			vec4 gl_Position;
			float gl_PointSize;
			float gl_ClipDistance[];
		} gl_in[];
		ע���䱻����Ϊ���飬��Ϊ������һ��ͼԪ�����ж��㡣
	
	���ݴ���׶Σ�
		ʹ��2��������ɫ��������EmitVertex��EndPrimitive���������µ����ݡ�
			gl_Position = gl_in[0].gl_Position + vec4(-0.1, 0.0, 0.0, 0.0); 
			EmitVertex();

			gl_Position = gl_in[0].gl_Position + vec4( 0.1, 0.0, 0.0, 0.0);
			EmitVertex();

			EndPrimitive();
		ÿ������EmitVertex()��ʱ��gl_Position�е������ͻᱻ��ӵ�ͼԪ�У�

		������EndPrimitive()�����з���(Emit)�Ķ��㶼�ᱻ�ϳ�ָ���������ȾͼԪ��

		��һ������EmitVertex����֮���ظ�����EndPrimitive�������ɶ��ͼԪ��

*/

/*Using Gemometry shaders
	ûʲô��˵�ġ���
	��shader���м���һ�����м�����ɫ���Ĺ��캯���ͺ��ˡ�
	Ȼ����Ⱦ���ǣ�
		����->����->Ƭ��

*/

/*����
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
//�㡢�ߡ���
void Point2Line_Test(GLFWwindow* window);
//����
void Explode_Test(GLFWwindow * window);
//���ӻ�������
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

	//ע�ỷ��
	glfwMakeContextCurrent(window);
	//��ʼ���ص�
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);	//�ر�����
	glfwSetCursorPosCallback(window, mouse_callback);				//�����ص�
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
��������ƶ���xֵ��С�������ƶ���xֵ����
��������ƶ���yֵ��С�������ƶ���yֵ����
*/
bool firstMouse = true;
static float lastX = 400.0f, lastY = 300.0f;
void mouse_callback(GLFWwindow* window, double xPos, double yPos) {
	std::cout << xPos << "   " << yPos << std::endl;
	if (firstMouse) // this bool variable is initially set to true
	{
		//��һ�ε���ʱ��xPos,yPos��lastX,lastY���޴���Ҫ����һ�ε���
		lastX = xPos;
		lastY = yPos;
		firstMouse = false;
	}
	//std::cout << "xPos:  " << xPos << "yPos:  " << yPos << std::endl;
	float sensitivity = 0.05f;	//���ж�
	float xOffset = xPos - lastX;	//�����ƶ�Ϊ���������ƶ�Ϊ����
	float yOffset = lastY - yPos;	//�����ƶ�Ϊ���������ƶ�Ϊ����

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
		//ʹ�ÿո������ؾ��淴��Ч��
		if (openEffect) openEffect = false;
		else openEffect = true;
	}
	lastStatus = glfwGetKey(window, GLFW_KEY_SPACE);

	//�����������
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
	//ʹ����Ȳ���
	glEnable(GL_DEPTH_TEST);//��Ȳ���

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
	//ʹ����Ȳ���
	glEnable(GL_DEPTH_TEST);//��Ȳ���

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
	//ʹ����Ȳ���
	glEnable(GL_DEPTH_TEST);//��Ȳ���

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

	//��Ȼ����ͨ������һ����������Ҫָ���价�ƺ͹��˷�����
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	/*
	ע�����ǽ�����ά�ȵĻ��Ʒ�ʽ������Ϊ��GL_CLAMP_TO_EDGE������Ϊ
	��������֮�������������ܲ����ܴ����ĸ��棨����Ӳ�����ƣ�������ͨ��ʹ��GL_CLAMP_TO_EDGE
	ÿ�����ǲɼ���Ե��ʱ��OpenGL�����Ƿ������ǵı�Եֵ��

	�����GL_TEXTURE_WRAP_R�����������R���꣨��Ӧ������ĵ�����ά�ȣ���

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
