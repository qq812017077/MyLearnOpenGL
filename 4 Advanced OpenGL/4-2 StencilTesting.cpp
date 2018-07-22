#include <glad\glad.h>
#include <glfw3.h>

//数学库
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include "Shaders.h"
#include "Camera.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
/*
	注意所有的缓冲区都是基于2D画面的，
	即模板缓冲描述的是投影之后的2D画面的绘制情况。
*/
/*
	Stencil Testing
	模板测试

	当片段着色器处理完片段之后，模板测试(Stencil Test) 就开始执行了
	和深度测试一样，它能丢弃一些片段。仍然保留下来的片段进入深度测试阶段，深度测试可能丢弃更多。

	开启模板缓冲，绘制一个物体作为我们的模板，这个过程实际上就是写入模板缓冲的过程

	模板测试基于另一个叫做模板缓冲区的缓冲区，这允许我们在渲染期间更新，以塑造更有趣的效果。

	每个模板值：	stencil value 包含8位。
		因此对于每一个像素/片段，总共有256种不同的模板值，因此

		stencil缓冲区首先使用0清空，然后设置一个用1填充的矩阵在其中
			则仅当片段的stencil值包含1的时候会被渲染
		
		Stencil缓冲区允许我们设置一个特殊值给要渲染的片段。
			当渲染的时候我们改变stencil缓冲区的内容，实际上是写入内容到stencil缓冲区。

		对于同一次的渲染迭代，我们可以读取这些值来丢弃或者通过指定了片段。

		使用Stencil需要注意以下：
			1.使能Stencil缓冲区写入
			2.渲染对象，更新缓冲区内容
			3.关闭Stencil缓冲区写入
			4.渲染其他对象，根据Stencil缓冲区的内容丢弃对应的片段/像素
		
		glStencilMask：
			该函数允许我们给模板值设置一个遮罩位，它与模板值进行按位与(AND)运算决定缓冲是否可写
			默认情况下遮罩位全为1，故而不影响输出。
				如果我们将遮罩位设置为全0，则所有的写入Stencil缓冲区的值最终都会为0.（ 如果深度中的 glDepthMask(GL_FALSE) ）
		
		大多数情况你的模板遮罩（stencil mask）写为0x00或0xFF就行，但是最好知道有一个选项可以自定义位遮罩。
		
*/

/*Stencil Function
	我们对于一个Stencil Test 何时通过或失败，以及它如何影响Stencil Buffer 有着明确的控制。
	有两个控制函数：  配置Stencil 
		glStencilFunc(GLenum func, GLint ref, GLuint mask)
			描述了OpenGL对模板缓冲做什么

				func:
					设置模板测试操作，该操作被用于保存的stencil值以及该函数的ref值，
					可选值： 含义与depth函数功能相似
						GL_NEVER；
						GL_LESS；
						GL_LEQUAL；
						GL_GREATER；
						GL_GEQUAL;
						GL_EQUAL；
						GL_NOTEQUAL；
						GL_ALWAYS

				ref:
					指定了 stencil test 的引用值，该值被用于和缓冲区的内容进行比较
				mask:
					指定一个mask，在模板测试对比引用值和储存的模板值前，
						对它们进行按位与（and）操作，初始设置为1
		eg：
				glStencilFunc(GL_EQUAL, 1, 0xFF)；
					含义为，
					如果片段的模板值等于（EQUAL）1，则通过测试，允许绘制
				例如GL_LESS通过，当且仅当
					满足: ( ref & mask ) < ( stencil & mask ).
				GL_GEQUAL通过，
					当且仅当( ref & mask ) >= ( stencil & mask )

		NOTE：该函数仅仅描述如何操作模板缓冲区（何时通过）
		void glStencilOp(GLenum sfail, GLenum dpfail, GLenum dppass)
				sfail： 如果模板测试失败将采取的动作。
				dpfail： 如果模板测试通过，但是深度测试失败时采取的动作。
				dppass： 如果深度测试和模板测试都通过，将采取的动作
			
			下面是这些参数可选的值：
				Action										Description
				GL_KEEP									保留当前的模板值
				GL_ZERO									将模板值设为0
				GL_REPLACE								模板值将被替换为引用值
				GL_INCR									若模板值不是最大值就将模板值+1
				GL_INCR_WRAP							和GL_INCR一样，但是一旦超出最大值将被设置为0
				GL_DECR									若模板值不是最小值就将模板值-1
				GL_DECR_WRAP							和GL_DECR一样，但是一旦为最小值则设置为最大值。
				GL_INVERT								Bitwise inverts the current stencil buffer value.
		默认设置为 (GL_KEEP, GL_KEEP, GL_KEEP) ；
			表示无论是任何测试结果，都保留值，默认不更新模板缓冲区。
	使用这两个函数，我们可以精确的控制：何时执行何种操作
				以及当操作通过或者失败的时候，何时丢弃片段。
*/

/*	
	Object Outline
		为每一个物体赋予一个有颜色的边。
		例如策略类游戏中的选中效果。
			
		步骤如下：
			1.在绘制带轮廓物体前，将Stencil函数设置为GL_ALWAYS，每当物体的片段渲染时，用1更新Stencil缓冲区。
			2.渲染物体
			3.关闭模板缓冲区写入以及深度测试
			4.每个物体放大一点点
			5.使用一个不同的片段着色器输出纯色
			6.再次绘制物体，但只是当它们的片段的模板值不为1时才进行
			7.开启模板写入和深度测试

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
	Stencil_Test(window);
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
	//使能深度测试
	glEnable(GL_DEPTH_TEST);
	//使能模板测试
	glDepthFunc(GL_LESS);//永远通过测试，意味着最后绘制的会永远在最上面，因此和不开深度测试的效果一致。

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
	1.激活贴图ID号
	2.将采集器绑定到当前的贴图ID上
	3.将贴图绑定到当前贴图ID
	NOTE:	2+3 将采集器和贴图链接了起来。
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
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);//通过测试则进行替换

		glClearColor(0.8f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glm::mat4 model;
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		
		
		
		glStencilMask(0x00);//关闭模板缓冲的写入。----不可写入

		shader.use();
		shader.setMartix("projection", projection);
		shader.setMartix("view", view);
		
		// floor
		glBindVertexArray(planeVAO);
		glBindTexture(GL_TEXTURE_2D, floorTexture);	//切换贴图
		shader.setMartix("model", glm::mat4());
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
		
	
		/*
		确保箱子的每个片段用模板值1更新模板缓冲。
		因为片段总会通过模板测试，在我们绘制它们的地方，模板缓冲用引用值更新。
		*/
		//为箱子的所有绘制的片段的模板缓冲更新为1：
		glStencilMask(0xFF);	//设置模板缓冲为可写状态
		glStencilFunc(GL_ALWAYS, 1, 0xFF);	//所有片段都要写入模板缓冲
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


		//绘制放大的箱子
		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);//仅绘制不是1的部分。
		glStencilMask(0x00); // 禁止修改模板缓冲
		glDisable(GL_DEPTH_TEST);	// 这里关闭深度测试 是为了让轮廓不因为处在前面的平面而被消去

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
	渲染平面，除了被cube遮挡的地方：
		1.关闭颜色写入
		2.开启模板写入，绘制cube从而构建相应的模板（对应位置设置为1）
		3.开启颜色写入
		4.关闭模板写入，绘制平面，并设置为仅绘制非1的部分，即非cube的位置被绘制。
		5.开启模板写入，保证之后的刷新缓冲区可以进行！！

*/
void Stencil_DrawRectangle(GLFWwindow* window) {
	//使能深度测试
	glEnable(GL_DEPTH_TEST);
	//使能模板测试
	glDepthFunc(GL_LESS);//永远通过测试，意味着最后绘制的会永远在最上面，因此和不开深度测试的效果一致。

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
	1.激活贴图ID号
	2.将采集器绑定到当前的贴图ID上
	3.将贴图绑定到当前贴图ID
	NOTE:	2+3 将采集器和贴图链接了起来。
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
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);//通过测试则进行替换

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glm::mat4 model;
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		
		
		shader.use();
		shader.setMartix("projection", projection);
		shader.setMartix("view", view);
	//section 1 绘制模板
		
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		//为箱子的所有绘制的片段的模板缓冲更新为1：
		glStencilMask(0xFF);	//设置模板缓冲为可写状态
		glStencilFunc(GL_ALWAYS, 1, 0xFF);	//所有片段都要写入模板缓冲
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


	// section 2绘制实际场景
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glStencilMask(0x00); // 禁止写入stencil
		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		// floor
		glBindVertexArray(planeVAO);
		glBindTexture(GL_TEXTURE_2D, floorTexture);	//切换贴图
		shader.setMartix("model", glm::mat4());
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);

		glStencilMask(0xFF);	
		//设置模板缓冲为可写状态,必须有这句话！！！！！
		//否则glClear将不起作用，因为禁止写入！！ 
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