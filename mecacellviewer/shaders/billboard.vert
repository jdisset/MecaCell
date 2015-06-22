

in vec3 squareVertices;
out vec2 UV;

uniform vec3 cameraRight;
uniform vec3 cameraUp;
uniform mat4 VP;
uniform vec3 pos;
uniform vec2 size;

void main(){
    vec3 center = pos;
    vec3 vertex = center + cameraRight * squareVertices.x * size.x +cameraUp * squareVertices.y * size.y;
    gl_Position = VP * vec4(vertex,1.0f);
    UV = squareVertices.xy + vec2(1.0f, 1.0f);
	 UV = UV*0.5f;
}


