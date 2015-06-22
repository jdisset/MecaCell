// Version du GLSL

#version 150 core


// Entrées

in vec3 in_Vertex;
in vec3 in_Color;
in vec2 in_TexCoord0;


// Uniform

uniform mat4 projection;
uniform mat4 modelview;


// Sortie
out vec3 color;
out vec2 coordTexture;


// Fonction main

void main()
{
    // Position finale du vertex en 3D

    gl_Position = projection * modelview * vec4(in_Vertex, 1.0);


    // Envoi des coordonnées de texture au Fragment Shader

    coordTexture = in_TexCoord0;
    color = in_Color;

}
