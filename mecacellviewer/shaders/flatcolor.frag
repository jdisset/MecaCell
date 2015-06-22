
uniform vec4 color;
out vec4 FragColor;
in vec2 UV;
in vec3 Scale;
void main(){
   FragColor = color;
	float b = 0.1/Scale.x;
	float w = 3.0/Scale.x;
	float l = 100.0/Scale.x;
	float dif = 0.05f;
	if (mod(UV.x,l) < w+b){
		float z = dif ;//gl_FragCoord.z / gl_FragCoord.w;
		z = z - mix(0,z,(max(0.0,mod(UV.x,l)-w))/b);
		FragColor -= vec4(z,z,z,0.0);
	}
	else if (mod(UV.y,l) < w+b){
		float z = dif ;//gl_FragCoord.z / gl_FragCoord.w;
		z = z - mix(0,z,(max(0.0,mod(UV.y,l)-w))/b);
		FragColor -= vec4(z,z,z,0.0);
	}
}

