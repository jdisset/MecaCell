
#define PI 3.14159265359

in vec2 UV;
uniform sampler2D tex;

out vec4 FragColor;

void main(){
   FragColor = texture(tex,UV);
   FragColor.a   = 1.0;
}


