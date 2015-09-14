uniform mat4 view;
uniform mat4 model;
uniform mat4 projection;
uniform mat4 normalMatrix;

in vec3 position;
in vec3 normal;
in vec2 texCoord;

out vec2 UV;
out vec3 surfacePosition;
out vec3 normalInterp;

void main(){
	UV = texCoord;
	mat4 mv = view*model;
	vec4 vertPos4 = mv * vec4(position, 1.0);
	gl_Position = projection * vertPos4;
	surfacePosition = vec3(vertPos4) / vertPos4.w;
	normalInterp = vec3(normalMatrix * vec4(normalize(normal),0.0));
}
