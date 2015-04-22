#version 330 core

in vec3 fragColour;

out vec4 finalColour;

void main(){
   finalColour = vec4(fragColour, 0.5);
}
