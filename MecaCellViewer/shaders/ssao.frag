#version 330 core
in vec2 UV;
uniform sampler2D tex;
uniform highp sampler2D depthBuf;
uniform highp float nearClip;
uniform highp float farClip;
uniform bool AOEnabled;
out vec4 FragColor;

highp float getLinearDepth(vec2 uv){
   return (2.0 * nearClip) / (farClip + nearClip - (texture(depthBuf,uv).r * (farClip - nearClip)));
}

highp float getAO(float z0, float r, float d0,vec2 samples, float nbSamples){
   highp float zr = z0-d0;
   highp float ao = 0.0;
   highp float dd = 2.0*d0;
   float sampleInc = 1.0/nbSamples;
   for (float i = sampleInc; i <= 1.0 ; i += sampleInc){
      highp float da = getLinearDepth(UV-samples*r*i) - zr;
      highp float db = getLinearDepth(UV+samples*r*i) - zr;
      if (da < dd && db < dd && db>0.0 && da > 0.0)
         ao += dd - da - db;
   }
   ao *=sampleInc;
   return min(max(ao/dd,0.0),1.0);
}

highp float computeAO(float z0, float r, float d, float sampleInc){
   highp float ao = 0.0;
   highp float r0 = r/z0;
   /*sampleInc = max(2.0,min(sampleInc*r0*200.0,2.0*sampleInc));*/
	 sampleInc = 4.0;
   ao += getAO(z0,r0,d,vec2(0.0,1.0),sampleInc);
   ao += getAO(z0,r0,d,vec2(1.0,0.0),sampleInc);
   ao += getAO(z0,r0,d,vec2(1.0,1.0),sampleInc);
   ao += getAO(z0,r0,d,vec2(-1.0,1.0),sampleInc);
   return 1.0-(ao*0.25);
}


void main(){
   vec4 color = texture(tex, UV);

   highp float z0;
   z0 = getLinearDepth(UV);
   if (z0<0.9){

   highp float aoLarge = computeAO(z0,0.0000025,0.00001,5.0);
   highp float aoMedium = computeAO(z0,0.0000018,0.000005,6.0);
   highp float aoSmall = computeAO(z0, 0.000001,0.0000006,5.0);

	FragColor.rgb = color.rgb;
	FragColor.rgb *= mix(FragColor.rgb*FragColor.rgb,vec3(1.0),aoSmall);
	FragColor.rgb *= mix(FragColor.rgb*FragColor.rgb,vec3(1.0),aoMedium);
	FragColor.rgb *= mix(FragColor.rgb*FragColor.rgb,vec3(1.0),aoLarge);
	FragColor.rgb = mix(FragColor.rgb*aoSmall*aoMedium*aoLarge,FragColor.rgb,0.7);
   /*FragColor.rgb = color.rgb*vec3(aoSmall)*vec3(aoMedium)*vec3(aoLarge);*/
   }
   else {
      FragColor.rgb = vec3(color);
   }
   FragColor.a=1.0;
   float contrast = 1.15;
   FragColor.rgb = (FragColor.rgb - 0.5) * max(contrast, 0.0) + 0.5;
	gl_FragDepth = texture(depthBuf,UV).r; 

}

/*highp uniform mat4 VPinv;*/
/*highp vec4 H = vec4(UV.x * 2.0 - 1.0, UV.y * 2.0 - 1.0, z, 1.0);*/
/*highp vec4 D = VPinv*H;*/
/*highp vec4 worldPos = D / D.w;*/
