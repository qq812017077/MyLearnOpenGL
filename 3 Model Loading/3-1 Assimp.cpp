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
#include "materialLibrary.h"

/*
	Assimp   ģ�ͼ��ؿ�
	ģ�ͽṹ����
		Scene�������˳����е��������ݣ������еĲ��ʺ�������Ϣ���Լ��������ڵ�����á�

		Root Node�� �����ĸ��ڵ������һϵ���ӽڵ㣬�Լ��ڳ��������mMeshes[]�е�һϵ��������ָ���������ݣ�
				���ڵ��mMeshes[]������ʵ�ʵ�Mesh����
				�� ÿ���ڵ��mMeshes[]��ֵ���Ǹ��ڵ�mMeshes[]������
		Mesh ����
				�������������Ⱦ����������Ϣ���� ����λ�ã������� ��ͼ���꣬ ����Ĳ��ʡ�
			һ��MeshҲ������һЩFace�� 
					һ�� Face �����˶����һ����Ⱦ������λ���㡢�ߡ�������Ƭ��������Ƭ��
					һ�� Face �������γ�һ��Բ�εĶ����������
					��ͨ�����������������mMeshes[]��Ѱ�ҵ���Ӧ�Ķ���λ�����ݡ���
					������������Ƿ���ģ���ʹ��ͨ��һ������������Ⱦ�ǱȽ����׵ģ�ͨ��VAO��VBO��NBO��TBO��IBO��
			һ��MeshҲ������һ�����ʶ�������ָ�������һЩ��������
				����ɫ��������ͼ����������ͼ���߹���ͼ�ȣ�

		�����������ĵ�һ���£�
			����һ������Scene���󣬱���ÿ���ڵ��ȡ��Ӧ��Mesh���󣨵ݹ��ѯÿһ���ڵ㣩��
				����ÿһ��Mesh�����Ի�ȡ�������ݣ���������������ԡ�
			�������ǵõ�һ������������Ҫ�����ݵ�Mesh���ϣ���λ�ڵ���Model�����С�

		NOTE��
			��Ҫע����ǣ����ʦͨ������ʹ�õ�����״������������ģ�ͣ�ÿ��ģ��ͨ�����������ģ�ͺ���״��
			
					����һ��ģ�͵�ÿ����״������Ϊһ��Mesh��
			������һ������ģ��ͨ������ͷ����֫���·�����������齨��϶��ɣ�

				����Mesh��OpenGL�пɻ��Ƶ���С��λ�������������ݣ��������������ԣ���һ��ģ�Ͱ������Mesh��
*/

void framebuffer_size_callback(GLFWwindow * window, int width, int height) { glViewport(0, 0, width, height); return; }
void mouse_callback(GLFWwindow* window, double xPos, double yPos);
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
void ProcessInput(GLFWwindow * window, float deltaTime);

unsigned int loadTexture(char const * path);
unsigned int loadVertexToVBO(float const * data);

const int SCR_WIDTH = 800;
const int SCR_HEIGHT = 600;

void MultipleLights_Task_Desert(GLFWwindow* window);
void MultipleLights_Task_Factory(GLFWwindow* window);
void MultipleLights_Task_Horror(GLFWwindow* window);
void MultipleLights_Task_Lab(GLFWwindow* window);

glm::vec3 cameraPos(0.0f, 0.0f, 2.0f);
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

	GLFWwindow * window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "2-6_MultipleLights", NULL, NULL);
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
	MultipleLights_Task_Lab(window);
	glfwTerminate();

}