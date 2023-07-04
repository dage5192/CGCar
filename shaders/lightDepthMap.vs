#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 5) in mat4 instanceMatrix;//使用一个mat4的顶点属性，让我们能够存储一个实例化数组的变换矩阵

//在fs中进行discard时使用
layout (location = 2) in vec2 aTexCoords;
out VS_OUT {
    vec2 texCoords;
} vs_out;

uniform mat4 model;
uniform bool isInstanced;

void main()
{
	vs_out.texCoords = aTexCoords;

	//将顶点变换到世界空间，然后直接发送到几何着色器
	//判断是否实例化
    if(isInstanced){
        gl_Position = instanceMatrix * vec4(aPos, 1.0);
	}else{
		gl_Position = model * vec4(aPos, 1.0);
	}
}