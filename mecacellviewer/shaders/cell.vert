
uniform mat4 view;
uniform mat4 model;
uniform mat4 projection;

in vec3 position;
in vec4 color;
in vec3 normal;
in vec3 tangent;
in vec3 bitangent;
in vec2 texCoord;

out vec4 vertColor;
out vec2 texCoordVar;
out highp vec3 objectSpaceNormal;
out vec3 objectSpaceTangent;
out vec3 objectSpaceBitangent;
out vec3 cameraPosition;
out highp vec3 surfacePosition;


void main(){
	gl_Position = projection * view * model * vec4(position, 1);
	vertColor = color;
	texCoordVar = texCoord;
	/*eyespaceNormal = normalize(mat3(normalMatrix) * normal);*/
	objectSpaceNormal = normal;
	objectSpaceTangent = tangent;
	objectSpaceBitangent = bitangent;
	cameraPosition = -view[3].xyz * mat3(view);
	surfacePosition = vec3(model * vec4(position, 1.0f));
}
