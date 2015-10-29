
uniform mat4 vp;
uniform mat4 model;

in vec3 position;
in vec3 normal;
in vec3 tangent;
in vec3 bitangent;
in vec2 texCoord;

out vec2 texCoordVar;


void main(){
	gl_Position = vp * model * vec4(position, 1);
	texCoordVar = texCoord;
}

