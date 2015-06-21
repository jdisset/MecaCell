

in vec2 UV;
in vec2 s;
in vec3 fragColor;
out vec4 finalColor;

void main(){
    /*vec2 centered = UV;*/
    /*finalColor = vec4(1,1,1,0.01);*/
    /*vec2 bord = 500/s;*/
    /*if (centered.x<bord.x || centered.x > 1-bord.x || centered.y < bord.y || centered.y > 1-bord.y){*/
       /*finalColor = vec4(color,1);*/
    /*}*/
    finalColor = vec4(fragColor,1);
}
