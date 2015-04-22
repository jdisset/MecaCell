#version 330 core
in vec2 UV;
uniform sampler2D tex;
uniform vec3 viewVector;
out vec4 FragColor;

void main(){

   FragColor = texture(tex,UV);
   FragColor.a   = 1.0;
   gl_FragDepth = 0.999999;
}

