#version 330 core
in vec4 FragPos;

uniform vec3 lightPos;
uniform float far_plane;

in vec2 TexCoords;
uniform sampler2D texture_diffuse;//漫反射贴图，用来discard

void main()
{

	//检测一个片段的alpha值是否低于某个阈值(0.1)
    vec4 texColor = texture(texture_diffuse, TexCoords);
    if(texColor.a < 0.1){
		//如果是的话，这个片段深度值设为1
        //lightDistance = 1.0f;
		discard;
	}

	//fragment位置和光源位置之间的线性距离
    float lightDistance = length(FragPos.xyz - lightPos);
    
    //将距离映射到0到1的范围
    lightDistance = lightDistance / far_plane;
    

    //写入为fragment的深度值
    gl_FragDepth = lightDistance;
}