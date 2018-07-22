#include <glad\glad.h>
#include <glfw3.h>

//数学库
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
	manipulate:操控
	这里有更多有趣的操控缓冲区的方法。
		而且使用纹理将大量数据传入着色器也有更有趣的方法。

	OpenGL中，
		一个缓冲区就是一个对象，其管理了一块指定内存，
		当我们将其绑定到一个指定的缓冲区对象的时候，才赋予了其意义。
		eg:
			当我们将其绑定在GL_ARRAY_BUFFER的时候，它就是一个顶点数组缓冲区。
			或者将其绑定到GL_ELEMENT_ARRAY_BUFFER。
			OpenGL内部会为每个目标储存一个缓冲，并且会根据目标的不同，以不同的方式处理缓冲。
	
	glBufferData：
			到目前为止，我们一直是通过该函数分配内存， 并且添加数据到内存中。
			如果我们仅将data参数设置为NULL,则我们仅仅构建缓冲区，而不进行填充。

			这在我们需要预留(Reserve)特定大小的内存，之后回到这个缓冲一点一点填充的时候会很有用。
	
	glBufferSubData：
			除了一次性填充整个缓冲区，我们还可以仅填充缓冲区的某个区域，通过上面的这个函数：
				该函数指定了渲染目标，偏移量，数据大小，以及实际数据，作为其的参数。
		
			不同的是，我们可以自己给出想要的偏移量，继而填充数据。
		
			glBufferSubData(GL_ARRAY_BUFFER, 24, sizeof(data), &data); // Range: [24, 24 + sizeof(data)]

		作用：
			这允许我们插入/或者更新某一部分数据。
			需要注意的是，缓冲区必须要有足够的空间，所以我们必须在调用该函数之前调用一次glBufferData.

	glMapBuffer：
		另一种导入数据到缓冲区的方法。 geting data into the buffer!!!
			1.首先获取缓冲区内存的指针：
				void *ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
				//注意提前要绑定好缓冲区
			2，获取缓冲区指针：
				void *ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
				
			3.拷贝数据：
				memcpy(ptr, data, sizeof(data));
			4.告诉缓冲区拷贝完成
				glUnmapBuffer(GL_ARRAY_BUFFER);
				之后指针将不再可用
			glMapBuffer可以直接将数据映射到缓冲中，而不需要先将其存储在临时内存中
				比如：我们可以直接从文件中读取数据然后拷贝到缓冲区内存中。	
*/

/*Batching vertex attributes.
	分批顶点属性
	之前，我们使用glVertexAttribPointer来指定顶点数组缓冲区内容的属性布局。
	在顶点数组缓冲区中，我们进行了交错处理（interleaved）。换句话说：
		我们将每一个顶点的位置、发现和/或纹理坐标紧密放置在一起。
	
	现在我们对缓冲区有了更多的了解，可以尝试一些骚操作了。
	我们可以做的是，将每一种属性类型的向量数据打包(Batch)为一个大的区块，而不是对它们进行交错储存。
		也就是取代使用123123123这样的交错布局，我们使用打包操作111222333

		当我们从文件中获取到顶点数据的时候，我们获取到的是：
			一个顶点位置数组；
			一个法向量数组；
			一个纹理坐标数组；
		可能我们在组合这些数据到一个大的交错数据数组中时要花点力气。
		通过批量的方式可能是更简单的方法：
			float positions[] = {.....};
			float normals[] = {.....};
			float textureCoords[] = {.....};
			//fill
			glBufferSubData(GL_ARRAY_BUFFER, 0 , sizeof(positions) , &positions);
			glBufferSubData(GL_ARRAY_BUFFER, sizeof(positions) , sizeof(normals) , &normals);
			glBufferSubData(GL_ARRAY_BUFFER,  sizeof(positions) + sizeof(normals) , sizeof(textureCoords) , &textureCoords);
		接下来，VAO的顶点属性设置也需要相应的修改：
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)(sizeof(positions)));
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)(sizeof(positions)+sizeof(normals) ));

	注意使用这两种方法并没有太多的优劣，看自己喜好而定！！！
*/

/*Copying Buffers
	当缓冲区填充数据之后，我们很可能想要和其他的缓冲区共享数据，或者想要复制一份到另一个缓冲区。

	glCopyBufferSubData 可以帮我们实现这个目的！
		void glCopyBufferSubData(GLenum readtarget, GLenum writetarget, GLintptr readoffset,
                         GLintptr writeoffset, GLsizeiptr size);
			readtarget：
			writetarget:这两个参数给出了复制的源缓冲区和目标缓冲区。
				eg:
					glCopyBufferSubData( VERTEX_ARRAY_BUFFER  , VERTEX_ELEMENT_ARRAY_BUFFER  , ....);
					表示从前者对应的绑定的缓冲区复制到后者绑定的缓冲区。---> VBO 到 EBO  ???
				问题：如果我们想要复制内容到同类型的缓冲区怎么办呢？我们没法同时绑定两个缓冲区到同一个缓冲目标！
					解决办法：
						OpenGL提供了两个缓冲目标叫做：
							GL_COPY_READ_BUFFER
							GL_COPY_WRITE_BUFFER
						因此可以将两个缓冲区绑定到这个上面，然后进行复制，
			readoffset:
			writeoffset:表示的是，两个缓冲区中读取的偏移量。一般都是0 0 ， 表示从源的起点往目标的起点开始。

			size：复制内容的多少。

			eg:
				float vertexData[] = { ... };
				glBindBuffer(GL_COPY_READ_BUFFER, vbo1);
				glBindBuffer(GL_COPY_WRITE_BUFFER, vbo2);
				glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, sizeof(vertexData));
			或者只绑定一个：
				glBindBuffer(GL_ARRAY_BUFFER, vbo1);
				glBindBuffer(GL_COPY_WRITE_BUFFER, vbo2);
				glCopyBufferSubData(GL_ARRAY_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, sizeof(vertexData));
		
*/

/*	如何使用纹理对象存储大量的数据，
		
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

	//注册环境
	glfwMakeContextCurrent(window);
	//初始化回调
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
鼠标向左移动，x值减小；向右移动，x值增大
鼠标向上移动，y值减小；向下移动，y值增大
*/
bool firstMouse = true;
static float lastX = 400.0f, lastY = 300.0f;
void mouse_callback(GLFWwindow* window, double xPos, double yPos) {
	std::cout << xPos << "   " << yPos << std::endl;
	if (firstMouse) // this bool variable is initially set to true
	{
		//第一次调用时的xPos,yPos和lastX,lastY相差巨大，需要进行一次调整
		lastX = xPos;
		lastY = yPos;
		firstMouse = false;
	}
	//std::cout << "xPos:  " << xPos << "yPos:  " << yPos << std::endl;
	float sensitivity = 0.05f;	//敏感度
	float xOffset = xPos - lastX;	//向左移动为负，向右移动为正。
	float yOffset = lastY - yPos;	//向上移动为正，向下移动为负。

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
		//使用空格来开关镜面反射效果
		if (openEffect) openEffect = false;
		else openEffect = true;
	}
	lastStatus = glfwGetKey(window, GLFW_KEY_SPACE);

	//相机按键控制
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
	//背面
	// positions          
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	//左侧面
	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,
	//右侧面
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	//正面
	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,
	//顶面
	-1.0f,  1.0f, -1.0f,
	1.0f,  1.0f, -1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,
	//底面
	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	1.0f, -1.0f,  1.0f
};

void CubeMap_Test(GLFWwindow* window) {
	//使能深度测试
	glEnable(GL_DEPTH_TEST);//深度测试

	Shader shader("Shader4-6.vertex", "Shader4-6.fragment");
	Shader skyboxShader("Shader4-6.vertex_skybox", "Shader4-6.fragment_skybox");

	//加载模型
	char model_path[] = "E:/C++/OpenGL_Course/OpenGL_Course/Model/nanosuit_reflrection/nanosuit.obj";
	Model ourModel(model_path);
	std::cout << "Model加载完成，总共有" << ourModel.GetMeshs().size() << "个Mesh" << std::endl;
	std::vector<Mesh> Meshes = ourModel.GetMeshs();
	std::vector<Mesh>::iterator it = Meshes.begin();
	int i = 0; int j = 0;
	for (std::vector<Mesh>::iterator it = Meshes.begin(); it != Meshes.end(); it++)
	{
		std::cout << "第" << i++ << "个Mesh" << std::endl;
		std::vector<Texture>::iterator texIt = it->textures.begin();
		for (std::vector<Texture>::iterator texIt = it->textures.begin(); texIt != it->textures.end(); texIt++)
		{
			std::cout << j << ":  " << texIt->path << "   " << texIt->type << std::endl;

		}
	}

	//Texture
	unsigned int cubeTexture = loadTexture(std::string("container.jpg").c_str(), GL_REPEAT);
	unsigned int floorTexture = loadTexture(std::string("wall.jpg").c_str(), GL_REPEAT);

	//textures_faces 包含了所有的贴图位置。注意加载顺序：按照循环中循环中的枚举顺序进行。
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
	1.激活贴图ID号
	2.将采集器绑定到当前的贴图ID上
	3.将贴图绑定到当前贴图ID
	NOTE:	2+3 将采集器和贴图链接了起来。
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

		//skybox---注意关闭深度写入，这样子才能成为背景。
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyTexture);
		glDepthFunc(GL_LEQUAL);//刷新后的缓冲区深度值为1，因此这里需要设置为当深度值小于等于1的时候都能通过。，否则背景还是写不上，
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

	//仍然和普通的纹理一样，我们需要指定其环绕和过滤方法。
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	/*
	注意我们将三个维度的环绕方式都设置为了GL_CLAMP_TO_EDGE，是因为
	在两个面之间的纹理坐标可能并不能触及哪个面（由于硬件限制），所以通过使用GL_CLAMP_TO_EDGE
	每当我们采集边缘的时候，OpenGL将总是返回他们的边缘值。

	这里的GL_TEXTURE_WRAP_R设置了纹理的R坐标（对应了纹理的第三个维度），

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
