#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec2 TexCoords;
	//未做改变的位置
    vec3 Normal;
    vec3 FragPos;
	vec3 lightPos;
	vec3 viewPos;
	
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;

	mat3 TBN;
} fs_in;

uniform sampler2D texture_diffuse;//漫反射贴图
uniform sampler2D texture_specular;//镜面反射贴图
uniform sampler2D texture_shininess;//光泽度贴图
uniform sampler2D texture_normal;//法向量贴图
uniform samplerCube depthMap;

uniform float far_plane;
uniform bool shadows;

uniform bool isShininessMap;
uniform bool isNormalMap;
uniform vec3 lightAmbient;

//采样的偏移量方向数组，它剔除了彼此接近的那些子方向
vec3 gridSamplingDisk[20] = vec3[]
(
   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);
float ShadowCalculation(vec3 fragPos, vec3 normal, vec3 lightDir, vec3 lightPos, vec3 viewPos)
{
    
    vec3 fragToLight = fragPos - lightPos;//从当前fragment到光源位置的向量 
    float currentDepth = length(fragToLight);//从当前fragment到光源位置的距离
    //float closestDepth = texture(depthMap, fragToLight).r;//从立方体贴图中采样，获取光源和它最接近的可见fragment之间的标准化的深度值
    //closestDepth *= far_plane;//closestDepth值现在在0到1的范围内，所以我们将其转换回0到far_plane的范围
    //float bias = 0.05;//阴影偏移，用以避免阴影失真
    //float shadow = currentDepth -  bias > closestDepth ? 1.0 : 0.0;//当前的fragment是否在阴影当中   

	if(currentDepth/far_plane > 1.0)return 0;//转换到0到1的范围，然后进行比较。在far_plane以外的区域当作没有阴影

    float shadow = 0.0;
    float bias = max(0.25 * (1.0 - dot(normal, lightDir)), 0.15);//根据表面法向和光线的角度更改偏移量。角度越大，偏移量越大
    int samples = 20;//采样数量
    float viewDistance = length(viewPos - fragPos);//从观察者到fragment的距离
    float diskRadius = (1.0 + (viewDistance / far_plane)) / 100.0;//根据观察者的距离来增加偏移半径。当距离更远的时候阴影更柔和，更近了就更锐利。
    for(int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(depthMap, fragToLight + gridSamplingDisk[i] * diskRadius).r;//从立方体贴图中采样
        closestDepth *= far_plane;//closestDepth值现在在0到1的范围内，所以我们将其转换回0到far_plane的范围
        if(currentDepth - bias > closestDepth)
            shadow += 1.0;
    }
    shadow /= float(samples);  
    //debug时显示closestDepth（以将深度立方体贴图可视化）
    // FragColor = vec4(vec3(closestDepth / far_plane), 1.0);    


    return shadow;
}

void main()
{
	//检测一个片段的alpha值是否低于某个阈值(0.1)
    vec4 texColor = texture(texture_diffuse, fs_in.TexCoords);
    if(texColor.a < 0.1){
		//如果是的话，这个片段不会被进一步处理
        discard;
	}
	
	vec3 normal;
	vec3 fragPos;
	vec3 lightPos;
	vec3 viewPos;
	if(isNormalMap){	
		//normal = texture(texture_normal, fs_in.TexCoords).rgb;
		//normal = normalize(normal * 2.0 - 1.0);
		//normal = normalize(fs_in.TBN * normal);
		//fragPos = fs_in.FragPos;
		//lightPos = fs_in.lightPos;
		//viewPos = fs_in.viewPos;

		
		//使用法线贴图
		normal = texture(texture_normal, fs_in.TexCoords).rgb;
		normal = normalize(normal * 2.0 - 1.0);
		fragPos = fs_in.TangentFragPos;
		lightPos = fs_in.TangentLightPos;
		viewPos = fs_in.TangentViewPos;
		
	}else{
		//normal = texture(texture_normal, fs_in.TexCoords).rgb;
		//normal = normalize(normal * 2.0 - 1.0);
		//normal = normalize(fs_in.TBN * normal);
		normal = normalize(fs_in.Normal);
		fragPos = fs_in.FragPos;
		lightPos = fs_in.lightPos;
		viewPos = fs_in.viewPos;
	}
    
    vec3 lightDiffuse = vec3(0.7f);
    vec3 lightSpecular = vec3(1.0f);
    //环境光（与点光源无关）
    vec3 ambient = lightAmbient * texture(texture_diffuse, fs_in.TexCoords).rgb;
    //漫反射
    vec3 lightDir = normalize(lightPos - fragPos);//光源方向
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightDiffuse * texture(texture_diffuse, fs_in.TexCoords).rgb;
    //镜面反射
    vec3 viewDir = normalize(viewPos - fragPos);//观察者方向
    vec3 reflectDir = reflect(-lightDir, normal);//光线反射方向
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);//光线方向和观察者方向之间的半程向量
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightSpecular * texture(texture_specular, fs_in.TexCoords).rgb;
	if(isShininessMap){//判断是否启用光泽度贴图
		 specular *= texture(texture_shininess, fs_in.TexCoords).r;
	}

    //光线的衰减（这组参数能覆盖3250距离）
	float constant = 1.0f;
    float linear = 0.0014f;
    float quadratic = 0.000007f;
    float distance    = length(lightPos - fragPos);//光源位置到fragment的距离
    float attenuation = 1.0 / (constant + linear * distance + quadratic * (distance * distance));//衰减公式
	ambient  *= attenuation; 
	diffuse  *= attenuation;
	specular *= attenuation;

    //计算阴影//!注意：计算阴影时必须使用世界空间而不是切线空间中的fragPos，lightDir，lightPos和viewPos
    float shadow = shadows ? ShadowCalculation(fs_in.FragPos, normal, normalize(fs_in.lightPos - fs_in.FragPos), fs_in.lightPos, fs_in.viewPos) : 0.0;                      
    vec3 lighting = ambient + (1.0 - shadow) * (diffuse + specular);    
    

    FragColor = vec4(lighting, 1.0);
}