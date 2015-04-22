#version 330 core

in layout(location=0)vec3 vertex;
uniform vec3 color;
uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

out vec3 fragColour;

void main(){
    gl_Position = projection * view * model * vec4(vertex, 1.0);
    fragColour = color;
}
