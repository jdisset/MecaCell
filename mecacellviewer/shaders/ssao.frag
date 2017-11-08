in vec2 UV;
uniform sampler2D tex;
uniform  sampler2D depthBuf;
uniform  float nearClip;
uniform  float farClip;
uniform bool AOEnabled;
out vec4 FragColor;

float getLinearDepth(vec2 uv){
	return (2.0 * nearClip) / (farClip + nearClip - (texture(depthBuf,uv).r * (farClip - nearClip)));
}

float rand(vec2 co){
	float a = 12.9898;
	float b = 78.233;
	float c = 43758.5453;
	float dt= dot(co.xy ,vec2(a,b));
	float sn= mod(dt,3.14);
	return fract(sin(sn) * c);
}

float getAO(const float z0, const float r, const float d0, const vec2 samples, const float nbSamples){
	float zr = z0-d0;
	float ao = 0.0;
	float dd = 2.0*d0;
	float sampleInc = 1.0/nbSamples;
	for (float i = sampleInc; i <= 1.0 ; i += sampleInc){
		float da = getLinearDepth(UV-samples*r*i) - zr;
		float db = getLinearDepth(UV+samples*r*i) - zr;
		if ( db>0.0 && da > 0.0)
			ao += max(0.0,dd - da - db);
	}
	ao *=sampleInc;
	return min(max(ao/dd,0.0),1.0);
}
float getThing(const float z0, const float r, const float d0, const vec2 samples, const float nbSamples){
	float zr = z0-d0;
	float ao = 0.0;
	float sampleInc = 1.0/nbSamples;
	for (float i = sampleInc; i <= 1.0 ; i += sampleInc){
		float da = getLinearDepth(UV-samples*r*i) - zr;
		float db = getLinearDepth(UV+samples*r*i) - zr;
		ao += abs(da - db);
	}
	ao *=sampleInc;
	return min(max(ao,0.0),1.0);
}
float computeThing(const float z0, const float r, const float d, const float nsamples){
	float ao = 0.0;
	float r0 = r/z0;
	ao += getThing(z0,r0,d,vec2(0.0,1.0),nsamples);
	ao += getThing(z0,r0,d,vec2(1.0,0.0),nsamples);
	ao += getThing(z0,r0,d,vec2(1.0,1.0),nsamples);
	ao += getThing(z0,r0,d,vec2(-1.0,1.0),nsamples);
	return 1.0-(ao*0.25);
}

float computeAO(const float z0, const float r, const float d, const float nsamples){
	float ao = 0.0;
	float r0 = r/z0;
	ao += getAO(z0,r0,d,vec2(0.0,1.0),nsamples);
	ao += getAO(z0,r0,d,vec2(0,0.0),nsamples);
	ao += getAO(z0,r0,d,vec2(0.70,0.70),nsamples);
	ao += getAO(z0,r0,d,vec2(-0.70,0.70),nsamples);
	return 1.0-(ao*0.25);
}


void main(){
	vec4 color = texture(tex, UV);

	float z0;
	z0 = getLinearDepth(UV);
	if (z0<0.9){

		float aoLarge = computeAO(z0,0.0008,0.017,12.0);
		float aoMedium = computeAO(z0,0.005,0.005,8.0);
		float aoSmall = computeAO(z0, 0.002,0.001,6.0);
		vec3 ao = vec3(pow(aoLarge*aoMedium*aoSmall,1.4f));
		FragColor.rgb = color.rgb*ao;
		FragColor.rgb += (1.0-computeThing(z0,0.001,0.005,7.0))*0.5;
		FragColor.a = 1.0;
		/*[>FragColor.rgb = vec3(computeThing(z0,0.000003,0.00001,5.0));<]*/
	}
	else {
		FragColor.rgb = vec3(color);
	}
	FragColor.a=color.a;
	float contrast = 1.14;
	FragColor.rgb = (FragColor.rgb - 0.5) * max(contrast, 0.0) + 0.5;
	gl_FragDepth = texture(depthBuf,UV).r;

}
