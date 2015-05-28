#version 330 core
in vec2 UV;
uniform sampler2D tex;
out vec4 FragColor;

void main(void){
	FragColor = texture(tex, UV);
}
