// Version du GLSL




// Entrée

in vec3 color;


// Sortie 

out vec4 out_Color;


// Fonction main

void main()
{
    // Couleur finale du pixel

    out_Color = vec4(color, 1.0);
}
