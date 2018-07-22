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
/*
	ע�����еĻ��������ǻ���2D����ģ�
	��ģ�建����������ͶӰ֮���2D����Ļ��������
*/
/*
	Stencil Testing
	ģ�����

	��Ƭ����ɫ��������Ƭ��֮��ģ�����(Stencil Test) �Ϳ�ʼִ����
	����Ȳ���һ�������ܶ���һЩƬ�Ρ���Ȼ����������Ƭ�ν�����Ȳ��Խ׶Σ���Ȳ��Կ��ܶ������ࡣ

	����ģ�建�壬����һ��������Ϊ���ǵ�ģ�壬�������ʵ���Ͼ���д��ģ�建��Ĺ���

	ģ����Ի�����һ������ģ�建�����Ļ���������������������Ⱦ�ڼ���£����������Ȥ��Ч����

	ÿ��ģ��ֵ��	stencil value ����8λ��
		��˶���ÿһ������/Ƭ�Σ��ܹ���256�ֲ�ͬ��ģ��ֵ�����

		stencil����������ʹ��0��գ�Ȼ������һ����1���ľ���������
			�����Ƭ�ε�stencilֵ����1��ʱ��ᱻ��Ⱦ
		
		Stencil������������������һ������ֵ��Ҫ��Ⱦ��Ƭ�Ρ�
			����Ⱦ��ʱ�����Ǹı�stencil�����������ݣ�ʵ������д�����ݵ�stencil��������

		����ͬһ�ε���Ⱦ���������ǿ��Զ�ȡ��Щֵ����������ͨ��ָ����Ƭ�Ρ�

		ʹ��Stencil��Ҫע�����£�
			1.ʹ��Stencil������д��
			2.��Ⱦ���󣬸��»���������
			3.�ر�Stencil������д��
			4.��Ⱦ�������󣬸���Stencil�����������ݶ�����Ӧ��Ƭ��/����
		
		glStencilMask��
			�ú����������Ǹ�ģ��ֵ����һ������λ������ģ��ֵ���а�λ��(AND)������������Ƿ��д
			Ĭ�����������λȫΪ1���ʶ���Ӱ�������
				������ǽ�����λ����Ϊȫ0�������е�д��Stencil��������ֵ���ն���Ϊ0.�� �������е� glDepthMask(GL_FALSE) ��
		
		�����������ģ�����֣�stencil mask��дΪ0x00��0xFF���У��������֪����һ��ѡ������Զ���λ���֡�
		
*/

/*Stencil Function
	���Ƕ���һ��Stencil Test ��ʱͨ����ʧ�ܣ��Լ������Ӱ��Stencil Buffer ������ȷ�Ŀ��ơ�
	���������ƺ�����  ����Stencil 
		glStencilFunc(GLenum func, GLint ref, GLuint mask)
			������OpenGL��ģ�建����ʲô

				func:
					����ģ����Բ������ò��������ڱ����stencilֵ�Լ��ú�����refֵ��
					��ѡֵ�� ������depth������������
						GL_NEVER��
						GL_LESS��
						GL_LEQUAL��
						GL_GREATER��
						GL_GEQUAL;
						GL_EQUAL��
						GL_NOTEQUAL��
						GL_ALWAYS

				ref:
					ָ���� stencil test ������ֵ����ֵ�����ںͻ����������ݽ��бȽ�
				mask:
					ָ��һ��mask����ģ����ԶԱ�����ֵ�ʹ����ģ��ֵǰ��
						�����ǽ��а�λ�루and����������ʼ����Ϊ1
		eg��
				glStencilFunc(GL_EQUAL, 1, 0xFF)��
					����Ϊ��
					���Ƭ�ε�ģ��ֵ���ڣ�EQUAL��1����ͨ�����ԣ��������
				����GL_LESSͨ�������ҽ���
					����: ( ref & mask ) < ( stencil & mask ).
				GL_GEQUALͨ����
					���ҽ���( ref & mask ) >= ( stencil & mask )

		NOTE���ú�������������β���ģ�建��������ʱͨ����
		void glStencilOp(GLenum sfail, GLenum dpfail, GLenum dppass)
				sfail�� ���ģ�����ʧ�ܽ���ȡ�Ķ�����
				dpfail�� ���ģ�����ͨ����������Ȳ���ʧ��ʱ��ȡ�Ķ�����
				dppass�� �����Ȳ��Ժ�ģ����Զ�ͨ��������ȡ�Ķ���
			
			��������Щ������ѡ��ֵ��
				Action										Description
				GL_KEEP									������ǰ��ģ��ֵ
				GL_ZERO									��ģ��ֵ��Ϊ0
				GL_REPLACE								ģ��ֵ�����滻Ϊ����ֵ
				GL_INCR									��ģ��ֵ�������ֵ�ͽ�ģ��ֵ+1
				GL_INCR_WRAP							��GL_INCRһ��������һ���������ֵ��������Ϊ0
				GL_DECR									��ģ��ֵ������Сֵ�ͽ�ģ��ֵ-1
				GL_DECR_WRAP							��GL_DECRһ��������һ��Ϊ��Сֵ������Ϊ���ֵ��
				GL_INVERT								Bitwise inverts the current stencil buffer value.
		Ĭ������Ϊ (GL_KEEP, GL_KEEP, GL_KEEP) ��
			��ʾ�������κβ��Խ����������ֵ��Ĭ�ϲ�����ģ�建������
	ʹ�����������������ǿ��Ծ�ȷ�Ŀ��ƣ���ʱִ�к��ֲ���
				�Լ�������ͨ������ʧ�ܵ�ʱ�򣬺�ʱ����Ƭ�Ρ�
*/

/*	
	Object Outline
		Ϊÿһ�����帳��һ������ɫ�ıߡ�
		�����������Ϸ�е�ѡ��Ч����
			
		�������£�
			1.�ڻ��ƴ���������ǰ����Stencil��������ΪGL_ALWAYS��ÿ�������Ƭ����Ⱦʱ����1����Stencil��������
			2.��Ⱦ����
			3.�ر�ģ�建����д���Լ���Ȳ���
			4.ÿ������Ŵ�һ���
			5.ʹ��һ����ͬ��Ƭ����ɫ�������ɫ
			6.�ٴλ������壬��ֻ�ǵ����ǵ�Ƭ�ε�ģ��ֵ��Ϊ1ʱ�Ž���
			7.����ģ��д�����Ȳ���

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

void Stencil_Test(GLFWwindow* window);

void Stencil_DrawRectangle(GLFWwindow* window);

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
	Stencil_Test(window);
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

void DrawTwoCube();
void DrawScaleUpTwoCube();
void Stencil_Test(GLFWwindow* window) {
	//ʹ����Ȳ���
	glEnable(GL_DEPTH_TEST);
	//ʹ��ģ�����
	glDepthFunc(GL_LESS);//��Զͨ�����ԣ���ζ�������ƵĻ���Զ�������棬��˺Ͳ�����Ȳ��Ե�Ч��һ�¡�

	Shader shader("Shader4-2.vertex", "Shader4-2.fragment");
	Shader shaderSingleColor("Shader4-2.vertex","Shader4-2.fragment_border");

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
		glEnable(GL_STENCIL_TEST);
		glEnable(GL_DEPTH_TEST);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);//ͨ������������滻

		glClearColor(0.8f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glm::mat4 model;
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		
		
		
		glStencilMask(0x00);//�ر�ģ�建���д�롣----����д��

		shader.use();
		shader.setMartix("projection", projection);
		shader.setMartix("view", view);
		
		// floor
		glBindVertexArray(planeVAO);
		glBindTexture(GL_TEXTURE_2D, floorTexture);	//�л���ͼ
		shader.setMartix("model", glm::mat4());
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
		
	
		/*
		ȷ�����ӵ�ÿ��Ƭ����ģ��ֵ1����ģ�建�塣
		��ΪƬ���ܻ�ͨ��ģ����ԣ������ǻ������ǵĵط���ģ�建��������ֵ���¡�
		*/
		//Ϊ���ӵ����л��Ƶ�Ƭ�ε�ģ�建�����Ϊ1��
		glStencilMask(0xFF);	//����ģ�建��Ϊ��д״̬
		glStencilFunc(GL_ALWAYS, 1, 0xFF);	//����Ƭ�ζ�Ҫд��ģ�建��
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


		//���ƷŴ������
		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);//�����Ʋ���1�Ĳ��֡�
		glStencilMask(0x00); // ��ֹ�޸�ģ�建��
		glDisable(GL_DEPTH_TEST);	// ����ر���Ȳ��� ��Ϊ������������Ϊ����ǰ���ƽ�������ȥ

		shaderSingleColor.use();
		shaderSingleColor.setMartix("projection", projection);
		shaderSingleColor.setMartix("view", view);
		glBindVertexArray(cubeVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, cubeTexture);
		model = glm::mat4();
		model = glm::translate(model, glm::vec3(-1.0f, 0.0f, -1.0f));
		model = glm::scale(model, glm::vec3(1.1f, 1.1f, 1.1f));
		shaderSingleColor.setMartix("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		model = glm::mat4();
		model = glm::translate(model, glm::vec3(2.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.1f, 1.1f, 1.1f));
		shaderSingleColor.setMartix("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		
		glStencilMask(0xFF);
		glEnable(GL_DEPTH_TEST);
		//swap and poll
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteVertexArrays(1, &planeVAO);
	glDeleteBuffers(1, &cubeVBO);
	glDeleteBuffers(1, &planeVBO);
}

/*
	��Ⱦƽ�棬���˱�cube�ڵ��ĵط���
		1.�ر���ɫд��
		2.����ģ��д�룬����cube�Ӷ�������Ӧ��ģ�壨��Ӧλ������Ϊ1��
		3.������ɫд��
		4.�ر�ģ��д�룬����ƽ�棬������Ϊ�����Ʒ�1�Ĳ��֣�����cube��λ�ñ����ơ�
		5.����ģ��д�룬��֤֮���ˢ�»��������Խ��У���

*/
void Stencil_DrawRectangle(GLFWwindow* window) {
	//ʹ����Ȳ���
	glEnable(GL_DEPTH_TEST);
	//ʹ��ģ�����
	glDepthFunc(GL_LESS);//��Զͨ�����ԣ���ζ�������ƵĻ���Զ�������棬��˺Ͳ�����Ȳ��Ե�Ч��һ�¡�

	Shader shader("Shader4-2.vertex", "Shader4-2.fragment");
	Shader shaderSingleColor("Shader4-2.vertex", "Shader4-2.fragment_border");

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
		glEnable(GL_STENCIL_TEST);
		glEnable(GL_DEPTH_TEST);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);//ͨ������������滻

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glm::mat4 model;
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		
		
		shader.use();
		shader.setMartix("projection", projection);
		shader.setMartix("view", view);
	//section 1 ����ģ��
		
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		//Ϊ���ӵ����л��Ƶ�Ƭ�ε�ģ�建�����Ϊ1��
		glStencilMask(0xFF);	//����ģ�建��Ϊ��д״̬
		glStencilFunc(GL_ALWAYS, 1, 0xFF);	//����Ƭ�ζ�Ҫд��ģ�建��
											//cube
		glBindVertexArray(cubeVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, cubeTexture);
		model = glm::mat4();
		model = glm::translate(model, glm::vec3(-1.0f, 0.0f, -1.0f));
		shader.setMartix("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		model = glm::mat4();
		model = glm::translate(model, glm::vec3(2.0f, 0.0f, 0.0f));
		shader.setMartix("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);


	// section 2����ʵ�ʳ���
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glStencilMask(0x00); // ��ֹд��stencil
		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		// floor
		glBindVertexArray(planeVAO);
		glBindTexture(GL_TEXTURE_2D, floorTexture);	//�л���ͼ
		shader.setMartix("model", glm::mat4());
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);

		glStencilMask(0xFF);	
		//����ģ�建��Ϊ��д״̬,��������仰����������
		//����glClear���������ã���Ϊ��ֹд�룡�� 
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