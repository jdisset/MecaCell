
uniform mat4 view;
uniform mat4 model;
uniform mat4 projection;

in vec3 position;
in vec3 normal;
in vec3 tangent;
in vec3 bitangent;
in vec2 texCoord;

out vec2 texCoordVar;


void main(){
    gl_Position = vec4(position, 1);
    /*gl_Position = projection * view * model * vec4(position, 1);*/
    texCoordVar = texCoord;
    /*eyespaceNormal = normalize(mat3(normalMatrix) * normal);*/
    objectSpaceNormal = normal;
    objectSpaceTangent = tangent;
    objectSpaceBitangent = bitangent;
    cameraPosition = -view[3].xyz * mat3(view);
    surfacePosition = vec3(model * vec4(position, 1.0f));
}

