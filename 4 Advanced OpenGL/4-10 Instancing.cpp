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

/*Instance
	ʵ����
	��Ҫ����һ������������ͬģ��λ�ò�ͬ�ĳ���,����ݡ�
	��ÿһ���ݶ��ǰ�����������ε�ģ�ͣ�
	�����Ĳݵ���Ⱦ������Ӱ������

	��Ⱦ��������������ƴ������£�
		for(unsigned int i = 0; i < amount_of_models_to_draw; i++)
		{
			DoSomePreparations(); // bind VAO, bind textures, set uniforms etc.
			glDrawArrays(GL_TRIANGLES, 0, amount_of_vertices);
		}
		����������ģ�͵Ĵ���ʵ��(Instance)����ܿ�ͻ���Ϊ���Ƶ��ù�����ﵽ����ƿ����
			����ƶ��㱾����ȣ�ʹ��glDrawArrays��glDrawElements��������GPUȥ�������
			�������ݻ����ĸ�������ܣ���ΪOpenGL�ڻ��ƶ�������֮ǰ��Ҫ���ܶ�׼������
			���������GPU�ô��ĸ������ȡ���ݣ�����Ѱ�Ҷ������ԣ�������Щ��������Ի�
			����CPU��GPU����(CPU to GPU Bus)�Ͻ��еģ������ԣ�������Ⱦ����ǳ��죬����GPUȥ��Ⱦȴδ�ء�

	������
		ʵ������ֻ����һ�����ݣ�����OpenGL���ƶ�ζ�ֻʹ��һ�λ��Ƶ��á�
		����������ʹ�� glDrawArrays �� glDrawElements�� ʹ��glDrawArraysInstanced �� glDrawElementsInstanced

		�µĺ�����Ҫ����Ĳ�������˵����������Ҫ��Ⱦ��ʵ������
		����������ݷ��͸�GPUһ�Σ�ͨ��һ�������ĵ���������GPU��λ������е�ʵ����

		NOTE:�����ĵ������������Ч����ͬһ��������ƶ�Σ����Ƕ���һ��λ�ã��Ӿ�Ч������һ����

		�����
			GLSL�ڶ�����ɫ���е����ñ�����gl_InstanceID

			��ʹ��ʵ������Ⱦ����ʱ��gl_InstanceID���0��ʼ����ÿ��ʵ������Ⱦʱ����1��
			eg��������Ⱦ��43��ʵ��ʱ�� gl_InstanceID = 42

*/

/*Instanced Array
	NOTE:ʵ��������һ���������ԣ�����˵ �������Կ�������Ϊʵ�����飡��
	ʵ������
		һ����Ҫע�������ǣ����������ڶ�����ɫ����������һ��ȫ�ֱ������飬��СΪ100.
		Ȼ�������ô������ޣ�����������1024(������Ȼ���ܲ��㣡)
	���Դ�������һ�����������
		Instanced Array ʵ�����飬�䱻����Ϊ�������ԡ����������ǿ��Ա����������ݡ�
		�����������Ⱦ�µ�ʵ��ʱ���и��¡�
		
	ʹ�ö������ԣ�ÿ�����ж�����ɫ������ʹ��GLSL��ȡһ���Ӧ�Ķ������ԣ�
		Ȼ���������Ƕ���һ������������Ϊʵ������ʱ��������ɫ��������ÿ��ʵ���ĸö����������ݶ�����ÿ�����㡣
		
		�������ǿ���ʹ�ñ�׼����������Ϊÿ����������ݣ�
		����ʹ��ʵ��������Ϊÿ��ʵ���Ķ�һ�޶��Ĵ洢���ݡ�

*/

/*
	NOTE:
		ʵ���ϣ�������������Ϊ�������Ե������������Ϊvec4.
		����������Ȼ��������Ϊmat4��
			��ʱ����Ϊmat4���ĸ�vec4.
			������mat4���͵����Բ����� location = 3 ��ʵ������ 3 �� 4 �� 5 �� 6 �ĸ�λ��

*/

unsigned int loadTexture(char const *path, unsigned int Tex_Wrap_Para);
unsigned int loadCubeMap(std::vector<std::string> texture_faces);
void framebuffer_size_callback(GLFWwindow * window, int width, int height) { glViewport(0, 0, width, height); return; }
void mouse_callback(GLFWwindow* window, double xPos, double yPos);
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
void ProcessInput(GLFWwindow * window, float deltaTime);

//����gl_InstanceID��ʵ����
void Instanced_Quad(GLFWwindow * window);
//ʵ������
void Instanced_Array_Quad(GLFWwindow * window);
//С���Ǵ�
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

	Asteroid_field_Instanced(window);
	
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
	//��׼����
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
	//ʵ������
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 100, &translations[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glVertexAttribDivisor(2, 1);	//!!!
	/*glVertexAttribDivisor
			�����������OpenGL��ʲôʱ����¶����������ݵ���һ��Ԫ�أ�
				��һ����������Ҫ�Ķ������ԡ���������Ϊ2��ָ��λ��λ��ֵ2�Ķ���������һ��ʵ��������
				�ڶ������������Գ�����Ĭ��Ϊ0����ÿ�ε�����ɫ�������¶������Ե����ݣ�
				�����ó�1��˵�����ǵ����ǿ�ʼ��Ⱦһ���µ�ʵ����ʱ�򣬸��¶������Ե�����
				  ������Ϊ2ʱ������ϣ��ÿ2��ʵ������һ�����ԣ��Դ����ơ���
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
		
		//�����Ǵ��ǻ���xzƽ���
		glm::mat4 model;
		// 1. translation: displace along circle with 'radius' in range [-offset, offset]
		float angle = (float)i / (float)amount * 360.0f; // to form a circle.
		float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset; 
		//notice: ȡģ�����Ҫ���Ƿ��ӷ�ĸΪ��������˷Ŵ�һ�ٱ���ζ�ž��ȷ�Χ��0.01
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

		//�����Ǵ��ǻ���xzƽ���
		glm::mat4 model;
		// 1. translation: displace along circle with 'radius' in range [-offset, offset]
		float angle = (float)i / (float)amount * 360.0f; // to form a circle.
		float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		//notice: ȡģ�����Ҫ���Ƿ��ӷ�ĸΪ��������˷Ŵ�һ�ٱ���ζ�ž��ȷ�Χ��0.01
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
