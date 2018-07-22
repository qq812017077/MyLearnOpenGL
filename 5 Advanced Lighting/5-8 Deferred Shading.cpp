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
#include "Model.h"

/*	Deferred Shading �ӳ���ɫ ������  Deferred Rendering �ӳ���Ⱦ��
	��ĿǰΪֹ������ʹ�õĹ��շ�ʽ������������Ⱦ(forward rendering)�ͻ���������ɫ(forward shading)
	����һ����Ⱦ�������ֱ�ӵķ�ʽ�����Ǹ��ݳ����е����й�Դһ��������Ⱦ�������塣
	���������⣬Ҳ������ʵ�֣�Ȼ��������ܵĸ���Ҳ���أ�
		����ÿһ����Դ����Ҫ��ÿһ����Ⱦ���������Ⱦ������������Ƿǳ����
	��һ������ȸ����Եĳ����У�������ȾҲ���˷Ѻܶ��Ƭ����ɫ������
	����������غ���ͬһ������λ�õ�ʱ������Ƭ����ɫ������ᱻ���ǣ���

	�ӳ���Ⱦ�����ӳ���ɫ�������ǳ����ڽ�����������⣬���ı�����Ⱦ����ķ�ʽ��
	����������µ�ѡ���������Ż�����������Դ�ĳ��������ǿ����ڿɽ���֡�ʵ�ǰ������Ⱦ�ɰ���ǧ�ĵ�Դ��


	�ӳ���ɫ��˼���ǣ����ӳٻ���˵�Ƴٴ�����Ĵ����������Ⱦ�����ǵƹ⣩�����ڴ����С�
	����������
		1.	��һ���������ν׶Σ��ý׶�������Ⱦһ�γ������ҴӶ�������ȡ�����еļ�����Ϣ����������һϵ�н���G-buffer
	�������У�����һ�£�λ����������ɫ���������������������淴��������
		��������Щ������G�����еļ�����Ϣ������֮��������(�����ӵ�)���ռ��㡣
		2.	�ڶ�����Ҳ�й⴦��׶�(lighting pass)�У�����ʹ����G-buffer�е�����
			��һ���У�������Ⱦһ�������������ı��Σ���ʹ�ñ�����G-buffer�еļ�����Ϣ��������Ϊÿһ��Ƭ��
	���㳡���Ĺ��գ�ÿ�����ض�ʹ��G-buffer���е�����
			��һ�Σ����ǲ���һ·����ÿ������Ӷ��㵽Ƭ����ɫ�������ǽ����ĸ߼�Ƭ�δ�����뵽���ڴ����С�
	���յļ����֮ǰ��ͬ��������һ�����Ǵ�G-buffer�н��������������������������ǴӶ�����ɫ���У�
		
		����������learnOpenGLͼƬ������������ͨ����һ���ᵽ��MRT����������Ϣ�ֱ𱣴���G-buffer�У�Ȼ���ں��ڴ���
	
	���ƣ�
		��G-buffer�����յ�Ƭ��ʵ���Ͼ���ʵ�ʵ�������Ϊ��Ļ���ص�Ƭ����Ϣ����Ϊ��Ȳ����Ѿ��ж����Ƭ����ϢΪ����Ƭ��
	���ȷ���������ڹ⴦��׶δ����ÿһ�����ض�������һ�Σ������������õ���Ⱦ���á����⣬�ӳ���Ⱦ����������Ⱦ��
	����������Ⱦ�ǳ���ĵƹ⡣

	ȱ�㣺
		1.	G-buffers ��Ҫ���Ǳ�������ĳ�������������ͼ����ɫ�������У����Ե��ܶ���ڴ棬
			�ر�����λ������������������Ҫ�ܸߵľ��ȡ�
		2.	��֧��Blending����Ϊ����ֻ���������ݣ���MSAAҲ������Ч��
		���ǿ���ʹ��һЩ�ֶ����˷���Щȱ�ݡ�
	
	�ڼ��ν׶����G-buffer�Ƿǳ���Ч�ģ���Ϊ����ֱ�ӱ�������λ�á���ɫ�����Ƿ��������������ݵ�֡�������У�
	���⼸���������Ĵ���ʱ�䡣ͨ��ʹ��MRT(multiple render target)���������������ڵ�����Ⱦ�����ж����еĹ���
*/

/* G-buffers
	G-buffer���������ڱ������������ݵ�������ܳƣ����������յĹ��մ���׶Ρ�
		����������Ⱦ��Ƭ�ι��ռ�������������ܽ����£�
			һ��3D��λ�����������ڼ���Ƭ��λ�ã�����LightDir�Լ�ViewDir��
			һ��RGB��������ɫ������Ҳ����albedo��������
			һ��3D�ķ����������ھ����������б�ȣ�slope
			һ�����淴��ǿ�ȣ�floatֵ��
			���еĹ�Դλ�ú���ɫ������
			��һ��߹۲��ߵ�λ��������
		ʹ����Щ���������ǿ��Լ���(Blinn-)Phong light�����������õĹ��ռ��㹫ʽ��
		
			��Դ��λ�ú���ɫ����ҵĹ۲�λ�ã�ͨ��uniform�������ã����Բ��÷��õ�G-buffer��
			�����ı���������ÿһ�������ÿ��Ƭ�ζ����������ض��ģ���

			��������ܹ���ĳ�ַ�ʽ����ȡͬ�������ݵ������ӳٹ��մ����н��У����ǾͿ��Լ����ͬ���Ĺ���Ч����
		��ʹ����������Ⱦһ���ķ��ε�Ƭ�Ρ�
			
	OpenGL�У������ܹ��洢��Texture�е�������û�����Ƶġ����԰����е���Ƭ�����ݴ洢��һ������ȫ������Ҳ����ΪG-buffers��
	�������������Ĺ��մ�����ΪG-buffer������͹⴦���2D�ķ�������ͬ���Ĵ�С�����ǿ��Եõ�����������Ⱦ�������õ�ͬ�������ݣ�
	ֻ���������ڹ⴦����ڡ�
	α���룺
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
	ע��������Ҫ�����ÿ��Ƭ�ε������ǣ�λ������������������ɫ�������������ǿ��ֵ��
	�ڼ��δ���׶Σ�
		������Ҫ��Ⱦ�������������壬������Щ�������������G-buffer�С��������ٴ�ʹ��MRT���ڵ�����Ⱦ����׶�
	��������Ⱦ�������ɫ�������У���Bloom���Ѿ����۹�����
		���ڼ��δ���׶Σ����ǽ���Ҫ��ʼ��һ��֡������󣬲�������ֱ�۵�(intuitively)��֮ΪgBuffer,���ϸ�����
	�����ɫ�����������һ����������Ȼ���������
		����λ�������ͷ������������Ǹ�Ը��ʹ��һ���߾�������ÿ�����Ϊ16λ��32λ�ĸ��㣩��
		�����ʺ;��淴��֮�����򱣴���Ĭ�ϵ�ÿ�����8λ���ȵ������У������㹻�ġ�
	
		G-buffer�����ô�����·���


	��Ϊ����ʹ�ö���ȾĿ��MRT�����Ǳ�����ʽ��ʹ��glDrawBuffers��������������Ҫ��Ⱦ����Щ����ɫ��������
	������Ȥ���ǣ����ǰ�λ�úͷ�������������һ��RGB������Ϊ�������������������������
	�����ǰ���ɫ�;��淴��ǿ�������������һ��������RGBA�����У��������ǿ���������һ���������ɫ��������
	
	�����ӳ���Ⱦ�ܵ����Խ��Խ���ӣ���ҪԽ��Խ������ݣ����ǽ�Ѹ�ٷ����µķ�ʽ��������ݵ�һ�������������С�


	��������Ⱦ��G-buffer�С�����ÿ��������һ��diffuse��һ��normal���Լ�һ�����淴��ǿ������
		������Ҫʹ����Shader5-8.fragment��������ɫ������Ⱦ��G-buffer�С�

	��Ϊ����ʹ����MRT��ͨ��ָ���Ĳ㼶����OpenGL˵������Ҫ��Ⱦ����ǰ�����֡����������һ����ɫ��������
	ע������û�аѾ��淢����ձ�����һ����������ɫ���������У���Ϊ�����ܱ������ĵ���float������һ����ɫ��������alphaͨ����

	������
	���ס����Ϊ�й��ռ��㣬���Ա�֤���б�����һ������ռ䵱��������Ҫ������������������ռ��д洢(������)���еı�����
	������


	����������Ҫ��Ⱦһ��ѵ�����װ��սʿ��gBuffer��֡�������С����������ݵ���ɫ������һ��һ����ͶӰ���ı����ϣ�

	������������ռ�λ�úͷ�����������ȷ�ġ�����˵��ָ���Ҳ�ķ��������ᱻ����ض��뵽��ɫ�ϣ�
	�ӳ���ԭ��ָ���Ҳ��λ��ʸ��Ҳͬ����������һ�����G�����е����������ˣ����Ǿ͸ý��뵽��һ�������մ���׶��ˡ�

*/

/*	Deferred lighting pass	�ӳٹ⴦��׶�
		������֧���G-buffer���д�����Ƭ�����ݣ����ǿ���ͨ����ÿ��G-buffer��������������ش�����������������
	�����������Ϊ�����㷨�����룬���Լ�������յĹ��ս����
		��ΪG-buffer�����ֵ������ת��������ֵ���������ǽ���Ҫ��ÿ������ִ��һ�ΰ���Ĺ��ռ��㡣
	��ʹ���ӳ���Ⱦ��÷ǳ���Ч���ر�������Ҫ���ô�������Ƭ����ɫ���ĸ��ӳ����С�

	����������մ���׶Σ����ǽ�����Ⱦһ��2Dȫ���ķ���(��һ������ڴ���Ч��)������ÿ������������
	һ������Ĺ���Ƭ����ɫ����
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shaderLightingPass.Use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gPosition);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gNormal);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
		// ͬ�����͹�����ص�uniform
		SendAllLightUniformsToShader(shaderLightingPass);
		glUniform3fv(glGetUniformLocation(shaderLightingPass.Program, "viewPos"), 1, &camera.Position[0]);
		RenderQuad(); 
	����Ⱦ֮ǰ������Ҫ�����е�G-buffer����������������uniform�������͵���ɫ���С�

	�µ���ɫ�����������Ǵ������л�ȡ���⣨�ǵ�����֮ǰ����VBO�������㣬���ɶ��㷢��Ƭ�Σ�����ÿ����������е㶼��Ҫ���з��ͣ�
	
	��������ֻ�ᷢ���ܿ��õ��ģ�
	
	����Ƭ����Ⱦ��ɫ��ΪShader5-8.fragment_render
		���մ�����ɫ����������uniform������������Ǵ�����G-buffer�����Ұ��������м��δ���׶�֮ǰ��������ݣ�
	�������Ҫʹ�õ�ǰ��Ƭ����������������ݣ����ǽ����ú�֮ǰ��ȫһ����Ƭ��ֵ����������ֱ����Ⱦ������һ����
	��Ƭ����ɫ���Ŀ�ʼ�����Ǵ�G-buffer�����н��չ���������ݣ�ͨ��һ���򵥵�ѭ����ע�����Ǵӵ���gAlbedoSpec������
	����Albedo��ɫ�;������ǿ��ֵ��

	��Ϊ�����������˼���Blinn-Phong���յ�ÿ��Ƭ���������ֵ���Լ���ص�uniform�����������Զ��ڹ��մ��벻��Ҫ�κα仯��
	Ψһ�ĸı���ǻ�ȡ��������ķ�����

	�ӳ���Ⱦ��һ�������ǣ�����Ӧ���ڻ��(blending)��ΪG-buffer�е�����ֵ����һ��fragment�����ģ�����ϲ�����Ҫ���Ƭ�ε���ϡ�
	
	��һ�������ǣ���ʾ��ȾҪ�����Ǳ���ʹ��ͬһ�ֹ����㷨�����ڴ󲿷ֳ������գ�����ͨ������������ڲ��ʵ����ݵ�
	G��������������һȱ�㡣

	Ϊ�˿˷���Щȱ��(�ر��ǻ��)������ͨ���ָ����ǵ���Ⱦ��Ϊ�������֣�
		һ�����ӳ���Ⱦ�Ĳ��֣���һ����ר��Ϊ�˻�ϻ����������ʺ��ӳ���Ⱦ���ߵ���ɫ��Ч������Ƶĵ�������Ⱦ�Ĳ��֡�
	Ϊ��չʾ������ι����ģ����ǽ���ʹ��������Ⱦ����Ⱦ��ԴΪһ��С�����壬��Ϊ�������������Ҫһ���������ɫ��
	(�򵥵����һ��������ɫ)��
*/

/*Combining deferred rendering with forward rendering ����ӳ���Ⱦ��������Ⱦ
	����������Ҫ�ѹ�Դ��Ⱦ��cube�����ڹ�Դ��λ�ò������ӳ���Ⱦ���������ɫ��
	��һ���£�
		�򵥵����ӳ���Ⱦ�ܵ���������ӳٹ⴦�����ķ���֮�Ͻ���������Ⱦ��
	��������ֻ��Ҫ�������������Ⱦ�����壬ֻҪע��������ӳ���Ⱦ֮�������������ͺá�
		������·��Ĵ���
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
		PS��postscript����
			��ʵ���Ҳ�֪��������ôд����Ϊ���ǿ��Եģ�ʵ���ϣ��ù������ߵĴ�����в��Ե�ʱ��
		��������������Ĵ��룬�ᷢ���޷���ʾ�������еķ���飡
			�����󣬵��Ǻ������ǻ�������ȵ����⡣��Ϊ������ʼ�ջ�����z:0��λ���ϵġ����λ�ÿ���ȷʵ������ϴ��ں�ǰ��
			�����ҳ�����һ�£�
				�ر���Ȳ��ԣ�Ȼ����ƣ�Ȼ���ٴ���Ȳ��ԣ��������ܳ��ֺ���������һģһ��������ˣ�������

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	����Ҫע���һ���ǣ�
		�����Ļ�����ֻ�ǰ���Щ��Ⱦ�����������岢û�п��ǵ����Ǵ�����ӳ���Ⱦ���ļ������(Depth)��Ϣ��
	���ҽ����������Ⱦ��֮ǰ��Ⱦ��������֮�ϣ��Ⲣ����������Ҫ�Ľ����
		��������Ҫ���������Ȼ�ȡ�ڼ��δ���׶α�����Ĭ����Ȼ������е������Ϣ�����ҽ���Ⱦ��Щ���Ҳλ��
	֮ǰ��Ⱦ��������Ϸ�����Щ���Ƭ�Ρ�

	����Ҳ����ͨ��glBlitFramebuffer������һ���������е����ݸ��Ƶ�����һ���������С������������Ҳ�ڿ���
	�ݵĽ̳���ʹ�ù���������ԭ���ز�����֡���塣
		��glBlitFramebuffer�����������ǽ�һ���û������֡�����������ݸ��Ƶ�����һ��֡����������

	���ӳ���Ⱦ�Ľ׶��У���������������Ϣ��������gBuffer���FBO�С���������Ǽ򵥵Ľ�������Ȼ�������
	�����ݸ��Ƶ�Ĭ�ϻ���������Ȼ������У�������ȾЧ���ͺ��������е����嶼��������Ⱦ�����һ����
	���翹��ݽ̳��еļ�̽���һ�������ǲ��ò�ָ��һ��һ��һ��֡����Ϊ��֡����(Read Framebuffer)��
	�������Ƶ�ָ��һ��֡����Ϊд֡����(Write Framebuffer)��
			glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
			glBlitFramebuffer(
			  0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST
			);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			// now render light cubes as before
			[...]  
	���ｫ������֡�������Ȼ�����Ϣ���Ƶ���Ĭ��֡�������Ȼ����С�		
	��������ɫ�����ģ�建������Ҳ������������
*/

/*A larger number of lights ����Ĺ�Դ
	�ӳ���Ⱦ���������ޣ���Ϊ���ܹ���Ⱦ�����Ĺ�Դ������������ܲ������صĸ�����
	��ʵ�ϣ���ʱ��Ⱦ��������������Ⱦ�ǳ���Ĺ�Դ����Ϊ�����Ա���Ϊÿһ�������Ĺ�Դ����ÿ��Ƭ�εĹ����֣�
	�����ô�����Դ��Ϊ���ܵ��������ܹ����ӳ���Ⱦ�������õ�һ���ǳ������Ż���
		�����(Light Volumes)
	
	ͨ���Ļ���������һ���������ճ����µ�Ƭ��ʱ��������Ҫ���㳡����ÿһ����Դ�Ĺ��ף�������������Ƭ�εľ��롣
	�ܴ�һ���ֵĹ�Դ�����Ͳ��ᵽ�����Ƭ�Σ�����Ϊʲô���ǻ�Ҫ�˷���ô����������أ�

	���Թ������Light Volumes�������˼���ǣ�
		�����Դ���ܵ�������������뾶�����ڴ󲿷ֹ�Դ��ʹ����ĳ����ʽ��˥��(Attenuation)�����ǿ���������
	�����Դ�ܹ���������·�̣�����˵�ǰ뾶�����ǽ�����ֻ��Ҫ����Щ��һ������������ڵ�Ƭ�ν��з��صĹ�����������ˡ�
	����Ը�����ʡ�����ܿɹ۵ļ���������Ϊ��������ֻ����Ҫ������¼�����ա�
		

	�����Դ�������뾶
		Ҫ��ù�Դ������뾶��������Ҫ�Ƚ��һ��������Ϊ�Ǻڰ�(Dark)������(Brightness)��˥�����̡�������0.0��Ҳ������
	һ���ǳ�С��ֵ�����ڿ��Ա���Ϊ�Ǻڰ��ģ�����0.03��
	
		Ϊ��չʾ������μ����Դ������뾶�����ǽ���ʹ��һ����Ͷ��������������һ�����Ӹ��ӣ�
	���ǳ�����˥�����̣�
		Flight =  I  / ( Kc + Kl * d + Kq * d^2)
		������Ҫ�����ǣ����������Flight = 0.0��ʱ��Ľ⣬Ҳ����˵���ڸþ�����ȫ�Ǻڰ��ġ�
	��ʵ�ϣ����Կ������÷��̲������0������Ӧ����������һ���ӽ���0��ֵ���ұ���Ϊ�Ǻڰ��ģ�
	�������һ�����Խ��ܵ�ֵ��5/256.����256����ΪĬ�ϵ�8-bit֡�������ÿ��������ʾ��ô��ǿ��ֵ(Intensity)��
		
		NOTE������ʹ�õ�˥�����������Ŀ��ӷ�Χ�ڻ������Ǻڰ��ģ��������������Ҫ������Ϊһ����5/256���Ӻڰ������ȣ�������ͻ�
	���̫��Ӷ���õ�Ч��ֻҪ���û������ڹ������Ե����һ��ͻأ�Ľضϣ����������û���ˡ���Ȼ�����������ڳ��������ͣ�һ
	���ߵ����ȷ�ֵ�������С�Ĺ�������Ӷ���ø��ߵ�Ч�ʣ�Ȼ����ͬ�������һ�������׷��ֵĸ����ã��Ǿ��ǹ���ڹ�����߽�
	������ͻȻ�ϵ���
		
		���Է��̽�ͱ���ˣ�
			5 / 256 = Imax / Attenuation
		�����Imax�ǹ�Դ�е�������ɫ���������ʹ��һ����Դ��������������
			��Ϊ���Դ������ǿ��ֵ������õط�ӳ�����������뾶��
			�ⷽ�����£�
				5 / 256 * Attenuation = Imax
			->	5 * Attenuation = Imax * 256
			->	Attenuation = Imax * 256 / 5
			->	Kc + Kl * d + Kq * d^2 - Imax * 256 / 5 = 0 
			-> ��һԪ���η��̣�
			ʹ�������ʽ���ɣ�

	*/	

/*	����ʹ��Light Volume - ������⣡
	ʵ���ϣ�Shader5-8.fragment_render_lightVolume �еĹ��������������Ч������
	��ֻ����ʾ������Ӧ�����ʹ�ù����ȥ���ٹ��ռ��㡣ʵ���ϣ�GPU��GLSL�ڴ���ѭ���ͷ�֧�ϲ�����ô���ڡ�
	ԭ������Ϊ��GPU�ϵ�shader��ִ���Ǹ߶Ȳ��еģ��󲿷ֵļܹ�������������Ҫ����Ϊ�����̵߳Ĵ��ڣ�
	GPU��Ҫ����������ȫһ������ɫ������Ӷ���ø�Ч�ʡ�
	��ͨ����ζ��һ����ɫ������ʱ����ִ��һ��if������еķ�֧�Ӷ���֤��ɫ�����ж���һ���ģ�
	��ʹ������֮ǰ�İ뾶����Ż���ȫ������ã�������Ȼ�ڶ����й�Դ������գ�

	Ҫʹ��light volume�����ʵķ�ʽ����Ⱦʵ�ʵ����壬���ŵ��������뾶��������ļ�ʹ��Դ��λ�á�
	�������Ǹ��ݹ�����뾶���ŵģ�����������ø����˹�Ŀ��������
		��������ǵļ��ɣ�
			����ʹ�ô�����ͬ���ӳ�Ƭ����ɫ������Ⱦ���塣��Ϊ�����������ȫƥ������Ӱ�����ص���ɫ�����ã���
		��ֻ��Ⱦ����Ӱ������ض��������������ء�
		
		����Ӧ���ڳ�����ÿ����Դ�ϣ��������õ�Ƭ����ӻ����һ����������֮ǰ������һ���ģ�
		����һ��ֻ��Ⱦ���ڹ�Դ��ص�Ƭ�Ρ�����Ч�ؼ����˴�nr_objects * nr_lights��nr_objects + nr_lights�ļ�������
		��ʹ�ö��Դ��������Ⱦ����ޱȸ�Ч��������Ϊʲô�ӳ���Ⱦ�ǳ��ʺ���Ⱦ�ܴ�������Դ��

		Ȼ�����������Ȼ��һ�����⣺
			���޳�(Face Culling)��Ҫ������(�������ǻ���Ⱦһ����Ч������)�������������õ�ʱ���û����ܽ���һ����Դ�Ĺ������
			Ȼ������֮���������Ͳ��ٱ���Ⱦ��(���ڱ����޳�)�����ʹ�ù�Դ��Ӱ����ʧ������������ͨ��һ��ģ�建�弼���������
	
	��Ⱦ�����
	ȷʵ��������ص����ܸ�������Ȼ��ͨ������ͨ���ӳ���Ⱦ���죬����Ȼ������õ��Ż���
	�������������ӳ���Ⱦ�ĸ�����(���Ҹ���Ч)����չ�����ӳٹ���(Deferred Lighting)����Ƭʽ�ӳ���ɫ��(Tile-based Deferred Shading)��
	��Щ������ܴ�̶�����ߴ�����Դ��Ⱦ��Ч�ʣ�����Ҳ������һ����Ը�Ч�Ķ��ز��������(MSAA)��
	Ȼ����������ƪ�̵̳ĳ��ȣ��ҽ�����֮��Ľ̳��н�����Щ�Ż���

*/

/*
�ӳ���Ⱦ vs ������Ⱦ
	�������ӳ���ɫ��������(û�й����)�Ѿ���һ���ܴ���Ż��ˣ�ÿ�����ؽ�������һ��������Ƭ����ɫ����Ȼ������������Ⱦ��
	����ͨ�����һ���������ж��Ƭ����ɫ������Ȼ���ӳ���Ⱦȷʵ����һЩȱ�㣺���ڴ濪����û��MSAA�ͻ��(����Ҫ������Ⱦ�����)��
	������һ����С�ĳ�������û�кܶ�Ĺ�Դʱ���ӳ���Ⱦ����һ�������һ�㣬������Щʱ�����ڿ��������������ŵ㻹�������
	Ȼ����һ�������ӵĳ����У��ӳ���Ⱦ����ٱ��һ����Ҫ���Ż����ر������˸��Ƚ����Ż���չ��ʱ��

	��󣬻�����������ͨ��������Ⱦ��ɵ�Ч���ܹ�ͬ�����ӳ���Ⱦ������ʵ�֣���ͨ����ҪһЩС�ķ��벽�衣
	�ٸ����ӣ����������Ҫ���ӳ���Ⱦ����ʹ�÷�����ͼ(Normal Mapping)��������Ҫ�ı伸����Ⱦ�׶���ɫ������
	��һ������ռ䷨��(World-space Normal)�����ӷ�����ͼ����ȡ����(ʹ��һ��TBN����)�����Ǳ��淨�ߣ�������
	Ⱦ�׶��еĹ�������һ�㶼����Ҫ�䡣�������Ҫ���Ӳ���ͼ��������������Ҫ�ڲ���һ������������䣬���棬��
	��������֮ǰ�����û�������Ⱦ�׶��е��������ꡣһ�����˽����ӳ���Ⱦ������������д�����������ʲô���¡�
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

void DeferredShading_Test(GLFWwindow * window);
void Model_Test(GLFWwindow * window);
/*
--����------------------------------
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
	DeferredShading_Test(window);
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

void Model_Test(GLFWwindow * window) {
	glEnable(GL_DEPTH_TEST);
	Shader geometryPass_Shader("Shader5-8.vertex_gbuffer", "Shader5-8.fragment_gbuffer");
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
//�ӳ���Ⱦ
void DeferredShading_Test(GLFWwindow * window) {
	glEnable(GL_DEPTH_TEST);
	//Shader
	Shader geometryPass_Shader("Shader5-8.vertex_gbuffer","Shader5-8.fragment_gbuffer");
	Shader lightPass_shader("Shader5-8.vertex_render", "Shader5-8.fragment_render_lightVolume");
	Shader light_shader("Shader.vertex_lamp","Shader.fragment_lamp");
	
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

	//G-buffer������
	GLuint gBuffer;
	glGenFramebuffers(1 , &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER ,gBuffer);
	GLuint gPosition, gNormal, gAlbedoSpec; //����������
	
	//0 - Position buffer
	glGenTextures(1, &gPosition);
	glBindTexture(GL_TEXTURE_2D , gPosition);
	glTexImage2D(GL_TEXTURE_2D ,0 , GL_RGB16F , SCR_WIDTH , SCR_HEIGHT , 0 , GL_RGB , GL_FLOAT , NULL);	//NULL��ʾ�յ�������
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D , GL_TEXTURE_WRAP_S , GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);	//Position - 0
	
	//1 - Normal buffer
	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);	//NULL��ʾ�յ�������
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);	//Normal - 1


	//2 - Color + specular buffer
	glGenTextures(1, &gAlbedoSpec);	//RGB color + specular value( 3 + 1 )
	glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);	//NULL��ʾ�յ�������
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
				float lightMax = std::fmaxf(std::fmaxf(lightColors[i].r, lightColors[i].g), lightColors[i].b);	//��ȡ������
				float fadius = (-linear + std::sqrtf(linear * linear - 4 * quadratic * (constant - (256.0 / 5.0) * lightMax)))
					/ (2 * quadratic);	//�����ʽ��
				//���᷵��һ�������1.0��5.0��Χ�ڵİ뾶ֵ����ȡ���ڹ�����ǿ�ȡ�
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
			blit��Ĭ��֡�������� ��ע�⣬�������Ч��Ҳ������Ч����ΪFBO��Ĭ��֡���������ڲ���ʽ����ƥ�䡣
			�ڲ���ʽ��ʵ�ֶ���ġ� ���������ҵ�����ϵͳ������������ϵͳ�ϲ��У�����ܲ��ò�����һ����ɫ
			���׶�д����Ȼ�����������ĳ�ַ�ʽƥ��Ĭ��֡���������ڲ���ʽ��FBO���ڲ���ʽ����
		*/
		glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		//glBlitFramebuffer�Ὣһ����4����Ļ�ռ������������Դ�����Ƶ�һ��ͬ����4����Ļ�ռ������������Ŀ�������С�
		//ǰ�ĸ���������Ļ�ռ�����꣬ ���ĸ�������Ļ�ռ����ꡣ
		//�������������ǰһ��˵�����������Ϣ����һ��˵�����Ʒ�ʽ
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
		if(!cubeRotateAxis.empty())
			model = glm::rotate(model, glm::radians(i * 15.0f), cubeRotateAxis[i]);
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
