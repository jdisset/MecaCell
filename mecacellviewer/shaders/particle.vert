

layout(location = 0) in vec3 squareVertices;
layout(location = 1) in vec3 pos;
layout(location = 2) in vec4 color;

out vec2 UV;
out vec4 fColor;

uniform vec3 cameraRight;
uniform vec3 cameraUp;
uniform mat4 VP;
uniform vec2 size;

void main(){
    vec3 vertex = pos + cameraRight * squareVertices.x * size.x +cameraUp * squareVertices.y * size.y;
    gl_Position = VP * vec4(vertex,1.0f);
    UV = squareVertices.xy*0.5f + vec2(0.5f, 0.5f);
	 fColor = color;
}


