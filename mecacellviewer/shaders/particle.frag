

in vec2 UV;
in vec4 fColor;

out vec4 finalColor;

uniform sampler2D tex;


void main(){
	/*finalColor = texture(tex,UV)*fColor;*/
	vec2 uv = UV-0.5f;
   float sql = uv.x*uv.x + uv.y*uv.y;
	finalColor = sql < 0.25f ? mix(vec4(fColor.rgb*fColor.rgb,fColor.a),fColor,sql*4.0f) : vec4(0.0);
}

