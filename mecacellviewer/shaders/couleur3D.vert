
vec3 position;
uniform vec4 color;
uniform mat4 vp;
uniform mat4 model;

out vec4 fragColour;

void main(){
	gl_Position = vp * model * vec4(position, 1.0);
	fragColour = color;
}
