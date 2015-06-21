

layout(location = 0) in vec3 vert;
layout(location = 1) in vec3 color;

out vec2 UV;
out vec2 s;
out vec3 fragColor;

uniform vec3 tangent;
uniform vec3 normal;
uniform mat4 VP;
uniform vec3 pos;
uniform vec2 size;

void main(){
    vec3 center = pos;
    vec3 vertex = center + tangent * vert.x * size.x + normal * vert.y * size.y;
    gl_Position = VP * vec4(vertex,1.0f);
    UV = vert.xy + vec2(0.5f, 0.5f);
    s = size;
    fragColor = color;
}

