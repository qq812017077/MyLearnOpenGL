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

/*	ע�����߿ռ䣺TBN
	T ��Ӧ �����U��
	B ��Ӧ �����V��
	N ��Ӧ ����Ĵ�ֱ����һ���뷨�����

	TBN�ռ�һ��������������ı��ؿռ䣬���ڽ�����ӱ��ؿռ�ת���������ڵ�����ռ�

	Ҫ����һ��������Ƭ�ε�TBN��
	������Ҫ֪����������εĶ���λ�ã��Լ���UV���꣡Ȼ�󷽿ɼ���õ����߿ռ䡣
*/

/*
	����������������˺ܶ��ϸ�ڣ����ǵ���������ʱ�򣬱����ֺܶ��©����
	���磬����ǽ��������
		������û�취���Ӱ��ݸеġ�ǽ���ǲ�����ƽ̹�ġ�

	�Թ���ӽ�����������⣺��ʲôʹ���汻��Ϊ��ȫƽ̹�ı������������𰸻��Ǳ���ķ���������

	����ÿ��fragmentʹ�ø��Եķ��ߣ����һ����������fragmentʹ��ͬһ�����ߵļ�������������ͼ��normal mapping��
	��͹��ͼ��bump mapping����

	��diffuse��specularһ����������ͼҲ��ʹ��2D���������档
	���ڷ��������Ǹ����ι��ߣ�������ͨ��ֻ���ڴ�����ɫ��Ϣ���������淨���������Ƿǳ�ֱ�ӡ�

		�����е���ɫ��������ʾ����3D��������r,g,b
		�������Ƶı����˷�������x��y��z
		���������ķ�Χ��-1��1֮�䣬����������Ҫ����ӳ�䵽0��1�ķ�Χ��
			vec3 rgb_normal = normal * 0.5 + 0.5 ; // �� [-1,1] ת���� [0,1]
		����������ת������������RGB���֮�����Ǿ��ܸ��ݱ������״��fragment�ķ��߱�����2D�����С�

		�󲿷ֵķ�������ͼ����ƫ��ɫ�ģ�������Ϊ�󲿷ֵķ�������ָ��z�᷽�򣺣�0,0,1��--->��ɫ
		��ɫ��΢��ƫ�ƴ����˷���������΢ƫ�ƣ�ƫ��z�������򣩡�

*/

/*	
	Ӧ�÷�������ͼ��ʱ�������һ�����⣺�������ķ�����ָ��z��ģ�
	����ƽ�泯��z���ʱ��û�е��壬���ǵ�ƽ�滻��һ�������ʱ��ͳ��������ˣ�
		
		�취һ��
			��¼ƽ�������еı任���Է���������ͬ���ı任��
		�취����
			��һ����ͬ������ռ��н��й��գ����������ռ��У���������ͼ��Զ����ָ��z������
		���������Ĺ���������ת���������z������ķ����������Ǿ�������ʹ��ͬһ��normalMap��
		�������ռ����tangent space -- ���߿ռ�

*/

/*tangent space ���߿ռ�
	�ڷ�����ͼ�еķ������������ݵ�tangent space�У������﷨��ͨ������ָ��z������
	Tangent�ռ���һ��λ�������α���Ŀռ䣺��������ڵ��������εı��زο�ϵ��ܡ�
	���԰������ǳɷ�����ͼ�����ı��ؿռ䡣��������ת����ʲô������������ȫ��������Ϊָ��z�᷽��
	ͨ��ʹ��һ���ض��ľ���
		���ǾͿ��Խ��������ӱ��ص�Tangent �ռ�ת��������������߹۲������У�
	����
		ʹ����ת�����յ���ͼ����ķ���

	Tangent�ռ��һ��ô��ǣ����ǿ���Ϊ�κ����͵ı����������һ������ʹ��
	�����ܹ�ǡ���ô��Ľ����߿ռ��z�᷽����뵽���淨��������

	���־����Ϊ��
		TBN���󣺷ֱ��ʾ�� tangent �� bitangent , normal������λ������
	tangent ���ߣ�   bitangent �����ߣ�   normal ����
	
	Ҫ��������һ�������߿ռ�ת�䵽��ͬ������ռ������һ���������
	������Ҫ�����໥��ֱ��������
		������һ��������ͼ�ı�������ڣ��ϡ��ҡ�ǰ�����������������̳����������ơ�
	
	���У�
		��������Up������Ҳ������ķ�������
		��������������
		ǰ��������������
	
	�������븱�������ļ��㣺
		������ͼ�����ߺ͸��������������������������룺 һ������U�� һ������V
		����������ȡ�������ڲ�ͬ�ı��ϡ�
			�ֱ�Ϊ�� P1(U1,V1)��P2(U2,V2)��P3(U3,V3) �����㹹����������
			��E1 = P1 - P2			E2 = P2 - P3
			E1=��U1T+��V1B
			E2=��U2T+��V2B
		ע��T��tangent  �� B: bitangent ���ǵ�λ����������	��U	��V ��ʾ���ȣ�T �� B��ʾ����
		�����У�
			(E1x , E1y, E1z) = ��U1(Tx,Ty,Tz)+��V1(Bx,By,Bz)
			(E2x , E2y, E2z) = ��U2(Tx,Ty,Tz)+��V2(Bx,By,Bz)
		| E1x  E1y  E1z | = | ��U1 ��V1 | | Tx  Ty  Tz |
		| E2x  E2y  E2z |	| ��U2 ��V2 | | Bx  By  Bz |
		�ѵ�ʽд�ɾ�����ʽ�ĺô��ǣ���T��B����˱�ú����ס����߶����Ԧ�U��V���������ڣ�
		| Tx  Ty  Tz | =	| ��U1 ��V1 |-1	| E1x  E1y  E1z |
		| Bx  By  Bz |		| ��U2 ��V2 |	| E2x  E2y  E2z |
		����Ҫ�����������������ǻ�����Ҫ֪��һЩ��Ϣ��
				�������㣬��U �� ��V

		�������ǾͿ����ù�ʽ�������ε��������Լ���������������������T�͸�����B

*/

/*�ֹ��������ߺ͸�����
	��ʹ������Ĺ�ʽ����ÿ�������ε�����/�����ߡ�
		
		ֻ��Ϊÿ�������μ���һ������/������(��Ϊ�������������ƽ��)�����Ƕ���ÿ���������ϵĶ��㶼��һ���ġ�
		
	Ҫע����ǣ�
		�����ʵ��ͨ�������κ�������֮�䶼�Ṳ���㡣��������¿�����ͨ����ÿ��
	����ķ��ߺ�����/�����ߵȶ�������ƽ�������Ի�ø�����͵�Ч�������ǵ�ƽ���������
	֮�������һЩ���㣬������Ϊ�����������໥���У���˲�����Ҫ�����ƽ�����������ۺ�
	ʱֻҪ���������������ס�����Ǽ����¡�

	�������ߺ͸�����������ֵӦ����(1, 0, 0)��(0, 1, 0)�����Ǻͷ���(0, 0, 1)����໥��ֱ��TBN����
	��ƽ������ʾ����TBNӦ���������ģ�

*/

/*Tangent space normal mapping ���߿ռ䷨����ͼ
	Ҫ��÷�����ͼ��������Ϊ����һ��TBN��������ɫ���С�
	�����Ƚ�ǰ�������������ߺ͸�������������������ɫ������Ϊ��������
	��ע��ÿ�������Σ�����û�ж��㶼�ж��������ߺ͸����ߣ����Ӧ��Ϊ���Խ��д��ݣ�
	��������
		#version 330 core
		layout (location = 0) in vec3 aPos;
		layout (location = 1) in vec3 aNormal;
		layout (location = 2) in vec2 aTexCoords;
		layout (location = 3) in vec3 aTangent;
		layout (location = 4) in vec3 aBitangent; 

	1.�������Ƚ����е�TBN����ת����������Ҫ����������ϵͳ�У��ڱ��ΰ��������ǽ���ת��������ռ��У�ͨ��model��
	2.���Ǵ���һ��ʵ�ʵ�TBN����ֱ�Ӱ���Ӧ������Ӧ�õ�mat3���������С�
		ע�����������Ҫ��ȷ�Ľ�������Ǿ�ʹ�÷��߾���������Ϊ�������ƽ�ƻ������Ŵ�������������Ӱ�졣
		��
			�Ӽ����Ͻ���������ɫ�������踱���ߡ����е�������TBN���������໥��ֱ��
		�������ǿ����ڶ�����ɫ����ʹ��T��N�����Ĳ�ˣ��Լ�����������ߣ�vec3 B = cross(T, N);
		��
	3.TBN��ʹ�ã�
		����������ʹ�÷�ʽ��������ת��������ռ��У� ���߽����մ�����ռ�ת�������߿ռ䣩
			1.����ֱ��ʹ��TBN�������������԰���������ռ������ת������������ռ䡣
		������ǰ�������Ƭ����ɫ���У���ͨ�������õ��ķ������������TBN����ת��������
		����ռ��У��������з��ߺ��������ձ�������ͬһ������ϵ���ˡ�
			
			����������������Ǵӷ�����ͼ�вɼ��ķ������������������߿ռ��У��������Ĺ�����������������ռ��С�
		ͨ����TBN���󴫵ݸ�fragment��ɫ�������ǿ��Խ��ɼ��ķ�������TBN������ˣ���ת�����͹�����ͬ���Ĳο�ϵ�ռ��С�

			2.����Ҳ����ʹ��TBN�������������������԰���������ռ������ת������������ռ䡣
	�������ʹ�������������������ձ�����������ת�������߿ռ䣬�������ߺ��������ձ�����һ����һ������ϵ���ˡ�
	��ע�⣺
		��������ÿ������ǵ�λ����ͬʱ�໥��ֱ����һ��������һ������������û�������������������
		��ǳ���Ҫ��
			��Ϊ��������ñ����û������󣻽��ȴ��һ����
	��
			��������ɫ�������ǲ��öԷ��������任��������Ҫ�������������ת�������߿ռ䣬
		������lightDir��viewDir������ÿ������������ͬһ���ռ䣨���߿ռ䣩���ˣ�
				
			shader��
				vs_out.TBN = transpose(mat3(T, B, N));

			fragment��
				vec3 normal = texture(normalMap, fs_in.TexCoords).rgb;
				normal = normalize(normal * 2.0 - 1.0);   

				vec3 lightDir = fs_in.TBN * normalize(lightPos - fs_in.FragPos);
				vec3 viewDir  = fs_in.TBN * normalize(viewPos - fs_in.FragPos);	
		
		�÷����ж���ĺô���
			���ǿ��԰����е���ر����ڶ�����ɫ����ת�������߿ռ䣬��������������ɫ���С�
		���ǿ��еģ���ΪlightPos��viewPos����ÿ��fragment���ж�Ҫ�ı䣬
		����fs_in.FragPos������Ҳ�����ڶ�����ɫ�������������߿ռ�λ�á�
		�����ϣ�����Ҫ���κ�������������ɫ���н��б任������һ�ַ����о��Ǳ���ģ�
		��Ϊ���������ķ�����������ÿ��������ɫ������һ����

		�������ڲ��ǰ�TBN�����������͸�������ɫ�������ǽ����߿ռ�Ĺ�Դλ�ã�
		�۲�λ���Լ�����λ�÷��͸�������ɫ�����������ǾͲ�����������ɫ������о���˷��ˡ�
		����һ�����ѵ��Ż�����Ϊ������ɫ��ͨ����������ɫ�����е��١�

		eg:
			shader:
				out VS_OUT {
					vec3 FragPos;
					vec2 TexCoords;
					vec3 TangentLightPos;
					vec3 TangentViewPos;
					vec3 TangentFragPos;
				} vs_out;

				uniform vec3 lightPos;
				uniform vec3 viewPos;
				
				void main()
				{
				[...]
					mat3 TBN = transpose(mat3(T, B, N));
					vs_out.TangentLightPos = TBN * lightPos;
					vs_out.TangentViewPos  = TBN * viewPos;
					vs_out.TangentFragPos  = TBN * vec3(model * vec4(position, 0.0));
				}
			fragment:
				��������ɫ��������ʹ����Щ�µ�����������������߿ռ�Ĺ��ա���Ϊ���������Ѿ������߿ռ����ˣ����վ��������ˡ�

*/

/*Complex objects
	���ڸ������壬�����������ߺ͸����ߵļ��㻹��ͨ��ʹ��ģ�ͼ�����ʵ�ֵģ�
	Assimp�и������õ����ã������Ǽ���ģ�͵�ʱ�����aiProcess_CalcTangentSpace��
	��aiProcess_CalcTangentSpaceӦ�õ�Assimp��ReadFile����ʱ��Assimp��Ϊÿ������
	�Ķ���������͵����ߺ͸���������������ʹ�õķ��������Ǳ��̳�ʹ�õ����ơ�

	const aiScene* scene = importer.ReadFile(
	path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace
	);
	����ͨ������Ĵ�����Assimp��ȡ������������߿ռ䣺
		vector.x = mesh->mTangents[i].x;
	Ȼ�󻹱������ģ�ͼ����������ԴӴ�����ģ���м��ط�����ͼ��
	wavefront��ģ�͸�ʽ��.obj�������ķ�����ͼ�е㲻һ����Assimp��
	aiTextureType_NORMAL������������ķ�����ͼ����aiTextureType_HEIGHTȴ�ܣ��������Ǿ��������������ǣ�

			vector<Texture> specularMaps = this->loadMaterialTextures(
			material, aiTextureType_HEIGHT, "texture_normal"
			);
	�˽�aiProcess_CalcTangentSpace���������ǺܺõĹ���Ҳ����Ҫ��
	������������Ҫ������������ģ���Щģ��������ʹ��һЩ����С���ɱ��羵��һ��ģ���ϵ��������ʱ
	Ҳ��������һ����������ꣻ���������������������ر������ʱ��Assimp�Ͳ����ǣ�����Ͳ����ˡ�

*/

/*���һ����
	���ڸ���������ϼ�������������ʱ�����������кܴ������Ĺ����㣬
	��������ͼӦ�õ���Щ����ʱ����������ƽ����ͨ���ܻ�ø��ø�ƽ���Ľ����
	�������и����⣬����TBN�������ܻ᲻�ܻ��ഹֱ������ζ��TBN�����������������ˡ�
	������ͼ���ܻ�����ƫ�ƣ�������Ȼ���ԸĽ���

	ͨ������ķ-ʩ�������������̣�Gram-Schmidt process������ѧ���ɣ���TBN������������������
	����ÿ���������ֻ����´�ֱ�ˡ��ڶ�����ɫ����������������
		vec3 T = normalize(vec3(model * vec4(tangent, 0.0)));
		vec3 N = normalize(vec3(model * vec4(normal, 0.0)));
		// re-orthogonalize T with respect to N
		T = normalize(T - dot(T, N) * N);
		// then retrieve perpendicular vector B with the cross product of T and N
		vec3 B = cross(T, N);

		mat3 TBN = mat3(T, B, N)

*/
/*
--����------------------------------
*/
void framebuffer_size_callback(GLFWwindow * window, int width, int height) { glViewport(0, 0, width, height);	return; }
void mouse_callback(GLFWwindow* window, double xPos, double yPos);
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
void ProcessInput(GLFWwindow * window, float deltaTime);


unsigned int loadTexture(char const *path, unsigned int Tex_Wrap_Para = GL_REPEAT);
//Brick Wall Normal Test
void BrickWall_NormalTest(GLFWwindow * window);

void TangentSpace(GLFWwindow * window);
/*
--����------------------------------
*/
const int SCR_WIDTH = 800;
const int SCR_HEIGHT = 600;

glm::vec3 cameraPos(-1.0f, 0.0f, 2.0f);
glm::vec3 WorldUp(0.0f, 1.0f, 0.0f);

float yaw = -90.0f, pitch = 0.0f;
bool openEffect = true;
bool functoinExchange = true;
int lastStatus;
int Exchange_lastStatus;
Camera camera(cameraPos, WorldUp, yaw, pitch);

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
	TangentSpace(window);
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
		if (openEffect) {
			openEffect = false;
			//glDisable(GL_MULTISAMPLE);
		}
		else {
			openEffect = true;
			//glEnable(GL_MULTISAMPLE);
		}
	}

	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE && Exchange_lastStatus == GLFW_PRESS) {
		//ʹ��B�������л���ɫ����
		if (functoinExchange) {
			functoinExchange = false;
		}
		else {
			functoinExchange = true;
		}
	}
	lastStatus = glfwGetKey(window, GLFW_KEY_SPACE);
	Exchange_lastStatus = glfwGetKey(window, GLFW_KEY_B);
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
	// x-z ƽ��
	// positions          // texture Coords		//Normal
	/*-4.0f, -1.0f, -4.0f,	0.0f, 5.0f,			0.0f,  1.0f, 0.0f,
	4.0f, -1.0f,  4.0f,		5.0f, 0.0f,			0.0f,  1.0f, 0.0f,
	4.0f, -1.0f, -4.0f,		5.0f, 5.0f,			0.0f,  1.0f, 0.0f,

	-4.0f, -1.0f, -4.0f,	0.0f, 5.0f,			0.0f,  1.0f, 0.0f,
	-4.0f, -1.0f,  4.0f,	0.0f, 0.0f,			0.0f,  1.0f, 0.0f,
	4.0f, -1.0f,  4.0f,		5.0f, 0.0f,			0.0f,  1.0f, 0.0f,
	*/
	// x-y ƽ��
	-2.0f,	-2.0f, -1.0f,	0.0f, 2.0f,			0.0f,  0.0f, 1.0f,
	2.0f,	 2.0f, -1.0f,	2.0f, 0.0f,			0.0f,  0.0f, 1.0f,
	2.0f,	-2.0f, -1.0f,	2.0f, 2.0f,			0.0f,  0.0f, 1.0f,
													    	  
	-2.0f,	-2.0f, -1.0f,	0.0f, 2.0f,			0.0f,  0.0f, 1.0f,
	-2.0f,	 2.0f, -1.0f,	0.0f, 0.0f,			0.0f,  0.0f, 1.0f,
	2.0f,	 2.0f, -1.0f,	2.0f, 0.0f,			0.0f,  0.0f, 1.0f
};

void BrickWall_NormalTest(GLFWwindow * window) {
	glEnable(GL_DEPTH_TEST);
	//Shader
	Shader light_Shader("Shader.vertex_lamp", "Shader.fragment_lamp");
	Shader shader("Shader5-4.vertex", "Shader5-4.fragment");
	//Texture
	unsigned int brickwallTexture = loadTexture("brickwall.jpg");
	unsigned int normalTexture = loadTexture("brickwall_normal.jpg");
	//VAO , VBO
	//plane
	unsigned int planeVAO, planeVBO;
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, 8 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(5 * sizeof(float)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);


	//light
	glm::vec3 lightPosition(0.0f, 0.0f, 3.0f);

	unsigned int lightVAO, lightVBO;
	glGenVertexArrays(1, &lightVAO);
	glGenBuffers(1, &lightVBO);
	glBindVertexArray(lightVAO);
	glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, 8 * sizeof(float), (void *)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//Shader��������
	shader.use();
	
	shader.setVec3("light.ambient", glm::vec3(0.2f, 0.2f, 0.2f));
	shader.setVec3("light.diffuse", glm::vec3(0.8f, 0.8f, 0.8f));
	shader.setVec3("light.specular", glm::vec3(0.6f, 0.6f, 0.6f));
	shader.setFloat1f("light.constant", 1.0f);
	shader.setFloat1f("light.linear", 0.09f);
	shader.setFloat1f("light.quadratic", 0.032f);
	shader.setInt("material.diffuse", 0);
	shader.setInt("material.specular", 1);
	shader.setInt("material.normalMap",2);
	shader.setFloat1f("material.shininess", 16.0f);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, brickwallTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, brickwallTexture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, normalTexture);
	float curTime{}, lastTime{};
	curTime = lastTime = glfwGetTime();
	while (!glfwWindowShouldClose(window)) {
		//Process Input
		curTime = glfwGetTime();
		ProcessInput(window, curTime - lastTime);
		lastTime = curTime;
		//Draw
		lightPosition.z = (sin(glfwGetTime()) + 1.0f) / 2.0 ;
		glm::mat4 model;
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//plane
		shader.use();
		model = glm::mat4();
		//model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		//model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));
		shader.setVec3("light.position", lightPosition);
		shader.setMartix("model", model);
		shader.setMartix("normalMatrix", glm::inverse(glm::transpose(model)));
		shader.setMartix("view", view);
		shader.setMartix("projection", projection);
		shader.setVec3("viewerPos", camera.Position);
		shader.setBool("openEffect", openEffect);
		glBindVertexArray(planeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		//light
		light_Shader.use();
		model = glm::mat4();
		model = glm::translate(model, lightPosition);
		model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
		light_Shader.setMartix("model", model);
		light_Shader.setMartix("view", view);
		light_Shader.setMartix("projection", projection);
		glBindVertexArray(lightVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		//swap and poll
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

void TangentSpace(GLFWwindow * window) {
	glEnable(GL_DEPTH_TEST);
	//Shader
	Shader light_Shader("Shader.vertex_lamp", "Shader.fragment_lamp");
	Shader shader("Shader5-4.vertex", "Shader5-4.fragment");
	//Texture
	unsigned int brickwallTexture = loadTexture("brickwall.jpg");
	unsigned int normalTexture = loadTexture("brickwall_normal.jpg");
	
	
	//���߿ռ�Ĵ���
	// positions----ƽ����ĸ�����				   1(0,1)__________4(1,1)
	glm::vec3 pos1(-2.0, 2.0, 0.0);		//����		|		   |
	glm::vec3 pos2(-2.0, -2.0, 0.0);		//����		|		   |
	glm::vec3 pos3(2.0, -2.0, 0.0);		//����		|		   |
	glm::vec3 pos4(2.0, 2.0, 0.0);		//����	    |__________|
										// texture coordinates								2(0,0)	   3(1,0)
	glm::vec2 uv1(0.0, 2.0);
	glm::vec2 uv2(0.0, 0.0);
	glm::vec2 uv3(2.0, 0.0);
	glm::vec2 uv4(2.0, 2.0);
	// normal vector
	glm::vec3 normal(0.0, 0.0, 1.0);

	//step1:��һ�������εı� �� ��UV
	glm::vec3 edge1 = pos2 - pos1;	//1 -> 2
	glm::vec3 edge2 = pos3 - pos1;	//1 -> 3
	glm::vec2 deltaUV1 = uv2 - uv1;
	glm::vec2 deltaUV2 = uv3 - uv1;
	//ע������ı����з���ģ����Ƿ�����ι�ϵ����ֻҪDelta��֮��Ӧ�ͺ��ˡ�
	/*��ʽ��
	| Tx  Ty  Tz | =	| ��U1 ��V1 |-1	| E1x  E1y  E1z |
	| Bx  By  Bz |		| ��U2 ��V2 |	| E2x  E2y  E2z |
	*/
	//���㷽��1��

	float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
	glm::vec3 tangent1;
	tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
	tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
	tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
	tangent1 = glm::normalize(tangent1);

	glm::vec3 bitangent1;
	bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
	bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
	bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
	bitangent1 = glm::normalize(bitangent1);

	//���㷽��2
	/*glm::mat2x2 uv(deltaUV1, deltaUV2);
	glm::mat2x3 edge(edge1, edge2);
	glm::mat2x3 result = edge * glm::inverse(uv) ;
	tangent = result[0];
	bitangent = result[1];*/

	//step2:�ڶ��������εı� �� ��UV ---ʵ���ϣ�����Ĳ����м���Ҳ�У���Ϊ����ƽ������ߺ�
	edge1 = pos3 - pos1;	//1 -> 3
	edge2 = pos1 - pos4;	//4 -> 1
	deltaUV1 = uv3 - uv1;
	deltaUV2 = uv1 - uv4;

	f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
	glm::vec3 tangent2;
	tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
	tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
	tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
	tangent2 = glm::normalize(tangent1);

	glm::vec3 bitangent2;
	bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
	bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
	bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
	bitangent2 = glm::normalize(bitangent1);

	GLfloat quadVertices[] = {
		// Positions            // normal						// TexCoords  // Tangent                         // Bitangent
		pos1.x, pos1.y, pos1.z, normal.x, normal.y, normal.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
		pos2.x, pos2.y, pos2.z, normal.x, normal.y, normal.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
		pos3.x, pos3.y, pos3.z, normal.x, normal.y, normal.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

		pos1.x, pos1.y, pos1.z, normal.x, normal.y, normal.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
		pos3.x, pos3.y, pos3.z, normal.x, normal.y, normal.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
		pos4.x, pos4.y, pos4.z, normal.x, normal.y, normal.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
	};
	//VAO , VBO
	//plane
	unsigned int planeVAO, planeVBO;
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, 14 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void *)(6 * sizeof(float)));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void *)(8 * sizeof(float)));
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void *)(11 * sizeof(float)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);


	//light
	glm::vec3 lightPosition(0.0f, 0.0f, 3.0f);

	unsigned int lightVAO, lightVBO;
	glGenVertexArrays(1, &lightVAO);
	glGenBuffers(1, &lightVBO);
	glBindVertexArray(lightVAO);
	glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, 8 * sizeof(float), (void *)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//Shader��������
	shader.use();

	shader.setVec3("light.ambient", glm::vec3(0.2f, 0.2f, 0.2f));
	shader.setVec3("light.diffuse", glm::vec3(0.8f, 0.8f, 0.8f));
	shader.setVec3("light.specular", glm::vec3(0.6f, 0.6f, 0.6f));
	shader.setFloat1f("light.constant", 1.0f);
	shader.setFloat1f("light.linear", 0.09f);
	shader.setFloat1f("light.quadratic", 0.032f);
	shader.setInt("material.diffuse", 0);
	shader.setInt("material.specular", 1);
	shader.setInt("material.normalMap", 2);
	shader.setFloat1f("material.shininess", 16.0f);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, brickwallTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, brickwallTexture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, normalTexture);
	float curTime{}, lastTime{};
	curTime = lastTime = glfwGetTime();
	while (!glfwWindowShouldClose(window)) {
		//Process Input
		curTime = glfwGetTime();
		ProcessInput(window, curTime - lastTime);
		lastTime = curTime;
		//Draw
		lightPosition.z = (sin(glfwGetTime()) + 1.0f) / 2.0 + 0.5f;
		glm::mat4 model;
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//plane
		shader.use();
		model = glm::mat4();
		model = glm::rotate(model, glm::radians(60.0f * (float)sin(glfwGetTime())), glm::vec3(1.0f, 0.0f, 0.0f));
		//model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));
		shader.setVec3("light.position", lightPosition);
		shader.setMartix("model", model);
		shader.setMartix("normalMatrix", glm::inverse(glm::transpose(model)));
		shader.setMartix("view", view);
		shader.setMartix("projection", projection);
		shader.setVec3("viewerPos", camera.Position);
		shader.setBool("openEffect", openEffect);
		glBindVertexArray(planeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		//light
		light_Shader.use();
		model = glm::mat4();
		model = glm::translate(model, lightPosition);
		model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
		light_Shader.setMartix("model", model);
		light_Shader.setMartix("view", view);
		light_Shader.setMartix("projection", projection);
		glBindVertexArray(lightVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		//swap and poll
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
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
