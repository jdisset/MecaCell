#version 330 core
uniform mat4 view;
uniform mat4 model;
uniform mat4 projection;

in vec3 vertex;
out vec2 UV;
out vec3 Scale;

void main(){
	gl_Position = projection * view * model * vec4(vertex, 1);
	Scale.x = length(model[0]);
	Scale.y = length(model[1]);
	Scale.z = length(model[2]);
	UV = (vertex.xy+vec2(1.0,1.0))*0.5;
}
