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
#include "Model.h"

/*	Deferred Shading 延迟着色 （或者  Deferred Rendering 延迟渲染）
	到目前为止，我们使用的光照方式叫做：正向渲染(forward rendering)和或者正向着色(forward shading)
	这是一种渲染物体的最直接的方式，我们根据场景中的所有光源一个个的渲染所有物体。
	这很容易理解，也很容易实现，然而这对性能的负担也很重：
		我们每一个光源都需要对每一个渲染物体进行渲染，这个工作量是非常大的
	在一个高深度复杂性的场景中，正向渲染也会浪费很多的片段着色器运行
	（多个物体重合在同一个像素位置的时候，许多的片段着色器输出会被覆盖），

	延迟渲染或者延迟着色技术正是尝试在解决这样的问题，它改变了渲染物体的方式！
	这给了我们新的选择以显著优化包含大量灯源的场景，我们可以在可接受帧率的前提下渲染成百上千的灯源。


	延迟着色的思想是，：延迟或者说推迟大多数的大计算量的渲染（像是灯光）到后期处理中。
	包含两步：
		1.	第一步叫做几何阶段：该阶段我们渲染一次场景并且从对象中提取出所有的几何信息，并储存在一系列叫做G-buffer
	的纹理中；想象一下，位置向量，颜色向量，法向量，包括镜面反射向量。
		场景中这些储存在G缓冲中的几何信息将会在之后用来做(更复杂的)光照计算。
		2.	第二步，也叫光处理阶段(lighting pass)中，我们使用在G-buffer中的纹理。
			这一步中，我们渲染一个包含场景的四边形，并使用保存在G-buffer中的几何信息，用它们为每一个片段
	计算场景的光照；每个像素都使用G-buffer进行迭代。
			这一次，我们不再一路传递每个对象从顶点到片段着色器，而是将它的高级片段处理分离到后期处理中。
	光照的计算和之前相同，但是这一次我们从G-buffer中接受所有所需的输入变量，而不是从顶点着色器中！
		
		（根据来自learnOpenGL图片的描述，我们通过上一节提到的MRT技术，将信息分别保存在G-buffer中，然后在后期处理）
	
	优势：
		在G-buffer中最终的片段实际上就是实际的最终作为屏幕像素的片段信息，因为深度测试已经判定这个片段信息为最顶层的片段
	这就确保了我们在光处理阶段处理的每一个像素都仅处理一次；免除了许多无用的渲染调用。此外，延迟渲染比起正向渲染，
	允许我们渲染非常多的灯光。

	缺点：
		1.	G-buffers 需要我们保存大量的场景数据在其贴图的颜色缓冲区中，这会吃掉很多的内存，
			特别是像位置向量这样的数据需要很高的精度。
		2.	不支持Blending（因为我们只有最顶层的数据），MSAA也不再有效。
		我们可以使用一些手段来克服这些缺陷。
	
	在几何阶段填充G-buffer是非常有效的，因为我们直接保存像是位置、颜色、或是法向量这样的数据到帧缓冲区中，
	而这几乎不会消耗处理时间。通过使用MRT(multiple render target)技术我们甚至能在单个渲染过程中多所有的工作
*/

/* G-buffers
	G-buffer是所有用于保存光照相关数据的纹理的总称，其用于最终的光照处理阶段。
		用于正向渲染的片段光照计算的所有数据总结如下：
			一个3D的位置向量：用于计算片段位置：包括LightDir以及ViewDir。
			一个RGB漫反射颜色向量：也叫做albedo：反射率
			一个3D的法向量：用于决定表面的倾斜度：slope
			一个镜面反射强度：float值。
			所有的光源位置和颜色向量。
			玩家或者观察者的位置向量。
		使用这些变量，我们可以计算(Blinn-)Phong light，我们所惯用的光照计算公式。
		
			光源的位置和颜色，玩家的观察位置：通过uniform变量配置，所以不用放置到G-buffer中
			其他的变量：对于每一个对象的每个片段都有所区别（特定的）。

			如果我们能够以某种方式，提取同样的数据到最后的延迟光照处理中进行，我们就可以计算出同样的光照效果，
		即使我们是在渲染一个四方形的片段。
			
	OpenGL中，我们能够存储在Texture中的数据是没有限制的。所以把所有的逐片段数据存储在一个或多个全屏纹理（也被称为G-buffers）
	并可以用于随后的光照处理。因为G-buffer的纹理和光处理的2D四方形有着同样的大小。我们可以得到和在正向渲染中所设置的同样的数据，
	只是现在是在光处理后期。
	伪代码：
			while(...) // render loop
			{
				// 1. geometry pass: render all geometric/color data to g-buffer 
				glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				gBufferShader.use();
				for(Object obj : Objects)
				{
					ConfigureShaderTransformsAndUniforms();
					obj.Draw();
				}  
				// 2. lighting pass: use g-buffer to calculate the scene's lighting
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				glClear(GL_COLOR_BUFFER_BIT);
				lightingPassShader.use();
				BindAllGBufferTextures();
				SetLightingUniforms();
				RenderQuad();
			}
	注意我们需要保存的每个片段的数据是：位置向量；法向量；颜色向量；镜面光照强度值。
	在几何处理阶段：
		我们需要渲染场景的所有物体，并将这些数据组件保存在G-buffer中。我们能再次使用MRT来在单个渲染处理阶段
	将数据渲染到多个颜色缓冲区中（在Bloom中已经讨论过），
		对于几何处理阶段，我们将需要初始化一个帧缓冲对象，并且我们直观得(intuitively)称之为gBuffer,其上附着了
	多个颜色缓冲区对象和一个单独的深度缓冲区对象。
		对于位置向量和法向量纹理，我们更愿意使用一个高精度纹理（每个组分为16位或32位的浮点），
		反射率和镜面反射之我们则保存在默认的每个组分8位精度的纹理中，这是足够的。
	
		G-buffer的配置代码见下方：


	因为我们使用多渲染目标MRT，我们必须显式的使用glDrawBuffers，来声明我们想要渲染到哪些个颜色缓冲区。
	这里有趣的是，我们把位置和法向量保存在了一个RGB纹理因为这两种向量都有三个分组件。
	而我们把颜色和镜面反射强度数据组合在了一个单独的RGBA纹理中；这让我们可以少声明一个额外的颜色缓冲区。
	
	随着延迟渲染管道变得越来越复杂，需要越来越多的数据，我们将迅速发现新的方式来组合数据到一个单独的纹理当中。


	将数据渲染到G-buffer中。假设每个对象都有一个diffuse，一个normal，以及一个镜面反射强度纹理。
		我们需要使用像Shader5-8.fragment这样的着色器来渲染到G-buffer中。

	因为我们使用了MRT，通过指定的层级来向OpenGL说明我们要渲染到当前激活的帧缓冲区的哪一个颜色缓冲区。
	注意我们没有把镜面发射光照保存在一个单独的颜色缓冲纹理中，因为我们能保存它的单个float变量到一个颜色缓冲区的alpha通道。

	！！！
	请记住，因为有光照计算，所以保证所有变量在一个坐标空间当中至关重要。在这里我们在世界空间中存储(并计算)所有的变量。
	！！！


	现在我们想要渲染一大堆的纳米装甲战士到gBuffer的帧缓冲区中。并将其内容的颜色缓冲区一个一个的投影到四边形上，

	尝试想象世界空间位置和法向量都是正确的。比如说，指向右侧的法向量将会被更多地对齐到红色上，
	从场景原点指向右侧的位置矢量也同样是这样。一旦你对G缓冲中的内容满意了，我们就该进入到下一步：光照处理阶段了。

*/

/*	Deferred lighting pass	延迟光处理阶段
		在我们支配的G-buffer下有大量的片段数据，我们可以通过对每个G-buffer的纹理进行逐像素处理，并将储存在它们
	里面的内容作为光照算法的输入，来对计算出最终的光照结果。
		因为G-buffer纹理的值是最终转换的纹理值，所以我们仅需要对每个像素执行一次昂贵的光照计算。
	这使得延迟渲染变得非常有效，特别是在需要调用大量重型片段着色器的复杂场景中。

	对于这个光照处理阶段，我们将会渲染一个2D全屏的方形(有一点像后期处理效果)并且在每个像素上运行
	一个昂贵的光照片段着色器。
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shaderLightingPass.Use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gPosition);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gNormal);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
		// 同样发送光照相关的uniform
		SendAllLightUniformsToShader(shaderLightingPass);
		glUniform3fv(glGetUniformLocation(shaderLightingPass.Program, "viewPos"), 1, &camera.Position[0]);
		RenderQuad(); 
	在渲染之前，我们要绑定所有的G-buffer的纹理，并将光的相关uniform变量发送到着色器中。

	新的着色器除了数据是从纹理中获取以外（记得我们之前是用VBO发给顶点，再由顶点发给片段，而且每个物体的所有点都需要进行发送）
	
	现在我们只会发送能看得到的！
	
	最后的片段渲染着色器为Shader5-8.fragment_render
		光照处理着色器接受三个uniform纹理变量，它们代表了G-buffer，并且包含了所有几何处理阶段之前保存的数据，
	如果我们要使用当前的片段纹理坐标采样数据，我们将会获得和之前完全一样的片段值，就像我们直接渲染几何体一样。
	在片段着色器的开始，我们从G-buffer纹理中接收光照相关数据，通过一个简单的循环。注意我们从单个gAlbedoSpec纹理中
	接收Albedo颜色和镜面光照强度值。

	因为我们现在有了计算Blinn-Phong光照的每个片段所必需的值（以及相关的uniform变量），所以对于光照代码不需要任何变化。
	唯一的改变就是获取输入变量的方法。

	延迟渲染的一个问题是，不能应用在混合(blending)因为G-buffer中的所有值都从一个fragment中来的，而混合操作需要多个片段的组合。
	
	另一个问题是：演示渲染要求我们必须使用同一种光照算法作用于大部分场景光照，可以通过包含更多关于材质的数据到
	G缓冲中来减轻这一缺点。

	为了克服这些缺点(特别是混合)，我们通常分割我们的渲染器为两个部分：
		一个是延迟渲染的部分，另一个是专门为了混合或者其他不适合延迟渲染管线的着色器效果而设计的的正向渲染的部分。
	为了展示这是如何工作的，我们将会使用正向渲染器渲染光源为一个小立方体，因为光照立方体会需要一个特殊的着色器
	(简单得输出一个光照颜色)。
*/

/*Combining deferred rendering with forward rendering 结合延迟渲染和正向渲染
	现在我们想要把光源渲染成cube，基于光源的位置并随着延迟渲染器发射出颜色。
	第一件事：
		简单的在延迟渲染管道的最后，在延迟光处理后的四方形之上进行正向渲染。
	所以我们只需要像正常情况下渲染立方体，只要注意在完成延迟渲染之后进行这个操作就好。
		代码见下方的代码
			// deferred lighting pass
			[...]
			RenderQuad();
  
			// now render all light cubes with forward rendering as we'd normally do
			shaderLightBox.use();
			shaderLightBox.setMat4("projection", projection);
			shaderLightBox.setMat4("view", view);
			for (unsigned int i = 0; i < lightPositions.size(); i++)
			{
				model = glm::mat4();
				model = glm::translate(model, lightPositions[i]);
				model = glm::scale(model, glm::vec3(0.25f));
				shaderLightBox.setMat4("model", model);
				shaderLightBox.setVec3("lightColor", lightColors[i]);
				RenderCube();
			}
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		PS（postscript）：
			老实讲我不知道作者这么写代码为何是可以的，实际上，拿过来作者的代码进行测试的时候，
		若仅仅将是上面的代码，会发现无法显示出来所有的发光块！
			很困惑，但是后来还是怀疑是深度的问题。因为画布是始终绘制在z:0的位置上的。这个位置可能确实在深度上处于很前方
			所以我尝试了一下：
				关闭深度测试，然后绘制，然后再打开深度测试，这样就能出现和作者描述一模一样的情况了！！！！

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	但是要注意的一点是：
		这样的话，就只是把这些渲染出来的立方体并没有考虑到我们储存的延迟渲染器的几何深度(Depth)信息。
	并且结果是它被渲染在之前渲染过的物体之上，这并不是我们想要的结果。
		我们现在要做的是首先获取在几何处理阶段保存在默认深度缓冲区中的深度信息。并且仅渲染那些深度也位于
	之前渲染的物体的上方的那些光的片段。

	我们也可以通过glBlitFramebuffer，来将一个缓冲区中的数据复制到另外一个缓冲区中。这个函数我们也在抗锯
	齿的教程中使用过，用来还原多重采样的帧缓冲。
		（glBlitFramebuffer函数允许我们将一个用户定义的帧缓冲区的内容复制到另外一个帧缓冲区。）

	在延迟渲染的阶段中，所有物体的深度信息都保存在gBuffer这个FBO中。如果我们是简单的将它的深度缓冲区中
	的内容复制到默认缓冲区的深度缓冲区中，光照渲染效果就好像是所有的物体都在正向渲染中完成一样。
	正如抗锯齿教程中的简短解释一样，我们不得不指定一个一个一个帧缓冲为读帧缓冲(Read Framebuffer)，
	并且类似地指定一个帧缓冲为写帧缓冲(Write Framebuffer)：
			glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
			glBlitFramebuffer(
			  0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST
			);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			// now render light cubes as before
			[...]  
	这里将整个读帧缓冲的深度缓冲信息复制到了默认帧缓冲的深度缓冲中。		
	（对于颜色缓冲和模板缓冲我们也可以这样处理）
*/

/*A larger number of lights 更多的光源
	延迟渲染常常被称赞，因为其能够渲染大量的光源，而不会对性能产生过重的负担。
	事实上，延时渲染本身不能让我们渲染非常多的光源，因为我们仍必须为每一个场景的光源计算每个片段的光的组分，
	真正让大量光源成为可能的是我们能够对延迟渲染管线引用的一个非常棒的优化：
		光体积(Light Volumes)
	
	通常的话当我们在一个大量光照场景下的片段时，我们需要计算场景下每一个光源的贡献，而无论他们与片段的距离。
	很大一部分的光源根本就不会到达这个片段，所以为什么我们还要浪费这么多光照运算呢？

	所以光体积（Light Volumes）背后的思想是：
		计算光源所能到达区域的体积或半径，由于大部分光源都使用了某种形式的衰减(Attenuation)，我们可以用它来
	计算光源能够到达的最大路程，或者说是半径。我们接下来只需要对那些在一个或多个光体积内的片段进行繁重的光照运算就行了。
	这可以给我们省下来很可观的计算量，因为我们现在只在需要的情况下计算光照。
		

	计算光源的体积或半径
		要获得光源的体积半径，我们需要先解决一个我们认为是黑暗(Dark)的亮度(Brightness)的衰减方程。可以是0.0，也可以是
	一个非常小的值以至于可以被认为是黑暗的，比如0.03。
	
		为了展示我们如何计算光源的体积半径，我们将会使用一个在投光物这节中引入的一个更加复杂，
	但非常灵活的衰减方程：
		Flight =  I  / ( Kc + Kl * d + Kq * d^2)
		我们想要做的是，这个方程在Flight = 0.0的时候的解，也就是说光在该距离完全是黑暗的。
	事实上，可以看出，该方程不会等于0，我们应该做的是找一个接近于0的值（且被认为是黑暗的）
	在这里的一个可以接受的值是5/256.除以256是因为默认的8-bit帧缓冲可以每个分量显示这么多强度值(Intensity)。
		
		NOTE：我们使用的衰减方程在它的可视范围内基本都是黑暗的，所以如果我们想要限制它为一个比5/256更加黑暗的亮度，光体积就会
	变得太大从而变得低效。只要是用户不能在光体积边缘看到一个突兀的截断，这个参数就没事了。当然它还是依赖于场景的类型，一
	个高的亮度阀值会产生更小的光体积，从而获得更高的效率，然而它同样会产生一个很容易发现的副作用，那就是光会在光体积边界
	看起来突然断掉。
		
		所以方程解就变成了：
			5 / 256 = Imax / Attenuation
		这里的Imax是光源中的最大的颜色组件，我们使用一个光源的最大亮度组件：
			因为解光源最亮的强度值方程最好地反映了理想光体积半径。
			解方程如下：
				5 / 256 * Attenuation = Imax
			->	5 * Attenuation = Imax * 256
			->	Attenuation = Imax * 256 / 5
			->	Kc + Kl * d + Kq * d^2 - Imax * 256 / 5 = 0 
			-> 解一元二次方程！
			使用求根公式即可：

	*/	

/*	真正使用Light Volume - 不能理解！
	实际上，Shader5-8.fragment_render_lightVolume 中的光体积处理并不能有效工作，
	它只是演示了我们应该如何使用光体积去减少光照计算。实际上，GPU和GLSL在处理循环和分支上并不怎么出众。
	原因是因为在GPU上的shader的执行是高度并行的，大部分的架构都有这样的需要，因为大量线程的存在，
	GPU需要对它运行完全一样的着色器代码从而获得高效率。
	这通常意味着一个着色器运行时总是执行一个if语句所有的分支从而保证着色器运行都是一样的，
	这使得我们之前的半径检测优化完全变得无用，我们仍然在对所有光源计算光照！

	要使用light volume，合适的方式是渲染实际的球体，缩放到光的体积半径。球的中心即使光源的位置。
	由于它是根据光体积半径缩放的，这个球体正好覆盖了光的可视体积。
		这就是我们的技巧：
			我们使用大体相同的延迟片段着色器来渲染球体。因为球体产生了完全匹配于受影响像素的着色器调用，我
		们只渲染了受影响的像素而跳过其它的像素。
		
		它被应用在场景中每个光源上，并且所得的片段相加混合在一起。这个结果和之前场景是一样的，
		但这一次只渲染对于光源相关的片段。它有效地减少了从nr_objects * nr_lights到nr_objects + nr_lights的计算量，
		这使得多光源场景的渲染变得无比高效。这正是为什么延迟渲染非常适合渲染很大数量光源。

		然而这个方法仍然有一个问题：
			面剔除(Face Culling)需要被启用(否则我们会渲染一个光效果两次)，并且在它启用的时候用户可能进入一个光源的光体积，
			然而这样之后这个体积就不再被渲染了(由于背面剔除)，这会使得光源的影响消失。这个问题可以通过一个模板缓冲技巧来解决。
	
	渲染光体积
	确实会带来沉重的性能负担，虽然它通常比普通的延迟渲染更快，这仍然不是最好的优化。
	另外两个基于延迟渲染的更流行(并且更高效)的拓展叫做延迟光照(Deferred Lighting)和切片式延迟着色法(Tile-based Deferred Shading)。
	这些方法会很大程度上提高大量光源渲染的效率，并且也能允许一个相对高效的多重采样抗锯齿(MSAA)。
	然而受制于这篇教程的长度，我将会在之后的教程中介绍这些优化。

*/

/*
延迟渲染 vs 正向渲染
	仅仅是延迟着色法它本身(没有光体积)已经是一个很大的优化了，每个像素仅仅运行一个单独的片段着色器，然而对于正向渲染，
	我们通常会对一个像素运行多次片段着色器。当然，延迟渲染确实带来一些缺点：大内存开销，没有MSAA和混合(仍需要正向渲染的配合)。
	当你有一个很小的场景并且没有很多的光源时候，延迟渲染并不一定会更快一点，甚至有些时候由于开销超过了它的优点还会更慢。
	然而在一个更复杂的场景中，延迟渲染会快速变成一个重要的优化，特别是有了更先进的优化拓展的时候。

	最后，基本上所有能通过正向渲染完成的效果能够同样在延迟渲染场景中实现，这通常需要一些小的翻译步骤。
	举个例子，如果我们想要在延迟渲染器中使用法线贴图(Normal Mapping)，我们需要改变几何渲染阶段着色器来输
	出一个世界空间法线(World-space Normal)，它从法线贴图中提取出来(使用一个TBN矩阵)而不是表面法线，光照渲
	染阶段中的光照运算一点都不需要变。如果你想要让视差贴图工作，首先你需要在采样一个物体的漫反射，镜面，和
	法线纹理之前首先置换几何渲染阶段中的纹理坐标。一旦你了解了延迟渲染背后的理念，变得有创造力并不是什么难事。
*/

/*
--函数------------------------------
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

void DeferredShading_Test(GLFWwindow * window);
void Model_Test(GLFWwindow * window);
/*
--变量------------------------------
*/
const int SCR_WIDTH = 800;
const int SCR_HEIGHT = 600;

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
	DeferredShading_Test(window);
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
	// x-z 平面
	// positions         	//Normal			 // texture Coords	
	-4.0f, -1.0f, -4.0f,	0.0f,  1.0f, 0.0f,		0.0f, 5.0f,
	4.0f, -1.0f,  4.0f,		0.0f,  1.0f, 0.0f,		5.0f, 0.0f,
	4.0f, -1.0f, -4.0f,		0.0f,  1.0f, 0.0f,		5.0f, 5.0f,

	-4.0f, -1.0f, -4.0f,	0.0f,  1.0f, 0.0f,		0.0f, 5.0f,
	-4.0f, -1.0f,  4.0f,	0.0f,  1.0f, 0.0f,		0.0f, 0.0f,
	4.0f, -1.0f,  4.0f,		0.0f,  1.0f, 0.0f,		5.0f, 0.0f,

	// x-y 平面
	/*-2.0f,	-2.0f, -1.0f,	0.0f, 2.0f,			0.0f,  0.0f, 1.0f,
	2.0f,	 2.0f, -1.0f,	2.0f, 0.0f,			0.0f,  0.0f, 1.0f,
	2.0f,	-2.0f, -1.0f,	2.0f, 2.0f,			0.0f,  0.0f, 1.0f,

	-2.0f,	-2.0f, -1.0f,	0.0f, 2.0f,			0.0f,  0.0f, 1.0f,
	-2.0f,	 2.0f, -1.0f,	0.0f, 0.0f,			0.0f,  0.0f, 1.0f,
	2.0f,	 2.0f, -1.0f,	2.0f, 0.0f,			0.0f,  0.0f, 1.0f*/
};

void Model_Test(GLFWwindow * window) {
	glEnable(GL_DEPTH_TEST);
	Shader geometryPass_Shader("Shader5-8.vertex_gbuffer", "Shader5-8.fragment_gbuffer");
	//纳米机器人
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
//延迟渲染
void DeferredShading_Test(GLFWwindow * window) {
	glEnable(GL_DEPTH_TEST);
	//Shader
	Shader geometryPass_Shader("Shader5-8.vertex_gbuffer","Shader5-8.fragment_gbuffer");
	Shader lightPass_shader("Shader5-8.vertex_render", "Shader5-8.fragment_render_lightVolume");
	Shader light_shader("Shader.vertex_lamp","Shader.fragment_lamp");
	
	//纳米机器人
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
	// - Colors
	const GLuint NR_LIGHTS = 32;
	std::vector<glm::vec3> lightPositions;
	std::vector<glm::vec3> lightColors;
	srand(glfwGetTime());
	for (GLuint i = 0; i < NR_LIGHTS; i++)
	{
		// Calculate slightly random offsets
		GLfloat xPos = ((rand() % 100) / 100.0) * 6.0 - 3.0;
		GLfloat yPos = ((rand() % 100) / 100.0) * 6.0 - 4.0;
		GLfloat zPos = ((rand() % 100) / 100.0) * 6.0 - 3.0;
		lightPositions.push_back(glm::vec3(xPos, yPos, zPos));
		// Also calculate random color
		GLfloat rColor = ((rand() % 100) / 200.0f) + 0.5; // Between 0.5 and 1.0
		GLfloat gColor = ((rand() % 100) / 200.0f) + 0.5; // Between 0.5 and 1.0
		GLfloat bColor = ((rand() % 100) / 200.0f) + 0.5; // Between 0.5 and 1.0
		lightColors.push_back(glm::vec3(rColor, gColor, bColor));
	}

	//G-buffer的配置
	GLuint gBuffer;
	glGenFramebuffers(1 , &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER ,gBuffer);
	GLuint gPosition, gNormal, gAlbedoSpec; //三个缓冲区
	
	//0 - Position buffer
	glGenTextures(1, &gPosition);
	glBindTexture(GL_TEXTURE_2D , gPosition);
	glTexImage2D(GL_TEXTURE_2D ,0 , GL_RGB16F , SCR_WIDTH , SCR_HEIGHT , 0 , GL_RGB , GL_FLOAT , NULL);	//NULL表示空的无数据
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D , GL_TEXTURE_WRAP_S , GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);	//Position - 0
	
	//1 - Normal buffer
	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);	//NULL表示空的无数据
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);	//Normal - 1


	//2 - Color + specular buffer
	glGenTextures(1, &gAlbedoSpec);	//RGB color + specular value( 3 + 1 )
	glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);	//NULL表示空的无数据
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);	//Color + specular - 1
	
	// - tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
	unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);

	//3 - Create and attach Depth buffer
	GLuint depthBuffer;
	glGenRenderbuffers(1, &depthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	lightPass_shader.use();
	lightPass_shader.setInt("gPosition", 0);
	lightPass_shader.setInt("gNormal", 1);
	lightPass_shader.setInt("gAlbedoSpec", 2);


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
		// 1. Geometry Pass: render scene's geometry/color data into gbuffer
		glBindFramebuffer(GL_FRAMEBUFFER , gBuffer);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			
			geometryPass_Shader.use();
			geometryPass_Shader.setMartix("projection",projection);
			geometryPass_Shader.setMartix("view", view);
			geometryPass_Shader.setBool("inverse_normals", false);
			for (GLuint i = 0; i < objectPositions.size(); i++) {
				model = glm::mat4();
				model = glm::translate(model, objectPositions[i]);
				model = glm::scale(model, glm::vec3(0.25f));
				geometryPass_Shader.setMartix("model", model);
				nanosuitModel.Draw(geometryPass_Shader);
			}

		// 2. Lighting Pass: calculate lighting by iterating over a screen filled quad pixel-by-pixel using the gbuffer's content.
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			lightPass_shader.use();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, gPosition);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, gNormal);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
			// Also send light relevant uniforms
			// update attenuation parameters and calculate radius
			const float constant = 1.0; // note that we don't send this to the shader, we assume it is always 1.0 (in our case)
			const float linear = 0.7;
			const float quadratic = 1.8;
			for (GLuint i = 0; i < lightPositions.size(); i++)
			{
				lightPass_shader.setVec3("lights[" + std::to_string(i) + "].Position", lightPositions[i]);
				lightPass_shader.setVec3("lights[" + std::to_string(i) + "].Color", lightColors[i]);
				
				lightPass_shader.setFloat1f("lights[" + std::to_string(i) + "].Linear", linear);
				lightPass_shader.setFloat1f("lights[" + std::to_string(i) + "].Quadratic", quadratic);
				float lightMax = std::fmaxf(std::fmaxf(lightColors[i].r, lightColors[i].g), lightColors[i].b);	//获取最大组分
				float fadius = (-linear + std::sqrtf(linear * linear - 4 * quadratic * (constant - (256.0 / 5.0) * lightMax)))
					/ (2 * quadratic);	//求根公式～
				//它会返回一个大概在1.0到5.0范围内的半径值，它取决于光的最大强度。
			}
			lightPass_shader.setVec3("viewPos",camera.Position);
			// Finally render quad
			RenderQuad(lightPass_shader);
		
		// 2.5. copy content of geometry's depth buffer to default framebuffer's depth buffer
		// ----------------------------------------------------------------------------------
		glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
		/*	blit to default framebuffer. Note that this may or may not work as the internal formats 
		of both the FBO and default framebuffer have to match. the internal formats are implementation defined. 
		This works on all of my systems, but if it doesn't on yours you'll likely have to write to the 		
		depth buffer in another shader stage (or somehow see to match the default framebuffer's internal 
		format with the FBO's internal format).
			blit到默认帧缓冲区。 请注意，这可能有效，也可能无效，因为FBO和默认帧缓冲区的内部格式必须匹配。
			内部格式是实现定义的。 这适用于我的所有系统，但如果在你的系统上不行，你可能不得不在另一个着色
			器阶段写入深度缓冲区（或以某种方式匹配默认帧缓冲区的内部格式与FBO的内部格式）。
		*/
		glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		//glBlitFramebuffer会将一个用4个屏幕空间坐标所定义的源区域复制到一个同样用4个屏幕空间坐标所定义的目标区域中。
		//前四个参数是屏幕空间的坐标， 后四个还是屏幕空间坐标。
		//最后两个参数：前一个说明复制深度信息，后一个说明复制方式
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//now render all light cubes with forward rendering as we'd normally do
		//glDisable(GL_DEPTH_TEST);
		light_shader.use();
		light_shader.setMartix("projection", projection);
		light_shader.setMartix("view", view);
		renderLight(light_shader, lightPositions , lightColors);
		//glEnable(GL_DEPTH_TEST);
		//swap and poll
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

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
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);		//顶点数据
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));	//法向量数据
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));	//纹理坐标数据
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
	if (!CubeisGenerated) {	//若数据没有生成，则先生成数据
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
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);		//顶点数据
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));	//法向量数据
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));	//纹理坐标数据
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		CubeisGenerated = true;
	}
	shader.use();
	glBindVertexArray(cubeVAO);

	for (int i = 0; i < cubePositions.size(); ++i) {
		glm::mat4 model = glm::mat4();
		model = glm::translate(model, cubePositions[i]);
		if(!cubeRotateAxis.empty())
			model = glm::rotate(model, glm::radians(i * 15.0f), cubeRotateAxis[i]);
		shader.setMartix("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
	glBindVertexArray(0);

}

void renderLight(Shader shader, std::vector<glm::vec3> lightPositions, std::vector<glm::vec3> lightColors) {
	if (!CubeisGenerated) {	//若数据没有生成，则先生成数据
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
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);		//顶点数据
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));	//法向量数据
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));	//纹理坐标数据
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
		if(!lightColors.empty())
			shader.setVec3("cubeColor", lightColors[i]);
		else
			shader.setVec3("cubeColor", glm::vec3(1.0f,1.0f,1.0f));
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
