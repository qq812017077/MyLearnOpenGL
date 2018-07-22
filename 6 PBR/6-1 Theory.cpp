#include <glad\glad.h>
#include <glfw3.h>

//��ѧ��
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <random>
#include <iostream>
#include "Shaders.h"
#include "Camera.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Model.h"

/*Theory  ����
	PBR�����߸�ͨ�׵ó�Ϊphysically based rendering������������Ⱦ������һ��һ���̶��ϻ�����ͬ�ķǳ��ӽ�
	��ʵ����Ļ������۵ļ������ϡ�	PBRĿ����Ϊ���Ը���������ķ�ʽ��ģ����ա�ͨ������������ԭ���Ĺ����㷨�����Ϲ��ա�Blinn-Phong��
	��������ʵ���������������ã����Ҹ�����ʵ������
	
	������ǣ�����������ʦ�ǣ�����ֱ�����������Ϊ��������д������ʣ��������������ӵ��޸���������ù���Ч������ȥ������
	������������������ʵ�һ���޴���������Щ���ʿ��������������Ķ����۹��չ���״�������ڷ�PBR����Ⱦ���ߵ�����Щ�����Ͳ�����ô��ʵ�ˡ�

	�������(onetheless)PBR��Ȼֻ��������ʵ������������ߣ������Ϊʲô������Ϊ������ɫ���ǻ���������ɫ��
	
	
	һ��PBR����ģ�ͱ���Ϊ�ǻ�����������Ҫ������������������
		1.����΢ƽ�棨microfacet������ģ�͡�
		2.�����غ㣨energy conserving��
		3. ʹ�û��������BRDF


	�ý̵̳�PBRΪ�����ɵ�ʿ��(Disney)���̽�ֲ���Epic Games����Ӧ����ʵʱ��Ⱦ��PBR������
		���ǻ��ڽ�����������metallic workflow���ķ����������Ƶļ�¼�������㷺Ӧ�����������棬Ч���ǳ����ˡ�
*/

/*The microfacet model ΢ƽ��ģ��
	���е�PBR����������΢ƽ�����ۣ�
		��������������΢�۳߶��£��κ��ܱ�΢С����ȫ���侵��������ƽ���Ϊ΢ƽ�档
	
	����ƽ��ֲڳ̶ȵĲ�ͬ����ЩϸС�����ȡ�����п����൱��һ�£�
		һ��ƽ��Խ�Ǵֲڣ�ƽ���ϵ�ÿ��΢ƽ������о�Խ�ǻ��ҡ���Щ΢С���������Ч������
	���ر��Ǿ������ʱ����������ڴֲ�ƽ���ϸ����ܳ��Ų�ͬ�ķ���ɢ���Ӷ�����һ�������ľ��淴�䡣
	�෴����һ����ƽ����ƽ�����������ܴ��Գ���ͬһ������ɢ��������һ����С��������ķ���Ч����

	��΢�۲����ϣ�û����ȫ�⻬�ı��棬Ȼ��������Щ΢ƽ���Ѿ�΢С���޷������صļ�������������֡�
	�������ֻ�м���һ���ֲڶ�(Roughness)������Ȼ����ͳ��ѧ�ķ��������ԵĹ���΢ƽ��Ĵֲڳ̶ȡ�

	����һ������Ĵֲڶȣ����ǿ��Լ����ĳ�������ķ�����΢ƽ��ƽ��ȡ����һ�µĸ��ʡ�
	����������ǰ��������λ�ڹ�������l����������v֮����м�����
	����������֮ǰ�ĸ߼����ս̳���̸�����м����������ļ��㷽�����£�
			h = ( l + v) / | l + v |  ���� h = ( normalize(l) + normalize(v) ) / 2
		
		΢ƽ���ȡ�������м������ķ���Խ��һ�£����淴���Ч����Խ��ǿ��Խ��������
	Ȼ���ټ���һ������0��1֮��ĴֲڶȲ������������Ǿ��ܸ��ԵĹ���΢ƽ���ȡ������ˣ�
	(�ϸߵĴֲڶ�ֵ��ʾ�����ľ��淴�������Ҫ����һЩ����֮�෴�أ���С�Ĵֲ�ֵ��ʾ���ľ��淴���������С��������)

*/

/* �����غ� Energy Conservation
	΢ƽ����Ʒ�ʹ���������غ����ʽ��
		�����Ĺ���������Զ�����������Ĺ�����������������⣩��
	���Ŵֲڶȵ��������淴������Ļ����ӣ����Ǿ��淴�������ȴ���½���
	�����ÿ�����صľ������ǿ����ͬ�����۾�����״�Ĵ�С�������ֲڵ�ƽ��ᷴ�������������Ӷ�Υ����
	�����غ㶨�ɡ������Ϊʲô���ǿ����ڹ⻬ƽ��ľ��淴���ǿ�����ֲ�ƽ��ĸ���
	
	Ϊ�����������غ㶨�ɣ�������Ҫ��������;��������һ�����������֡��ڹ�����ƽ���ʱ�������ֳ���
	���䲿�ֺͷ��䲿�֡�
		���䲿�֣���������ƽ���ڶ������䣬�⼴�Ǿ������
		���䲿�֣�ʣ�µĹ����ƽ��������ա��⼴��������⡣	������ ������������⣿

	������һЩϸ΢�����Ϊ������ڽӴ���ƽ��ʱ���ᱻ�������գ�
	ͨ������ѧ���ǿ��Ե�֪������ʵ���Ͽ��Ա���Ϊ��һ��û�кľ��Ͳ�ͣ��ǰ�˶���������
	��������ͨ����ײ�ķ�ʽ������������ÿһ�ֲ��ʶ�������΢С�Ŀ��������Ǻ͹�����ײ��
	��Щ����ͨ��ÿ����ײ���ղ��ֻ�ȫ�����ܣ����Ǳ�ת��Ϊ���ܡ�

	ͨ�����������еĹ��ն��ᱻ���գ�����ں�����΢����ײ���������������ɢ��ֱ�������ľ���depleted�������뿪ƽ�档
	���뿪ƽ��Ĺ��߽���Э���γɸñ���Ĺ۲죨�����䣩��ɫ����PBR�У����������˼򻯣����Ǽ����ƽ���ϵ�ÿһ��
	���е�����ⶼ�ᱻ��ȫ���ղ���ɢ��һ����С��ƽ���ڣ�����������Щ�뿪ƽ��һ������Ĺ��ߡ�

	һЩ�α���ɢ��(Subsurface Scattering)����ɫ���������俼�ǵ������У��������������һЩ���ʵ��Ӿ���������Ƥ��������ʯ������
	������������Ϊ���ۡ�

	���ڽ���(Metallic)���棬�����۵������������ʱ����һ��ϸ����Ҫע�⣺
		��������Թ�ķ�Ӧ��ǽ������ϣ�Ҳ��Ϊ�����(Dielectrics)����������ǲ�ͬ�ġ�
		����������ѭ��ͬ���ķ��������ԭ�򣬵������е�����ⶼ��ֱ�����ն�û�з�ɢ�������·������⡣
	�������治��ʾ�������䡣
		��Ϊ������Ե����𣬽����͵������PBR�ܵ��б��Բ�ͬ�ķ�ʽ�Դ���

	��������������������������һ�������غ�ľ����ܽ᣺�����ǻ����ų�ģ�mutually exclusive��
		���ۺ��ֹ��ߣ��䱻���ʱ�����������������޷��ٱ��������ա�
		��ˣ��������������ʣ�µĽ������֮�е��������þ��ǳ����������ʣ�µ�������

	���ǰ��������غ�Ĺ�ϵ�����ȼ��㾵�淴�䲿�֣�����ֵ����������߱������������ռ�İٷֱȡ�
	Ȼ������ⲿ�־Ϳ���ֱ���ɾ��淴�䲿�ּ���ó���
		float kS = calculateSpecularComponent(...); // ����/���� ����
		float kD = 1.0 - ks;                        // ����/������ ����
	ͨ�����ַ�ʽ�����Ǿ��ܻ�ȡ�����ķ��䲿�ֺ����䲿�֣�������ѭ�������غ㡣
		�������������������ۺϾͲ��ᳬ��1.0��
		
*/

/*The reflectance equation	���䷽��
	����������������һ�ֱ���Ϊ��Ⱦ����(Render Equation)�Ķ�����
		����ĳЩ���������������������һ�����elaborate���ķ���ʽ�����ǵ�ǰ����������ģ������Ӿ�����õ�ģ�͡�
	
	PBR�����ѭһ����Ⱦ���̵�ר�Ű汾���������䷽�̣�Reflectance equation��


*/

/*
--����------------------------------
*/
void framebuffer_size_callback(GLFWwindow * window, int width, int height) { glViewport(0, 0, width, height);	return; }
void mouse_callback(GLFWwindow* window, double xPos, double yPos);
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
void ProcessInput(GLFWwindow * window, float deltaTime);

void renderPlane(Shader shader);
void renderCube(Shader shader, std::vector<glm::vec3> cubePositions, std::vector<glm::vec3> cubeRotateAxis = std::vector<glm::vec3>());
void renderLight(Shader shader, std::vector<glm::vec3> lightPositions, std::vector<glm::vec3> lightColors = std::vector<glm::vec3>());
void RenderQuad(Shader shader);

unsigned int loadTexture(char const *path, unsigned int Tex_Wrap_Para = GL_REPEAT, bool gammaCorrection = false);

void SSAO_Test(GLFWwindow * window);
void SSAO_Gbuff_Test(GLFWwindow * window);

/*
--����------------------------------
*/
const int SCR_WIDTH = 1280;
const int SCR_HEIGHT = 720;

glm::vec3 cameraPos(0.0f, 0.0f, 5.0f);
glm::vec3 WorldUp(0.0f, 1.0f, 0.0f);

float yaw = -90.0f, pitch = 0.0f;
bool hdr = true;
bool hdrKeyPressed = false;
float exposure = 1.0f;

Camera camera(cameraPos, WorldUp, yaw, pitch);
int amount = 10;
int main(void) {
	if (!glfwInit()) {
		std::cout << "Initializing Failed" << std::endl;
	}

	//Set
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow * window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "5-1 Advanced Lighting", NULL, NULL);
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

	//glad
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		system("pause");
		return -1;
	}
	
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
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !hdrKeyPressed)
	{
		hdrKeyPressed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
	{
		hdrKeyPressed = false;
	}

	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
	{
		if (exposure > 0.0f)
			exposure -= 0.001f;
		else
			exposure = 0.0f;

	}
	else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
	{
		exposure += 0.001f;
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		if (amount > 2)
			amount -= 2;
	}
	else if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		if (amount < 20)
			amount += 2;
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
	//std::cout << "extract : " << extract << std::endl;
	camera.ProcessKeyboard(curMovement, deltaTime, false);
}

float cubeVertices[] = {
	// positions         	//Normal				 // texture Coords	
	-0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,		 0.0f, 0.0f,
	0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,		1.0f, 0.0f,
	0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,		1.0f, 1.0f,
	0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,		1.0f, 1.0f,
	-0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,		 0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,		 0.0f, 0.0f,

	-0.5f, -0.5f,  0.5f,   0.0f,  0.0f, 1.0f,		 0.0f, 0.0f,
	0.5f, -0.5f,  0.5f,   0.0f,  0.0f, 1.0f,		1.0f, 0.0f,
	0.5f,  0.5f,  0.5f,   0.0f,  0.0f, 1.0f,		1.0f, 1.0f,
	0.5f,  0.5f,  0.5f,   0.0f,  0.0f, 1.0f,		1.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,   0.0f,  0.0f, 1.0f,		 0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,   0.0f,  0.0f, 1.0f,		 0.0f, 0.0f,

	-0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,		 1.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,		 1.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,		 0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,		 0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,		 0.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,		 1.0f, 0.0f,

	0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,		1.0f, 0.0f,
	0.5f,  0.5f, -0.5f,   1.0f,  0.0f,  0.0f,		1.0f, 1.0f,
	0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,		0.0f, 1.0f,
	0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,		0.0f, 1.0f,
	0.5f, -0.5f,  0.5f,   1.0f,  0.0f,  0.0f,		0.0f, 0.0f,
	0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,		1.0f, 0.0f,

	-0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,		 0.0f, 1.0f,
	0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,		1.0f, 1.0f,
	0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,		1.0f, 0.0f,
	0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,		1.0f, 0.0f,
	-0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,		 0.0f, 0.0f,
	-0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,		 0.0f, 1.0f,

	-0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,		 0.0f, 1.0f,
	0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,		1.0f, 1.0f,
	0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,		1.0f, 0.0f,
	0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,		1.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,		 0.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,		 0.0f, 1.0f
};

float planeVertices[] = {
	// x-z ƽ��
	// positions         	//Normal			 // texture Coords	
	-4.0f, -1.0f, -4.0f,	0.0f,  1.0f, 0.0f,		0.0f, 5.0f,
	4.0f, -1.0f,  4.0f,		0.0f,  1.0f, 0.0f,		5.0f, 0.0f,
	4.0f, -1.0f, -4.0f,		0.0f,  1.0f, 0.0f,		5.0f, 5.0f,

	-4.0f, -1.0f, -4.0f,	0.0f,  1.0f, 0.0f,		0.0f, 5.0f,
	-4.0f, -1.0f,  4.0f,	0.0f,  1.0f, 0.0f,		0.0f, 0.0f,
	4.0f, -1.0f,  4.0f,		0.0f,  1.0f, 0.0f,		5.0f, 0.0f,

	// x-y ƽ��
	/*-2.0f,	-2.0f, -1.0f,	0.0f, 2.0f,			0.0f,  0.0f, 1.0f,
	2.0f,	 2.0f, -1.0f,	2.0f, 0.0f,			0.0f,  0.0f, 1.0f,
	2.0f,	-2.0f, -1.0f,	2.0f, 2.0f,			0.0f,  0.0f, 1.0f,

	-2.0f,	-2.0f, -1.0f,	0.0f, 2.0f,			0.0f,  0.0f, 1.0f,
	-2.0f,	 2.0f, -1.0f,	0.0f, 0.0f,			0.0f,  0.0f, 1.0f,
	2.0f,	 2.0f, -1.0f,	2.0f, 0.0f,			0.0f,  0.0f, 1.0f*/
};




static unsigned int cubeVAO, cubeVBO;
static unsigned int planeVAO, planeVBO;
static unsigned int quadVAO, quadVBO;
static bool CubeisGenerated = false;
static bool PlaneisGenerated = false;
static bool QuadisGenerated = false;
void renderPlane(Shader shader) {
	if (!PlaneisGenerated) {
		//VAO , VBO
		//Cube
		glGenVertexArrays(1, &planeVAO);
		glGenBuffers(1, &planeVBO);
		// fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
		// link vertex attributes
		glBindVertexArray(planeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);		//��������
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));	//����������
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));	//������������
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		PlaneisGenerated = true;
	}
	shader.use();
	glBindVertexArray(planeVAO);
	glm::mat4 model = glm::mat4();
	model = glm::scale(model, glm::vec3(3.0f, 1.0f, 3.0f));
	shader.setMartix("model", model);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

}
void renderCube(Shader shader, std::vector<glm::vec3> cubePositions, std::vector<glm::vec3> cubeRotateAxis) {
	if (!CubeisGenerated) {	//������û�����ɣ�������������
							//VAO , VBO
							//Cube
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		// fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
		// link vertex attributes
		glBindVertexArray(cubeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);		//��������
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));	//����������
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));	//������������
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		CubeisGenerated = true;
	}
	shader.use();
	glBindVertexArray(cubeVAO);

	for (int i = 0; i < cubePositions.size(); ++i) {
		glm::mat4 model = glm::mat4();
		model = glm::translate(model, cubePositions[i]);
		if (!cubeRotateAxis.empty())
			model = glm::rotate(model, glm::radians(i * 15.0f), cubeRotateAxis[i]);
		model = glm::scale(model, glm::vec3(10.0f, 10.0f, 10.0f));
		shader.setMartix("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
	glBindVertexArray(0);

}

void renderLight(Shader shader, std::vector<glm::vec3> lightPositions, std::vector<glm::vec3> lightColors) {
	if (!CubeisGenerated) {	//������û�����ɣ�������������
							//VAO , VBO
							//Cube
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		// fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
		// link vertex attributes
		glBindVertexArray(cubeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);		//��������
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));	//����������
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));	//������������
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		CubeisGenerated = true;
	}
	//light
	for (unsigned int i = 0; i < lightPositions.size(); i++) {
		glm::mat4 model = glm::mat4();
		model = glm::translate(model, lightPositions[i]);
		model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));
		shader.setMartix("model", model);
		if (!lightColors.empty())
			shader.setVec3("cubeColor", lightColors[i]);
		else
			shader.setVec3("cubeColor", glm::vec3(1.0f, 1.0f, 1.0f));
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
	}
}

void RenderQuad(Shader shader) {
	if (!QuadisGenerated) {
		float quadVertices[] = {
			// positions			// texture Coords
			-1.0f,  1.0f, 0.0f,		0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f,		0.0f, 0.0f,
			1.0f,  1.0f, 0.0f,		1.0f, 1.0f,
			1.0f, -1.0f, 0.0f,		1.0f, 0.0f,
		};
		// setup plane VAO

		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		glBindVertexArray(0);

		QuadisGenerated = true;
	}

	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}
unsigned int loadTexture(char const *path, unsigned int Tex_Wrap_Para, bool gammaCorrection)
{
	/*
	Gamma�������������ڣ�
	�����ȡ������ɫ���ݲ�����ʵ������ʾ���������ݣ�
	������ʾ����ɫΪCrgb,�����ﱣ�����ɫӦΪ Crgb^(1/2.2)��
	���������κδ���֮�����һ��gamma��������Ϊ Crgb^(1/2.2)^2,�ڽ���gamma���ֻص���Crgb^(1/2.2)�����Ի����
	�����������Crgb����֮�����һ��gamma��������Ϊ Crgb^(1/2.2),�ڽ���gamma���ֻص���Crgb�������ͱ��������Թ�ϵ
	*/
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum internalFormat;
		GLenum dataFormat;
		if (nrComponents == 1) {
			internalFormat = dataFormat = GL_RED;
		}
		else if (nrComponents == 3) {
			internalFormat = gammaCorrection ? GL_SRGB : GL_RGB;
			dataFormat = GL_RGB;
		}
		else if (nrComponents == 4) {
			internalFormat = gammaCorrection ? GL_SRGB_ALPHA : GL_RGBA;
			dataFormat = GL_RGBA;
		}

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
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

void Model_Test(GLFWwindow * window) {
	glEnable(GL_DEPTH_TEST);
	Shader geometryPass_Shader("Shader5-8.vertex_gbuffer", "Shader5-9.fragment_SSAO_gbuffer");
	//���׻�����
	char model_path[] = "E:/C++/OpenGL_Course/OpenGL_Course/Model/nanosuit/nanosuit.obj";
	Model nanosuitModel(model_path);


	std::vector<glm::vec3> objectPositions;
	objectPositions.push_back(glm::vec3(-3.0, -3.0, -3.0));
	objectPositions.push_back(glm::vec3(0.0, -3.0, -3.0));
	objectPositions.push_back(glm::vec3(3.0, -3.0, -3.0));
	objectPositions.push_back(glm::vec3(-3.0, -3.0, 0.0));
	objectPositions.push_back(glm::vec3(0.0, -3.0, 0.0));
	objectPositions.push_back(glm::vec3(3.0, -3.0, 0.0));
	objectPositions.push_back(glm::vec3(-3.0, -3.0, 3.0));
	objectPositions.push_back(glm::vec3(0.0, -3.0, 3.0));
	objectPositions.push_back(glm::vec3(3.0, -3.0, 3.0));

	float curTime{}, lastTime{};
	curTime = lastTime = glfwGetTime();
	while (!glfwWindowShouldClose(window)) {
		//Process Input
		curTime = glfwGetTime();
		ProcessInput(window, curTime - lastTime);
		lastTime = curTime;

		//Draw
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 projection = glm::perspective(camera.Zoom, (GLfloat)SCR_WIDTH / (GLfloat)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 model;
		geometryPass_Shader.use();
		geometryPass_Shader.setMartix("projection", projection);
		geometryPass_Shader.setMartix("view", view);
		geometryPass_Shader.setBool("inverse_normals", false);
		for (GLuint i = 0; i < objectPositions.size(); i++) {
			model = glm::mat4();
			model = glm::translate(model, objectPositions[i]);
			model = glm::scale(model, glm::vec3(0.25f));
			geometryPass_Shader.setMartix("model", model);
			nanosuitModel.Draw(geometryPass_Shader);
		}
		//swap and poll
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}