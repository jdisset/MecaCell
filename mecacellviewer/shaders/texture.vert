uniform mat4 view;
uniform mat4 model;
uniform mat4 projection;
uniform float texrepeat;

in vec3 position;
in vec3 normal;
/*in vec2 texCoord;*/

out vec2 UV;

void main() {
	UV = (texrepeat*position.xy+highp vec2(1.0,1.0))*0.5;
	mat4 mv = view*model;
	vec4 vertPos4 = mv * vec4(position, 1.0);
	gl_Position = projection * vertPos4;
}
