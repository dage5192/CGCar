#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;
layout (location = 5) in mat4 instanceMatrix;//使用一个mat4的顶点属性，让我们能够存储一个实例化数组的变换矩阵

out vec2 TexCoords;

out VS_OUT {
    vec2 TexCoords;
	//未做改变的位置
    vec3 Normal;
    vec3 FragPos;
	vec3 lightPos;
	vec3 viewPos;

	//转换到切线空间后的位置
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;

	mat3 TBN;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform bool isInstanced;//判断是否实例化

uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
	//判断是否实例化
    if(isInstanced){
        gl_Position = projection * view * instanceMatrix * vec4(aPos, 1.0);
	}else{
		gl_Position = projection * view * model * vec4(aPos, 1.0);
	}

	//直接发送到fs
    vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
    vs_out.TexCoords = aTexCoords;
    vs_out.Normal = transpose(inverse(mat3(model))) * aNormal;
	vs_out.lightPos = lightPos;
	vs_out.viewPos = viewPos;

	//计算TBN矩阵
	vec3 T = normalize(vec3(model * vec4(aTangent,   0.0)));
	vec3 B = normalize(vec3(model * vec4(aBitangent, 0.0)));
	vec3 N = normalize(vec3(model * vec4(aNormal,    0.0)));
	mat3 TBN = transpose(mat3(T, B, N));//求逆
	//将光源、观察者、转换到切线空间后再发送
    vs_out.TangentLightPos = TBN * lightPos;
    vs_out.TangentViewPos  = TBN * viewPos;
    vs_out.TangentFragPos  = TBN * vs_out.FragPos;

	vs_out.TBN = mat3(T, B, N);
}