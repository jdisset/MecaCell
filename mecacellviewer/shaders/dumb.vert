
in vec3 vertex;

out highp vec2 UV;

void main(void){
	gl_Position = vec4(vertex,1.0);
	UV = (vertex.xy+highp vec2(1.0,1.0))*0.5;
}

