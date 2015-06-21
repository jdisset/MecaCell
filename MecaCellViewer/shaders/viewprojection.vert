
uniform mat4 viewProjection;

in vec3 position;

void main(){
    gl_Position = viewProjection * vec4(position, 1.0);
}

