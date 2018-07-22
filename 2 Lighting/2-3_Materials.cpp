#include <glad\glad.h>
#include <glfw3.h>

//��ѧ��
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
//ͼ���
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>
#include "Shaders.h"
#include "Camera.h"

/*Material
	if we want to simulate several types of objects in OpenGL, we have to define material properties specific to each other.
	����ɫ���ж���һ��struct��
		struct Material {
			vec3 ambient;		�����⣺�������ڻ�����������ķ�����ɫ
			vec3 diffuse;		�����䣺����������������µ���ɫ���ͻ�����һ��������Ϊ�������ɫ��
			vec3 specular;		���淴�䣺�����������ϵľ�������ɫ��Ӱ�죨���������Ƿ���һ�������ض��ľ���߹���ɫ����
			float shininess;	���ȣ������˾���߹��ɢ��/�뾶��
		}; 
  
		uniform Material material;	
	Ȼ��ʹ��uniform��������Ӧ���ͱ�����
		


*/
void framebuffer_size_callback(GLFWwindow * window, int width, int height) { glViewport(0, 0, width, height);	return; }
void mouse_callback(GLFWwindow * window, double xPos, double yPos);
void scroll_callback(GLFWwindow * window, double xOffset, double yOffset);
void ProcessInput(GLFWwindow * window, float deltaTime);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

static float lastX = 400.0f, lastY = 300.0f;
float pitch, yaw = -90.0f;
float aspect = 45.0f;


glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

Camera camera(cameraPos, cameraUp, yaw, 0.0f);//������һ�����, front ��yaw �� pitchȷ��


bool openSpecular = true;
int lastStatus;
void Material(GLFWwindow * window);
int main(void) {
	if (!glfwInit()) {
		std::cout << "Failed to Initialization" << std::endl;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow * window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "2-2 BasicLight", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create the window" << std::endl;
		glfwTerminate();
		system("pause");
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);	//���ص�����
	glfwSetScrollCallback(window, scroll_callback);		//�����ּ��
														//loading GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		system("pause");
		return -1;
	}

	Material(window);
	glfwTerminate();
	return 0;
}


void ProcessInput(GLFWwindow * window, float deltaTime) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	Camera_Movement camera_Movement;

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE && lastStatus == GLFW_PRESS) {
		//ʹ�ÿո������ؾ��淴��Ч��
		if (openSpecular) openSpecular = false;
		else openSpecular = true;
	}
	lastStatus = glfwGetKey(window, GLFW_KEY_SPACE);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera_Movement = Camera_Movement::FORWARD;
	else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera_Movement = Camera_Movement::BACKWARD;
	else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera_Movement = Camera_Movement::LEFT;
	else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera_Movement = Camera_Movement::RIGHT;
	else
		camera_Movement = Camera_Movement::NONE;
	//std::cout << deltaTime << std::endl;
	camera.ProcessKeyboard(camera_Movement, deltaTime, false);
	return;
}


/*������ÿ�����㼰�䷨����
*/
float cubeVertices[] = {
	//ÿ����6�����㣬
	-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

	-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
	0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
	0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
	0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,

	-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
	-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
	-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

	0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
	0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
	0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
	0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
	0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
	0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

	-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
	0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
	0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

	-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
	0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
	0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
};

void Material(GLFWwindow * window) {
	glEnable(GL_DEPTH_TEST);
	//Shader shader("shader2-1.vertex", "shader2-1.fragment");
	Shader lightShader("shader2-3.vertex", "shader2-3.fragment");
	Shader lampShader("lamp.vertex", "lamp.fragment");
	//Vertices
	unsigned int VAO, VBO, lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glBindVertexArray(lightVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//֮ǰ�Ѿ����������ˣ�����Ͳ����ظ�����
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//while
	glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
	lightShader.use();
	glm::vec3 objectColor(1.0f, 0.5f, 0.31f);
	
	
	//�⼴���������ɫ
	glm::vec3 ambient(1.0f, 0.5f, 0.31f);	lightShader.setVec3("material.ambient", ambient);
	glm::vec3 diffuse(1.0f, 0.5f, 0.31f);	lightShader.setVec3("material.diffuse", diffuse);
	glm::vec3 specular(0.5f, 0.5f, 0.5f);	lightShader.setVec3("material.specular", specular);
	lightShader.setFloat1f("material.shininess", 32.0f);


	//lightColor
	lightShader.setVec3("light.position", lightPos);

	float lastTime{};
	while (!glfwWindowShouldClose(window)) {
		
		//Process Input
		float curTime = glfwGetTime();
		ProcessInput(window , curTime - lastTime);
		lastTime = curTime;
		
		//Color 
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::vec3 lightColor , ambientColor , diffuseColor , specularColor;
		lightColor.x = sin(glfwGetTime() * 2.0f);
		lightColor.y = sin(glfwGetTime() * 0.7f);
		lightColor.z = sin(glfwGetTime() * 1.3f);

		lightShader.setVec3("light.ambient", ambientColor = lightColor * glm::vec3(0.5f));
		lightShader.setVec3("light.diffuse", diffuseColor = lightColor * glm::vec3(0.2f));
		lightShader.setVec3("light.specular", specularColor = lightColor);

		//render
		lightShader.setVec3("viewerPos", camera.Position);

		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 projection;
		view = camera.GetViewMatrix();	//ֱ�ӻ�ȡ���
		projection = glm::perspective(glm::radians(aspect), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		lampShader.setMartix("projection", projection);

		//��Ⱦlamp
		lampShader.use();
		lampShader.setMartix("view", view);
		lampShader.setMartix("projection", projection);
		model = glm::mat4();
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.2f));	//����
		lampShader.setMartix("model", model);
		glBindVertexArray(lightVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		//��Ⱦ��������.
		lightShader.use();
		//lightShader.setVec3("light.position", lightPos);	//����lamp��λ�á�
		lightShader.setBool("openSpecular", openSpecular);

		lightShader.setMartix("view", view);
		lightShader.setMartix("projection", projection);
		model = glm::mat4();
		model = glm::rotate(model, (float)glm::radians(40.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		//model = glm::rotate(model, (float)glm::radians(10.0f * glfwGetTime()), glm::vec3(0.0f, 1.0f, 0.0f));
		lightShader.setMartix("model", model);//ʹ�ñ�׼model,�������κβ���!
		lightShader.setMartix("normalMatrix", glm::inverse(glm::transpose(model)));	//��Ӧ�ڷ������ı任����
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		//swap & Polly
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	return;
}

bool firstMouse = true;
/*
��������ƶ���xֵ��С�������ƶ���xֵ����
��������ƶ���yֵ��С�������ƶ���yֵ����
*/
void mouse_callback(GLFWwindow * window, double xPos, double yPos) {
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

void scroll_callback(GLFWwindow * window, double xOffset, double yOffset) {
	camera.ProcessMouseScroll(xOffset);
}