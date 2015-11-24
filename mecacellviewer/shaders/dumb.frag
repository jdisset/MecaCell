
in vec2 UV;
uniform sampler2D tex;
out vec4 FragColor;

void main(void){
	vec4 col = texture(tex,UV);
	FragColor = col;
	/*FragColor = vec4(1.0,0.8,0.2,1.0);*/
}
