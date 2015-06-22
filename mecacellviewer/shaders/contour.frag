in sampler2D tex;
uniform sampler2D depthBuf;
uniform float nearClip;
uniform float farClip;

bool cond = false;

float getLinearDepth(vec2 uv){
   float z = texture2D(depthBuf, uv); //en fait c'est z/w (division perspective)
   z = (2.0 * nearClip) / (farClip + nearClip - z * (farClip - nearClip)); // là on a le vrai z
   return z;
}

float getAO(float z0, float r, float d0, vec2 uv, vec2 samples){
      float zr = z0-d0;
      float ao = 0.0;
   for (int i = 0 ; i < 5 ; i ++){
      float coef = (float)i/5.0;
      float za = getLinearDepth(uv-samples*r*coef);
      float zb = getLinearDepth(uv+samples*r*coef);
      float da = max(min(za - zr,2.0*d0),0.0);
      float db = max(min(zb - zr,2.0*d0),0.0);
      ao += 2.0*d0 - da - db; 
      // si très "occludé", a0 = 2*d0;
      // si "pointe" (très peu occludé), a0 = -2*d0 ;
      // si plat, a0 = 0;
   }
   ao = ao/5.0;
   return min(max(ao/(2.0*d0),0.0),1.0);
}

void main(){

   vec4 color = texture2D(tex, UV);

   float z0 = getLinearDepth(UV);
   float r = 0.01;
   float d0 = 10.0;

   float ao = 0.0;

   ao += getAO(z0,r,d0,UV,vec2(0.0,1.0));
   ao += getAO(z0,r,d0,UV,vec2(1.0,0.0));
   ao += getAO(z0,r,d0,UV,vec2(1.0,1.0));
   ao += getAO(z0,r,d0,UV,vec2(-1.0,1.0));
   ao = ao/4.0;

   gl_FragColor.rgb = z0;// 1.0-(ao*10.0);
   gl_FragColor.a=1.0;

   if (cond) gl_FragColor.rgb = vec3(1.0,0.0,0.0);
   /*gl_FragColor.rg=UV;*/
}

/*highp uniform mat4 VPinv;*/
/*highp vec4 H = vec4(UV.x * 2.0 - 1.0, UV.y * 2.0 - 1.0, z, 1.0);*/
/*highp vec4 D = VPinv*H;*/
/*highp vec4 worldPos = D / D.w;*/
