
in vec2 UV;
uniform sampler2D tex;
uniform float xRatio;
uniform vec2 dir;
out vec4 FragColor;
#define C 0.00585f
const float offset[3] = float[]( 0.0, 1.3846153846*C , 3.2307692308*C );
const float weight[3] = float[]( 0.2270270270, 0.3162162162, 0.0702702703 );

void main(void){
	if (UV.x <= xRatio + xRatio*0.1){
		FragColor = texture(tex, UV) * weight[0];
		for (int i=1; i<3; i++) {
			FragColor += texture(tex, UV + dir * offset[i]) * weight[i];
			FragColor += texture(tex, UV - dir * offset[i]) * weight[i];
		}
	}
}
