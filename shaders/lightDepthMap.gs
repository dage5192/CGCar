#version 330 core
layout (triangles) in;//以3个三角形的顶点作为输入
layout (triangle_strip, max_vertices=18) out;//输出6个三角形

uniform mat4 shadowMatrices[6];//光空间变换矩阵

out vec4 FragPos; // FragPos from GS (output per emitvertex)

//在fs中进行discard时使用
in VS_OUT {
    vec2 texCoords;
} gs_in[];
out vec2 TexCoords;

void main()
{
	//遍历立方体贴图的6个面
    for(int face = 0; face < 6; ++face)
    {
        gl_Layer = face; //指定立方体贴图当前的那一个面为输出面
        for(int i = 0; i < 3; ++i) //遍历三角形的每一个顶点
        {
            FragPos = gl_in[i].gl_Position;
            gl_Position = shadowMatrices[face] * FragPos;//将世界空间的顶点变换到相关的光空间
			TexCoords = gs_in[i].texCoords;
            EmitVertex();
        }    
        EndPrimitive();
    }
} 