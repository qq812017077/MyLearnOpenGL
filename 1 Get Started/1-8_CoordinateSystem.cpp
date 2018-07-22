#include <glad\glad.h>
#include <glfw3.h>

//数学库
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//图像库
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>
#include "Shaders.h"

/*Normalized Device Coordinate	NDC
	标准化设备坐标
	OpenGL希望，在执行shader程序之后，所有的可见点都被转换为标准化设备坐标
	也就是说，每个顶点的x，y，z坐标都应该在-1.0到1.0之间，超出这个坐标范围的顶点都将不可见。

	我们通常自己指定一个坐标范围，之后在顶点着色器中将这些坐标转换为 NDC。
			然后将这些  NDC  传入光栅器(Rasterizer)，再将他们转换为屏幕上的二维坐标或像素。

*/

/*	Coordinate  System
		在将对象顶点转换到屏幕坐标之前，通常会将它们转换到一些坐标系统中，
			这样做的好处是：
				使得运算/计算 变得更容易执。
	常见的有五种坐标系统
			Local space (or Object space)
			World space
			View space (or Eye space)
			Clip space
			Screen space
	这些就是我们将所有顶点转换为片段之前，顶点需要处于的不同的状态。
*/

/*Coordinate Transformation
		我们的顶点坐标首先从本地空间作为本地坐标开始，之后转换为：
				世界坐标、观察坐标、裁剪坐标，
			并最终转换为屏幕坐标
	1.Local coordinates
			局部坐标是对象相对于局部原点的坐标；也是对象开始的坐标。
	2.world-space coordinates 
			第二步即：转换局部坐标为世界坐标，
				基于世界的原点定义。
		！！！——————————————————————
		将对象从本地坐标转换到世界坐标是由模型矩阵完成的！
		model matrix：
			实现将对象从转换到它应该在的地方：通过平移、缩放旋转。

		！！！
	3.view-space coordinates	--eye coordinates
			观察标准（或者视图坐标）
		将世界坐标转换为摄像机或观察者的角度观察的坐标。
		！！！——————————————————————
			通常是由一系列的平移和旋转的组合来平移和旋转场景从而使得特定的对象被转换到摄像机前面。
			这些组合在一起的转换通常存储在一个观察矩阵(View Matrix)里，用来将世界坐标转换到观察空间。
		！！！
	4.clip coordinates
			将观察坐标投影到裁剪坐标。
			裁剪坐标被处理到 -1.0 - 1.0 之间
			这决定了那些顶点被显示
				先将 eye coordinate 转换为  clip coordinate  再转换为  NDC
		！！！——————————————————————
			在顶点着色器运行到最后，OpenGL将裁减掉指定范围以外的点，并将他们丢弃掉。
			剩下的坐标最后成为屏幕上可显示的碎片

			因为将所有可见的坐标都放置在-1.0到1.0的范围内不是很直观，
			所以我们会指定自己的坐标集(Coordinate Set)并将它转换回标准化设备坐标系，就像OpenGL期望它做的那样。
		具体如下：
			我们会定义一个投影矩阵，将顶点坐标从观察空间转换到裁减空间。
			投影矩阵指定了一个坐标范围，比如每个维度为-1000-1000.
			投影矩阵将其指定的坐标范围转换到标准设备坐标中。而在此范围外的坐标不会被转换到 -1 到 1之间。
		
		由投影矩阵所创建的观察区域（Viewing Box）被称为：frustum --即截头锥体
			每个出现在平截头体范围内的坐标都会最终出现在用户的屏幕上
			
			整个过程（将指定范围内的坐标转换为NDC，注意NDC很容易映射到2维的观察空间坐标）被称为投影：Projection.
			投影将3维坐标非常容易的映射到2维的NDC：标准设备坐标。

		一旦所有的点被映射到裁剪空间，最后一个操作将被执行————透视划分：Perspective division
			这一步中，我们会将位置向量x,y,z均除以齐次w分量，
				该过程将4维的裁剪空间坐标转换为3维的NDC。

				这一步会在每一个顶点着色器运行的最后被自动执行。
		
		这一步结束这后，坐标经过转换的结果将会被映射到屏幕空间(由glViewport设置)且被转换成片段。
			投影矩阵将观察坐标转换为裁剪坐标的过程采用两种不同的方式，每种方式分别定义自己的平截头体。
			我们可以创建一个正射投影矩阵(Orthographic Projection Matrix)或一个透视投影矩阵(Perspective Projection Matrix)。
		！！！

	5.screen coordinates 
			将裁剪坐标最终转换为屏幕坐标
		该过程即将-1.0-1.0范围的坐标转换到由glViewPort所定义的屏幕坐标
			
		这一过程称为：视口变换(Viewport Transform)
		最后转换的坐标将会送到光栅器，由光栅器将其转化为片段。
			

	不同的操作在不同的坐标系下会变的极具意义或极易操作
*/


/*Orthographic Projection	正交投影
	正交投影矩阵定义了一个立方体的平面椎体，该椎体指定了裁剪空间
		要创建正交投影矩阵我们必须指定可视锥体的宽、高以及长度。
		在使用该矩阵转换后仍在裁减空间内的话则不会被裁剪。

	平截头体定义了由宽、高、近平面和远平面决定的可视的坐标系。任何出现在近平面前面或远平面后面的坐标都会被裁剪掉。
	正视平截头体直接将平截头体内部的顶点映射到标准化设备坐标系中，
	因为每个向量的w分量都是不变的；如果w分量等于1.0，则透视划分不会改变坐标的值。

	要创建正射投影矩阵，利用GLM提供的构建函数：
		glm::ortho(0.0f, 800.0f, 0.0f, 600.0f, 0.1f, 100.0f);
		参数含义如下：
				第一个参数：平截锥体的	left coordinate
				第二个参数：平截锥体的	right coordinate：		第一二个参数定义了：宽
				第三个参数：平截锥体的	bottom coordinate
				第四个参数：平截锥体的	top coordinate			第三四个参数定义了：高
				第五个参数：近平面的距离
				第六个参数：远平面的距离
		该参数实际上指定了 x , y , z 的范围：
				x: 0 - 800
				y: 0 - 600
				z: 0.1 - 100
			在该范围内的所有坐标将被映射到NDC上
	正射投影矩阵直接将坐标映射到屏幕的二维平面内，但实际上一个直接的投影矩阵将会产生不真实的结果
*/

/*Perspective projection：透视投影
		透视： 即越远越小的效果。我们对其如此称呼！（即人眼中的火车轨道在极远处相交的现象）
		透视投影矩阵模拟(mimic	)了实际中的透视效果
		（注意人眼所看到的发散的内容，可以理解为能进入人眼的光线会形成一个圆锥的感觉
			从近到远截面越来越大，而实际上这些东西最后会被整合成为一个画面，故越远就被缩放的越明显！
			）

		这个投影矩阵不仅将给定的平截头体范围映射到裁剪空间，同样还修改了每个顶点坐标的w值，从而使得离观察者越远的顶点坐标w分量越大。
		被转换后的坐标都处于 -w 到 w之间，而最后顶点着色器输出的坐标可见坐标落在 -1 到 1 之间。因此只要在裁剪空间之内，就会进行这样的操作：
					| x / w |
			out  =  | y / w |
					| z / w |
			这样就将  -w 到 w的范围的坐标转换到了 -1 到 1 之间。

		每个顶点坐标的分量都会除以它的w分量，得到一个距离观察者的较小的顶点坐标。


	透视视图的截头椎体是一个不规则的盒子。在这个盒子内部的每个坐标都会被映射到裁剪空间的点。
	GLM的定义如下：
		glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)width/(float)height, 0.1f, 100.0f);

		第一个参数：FOV，表示视野（field of view）.
		第二个参数：宽高比，宽/高。
		第三、四个参数定义了：近平面和远平面的距离。
		我们通常设置近平面距离0.1f，远平面为100.0f.
		（这是因为如果近平面设置的过远，比如10.0f  那么当我们过分靠近一个物体时，视野会穿透物体 ）

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

	//初始化GLAD库
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
经过上面的步骤，我们建立了如下几个步骤：
首先3个矩阵
Model Martix			-->		From	Local	to  World
View Martix				-->		From	World   to  Eye
Projection Martix		-->		From	Eye		to  Clip
消除w分量
Divide	by w			-->		From    Clip	to	NDC

顶点着色器的输出需要所有的顶点都在裁剪空间内，而这是我们的转换矩阵所做的

OpenGL然后在裁剪空间中执行透视划分从而将它们转换到标准化设备坐标。

OpenGL会使用glViewPort内部的参数来将标准化设备坐标映射到屏幕坐标，

每个坐标都关联了一个屏幕上的点(在我们的例子中屏幕是800 *600)。这个过程称为视口转换。
*/
const char * Photo_Path1 = "awesomeface.png";
const char * Photo_Path2 = "container.jpg";


void CoordSystem(GLFWwindow * window) 
{
	//Going 3D
	
	//1.model matrix
	//通过将顶点坐标乘以	model matrix，我们将顶点坐标转换到了世界坐标。
	glm::mat4 model;	//model matrix consists of translations, scaling and/or rotations 
	model = glm::rotate(model , glm::radians(-55.0f),glm::vec3(1.0f,0.0f,0.0f));	//沿着x轴旋转，使之看起来像是floor
	

	//2.view matrix
	//To move a camera backwards, is the same as moving the entire scene forward. 向后移动相机，
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
	stbi_set_flip_vertically_on_load(true);//	调转y轴
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
	stbi_set_flip_vertically_on_load(true);//	调转y轴
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
	glBindTexture(GL_TEXTURE_2D, 0);//解绑
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
	glUniform1i(glGetUniformLocation(shader.ID, "ourTexture1"), 0); // set it manually	把0 给1
	shader.setInt("ourTexture2", 1); // or with shader class	把1给2
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

//立方体的某些本应被遮挡住的面被绘制在了这个立方体的其他面的上面。
//之所以这样是因为OpenGL是通过画一个一个三角形来画你的立方体的，所以它将会覆盖之前已经画在那里的像素。
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
	stbi_set_flip_vertically_on_load(true);//	调转y轴
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
	glBindTexture(GL_TEXTURE_2D, 0);//解绑

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
	glUniform1i(glGetUniformLocation(shader.ID, "ourTexture1"), 0); // set it manually	把0 给1
	shader.setInt("ourTexture2", 1); // or with shader class	把1给2
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
		//通过将顶点坐标乘以	model matrix，我们将顶点坐标转换到了世界坐标。

		glm::mat4 model;
		model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));

		//2.view matrix
		//To move a camera backwards, is the same as moving the entire scene forward. 向后移动相机，
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

//幸运的是，OpenGL存储深度信息在z缓冲区(Z-buffer)里面，它允许OpenGL决定何时覆盖一个像素何时不覆盖。通过使用z缓冲区我们可以设置OpenGL来进行深度测试。
void CoordSystem_Cube_Z_buffer(GLFWwindow * window) {
	glEnable(GL_DEPTH_TEST);//打开深度测试
	
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
	stbi_set_flip_vertically_on_load(true);//	调转y轴
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
	glBindTexture(GL_TEXTURE_2D, 0);//解绑

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
	glUniform1i(glGetUniformLocation(shader.ID, "ourTexture1"), 0); // set it manually	把0 给1
	shader.setInt("ourTexture2", 1); // or with shader class	把1给2
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
		//通过将顶点坐标乘以	model matrix，我们将顶点坐标转换到了世界坐标。

		glm::mat4 model;
		model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));

		//2.view matrix
		//To move a camera backwards, is the same as moving the entire scene forward. 向后移动相机，
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
	glEnable(GL_DEPTH_TEST);//打开深度测试

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
	stbi_set_flip_vertically_on_load(true);//	调转y轴
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
	glBindTexture(GL_TEXTURE_2D, 0);//解绑

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
	glUniform1i(glGetUniformLocation(shader.ID, "ourTexture1"), 0); // set it manually	把0 给1
	shader.setInt("ourTexture2", 1); // or with shader class	把1给2
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
		//通过将顶点坐标乘以	model matrix，我们将顶点坐标转换到了世界坐标。
		glBindVertexArray(VAO);
		
		//2.view matrix
		//To move a camera backwards, is the same as moving the entire scene forward. 向后移动相机，
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