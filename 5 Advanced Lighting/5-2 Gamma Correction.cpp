/*
	存在问题：
		教程中说明了sRGB纹理可以自动进行gamma矫正，然而此处尝试之后似乎无效。
		：
			将shader中的gamma相关内容全部静态化。
			
			设置为按下B键打开gamma：使用矫正后的纹理
						关闭gamma：使用原来的纹理。
				按照教程的描述，矫正后应该变暗（颜色进行了2.2次幂操作）

				然而实际上并没有任何变化！！
				不知道问题出在何处。
*/

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

/* Gamma
		伽玛校正：
		当我们计算完成所有的像素颜色后，我们需要将其显示在显示器上。
		在过去的数字图像显示器大多是阴极射线管显示器（ cathode-ray tube (CRT) monitors）

		这种显示器的一个物理特点是：两倍的输入电压不代表两倍的亮度：
			加倍输入电压会产生一个大约2.2次幂的关系的亮度值，这被称为一个显示器的Gamma：伽玛

	补充：
	Gamma也叫灰度系数，每种显示设备都有自己的Gamma值，都不相同，有一个公式：
			设备输出亮度 = 电压的Gamma次幂，任何设备Gamma基本上都不会等于1，等于1是一种理想的线性状态，
		这种理想状态是：如果电压和亮度都是在0到1的区间，那么多少电压就等于多少亮度。
		对于CRT，Gamma通常为2.2，因而，输出亮度 = 输入电压的2.2次幂，你可以从本节第二张图中
		看到Gamma2.2实际显示出来的总会比预期暗，相反Gamma0.45就会比理想预期亮，
		如果你将Gamma0.45叠加到Gamma2.2的显示设备上，便会对偏暗的显示效果做到校正，这个简单的思路就是本节的核心

		人类所感知的亮度恰好和CRT所显示出来相似的指数关系非常匹配。

	人眼感知亮度能力和物理亮度不太匹配。
	（人眼对暗环境的敏感度高：
		因此实际上，物理亮度增加一倍：在人眼里表现出来的可能翻了几番！

		对比结果可能如下；
				人眼感知亮度：	0.0		0.1		0.2		0.3		0.4		0.5		0.6		0.7		0.8		
				实际物理亮度：	0.0		0.1										0.2						
			即实际上物理亮度增加一倍，在人眼里可能已经在0.6 0.7 左右了,即亮度提高了好多倍.(因为人眼在低亮度下会比较敏锐)


		因为人眼看亮度是第一行的刻度，所以直到今天，显示器也是使用指数关系来显示输出颜色，
			所以原始物理亮度被映射到 第一行的非线性亮度颜色，因为它看起来更好。
			（即我们会使用公式来模拟人眼的刻度关系，而非实际情况，以期给予更好的体验）
		
		这种非线性的映射的确让亮度看起来更好，但是在渲染图像的时候这又是一个问题！
			
			我们在我们的应用中配置的颜色和亮度都是基于我们观察到的！
			所以这些选项实际上也是非线性亮度/颜色选项。


		之前我们一直假设在线性空间中工作，但实际上我们一直运行在由显示器的输出颜色空间定义的颜色空间。
			或者说：我们之前使用的亮度值在物理上来讲是不正确的
		
		显示器所显示出来的图像和线性图像的最小亮度是相同的，它们最大的亮度也是相同的；只是中间亮度部分会被压暗。
			
		因为所有中间亮度都是线性空间计算出来的，监视器显以后，实际上都会不正确。

	）
*/

/*Gamma  Correction
	伽玛校正
		伽玛校正的思路是应用显示器伽玛的倒数到最终的颜色（在现实到监控器之前），
		我们在颜色显示到监视器的时候把每个颜色输出都加上这个反转的Gamma曲线，
		这样应用了监视器Gamma以后最终的颜色将会变为线性的。

		假设伽玛的相关系数为2.2.
			即，若我们要输出一个颜色（0.5，0.0，0.0，0.0）

			未进行Gama矫正：
				输出为 (0.5, 0.0, 0.0, 0.0)^2.2 --> (0.218, 0.0, 0.0, 0.0)	-->变暗。

			进行Gama矫正：
				在输出前进行Gama矫正：
				矫正： (0.5, 0.0, 0.0, 0.0)^1/2.2 --> (0.73, 0.0, 0.0, 0.0)	-->变亮。
				对矫正值进行输出：
					(0.73, 0.0, 0.0, 0.0)^2.2 --> (0.5, 0.0, 0.0, 0.0)	-->恢复

		因此Gama矫正的作用：将变换转换为线性

		2.2是基于大部分显式Gamma的一个平均值， Gamma2.2的颜色空间称为sRGB颜色空间。
		每一个显示器都有自己的Gamma曲线。但是gamma2.2在大多数监视器上表现都不错。
		处于这个原因，游戏通常允许玩家改变游戏的Gamma设置，以适应每个显示器



	使用Gamma矫正的两种方法：
		1.使用OpenGL内置的sRGB帧缓冲
			这样就非常简单，只需要使能GL_FRAMEBUFFER_SRGB。
			就可以告诉OpenGL随后的命令中，
				在保存在颜色缓冲区之前，应该先从sRGB颜色空间使用伽玛校正
			sRGB是一个大致对应gamma2.2的颜色空间，它也是家用设备的一个标准。
			在使能GL_FRAMEBUFFER_SRGB之后，
					OpenGL将在每个片段着色器运行之后自动的对所有随后的帧缓冲区执行gamma矫正！ 包括默认的帧缓冲区。
			
			glEnable(GL_FRAMEBUFFER_SRGB); 

		在使能之后，渲染的画面将会自动的进行gamma矫正了，因为它是硬件执行的。
			！你应该记得这个建议：
			gamma校正将把线性颜色空间转变为非线性空间，所以在最后一步进行gamma校正是极其重要的。
			如果，在最后的输出之前进行gamma矫正：
					那所有随后基于这个颜色的操作都将基于一个不正确的颜色进行操作！
			例如，如果你使用多个帧缓冲，你可能打算让两个帧缓冲之间
			传递的中间结果仍然保持线性空间颜色，只是给发送给监视器的最后的那个帧缓冲应用gamma校正。

		2.在片段着色器中自己进行gamma矫正
			稍微复杂一点，但是还行～
				我们在每个相关像素着色器运行的最后应用gamma校正，所以在发送到帧缓冲前，颜色就被校正了。

				void main()
				{
					// do super fancy lighting
					[...]
					// apply gamma correction
					float gamma = 2.2;
					fragColor.rgb = pow(fragColor.rgb, vec3(1.0/gamma));
				}
			就两行代码而已。。
			这里的问题是：
				为了保持一致，你必须在像素着色器里加上这个gamma校正，所以如果你有很多像素着色器，
				它们可能分别用于不同物体，那么你就必须在每个着色器里都加上gamma校正了。
			一个解决方案：
				在所有处理之后进行一个后期处理，并在后期处理的时候进行gamma矫正，这样只需要一次矫正就oK了哈

*/	

/*	sRGB textures
		因为显示器总是显示在sRGB空间中应用了gamma的颜色，
		所以无论在计算机上绘制什么东西，你选择的颜色都是都是基于显示器看到的颜色。
		这意味着，所有创造和编辑的图片都不是基于线性空间，而是在sRGB空间（）。
			eg：在屏幕上加倍一个暗红色颜色，是根据所观察到的亮度，并不意味着加倍了红色元素。
		
		纹理设计师是在sRGB空间中创建的所有纹理贴图。所以如果我们在我们的渲染应用中使用这些纹理，我们必须考虑这个。

		在我们使用应用gamma矫正之前，这不是问题，因为纹理在sRGB空间中看起来还可以，而且我们也是工作在sRGB空间中，
		所以没有矫正，纹理看起来也没问题。
		（
			艺术家们是基于我们所观察到的显示屏创建的图片，所以实际上，这里已经使用了Gamma矫正，我们看到的是矫正后的！
			
			就是说：创建纹理的时候，得到的是gamma矫正后的颜色纹理
			
			所以假设实际的颜色值为 Crgb, 则：
						Crgb^2.2 得到的才是我们看到的所对应的实际颜色。 而我们的代码是不知道的，
					我们的代码工作在线性空间中，会对所有的进行矫正： Crgb^(1/2.2)
					所以最后显示的是 Crgb,  而不是Crgb^2.2。所以会亮，不正常。
			解决办法就是：
				将其处理为  Crgb^2.2， 这样就是实际的颜色了！
		）
					
		但是我们如果要给每个纹理进行矫正处理，会很麻烦！

		幸运的是，OpenGL给我们提供了另一个方案来解决我们的麻烦，这就是GL_SRGB和GL_SRGB_ALPHA内部纹理格式。

		如果我们在OpenGL中创建了一个纹理，把它指定为以上两种sRGB纹理格式其中之一，
		OpenGL将自动把颜色校正到线性空间中，这样我们所使用的所有颜色值都是在线性空间中的了。

		我们可以这样把一个纹理指定为一个sRGB纹理：
			glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
			
			如果你还打算在你的纹理中引入alpha元素，必究必须将纹理的内部格式指定为GL_SRGB_ALPHA。

			需要注意的是：
				使用颜色的纹理，像是diffuse纹理，几乎都是在sRGB中的。
				但是specular_maps（保存了法向量）和 normal_maps（保存了镜面向量：越大越亮）则总是在线性空间中


*/

/*
--函数------------------------------
*/
void framebuffer_size_callback(GLFWwindow * window, int width, int height) { glViewport(0, 0, width, height);	return; }
void mouse_callback(GLFWwindow* window, double xPos, double yPos);
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
void ProcessInput(GLFWwindow * window, float deltaTime);


unsigned int loadTexture(char const *path, unsigned int Tex_Wrap_Para = GL_REPEAT , bool gammaCorrection = false);
//Blinn_Phong
void Gamma_Test(GLFWwindow * window);
/*
--变量------------------------------
*/
const int SCR_WIDTH = 800;
const int SCR_HEIGHT = 600;

glm::vec3 cameraPos(0.0f, 1.0f, 3.0f);
glm::vec3 WorldUp(0.0f, 1.0f, 0.0f);

float yaw = -90.0f, pitch = 0.0f;
bool openEffect = true;
bool gammaEnabled = true;
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

	//注册环境
	glfwMakeContextCurrent(window);
	//初始化回调
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);	//关闭鼠光标
	glfwSetCursorPosCallback(window, mouse_callback);				//打开鼠标回调
	glfwSetScrollCallback(window, scroll_callback);

	//glad
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		system("pause");
		return -1;
	}
	Gamma_Test(window);
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
		//使用B来控制切换着色方案
		if (gammaEnabled) {
			gammaEnabled = false;
		}
		else {
			gammaEnabled = true;
		}
	}
	lastStatus = glfwGetKey(window, GLFW_KEY_SPACE);
	Exchange_lastStatus = glfwGetKey(window, GLFW_KEY_B);
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
	// x-z 平面
	// positions          // texture Coords		//Normal
	-8.0f, -1.0f, -8.0f,	0.0f, 5.0f,			0.0f,  1.0f, 0.0f,
	8.0f, -1.0f,  8.0f,	5.0f, 0.0f,			0.0f,  1.0f, 0.0f,
	8.0f, -1.0f, -8.0f,	5.0f, 5.0f,			0.0f,  1.0f, 0.0f,

	-8.0f, -1.0f, -8.0f,	0.0f, 5.0f,			0.0f,  1.0f, 0.0f,
	-8.0f, -1.0f,  8.0f,	0.0f, 0.0f,			0.0f,  1.0f, 0.0f,
	8.0f, -1.0f,  8.0f,	5.0f, 0.0f,			0.0f,  1.0f, 0.0f,

};

void Gamma_Test(GLFWwindow * window) {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);	//透明
	
	//Shader
	Shader light_Shader("Shader.vertex_lamp", "Shader.fragment_lamp");
	Shader plane_Shader("Shader5-2.vertex", "Shader5-2.fragment");
	//Texture
	unsigned int wallTexture = loadTexture("wall.jpg",false);							//不进行矫正
	unsigned int wallTexture_GammaCorrection = loadTexture("wall.jpg",true);	//进行矫正

	//VAO , VBO
	//plane
	unsigned int planeVAO, planeVBO;
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(5 * sizeof(float)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);


	//light
	// lighting info
	// -------------
	glm::vec3 lightPositions[] = {
		glm::vec3(-3.0f, 0.0f, 0.0f),
		glm::vec3(-1.0f, 0.0f, 0.0f),
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(3.0f, 0.0f, 0.0f)
	};
	glm::vec3 lightColors[] = {
		glm::vec3(0.25),
		glm::vec3(0.50),
		glm::vec3(0.75),
		glm::vec3(1.00)
	};
	glm::vec3 lightPosition(0.0f, 0.0f, 0.0f);

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

	//Shader参数设置
	plane_Shader.use();
	plane_Shader.setVec3("light[0].position", lightPositions[0]);
	plane_Shader.setVec3("light[0].ambient",	glm::vec3(0.2f, 0.2f, 0.2f));
	plane_Shader.setVec3("light[0].diffuse",	lightColors[0]);
	plane_Shader.setVec3("light[0].specular",	lightColors[0]);
	plane_Shader.setVec3("light[1].position", lightPositions[1]);
	plane_Shader.setVec3("light[1].ambient",	glm::vec3(0.2f, 0.2f, 0.2f));
	plane_Shader.setVec3("light[1].diffuse",	lightColors[1]);
	plane_Shader.setVec3("light[1].specular",	lightColors[1]);
	plane_Shader.setVec3("light[2].position", lightPositions[2]);
	plane_Shader.setVec3("light[2].ambient",	glm::vec3(0.2f, 0.2f, 0.2f));
	plane_Shader.setVec3("light[2].diffuse",	lightColors[2]);
	plane_Shader.setVec3("light[2].specular",	lightColors[2]);
	plane_Shader.setVec3("light[3].position", lightPositions[3]);
	plane_Shader.setVec3("light[3].ambient",	glm::vec3(0.2f, 0.2f, 0.2f));
	plane_Shader.setVec3("light[3].diffuse",	lightColors[3]);
	plane_Shader.setVec3("light[3].specular",	lightColors[3]);

	plane_Shader.setInt("material.diffuse", 0);
	plane_Shader.setInt("material.specular", 1);
	plane_Shader.setFloat1f("material.shininess", 16.0f);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gammaEnabled ? wallTexture_GammaCorrection : wallTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D,  wallTexture);

	float curTime{}, lastTime{};
	curTime = lastTime = glfwGetTime();
	while (!glfwWindowShouldClose(window)) {
		//Process Input
		curTime = glfwGetTime();
		ProcessInput(window, curTime - lastTime);
		lastTime = curTime;
		//Draw
		glm::mat4 model;
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//plane
		plane_Shader.use();
		model = glm::mat4();
		plane_Shader.setMartix("model", model);
		plane_Shader.setMartix("normalMatrix", glm::inverse(glm::transpose(model)));
		plane_Shader.setMartix("view", view);
		plane_Shader.setMartix("projection", projection);
		plane_Shader.setVec3("viewerPos", camera.Position);
		plane_Shader.setBool("openEffect", openEffect);
		plane_Shader.setBool("gamma", gammaEnabled);
		glBindVertexArray(planeVAO);
		if (gammaEnabled) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, wallTexture_GammaCorrection);
			//glActiveTexture(GL_TEXTURE1);
			//glBindTexture(GL_TEXTURE_2D, wallTexture_GammaCorrection);
		}
		else {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, wallTexture);
			//glActiveTexture(GL_TEXTURE1);
			//glBindTexture(GL_TEXTURE_2D, wallTexture);
		}
		
		
		glDrawArrays(GL_TRIANGLES, 0, 6);
		std::cout << (gammaEnabled ? "Gamma enabled" : "Gamma disabled") << std::endl;
		//light
		light_Shader.use();
		light_Shader.setMartix("view", view);
		light_Shader.setMartix("projection", projection);
		for (int i = 0; i < 4 ; ++i) {
			model = glm::mat4();
			model = glm::translate(model, lightPositions[i]);
			model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
			light_Shader.setMartix("model", model);
			glBindVertexArray(lightVAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		//swap and poll
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

unsigned int loadTexture(char const *path, unsigned int Tex_Wrap_Para, bool gammaCorrection)
{
	/*
		Gamma矫正的意义在于：
			这里获取到的颜色数据并不是实际所显示出来的数据，
				假设显示的颜色为Crgb,则这里保存的颜色应为 Crgb^(1/2.2)，
			若不进行任何处理：之后进行一次gamma矫正，成为 Crgb^(1/2.2)^2,在进行gamma则又回到了Crgb^(1/2.2)，所以会很亮
			若程序矫正到Crgb，则之后进行一次gamma矫正，成为 Crgb^(1/2.2),在进行gamma则又回到了Crgb，这样就保持了线性关系
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
