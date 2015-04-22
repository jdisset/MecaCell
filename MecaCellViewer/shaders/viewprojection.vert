#version 330 core
uniform mat4 view;
uniform mat4 projection;

in vec3 position;

void main(){
    gl_Position = projection * view * vec4(position, 1.0);
}

