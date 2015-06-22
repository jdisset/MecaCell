// Version du GLSL

#version 150 core


// Entrée
in vec3 color;
in vec2 coordTexture;


// Uniform

uniform sampler2D tex;


// Sortie 

out vec4 out_Color;


// Fonction main

void main()
{
    // Couleur du pixel

    out_Color = texture(tex, coordTexture)*vec4(color,1.0);
}
