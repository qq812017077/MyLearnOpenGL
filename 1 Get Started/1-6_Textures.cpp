#include <glad\glad.h>
#include <glfw3.h>
#include <iostream>
#include "Shaders.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
/*
	stb_image.h ��һ���ܺ��õ�ͷ�ļ��������������ش󲿷ֵ��ļ���ʽ��
	���Һ����׼��ɵ���Ŀ�С�

	����STB_IMAGE_IMPLEMENTATION
		ʹ��Ԥ�������������ͷ�ļ����������������ض����Դ����
*/


void framebuffer_size_callback(GLFWwindow * window, int width, int height);
void processInput(GLFWwindow * window);
void LoadingPhoto(unsigned int target, const char * photoFilePath, unsigned int format);
/*glTexParameteri
�ú���Ϊָ����Ŀ�꣨����������2D������ÿһ���������ò�����ʽ
*/
//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);//����ʹ�þ���
//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);//����ʹ�þ���
																	  /*	��һ��������ָ��Ŀ��
																	  �ڶ���������ָ������һ����ִ��ʲô����������Ϊ���������wrap������
																	  ������������ѡ��wrap��ģʽ��
																	  GL_REPEAT������Ĭ����Ϊ�����ظ�
																	  GL_MIRRORED_REPEAT���ظ�+����
																	  GL_CLAMP_TO_EDGE��������Լ����0��1֮�䣬�����Ĳ��ֻ��ظ���������ı�Ե������һ�ֱ�Ե�������Ч��
																	  GL_CLAMP_TO_BORDER������������Ϊ�û�ָ���ı�Ե��ɫ��
																	  float borderColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
																	  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);		���ﴫ��һ����ɫ
																	  */

/*����Ĳɼ���ʽ��
Texture Sampling:has a loose interpretation and can be done in many different ways.

��������:(bottom-left: 0,0    top-right: 1,1)

Texture Wrapping �����Ʒ�ʽ:
GL_REPEAT������Ĭ����Ϊ�����ظ�
GL_MIRRORED_REPEAT���ظ�+����
GL_CLAMP_TO_EDGE��������Լ����0��1֮�䣬�����Ĳ��ֻ��ظ���������ı�Ե������һ�ֱ�Ե�������Ч��
GL_CLAMP_TO_BORDER������������Ϊ�û�ָ���ı�Ե��ɫ��

�������У�������Ϊs,t
t(��Ӧy��)
|   /
|  / r(��Ӧz��)
| /
|/___________s(��Ӧx��)
*/

/*�������
	����ͬһ������ͼ���ڲ�ͬ��С����ʱ������������֮�Ķ�ӦӦ��δ���
		
		��������������������ص�����
				�������꣺��ģ�Ͷ������õ��Ǹ����飬
				�������أ���ͼƬ����Ӧ�������أ�
					
					OpenGL����������������������ȥ��������ͼ���ϵ����أ�Ȼ����в�����ȡ�������ص���ɫ

		��������бȽ���Ҫ��������ʽ��
			1.GL_NEAREST : �ٽ����ˣ�OpenGL��ѡ��
													�����������ĵ�����������������Ǹ����ص���ɫ
					ԭ����OpenGL selects the pixel which center is closest to the texture coordinate
			2.GL_LINEAR : ���Թ��ˣ�OpenGL���ݣ�
													����������Χ�����ؽ���һ�����Բ�ֵ��ֻ��������������ĵ㹱�׸�����ѡ�

	��������������С��ʱ��ʹ���ڽ����ˣ����Ŵ�ʱʹ�����Թ��ˣ�
	����������
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
*/


/*Mipmaps
	�༶��Զ����
		һϵ�е�����ͼ�񣬺�һ������ͼ����ǰһ���Ķ���֮һ
		ͨ��	glGenerateMipmaps	����������MipMaps
		����Ⱦ���л��༶��Զ�����Levelʱ��������Զ�����ᵼ�¸����󣩣�OpenGL����������ͬ����������֮���������ʵ����Ӳ�߽硣
		ʵ�������ǿ���ָ����ͬ�༶�����֮��Ĺ��˷�ʽ��

				GL_NEAREST_MIPMAP_NEAREST��ʹ�����ڽ��Ķ༶��Զ������ƥ�����ش�С����ʹ���ڽ���ֵ�����������
				GL_LINEAR_MIPMAP_NEAREST��ʹ�����ڽ��Ķ༶��Զ�����𣬲�ʹ�����Բ�ֵ���в���
				GL_NEAREST_MIPMAP_LINEAR����������ƥ�����ش�С�Ķ༶��Զ����֮��������Բ�ֵ��ʹ���ڽ���ֵ���в���
				GL_LINEAR_MIPMAP_LINEAR���������ڽ��Ķ༶��Զ����֮��ʹ�����Բ�ֵ����ʹ�����Բ�ֵ���в���

			ͬ��������ͨ��glTexParameteri�������ģʽ����

			һ�������Ĵ����ǣ����Ŵ���˵�ѡ������Ϊ�༶��Զ�������ѡ��֮һ������û���κ�Ч����
			��Ϊ�༶��Զ������Ҫ��ʹ����������С������µģ�����Ŵ󲻻�ʹ�ö༶��Զ����
			Ϊ�Ŵ�������ö༶��Զ�����ѡ������һ��GL_INVALID_ENUM������롣
*/

/*Loading and creating textures
		����texture

*/


/*Texture Unit
	ʵ�������ǿ��Ը�����ɼ�����Sampler��λ����ɫ���У�����λ��ֵ��
		���������ǿ���ͬʱʹ�ö������
	���ǳ�һ�������λ�ý��� ����Ԫ(Texture Unit)�� Ĭ��ֵΪ0����Ĭ�ϵ�����Ԫ�Զ������˲���Ҫ���ǽ���λ�õĸ�ֵ������
	��ע�ⲻ�����е��Կ��������ḳ��Ĭ��ֵ����

	OpenGL���ٱ�֤��16������Ԫ����ʹ�ã�Ҳ����˵����Լ����GL_TEXTURE0��GL_TEXTRUE15��
	���Ƕ��ǰ�˳����ģ���������Ҳ����ͨ��GL_TEXTURE0 + 8�ķ�ʽ���GL_TEXTURE8�����ڵ�������Ҫѭ��һЩ����Ԫ��ʱ�������á�

	glActiveTexture(GL_TEXTURE0); //�ڰ�����֮ǰ�ȼ�������Ԫ
	glBindTexture(GL_TEXTURE_2D, texture);

*/


const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

void Texture_1(GLFWwindow * window);

int main(void)
{
	if (!glfwInit()) {
		std::cout << "Failed to Initialization" << std::endl;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow * window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "1-5_Shaders_", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create the window" << std::endl;
		glfwTerminate();
		system("pause");
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//��ʼ��GLAD��
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		system("pause");
		return -1;
	}

	Texture_1(window);
	
	//Final
	glfwTerminate();
	return 0;
}

//GLfloat texCoords[] = {
//	0.0f, 0.0f, // ���½�
//	1.0f, 0.0f, // ���½�
//	0.5f, 1.0f // ����
//};
void Texture_1(GLFWwindow * window){

	//Shader����
	Shader shader("shader1-6.vertex","shader1-6.fragment");

	//������ͼ
	unsigned int texture[2];
	glGenTextures(2, texture);//ͨ��ID���ã���һ������ͬ����ʾ������

	//ͬ����Ҫ��
	glBindTexture(GL_TEXTURE_2D, texture[0]);//��֮����������ö���������
	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//Wrap mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//Filter mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	/*����Picture*/
	stbi_set_flip_vertically_on_load(true);//��תy�ᣬ����ͼƬ���µߵ���
	LoadingPhoto(GL_TEXTURE_2D, "container.jpg",GL_RGB);

	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//Wrap mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//Filter mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	LoadingPhoto(GL_TEXTURE_2D, "awesomeface.png", GL_RGBA);
	glBindTexture(GL_TEXTURE_2D, 0);//���


	float vertices[] = {
		// positions          // colors           // texture coords
		-0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f,  // top left 
		0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
		-0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
		0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f   // bottom right	
	};
	unsigned int indices[] = {
		0,1,2,
		1,2,3
	};

	unsigned int VAO, VBO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(0 + 3 * sizeof(float) ));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)( 0 + 6 * sizeof(float) ));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);//��ʹ��VBO֮�󣬽����
	glBindVertexArray(0);

	shader.use();
	glUniform1i(glGetUniformLocation(shader.ID, "ourTexture1"), 0); // set it manually
	shader.setInt("ourTexture2", 1); // or with shader class
	while (!glfwWindowShouldClose(window)) {
		//process input 
		processInput(window);

		//rendering commands here....
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		
		//glBindTexture(GL_TEXTURE_2D, texture);	//���ﲻ��Ҫ�ֶ��Ľ���uniform�ĸ�ֵ������Զ�����
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture[0]);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture[1]);

		shader.use();
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		//
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

}
//��ⰴ������
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}
//�ص�����
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	return;
}

void LoadingPhoto(unsigned int target, const char * photoFilePath, unsigned int format) {
	/*����Picture*/
	int width, height, nrChannels;//���ߡ� number of color channels����ɫͨ��������
	unsigned char *data = stbi_load(photoFilePath, &width, &height, &nrChannels, 0);

	if (data) {
		switch (target) {
		case GL_TEXTURE_2D:
			glTexImage2D(target, 0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, data);/*��������
																							  ��һ��������the texture target����ζ�Ż������뵱ǰ�󶨵����������ͬһ��Ŀ���ϵ�����������1D��3D��
																							  �ڶ���������Ϊ����ָ��mipmap�ĵȼ�������������0��Ҳ���ǻ�������
																							  ������������˵����Ҫ��������Ϊ���ָ�ʽ�������ͼƬΪRGB��ʽ��
																							  ���ĺ͵�����������������Ŀ�͸ߡ�
																							  ���������������Ǳ���Ϊ0����ʷ�������⣩
																							  ���ߡ��˸�������ָ��ԴͼƬ�ĸ�ʽ���������͡�
																							  ����ʹ��RGBֵ�������ͼ�񣬲������Ǵ���Ϊchar(byte)���飬���ǽ��ᴫ���Ӧֵ��
																							  */
																							  //������glTexImage2Dʱ����ǰ�󶨵��������ͻᱻ����������ͼ�񡣵���ע�⣺
																							  //���ص�����ͼƬ����������ļ������ʱ�������ʹ��mipmap�������ò��ֶ����ɲ�ͬ��ͼƬ��ͨ���������ӵڶ���������ֵ��
																							  //����Ϣ�ǣ������������glGenerateMipmap ���԰��������Զ�����������Ҫ������

			glGenerateMipmap(GL_TEXTURE_2D);
			break;
		default:
			break;
		}
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}
	//�������������Ӧ�Ķ༶��Զ������ͷ�ͼ����ڴ沢������������һ���ܺõ�ϰ�ߡ�
	stbi_image_free(data);
}