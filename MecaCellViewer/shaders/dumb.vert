#version 330 core
in vec3 vertex;

out vec2 UV;

void main(void){
   gl_Position = vec4(vertex,1.0);
	UV = (vertex.xy+vec2(1.0,1.0))*0.5;
}

