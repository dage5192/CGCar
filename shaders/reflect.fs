#version 330 core
out vec4 FragColor;
in vec3 Normal;
in vec3 Position;

uniform vec3 cameraPos;
uniform samplerCube skybox;

void main(){
	vec3 sightLine = normalize(Position - cameraPos);
	vec3 reflectLine = reflect(sightLine, normalize(Normal));
	FragColor = vec4(texture(skybox, reflectLine).rgb, 1.0);
}