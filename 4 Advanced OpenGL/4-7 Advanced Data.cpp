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
#include "Model.h"  //Offical_

/*Advanced Data
	manipulate:�ٿ�
	�����и�����Ȥ�Ĳٿػ������ķ�����
		����ʹ���������������ݴ�����ɫ��Ҳ�и���Ȥ�ķ�����

	OpenGL�У�
		һ������������һ�������������һ��ָ���ڴ棬
		�����ǽ���󶨵�һ��ָ���Ļ����������ʱ�򣬲Ÿ����������塣
		eg:
			�����ǽ������GL_ARRAY_BUFFER��ʱ��������һ���������黺������
			���߽���󶨵�GL_ELEMENT_ARRAY_BUFFER��
			OpenGL�ڲ���Ϊÿ��Ŀ�괢��һ�����壬���һ����Ŀ��Ĳ�ͬ���Բ�ͬ�ķ�ʽ�������塣
	
	glBufferData��
			��ĿǰΪֹ������һֱ��ͨ���ú��������ڴ棬 �����������ݵ��ڴ��С�
			������ǽ���data��������ΪNULL,�����ǽ�������������������������䡣

			����������ҪԤ��(Reserve)�ض���С���ڴ棬֮��ص��������һ��һ������ʱ�������á�
	
	glBufferSubData��
			����һ����������������������ǻ����Խ���仺������ĳ������ͨ����������������
				�ú���ָ������ȾĿ�꣬ƫ���������ݴ�С���Լ�ʵ�����ݣ���Ϊ��Ĳ�����
		
			��ͬ���ǣ����ǿ����Լ�������Ҫ��ƫ�������̶�������ݡ�
		
			glBufferSubData(GL_ARRAY_BUFFER, 24, sizeof(data), &data); // Range: [24, 24 + sizeof(data)]

		���ã�
			���������ǲ���/���߸���ĳһ�������ݡ�
			��Ҫע����ǣ�����������Ҫ���㹻�Ŀռ䣬�������Ǳ����ڵ��øú���֮ǰ����һ��glBufferData.

	glMapBuffer��
		��һ�ֵ������ݵ��������ķ����� geting data into the buffer!!!
			1.���Ȼ�ȡ�������ڴ��ָ�룺
				void *ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
				//ע����ǰҪ�󶨺û�����
			2����ȡ������ָ�룺
				void *ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
				
			3.�������ݣ�
				memcpy(ptr, data, sizeof(data));
			4.���߻������������
				glUnmapBuffer(GL_ARRAY_BUFFER);
				֮��ָ�뽫���ٿ���
			glMapBuffer����ֱ�ӽ�����ӳ�䵽�����У�������Ҫ�Ƚ���洢����ʱ�ڴ���
				���磺���ǿ���ֱ�Ӵ��ļ��ж�ȡ����Ȼ�󿽱����������ڴ��С�	
*/

/*Batching vertex attributes.
	������������
	֮ǰ������ʹ��glVertexAttribPointer��ָ���������黺�������ݵ����Բ��֡�
	�ڶ������黺�����У����ǽ����˽���������interleaved�������仰˵��
		���ǽ�ÿһ�������λ�á����ֺ�/������������ܷ�����һ��
	
	�������ǶԻ��������˸�����˽⣬���Գ���һЩɧ�����ˡ�
	���ǿ��������ǣ���ÿһ���������͵��������ݴ��(Batch)Ϊһ��������飬�����Ƕ����ǽ��н������档
		Ҳ����ȡ��ʹ��123123123�����Ľ������֣�����ʹ�ô������111222333

		�����Ǵ��ļ��л�ȡ���������ݵ�ʱ�����ǻ�ȡ�����ǣ�
			һ������λ�����飻
			һ�����������飻
			һ�������������飻
		���������������Щ���ݵ�һ����Ľ�������������ʱҪ����������
		ͨ�������ķ�ʽ�����Ǹ��򵥵ķ�����
			float positions[] = {.....};
			float normals[] = {.....};
			float textureCoords[] = {.....};
			//fill
			glBufferSubData(GL_ARRAY_BUFFER, 0 , sizeof(positions) , &positions);
			glBufferSubData(GL_ARRAY_BUFFER, sizeof(positions) , sizeof(normals) , &normals);
			glBufferSubData(GL_ARRAY_BUFFER,  sizeof(positions) + sizeof(normals) , sizeof(textureCoords) , &textureCoords);
		��������VAO�Ķ�����������Ҳ��Ҫ��Ӧ���޸ģ�
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)(sizeof(positions)));
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)(sizeof(positions)+sizeof(normals) ));

	ע��ʹ�������ַ�����û��̫������ӣ����Լ�ϲ�ö���������
*/

/*Copying Buffers
	���������������֮�����Ǻܿ�����Ҫ�������Ļ������������ݣ�������Ҫ����һ�ݵ���һ����������

	glCopyBufferSubData ���԰�����ʵ�����Ŀ�ģ�
		void glCopyBufferSubData(GLenum readtarget, GLenum writetarget, GLintptr readoffset,
                         GLintptr writeoffset, GLsizeiptr size);
			readtarget��
			writetarget:���������������˸��Ƶ�Դ��������Ŀ�껺������
				eg:
					glCopyBufferSubData( VERTEX_ARRAY_BUFFER  , VERTEX_ELEMENT_ARRAY_BUFFER  , ....);
					��ʾ��ǰ�߶�Ӧ�İ󶨵Ļ��������Ƶ����߰󶨵Ļ�������---> VBO �� EBO  ???
				���⣺���������Ҫ�������ݵ�ͬ���͵Ļ�������ô���أ�����û��ͬʱ��������������ͬһ������Ŀ�꣡
					����취��
						OpenGL�ṩ����������Ŀ�������
							GL_COPY_READ_BUFFER
							GL_COPY_WRITE_BUFFER
						��˿��Խ������������󶨵�������棬Ȼ����и��ƣ�
			readoffset:
			writeoffset:��ʾ���ǣ������������ж�ȡ��ƫ������һ�㶼��0 0 �� ��ʾ��Դ�������Ŀ�����㿪ʼ��

			size���������ݵĶ��١�

			eg:
				float vertexData[] = { ... };
				glBindBuffer(GL_COPY_READ_BUFFER, vbo1);
				glBindBuffer(GL_COPY_WRITE_BUFFER, vbo2);
				glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, sizeof(vertexData));
			����ֻ��һ����
				glBindBuffer(GL_ARRAY_BUFFER, vbo1);
				glBindBuffer(GL_COPY_WRITE_BUFFER, vbo2);
				glCopyBufferSubData(GL_ARRAY_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, sizeof(vertexData));
		
*/

/*	���ʹ����������洢���������ݣ�
		
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
	CubeMap_Test(window);
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
	//����
	// positions          
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	//�����
	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,
	//�Ҳ���
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	//����
	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,
	//����
	-1.0f,  1.0f, -1.0f,
	1.0f,  1.0f, -1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,
	//����
	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	1.0f, -1.0f,  1.0f
};

void CubeMap_Test(GLFWwindow* window) {
	//ʹ����Ȳ���
	glEnable(GL_DEPTH_TEST);//��Ȳ���

	Shader shader("Shader4-6.vertex", "Shader4-6.fragment");
	Shader skyboxShader("Shader4-6.vertex_skybox", "Shader4-6.fragment_skybox");

	//����ģ��
	char model_path[] = "E:/C++/OpenGL_Course/OpenGL_Course/Model/nanosuit_reflrection/nanosuit.obj";
	Model ourModel(model_path);
	std::cout << "Model������ɣ��ܹ���" << ourModel.GetMeshs().size() << "��Mesh" << std::endl;
	std::vector<Mesh> Meshes = ourModel.GetMeshs();
	std::vector<Mesh>::iterator it = Meshes.begin();
	int i = 0; int j = 0;
	for (std::vector<Mesh>::iterator it = Meshes.begin(); it != Meshes.end(); it++)
	{
		std::cout << "��" << i++ << "��Mesh" << std::endl;
		std::vector<Texture>::iterator texIt = it->textures.begin();
		for (std::vector<Texture>::iterator texIt = it->textures.begin(); texIt != it->textures.end(); texIt++)
		{
			std::cout << j << ":  " << texIt->path << "   " << texIt->type << std::endl;

		}
	}

	//Texture
	unsigned int cubeTexture = loadTexture(std::string("container.jpg").c_str(), GL_REPEAT);
	unsigned int floorTexture = loadTexture(std::string("wall.jpg").c_str(), GL_REPEAT);

	//textures_faces ���������е���ͼλ�á�ע�����˳�򣺰���ѭ����ѭ���е�ö��˳����С�
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
	glGenVertexArrays(1, &skyVAO);
	glGenBuffers(1, &skyVBO);
	glBindVertexArray(skyVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
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
	1.������ͼID��
	2.���ɼ����󶨵���ǰ����ͼID��
	3.����ͼ�󶨵���ǰ��ͼID
	NOTE:	2+3 ���ɼ�������ͼ������������
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
		shader.setVec3("cameraPos", camera.Position);
		model = glm::mat4();
		model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
		shader.setMartix("model", model);
		shader.setMartix("normalMatrix", glm::transpose(glm::inverse(model)));
		shader.setMartix("view", view);
		shader.setMartix("projection", projection);
		/*glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);*/
		ourModel.Draw(shader);

		//skybox---ע��ر����д�룬�����Ӳ��ܳ�Ϊ������
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyTexture);
		glDepthFunc(GL_LEQUAL);//ˢ�º�Ļ��������ֵΪ1�����������Ҫ����Ϊ�����ֵС�ڵ���1��ʱ����ͨ���������򱳾�����д���ϣ�
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

	�����GL_TEXTURE_WRAP_R������������R���꣨��Ӧ�������ĵ�����ά�ȣ���

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