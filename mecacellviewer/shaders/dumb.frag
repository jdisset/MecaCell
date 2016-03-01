uniform sampler2D tex;
uniform float alpha = 1.0;
in vec2 UV;
out vec4 FragColor;

void main(void){
	FragColor = vec4(texture(tex,UV).rgb, alpha);
}
