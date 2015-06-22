

in vec2 UV;
out vec4 finalColor;
uniform sampler2D tex;

uniform vec4 color;

void main(){
	finalColor = texture(tex,UV)*color;
	/*finalColor = color;*/
}
