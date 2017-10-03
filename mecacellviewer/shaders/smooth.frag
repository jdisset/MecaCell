uniform vec4 color;
uniform vec3 eyeVec;
uniform vec3 upVec;

in vec2 UV;
in vec3 surfacePosition;
in vec3 normalInterp;

out vec4 fragColor;

struct DirectionalLight {
	vec3 direction;
	vec3 color;
	float intensity;
};

const int nbLights = 3;
const float shininess = 20.0;
const float screenGamma = 2.2;


void main(){
	vec4 modelDiffuseColor = color;
	vec4 modelAmbientColor = vec4(modelDiffuseColor.xyz * 0.07, modelDiffuseColor.a);

	vec3 rightVec = normalize(cross(upVec,eyeVec));
	DirectionalLight lights[nbLights];
	lights[0].direction = upVec;
	lights[0].color = vec3(1.0);
	lights[0].intensity = 0.6;
	lights[1].direction = eyeVec;
	lights[1].color = vec3(1.0);
	lights[1].intensity = 1.0;
	lights[2].direction = rightVec;
	lights[2].color = vec3(1.0);
	lights[2].intensity = 0.6;

	vec3 normal=normalInterp;


	vec4 colorLinear;

	float shininess = 0.5;
	for(int i = 0 ; i< nbLights ; ++i){
		vec3 lightDir = normalize(lights[i].direction);

		//calculate Ambient Term:
		vec4 Iamb = vec4(lights[i].color*0.1,1.0);

		//calculate Diffuse Term:
		vec4 Idiff = vec4(lights[i].color * (abs(dot(lightDir,normal))),1.0);
		Idiff = clamp(Idiff, 0.0, 1.0);

		// write Total Color:
		colorLinear += Iamb*modelDiffuseColor  + Idiff * modelDiffuseColor * lights[i].intensity;
	}
	fragColor = vec4(pow(colorLinear.rgb, vec3(1.0/screenGamma)),colorLinear.a);
}


