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

/*Basic Light
	OpenGL�Ĺ��ս���ʹ���˼򻯵�ģ�Ͳ����ڶ���ʵ�Ĺ���������ģ�⣬�������������������һЩ

	light model������ģ��
		Phong lighting model��
			���Ϲ���ģ��(Phong Lighting Model)
			��ģ�Ͱ���3��Ԫ�أ�
				Ambient:����
					��ʹ�ںڰ�������£�������Ҳ��Ȼ��һЩ����(������һ������Զ���Ĺ�)��
					����������Զ��������ȫ�ڰ��ġ�����ʹ�û���������ģ�����������Ҳ�������������Զ��������һЩ��ɫ��
				Diffuse:������
					ģ��һ�������������ķ�����Ӱ��(Directional Impact)�����Ƿ��Ϲ���ģ������������ɲ��֡������Դ��һ���������������
				Specular:����
					ģ���й�������������ֵ����㣬������յ���ɫ��������������ɫ�������ڹ����ɫ��
*/

/*Ambient lighting
	��ͨ�����������һ����Դ��
		���һ�������ǣ�
				�������ز�ͬ������з�ɢ�ͷ�����������һЩ����ֱ�����䵽�ĵ㡣
			���ܹ�����������(Reflect)���������棬һ������Ĺ��տ����ܵ�����һ����ֱ��Ĺ�ԴӰ�졣
		���ǵ�����������㷨����ȫ������(Global Illumination)�㷨�����������㷨�ȿ����߰��ּ��临�ӡ�
	
	ȫ�������ļ򻯰汾��
		ambient lighting ��������
		ʹ��һ��(��ֵ)��С�ĳ���(��)��ɫ��ӵ�����Ƭ�ε�������ɫ��(Ƭ�Σ�Fragment��ָ��ǰ���۵Ĺ����������ϵ������)
		�⿴��������ʹû��ֱ���ԴҲʼ�մ�����һЩ��ɢ�Ĺ⡣
*/

/*Diffuse lighting
	���������(Diffuse Lighting)
	�������ʹ������������Ų�Խ����Ƭ��Խ�ܴӹ�Դ����ø�������ȡ�

	������Ҫ������������������Ӵ�Ƭ��֮��ĽǶȡ�������ߴ�ֱ��������棬������������Ӱ������
		���Ҫ���������䣬������Ҫ��
					1.������(Normal Vector)����ֱ�ڶ�����������
					2.����Ĺ��ߣ���Ϊ���λ�ú�Ƭ�ε�λ��֮���������ķ���������ͨ����Դλ�úͶ���λ�û�á�
	
	Cube������ת�����˿ռ����꣬��FragDirectionҲ��ת����ĵ㵽��Դ�ķ��򣬵���
		�������Ǵ���û�б仯���ģ���Ϊ�ǹ̶��ģ��������й�ĵ�Ҳ������ֶ��ٱ仯��
	���Ҫ���������⣬����Ҫ�Է��������и��£�ʹ֮�ܸ����ı仯������Ӧ�ı仯��һ����������ת��Ӱ�죩��
		ʵ����:
			�ⲻ�Ǽ򵥵ذ�������һ��ģ�;�����ܸ㶨��
			���ȣ�������ֻ��һ���������������ܱ��ռ��е��ض�λ�á�
			ͬʱ��������û���������(����λ���е�w����)������ζ�ţ�ƽ�Ʋ�Ӧ��Ӱ�쵽��������
			��ˣ�������Ǵ���ѷ���������һ��ģ�;������Ǿ�Ҫ��ģ�;����ƽ�Ʋ����Ƴ���������ʹ�øþ�������Ͻǵ�3*3���֡�
			(ע�⣬����Ҳ���԰ѷ�������w��������Ϊ0���ٳ���4��4����ͬ�������Ƴ�ƽ��)��
			���ڷ�����������ֻ�ܶ���Ӧ������(Scale)����ת(Rotation)�任��
		
		���������˲��ȱ����ŵ�ʱ�򣬷������ܿ��ܱ��ƻ�����������ʹ��ģ�;���ת����ķ����������ٴ�ֱ�ڱ��棡
			��ˣ�ʵ���ϣ����ǿ���Ϊ��������������һ��ģ�;�������ת�����þ��󱻳�ΪNormal matrix:
				��Ϊmodel�����ת�ã���ת�õ���Ҳһ��ok�� ��ת�ú���Ŀɽ����ԣ�
				Normal = mat3(transpose(inverse(model))) * normal;
			
			֤�����£�
				��	NΪ	normal vector	������
					TΪ tangent vector	����������	�� NT = TN = 0	������ָ���

					��T�ı任����ΪM	N�ı任����ΪG
					�任ΪΪ T'				N'
					����
						T'N' = TN = 0 = NT = N'T'
					����	N' = G*N		T' = M*T	
					N'T' = transpose(T') * T'		transpose����ʾת�ã������������˵���;���Ĺ�ϵ	
					���У�
						N'T' = transpose(M * T) * (G * N) = transpose(T) * transpose(M) * G * N		����ʹ����ת�õĽ���

						����ΪN'T' = 0 = T * N = transpose(T) * N
						��transpose(M) * G = I ����λ����
							�� G = inverse(transpose(M))	��model�����ת�õ��棬�������ת�ã���Ϊ�ɽ�������
						
						�ù�ʽ���������еķ������任����

			
*/

/*Specular Lighting	���淴��
	
*/

void framebuffer_size_callback(GLFWwindow * window, int width, int height) { glViewport(0, 0, width, height);	return; }
void mouse_callback(GLFWwindow * window, double xPos, double yPos);
void scroll_callback(GLFWwindow * window, double xOffset, double yOffset);
void ProcessInput(GLFWwindow * window);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

static float lastX = 400.0f, lastY = 300.0f;
float pitch, yaw = -90.0f;
float aspect = 45.0f;

float deltaTime, curTime, lastTime;
glm::vec3 cameraPos = glm::vec3(0.0f, 3.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

Camera camera(cameraPos, cameraUp, yaw, -45.0f);//������һ�����, front ��yaw �� pitchȷ��



void Light(GLFWwindow * window);
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

	Light(window);
	glfwTerminate();
	return 0;
}


void ProcessInput(GLFWwindow * window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	Camera_Movement camera_Movement;


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

void Light(GLFWwindow * window) {
	glEnable(GL_DEPTH_TEST);
	//Shader shader("shader2-1.vertex", "shader2-1.fragment");
	Shader lightShader("shader2-2.vertex", "shader2-2.fragment");
	Shader lampShader("shader2-2.vertex", "lamp.fragment");
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
	glm::vec3 lightPos(1.2f, 1.0f, -2.0f);
	lightShader.use();
	glm::vec3 objectColor(1.0f, 0.5f, 0.31f);
	glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
	lightShader.setVec3("objectColor", objectColor);
	lightShader.setVec3("lightColor", lightColor);
	lightShader.setVec3("lightPos", lightPos);

	while (!glfwWindowShouldClose(window)) {
		//Process Input
		ProcessInput(window);

		curTime = glfwGetTime();
		deltaTime = curTime - lastTime;
		lastTime = curTime;

		//render
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
		lightShader.setMartix("view", view);
		lightShader.setMartix("projection", projection);
		model = glm::mat4();
		model = glm::rotate(model,(float)glm::radians(10.0f * glfwGetTime()),glm::vec3(0.0f,1.0f,0.0f));
		lightShader.setMartix("model", model);//ʹ�ñ�׼model,�������κβ���!
		lightShader.setMartix("normalMatrix",glm::inverse(glm::transpose(model)));	//��Ӧ�ڷ������ı任����
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