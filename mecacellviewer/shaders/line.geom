
layout(lines) in;
layout(triangle_strip, max_vertices=4) out;

void main(){	
	vec2 line  = (gl_in[1].gl_Position.xy - gl_in[0].gl_Position.xy);
	line = normalize(line);
	vec4 h = 3.0*vec4(line.y, -line.x, 0.0, 0.0);
	gl_Position = gl_in[0].gl_Position - h;
	EmitVertex();
	gl_Position = gl_in[0].gl_Position + h;
	EmitVertex();
	gl_Position = gl_in[1].gl_Position - h;
	EmitVertex();
	gl_Position = gl_in[1].gl_Position + h;
	EmitVertex();
	EndPrimitive();
} 

