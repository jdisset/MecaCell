
uniform mat4 view;
uniform mat4 model;
uniform mat4 projection;

in vec3 position;
in vec2 texCoord;

out vec2 UV;

void main(void){
	gl_Position = projection * view * model * vec4(position, 1.0);
	UV = texCoord; 
}


