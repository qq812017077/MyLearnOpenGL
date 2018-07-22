#include <glad\glad.h>
#include <glfw3.h>

//��ѧ��
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include "Shaders.h"
#include "Camera.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
/*Depth testing
	
		��Ȼ�����������ÿ��Ƭ�ε���Ϣ�����Ⱥ͸߶�����ɫ������ͨ����ͬ��
		ͨ���ɴ���ϵͳ�Զ����������������ֵ����Ϊ16��24��32λ��	�������ϵͳ�е����ָ��24λ���ȡ�

	ִ��ʱ�䣺
			��Ƭ����ɫ���Ѿ�����֮�󣬲���ģ���������֮����screen�ռ���ִ�С�
			screen space����ֱ����OpenGL��glViewPort������ء�
			���ҿ���ͨ��GLSL��Ƭ����ɫ�������õ� gl_FragCoord��������
				gl_FragCoord.x:
				gl_FragCoord.y:
						������ screen space �����꣺���½���0,0
				gl_FragCoord.z:
						������Ƭ�ε�ʵ�����ֵ�������ں���Ȼ������е�ֵ���бȽϡ�

	ִ������
		ʹ����Ȼ����OPENGL�������Ȼ������ڵ�Ƭ�ε����ֵ��
			����ͨ������Ȼ���ʹ���µ����ֵ���»�����
			����ʧ�ܣ�����Ƭ�Ρ�
		NOTE��ʹ����Ȳ��Ժ�Ӧȷ��ÿ����Ⱦ֮ǰ��ջ����������򽫱�����һ����Ⱦ�����ֵ��
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ĳ������£����ǿ���ֻϣ��������Ȳ��ԣ�������������Ҫ�ģ����ǲ�����»�������ֵ�����ǿ���ʹ�ã�
				glDepthMask(GL_FALSE);	����Ȼ���ȥֻ����
		


	��Ȳ��Ժ�����
			OpenGL ���������޸�����Ȳ���ʹ�õıȽ��������
				glDepthFunc(comparison_operators);
				GL_ALWAYS	��Զͨ������
				GL_NEVER	��Զ��ͨ������
				GL_LESS	��Ƭ�����ֵС�ڻ����������ʱͨ������
				GL_EQUAL	��Ƭ�����ֵ���ڻ����������ʱͨ������
				GL_LEQUAL	��Ƭ�����ֵС�ڵ��ڻ����������ʱͨ������
				GL_GREATER	��Ƭ�����ֵ���ڻ����������ʱͨ������
				GL_NOTEQUAL	��Ƭ�����ֵ�����ڻ����������ʱͨ������
				GL_GEQUAL	��Ƭ�����ֵ���ڵ��ڻ����������ʱͨ������

*/


/*Depth value precision
	
	��������  ���ֵ��ʽǶ����ͶӰ������
		�������뽫
			һ����������ӹ۲�ռ䣨������ӽǣ����ü��ռ�Ȼ������Ļ�ռ��ʱ����������Է��̾ͱ�Ӧ����

	���ֵ����
		���ֵ��ʽ��ʽ���£�
				Fdepth = (z-near)/(far - near) �䷶Χ�� 0-1֮�䡣
			zֵ������͸��׶���near��farƽ��֮�������ֵ��
		NOTE��	����һ�����ԵĹ�ʽ
		
		��ʵ�����Ǽ���������ʹ��������������Ȼ���������ȷ��ͶӰ���Եķ�������ȷ����Ǻ�1/z�����ȵ� ��
				Fdepth = ( 1/z - 1/near )/( 1/far - 1/near ) �䷶Χ�� 0-1֮�䡣
		
		(������������б�ʱ仯��)
			�����ӵĺô��ǣ�zС��ʱ�򾫶ȸߣ�z�ǳ�Զ��ʱ���򾫶Ȼ��ĺ�С
				��Ȼ�����ǲ�����Ҫ������һǧ��Զ�������������1mԶ����������ͬ����zֵ���ȡ�

			�������Ժ�����1/z�����ȣ�����
				1.0��2.0��zֵ����Ӧ��0.5-1.0�ľ��ȷ�Χ����һ��ľ��ȣ�
				50��100��zֵ����Ӧ��0.02��Χ�ľ��ȣ�

		��������0-0.9����ȷ�Χ����������1-10�� 10����λ��zֵ���ݣ�
*/


/*	
	Visualizing the depth buffer
		��Ȼ��������ӻ�

		��������ת��Ϊ���ԣ�
			1.���Ƚ����ֵ��[0,1]ת��Ϊ�ü��ռ�[-1,1]
				float z = depth * 2.0 - 1.0; 
			2.ʹ�õõ���z�Խ��ֻ���з�ת
				float linearDepth = (2.0 * near * far) / (far + near - z * (far - near));

*/


/*	Z-fighting
	��ȳ�ͻ��
		������ƽ��ǳ�����ʱ����Ȼ�������û���㹻�ľ��ȷֱ浽����һ����ǰ����

		������ȳ�ͻ�����ּ�����
				1.��Ҫ�������ľ���̫����
				2.����ƽ�����õ�ԶһЩ����Ϊ������ƽ��Ĳ��־��Ⱥܸߣ���������������������
				3.����һЩ��������߾��ȡ�ʹ�ø��߾��ȵĻ�������

*/
unsigned int loadTexture(char const *path);
void framebuffer_size_callback(GLFWwindow * window, int width, int height) { glViewport(0, 0, width, height); return; }
void mouse_callback(GLFWwindow* window, double xPos, double yPos);
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
void ProcessInput(GLFWwindow * window, float deltaTime);

unsigned int loadTexture(char const * path);
unsigned int loadVertexToVBO(float const * data);

const int SCR_WIDTH = 800;
const int SCR_HEIGHT = 600;

void Depth_Test(GLFWwindow* window);


glm::vec3 cameraPos(0.0f, 2.0f, 2.0f);
glm::vec3 WorldUp(0.0f, 1.0f, 0.0f);

float yaw = -90.0f, pitch = 0.0f;

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
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		system("pause");
		return -1;
	}

	//
	Depth_Test(window);
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
void Depth_Test(GLFWwindow* window) {
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);//��Զͨ�����ԣ���ζ�������ƵĻ���Զ�������棬��˺Ͳ�����Ȳ��Ե�Ч��һ�¡�

	Shader shader("Shader4-1.vertex", "Shader4-1.fragment");

	//Texture
	unsigned int cubeTexture = loadTexture(std::string("wall.jpg").c_str());
	unsigned int floorTexture = loadTexture(std::string("wall.jpg").c_str());

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
	1.������ͼID��
	2.���ɼ����󶨵���ǰ����ͼID��
	3.����ͼ�󶨵���ǰ��ͼID
		NOTE:	2+3 ���ɼ�������ͼ������������
	*/
	shader.use();
	shader.setInt("texture1", 0);
	


	float curTime{}, lastTime{};
	curTime = lastTime = glfwGetTime();
	while (!glfwWindowShouldClose(window)) {

		//ProcessInput
		curTime = glfwGetTime();
		ProcessInput(window, curTime - lastTime);
		lastTime = curTime;

		// render
		// ------
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glm::mat4 model;
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		shader.use();
		shader.setMartix("projection", projection);
		shader.setMartix("view", view);
		//cube
		glBindVertexArray(cubeVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, cubeTexture);
		model = glm::translate(model, glm::vec3(-1.0f, 0.0f, -1.0f));
		shader.setMartix("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		model = glm::mat4();
		model = glm::translate(model, glm::vec3(2.0f, 0.0f, 0.0f));
		shader.setMartix("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// floor
		glBindVertexArray(planeVAO);
		glBindTexture(GL_TEXTURE_2D, floorTexture);	//�л���ͼ
		shader.setMartix("model", glm::mat4());
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

unsigned int loadTexture(char const *path)
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

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
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