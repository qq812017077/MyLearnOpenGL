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

/*Normalized Device Coordinate	NDC
	��׼���豸����
	OpenGLϣ������ִ��shader����֮�����еĿɼ��㶼��ת��Ϊ��׼���豸����
	Ҳ����˵��ÿ�������x��y��z���궼Ӧ����-1.0��1.0֮�䣬����������귶Χ�Ķ��㶼�����ɼ���

	����ͨ���Լ�ָ��һ�����귶Χ��֮���ڶ�����ɫ���н���Щ����ת��Ϊ NDC��
			Ȼ����Щ  NDC  �����դ��(Rasterizer)���ٽ�����ת��Ϊ��Ļ�ϵĶ�ά��������ء�

*/

/*	Coordinate  System
		�ڽ����󶥵�ת������Ļ����֮ǰ��ͨ���Ὣ����ת����һЩ����ϵͳ�У�
			�������ĺô��ǣ�
				ʹ������/���� ��ø�����ִ��
	����������������ϵͳ
			Local space (or Object space)
			World space
			View space (or Eye space)
			Clip space
			Screen space
	��Щ�������ǽ����ж���ת��ΪƬ��֮ǰ��������Ҫ���ڵĲ�ͬ��״̬��
*/

/*Coordinate Transformation
		���ǵĶ����������ȴӱ��ؿռ���Ϊ�������꿪ʼ��֮��ת��Ϊ��
				�������ꡢ�۲����ꡢ�ü����꣬
			������ת��Ϊ��Ļ����
	1.Local coordinates
			�ֲ������Ƕ�������ھֲ�ԭ������ꣻҲ�Ƕ���ʼ�����ꡣ
	2.world-space coordinates 
			�ڶ�������ת���ֲ�����Ϊ�������꣬
				���������ԭ�㶨�塣
		��������������������������������������������������
		������ӱ�������ת����������������ģ�;�����ɵģ�
		model matrix��
			ʵ�ֽ������ת������Ӧ���ڵĵط���ͨ��ƽ�ơ�������ת��

		������
	3.view-space coordinates	--eye coordinates
			�۲��׼��������ͼ���꣩
		����������ת��Ϊ�������۲��ߵĽǶȹ۲�����ꡣ
		��������������������������������������������������
			ͨ������һϵ�е�ƽ�ƺ���ת�������ƽ�ƺ���ת�����Ӷ�ʹ���ض��Ķ���ת���������ǰ�档
			��Щ�����һ���ת��ͨ���洢��һ���۲����(View Matrix)���������������ת�����۲�ռ䡣
		������
	4.clip coordinates
			���۲�����ͶӰ���ü����ꡣ
			�ü����걻���� -1.0 - 1.0 ֮��
			���������Щ���㱻��ʾ
				�Ƚ� eye coordinate ת��Ϊ  clip coordinate  ��ת��Ϊ  NDC
		��������������������������������������������������
			�ڶ�����ɫ�����е����OpenGL���ü���ָ����Χ����ĵ㣬�������Ƕ�������
			ʣ�µ���������Ϊ��Ļ�Ͽ���ʾ����Ƭ

			��Ϊ�����пɼ������궼������-1.0��1.0�ķ�Χ�ڲ��Ǻ�ֱ�ۣ�
			�������ǻ�ָ���Լ������꼯(Coordinate Set)������ת���ر�׼���豸����ϵ������OpenGL����������������
		�������£�
			���ǻᶨ��һ��ͶӰ���󣬽���������ӹ۲�ռ�ת�����ü��ռ䡣
			ͶӰ����ָ����һ�����귶Χ������ÿ��ά��Ϊ-1000-1000.
			ͶӰ������ָ�������귶Χת������׼�豸�����С����ڴ˷�Χ������겻�ᱻת���� -1 �� 1֮�䡣
		
		��ͶӰ�����������Ĺ۲�����Viewing Box������Ϊ��frustum --����ͷ׶��
			ÿ��������ƽ��ͷ�巶Χ�ڵ����궼�����ճ������û�����Ļ��
			
			�������̣���ָ����Χ�ڵ�����ת��ΪNDC��ע��NDC������ӳ�䵽2ά�Ĺ۲�ռ����꣩����ΪͶӰ��Projection.
			ͶӰ��3ά����ǳ����׵�ӳ�䵽2ά��NDC����׼�豸���ꡣ

		һ�����еĵ㱻ӳ�䵽�ü��ռ䣬���һ����������ִ�С�������͸�ӻ��֣�Perspective division
			��һ���У����ǻὫλ������x,y,z���������w������
				�ù��̽�4ά�Ĳü��ռ�����ת��Ϊ3ά��NDC��

				��һ������ÿһ��������ɫ�����е�����Զ�ִ�С�
		
		��һ������������꾭��ת���Ľ�����ᱻӳ�䵽��Ļ�ռ�(��glViewport����)�ұ�ת����Ƭ�Ρ�
			ͶӰ���󽫹۲�����ת��Ϊ�ü�����Ĺ��̲������ֲ�ͬ�ķ�ʽ��ÿ�ַ�ʽ�ֱ����Լ���ƽ��ͷ�塣
			���ǿ��Դ���һ������ͶӰ����(Orthographic Projection Matrix)��һ��͸��ͶӰ����(Perspective Projection Matrix)��
		������

	5.screen coordinates 
			���ü���������ת��Ϊ��Ļ����
		�ù��̼���-1.0-1.0��Χ������ת������glViewPort���������Ļ����
			
		��һ���̳�Ϊ���ӿڱ任(Viewport Transform)
		���ת�������꽫���͵���դ�����ɹ�դ������ת��ΪƬ�Ρ�
			

	��ͬ�Ĳ����ڲ�ͬ������ϵ�»��ļ���������ײ���
*/


/*Orthographic Projection	����ͶӰ
	����ͶӰ��������һ���������ƽ��׵�壬��׵��ָ���˲ü��ռ�
		Ҫ��������ͶӰ�������Ǳ���ָ������׶��Ŀ����Լ����ȡ�
		��ʹ�øþ���ת�������ڲü��ռ��ڵĻ��򲻻ᱻ�ü���

	ƽ��ͷ�嶨�����ɿ��ߡ���ƽ���Զƽ������Ŀ��ӵ�����ϵ���κγ����ڽ�ƽ��ǰ���Զƽ���������궼�ᱻ�ü�����
	����ƽ��ͷ��ֱ�ӽ�ƽ��ͷ���ڲ��Ķ���ӳ�䵽��׼���豸����ϵ�У�
	��Ϊÿ��������w�������ǲ���ģ����w��������1.0����͸�ӻ��ֲ���ı������ֵ��

	Ҫ��������ͶӰ��������GLM�ṩ�Ĺ���������
		glm::ortho(0.0f, 800.0f, 0.0f, 600.0f, 0.1f, 100.0f);
		�����������£�
				��һ��������ƽ��׶���	left coordinate
				�ڶ���������ƽ��׶���	right coordinate��		��һ�������������ˣ���
				������������ƽ��׶���	bottom coordinate
				���ĸ�������ƽ��׶���	top coordinate			�����ĸ����������ˣ���
				�������������ƽ��ľ���
				������������Զƽ��ľ���
		�ò���ʵ����ָ���� x , y , z �ķ�Χ��
				x: 0 - 800
				y: 0 - 600
				z: 0.1 - 100
			�ڸ÷�Χ�ڵ��������꽫��ӳ�䵽NDC��
	����ͶӰ����ֱ�ӽ�����ӳ�䵽��Ļ�Ķ�άƽ���ڣ���ʵ����һ��ֱ�ӵ�ͶӰ���󽫻��������ʵ�Ľ��
*/

/*Perspective projection��͸��ͶӰ
		͸�ӣ� ��ԽԶԽС��Ч�������Ƕ�����˳ƺ������������еĻ𳵹���ڼ�Զ���ཻ������
		͸��ͶӰ����ģ��(mimic	)��ʵ���е�͸��Ч��
		��ע�������������ķ�ɢ�����ݣ��������Ϊ�ܽ������۵Ĺ��߻��γ�һ��Բ׶�ĸо�
			�ӽ���Զ����Խ��Խ�󣬶�ʵ������Щ�������ᱻ���ϳ�Ϊһ�����棬��ԽԶ�ͱ����ŵ�Խ���ԣ�
			��

		���ͶӰ���󲻽���������ƽ��ͷ�巶Χӳ�䵽�ü��ռ䣬ͬ�����޸���ÿ�����������wֵ���Ӷ�ʹ����۲���ԽԶ�Ķ�������w����Խ��
		��ת��������궼���� -w �� w֮�䣬����󶥵���ɫ�����������ɼ��������� -1 �� 1 ֮�䡣���ֻҪ�ڲü��ռ�֮�ڣ��ͻ���������Ĳ�����
					| x / w |
			out  =  | y / w |
					| z / w |
			�����ͽ�  -w �� w�ķ�Χ������ת������ -1 �� 1 ֮�䡣

		ÿ����������ķ��������������w�������õ�һ������۲��ߵĽ�С�Ķ������ꡣ


	͸����ͼ�Ľ�ͷ׵����һ��������ĺ��ӡ�����������ڲ���ÿ�����궼�ᱻӳ�䵽�ü��ռ�ĵ㡣
	GLM�Ķ������£�
		glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)width/(float)height, 0.1f, 100.0f);

		��һ��������FOV����ʾ��Ұ��field of view��.
		�ڶ�����������߱ȣ���/�ߡ�
		�������ĸ����������ˣ���ƽ���Զƽ��ľ��롣
		����ͨ�����ý�ƽ�����0.1f��Զƽ��Ϊ100.0f.
		��������Ϊ�����ƽ�����õĹ�Զ������10.0f  ��ô�����ǹ��ֿ���һ������ʱ����Ұ�ᴩ͸���� ��

*/


void framebuffer_size_callback(GLFWwindow * window, int width, int height) { glViewport(0,0,width,height);	return; }
void ProcessInput(GLFWwindow * window);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

void CoordSystem(GLFWwindow * window);
void CoordSystem_Cube_No_Z_buffer(GLFWwindow * window);
void CoordSystem_Cube_Z_buffer(GLFWwindow * window);
void CoordSystem_MoreCube(GLFWwindow * window);
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
	
	CoordSystem_MoreCube(window);
	//CoordSystem_Cube_Z_buffer(window);

	glfwTerminate();
	//system("pause");
	return 0;
}

/**************************************************************************************
��������Ĳ��裬���ǽ��������¼������裺
����3������
Model Martix			-->		From	Local	to  World
View Martix				-->		From	World   to  Eye
Projection Martix		-->		From	Eye		to  Clip
����w����
Divide	by w			-->		From    Clip	to	NDC

������ɫ���������Ҫ���еĶ��㶼�ڲü��ռ��ڣ����������ǵ�ת������������

OpenGLȻ���ڲü��ռ���ִ��͸�ӻ��ִӶ�������ת������׼���豸���ꡣ

OpenGL��ʹ��glViewPort�ڲ��Ĳ���������׼���豸����ӳ�䵽��Ļ���꣬

ÿ�����궼������һ����Ļ�ϵĵ�(�����ǵ���������Ļ��800 *600)��������̳�Ϊ�ӿ�ת����
*/
const char * Photo_Path1 = "awesomeface.png";
const char * Photo_Path2 = "container.jpg";


void CoordSystem(GLFWwindow * window) 
{
	//Going 3D
	
	//1.model matrix
	//ͨ���������������	model matrix�����ǽ���������ת�������������ꡣ
	glm::mat4 model;	//model matrix consists of translations, scaling and/or rotations 
	model = glm::rotate(model , glm::radians(-55.0f),glm::vec3(1.0f,0.0f,0.0f));	//����x����ת��ʹ֮����������floor
	

	//2.view matrix
	//To move a camera backwards, is the same as moving the entire scene forward. ����ƶ������
	glm::mat4 view;
	view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));	// note that we're translating the scene in the reverse direction of where we want to move

	//3.projection matrix
	glm::mat4 projection;
	projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH /(float)SCR_HEIGHT, 0.1f, 100.0f);

	//Shader
	Shader shader("shader1-8.vertex","shader1-8.fragment");

	//Texture
	unsigned int texture[2];
	glGenTextures(2, texture);
	glBindTexture(GL_TEXTURE_2D,texture[0]);
	//set parameter
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//wrap mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//Filter mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);//	��תy��
	unsigned char * data = stbi_load(Photo_Path1, &width, &height, &nrChannels, 0);
	if (data) {
		//
		glTexImage2D(GL_TEXTURE_2D, 0 , GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	glBindTexture(GL_TEXTURE_2D, texture[1]);
	//set parameter
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//wrap mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//Filter mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//
	stbi_set_flip_vertically_on_load(true);//	��תy��
	data = stbi_load(Photo_Path2, &width, &height, &nrChannels, 0);
	if (data) {
		//
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
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
	//VAO(vertex array object), VBO() , EAO
	unsigned int VAO, VBO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1,&VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(0 + 3 * sizeof(float)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(0 + 6 * sizeof(float)));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER,0);
	glBindVertexArray(0);

	shader.use();
	glUniform1i(glGetUniformLocation(shader.ID, "ourTexture1"), 0); // set it manually	��0 ��1
	shader.setInt("ourTexture2", 1); // or with shader class	��1��2
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture[1]);

	while (!glfwWindowShouldClose(window)) {
		//process input
		ProcessInput(window);

		//render
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		shader.use();
		glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1 , GL_FALSE , glm::value_ptr(model));
		glUniformMatrix4fv(glGetUniformLocation(shader.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		//Swap and poll
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}	

float cubeVertices[] = {
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

//�������ĳЩ��Ӧ���ڵ�ס���汻����������������������������档
//֮������������ΪOpenGL��ͨ����һ��һ���������������������ģ����������Ḳ��֮ǰ�Ѿ�������������ء�
//
void CoordSystem_Cube_No_Z_buffer(GLFWwindow * window)
{
	//Going 3D


	//Shader
	Shader shader("shader1-8.vertex", "shader1-8.fragment");

	//Texture
	unsigned int texture[2];
	glGenTextures(2, texture);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	//set parameter
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//wrap mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//Filter mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);//	��תy��
	unsigned char * data = stbi_load(Photo_Path1, &width, &height, &nrChannels, 0);
	if (data) {
		//
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	glBindTexture(GL_TEXTURE_2D, texture[1]);
	//set parameter
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//wrap mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//Filter mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//
	data = stbi_load(Photo_Path2, &width, &height, &nrChannels, 0);
	if (data) {
		//
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
	glBindTexture(GL_TEXTURE_2D, 0);//���

	unsigned int indices[] = {
		0,1,2,
		1,2,3
	};
	//VAO(vertex array object), VBO() , EAO
	unsigned int VAO, VBO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(0 + 3 * sizeof(float)));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	shader.use();
	glUniform1i(glGetUniformLocation(shader.ID, "ourTexture1"), 0); // set it manually	��0 ��1
	shader.setInt("ourTexture2", 1); // or with shader class	��1��2
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture[1]);

	while (!glfwWindowShouldClose(window)) {
		//process input
		ProcessInput(window);

		//render
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		shader.use();
		//1.model matrix
		//ͨ���������������	model matrix�����ǽ���������ת�������������ꡣ

		glm::mat4 model;
		model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));

		//2.view matrix
		//To move a camera backwards, is the same as moving the entire scene forward. ����ƶ������
		glm::mat4 view;
		view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));	// note that we're translating the scene in the reverse direction of where we want to move

		//3.projection matrix
		glm::mat4 projection;
		projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

		glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(glGetUniformLocation(shader.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		//Swap and poll
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

//���˵��ǣ�OpenGL�洢�����Ϣ��z������(Z-buffer)���棬������OpenGL������ʱ����һ�����غ�ʱ�����ǡ�ͨ��ʹ��z���������ǿ�������OpenGL��������Ȳ��ԡ�
void CoordSystem_Cube_Z_buffer(GLFWwindow * window) {
	glEnable(GL_DEPTH_TEST);//����Ȳ���
	
	//Shader
	Shader shader("shader1-8.vertex", "shader1-8.fragment");

	//Texture
	unsigned int texture[2];
	glGenTextures(2, texture);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	//set parameter
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//wrap mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//Filter mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);//	��תy��
	unsigned char * data = stbi_load(Photo_Path1, &width, &height, &nrChannels, 0);
	if (data) {
		//
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	glBindTexture(GL_TEXTURE_2D, texture[1]);
	//set parameter
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//wrap mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//Filter mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//
	data = stbi_load(Photo_Path2, &width, &height, &nrChannels, 0);
	if (data) {
		//
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
	glBindTexture(GL_TEXTURE_2D, 0);//���

	unsigned int indices[] = {
		0,1,2,
		1,2,3
	};
	//VAO(vertex array object), VBO() , EAO
	unsigned int VAO, VBO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(0 + 3 * sizeof(float)));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	shader.use();
	glUniform1i(glGetUniformLocation(shader.ID, "ourTexture1"), 0); // set it manually	��0 ��1
	shader.setInt("ourTexture2", 1); // or with shader class	��1��2
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture[1]);

	while (!glfwWindowShouldClose(window)) {
		//process input
		ProcessInput(window);

		//render
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.use();
		//1.model matrix
		//ͨ���������������	model matrix�����ǽ���������ת�������������ꡣ

		glm::mat4 model;
		model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));

		//2.view matrix
		//To move a camera backwards, is the same as moving the entire scene forward. ����ƶ������
		glm::mat4 view;
		view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));	// note that we're translating the scene in the reverse direction of where we want to move

																	//3.projection matrix
		glm::mat4 projection;
		projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

		glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(glGetUniformLocation(shader.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		//Swap and poll
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

void CoordSystem_MoreCube(GLFWwindow * window) {
	glEnable(GL_DEPTH_TEST);//����Ȳ���

							//Shader
	Shader shader("shader1-8.vertex", "shader1-8.fragment");

	//Texture
	unsigned int texture[2];
	glGenTextures(2, texture);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	//set parameter
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//wrap mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//Filter mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);//	��תy��
	unsigned char * data = stbi_load(Photo_Path1, &width, &height, &nrChannels, 0);
	if (data) {
		//
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	glBindTexture(GL_TEXTURE_2D, texture[1]);
	//set parameter
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//wrap mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//Filter mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//
	data = stbi_load(Photo_Path2, &width, &height, &nrChannels, 0);
	if (data) {
		//
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
	glBindTexture(GL_TEXTURE_2D, 0);//���

	unsigned int indices[] = {
		0,1,2,
		1,2,3
	};
	//VAO(vertex array object), VBO() , EAO
	unsigned int VAO, VBO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(0 + 3 * sizeof(float)));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	shader.use();
	glUniform1i(glGetUniformLocation(shader.ID, "ourTexture1"), 0); // set it manually	��0 ��1
	shader.setInt("ourTexture2", 1); // or with shader class	��1��2
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture[1]);


	glm::vec3 cubePositions[] = {
		glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(2.0f,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f,  2.0f, -2.5f),
		glm::vec3(1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	};
	while (!glfwWindowShouldClose(window)) {
		//process input
		ProcessInput(window);

		//render
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.use();
		//1.model matrix
		//ͨ���������������	model matrix�����ǽ���������ת�������������ꡣ
		glBindVertexArray(VAO);
		
		//2.view matrix
		//To move a camera backwards, is the same as moving the entire scene forward. ����ƶ������
		glm::mat4 view;
		view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));	// note that we're translating the scene in the reverse direction of where we want to move

																	//3.projection matrix
		glm::mat4 projection;
		projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);


		glUniformMatrix4fv(glGetUniformLocation(shader.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		for (int i = 1; i < 10; ++i) {
			glm::mat4 model;
			model = glm::translate(model, cubePositions[i]);
			float angle = 20.0f * i;
			model = glm::rotate(model, (float)glfwGetTime() * glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
			glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, &model[0][0]);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		
		//Swap and poll
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}
void ProcessInput(GLFWwindow * window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window,true);
	}
}