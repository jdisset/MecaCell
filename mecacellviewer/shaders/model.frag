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
const float shininess = 8.0;
const float screenGamma = 2.2;
const vec3 modelDiffuseColor = vec3(0.92,0.92,0.92);
const vec3 modelAmbientColor = modelDiffuseColor * 0.07; 


void main(){
	DirectionalLight lights[nbLights];
	lights[0].direction = vec3(10,10,5);
	lights[0].color = vec3(1.0,1.0,1.0);
	lights[0].intensity = 0.5;
	lights[1].direction = vec3(-0.8,0.2,-0.1);
	lights[1].color = vec3(1.0,0.63,0.57);
	lights[1].intensity = 0.42;
	lights[2].direction = vec3(0.8,0.2,0.0);
	lights[2].color = vec3(0.56,0.65,1.0);
	lights[2].intensity = 0.42;

	vec3 normal = normalize(normalInterp);


	vec3 colorLinear;

	for(int i = 0 ; i< nbLights ; ++i){

		vec3 lightPos = vec3(5000.0,5000.0,-200.0);
		/*vec3 lightDir = normalize(lightPos - surfacePosition);*/
		vec3 lightDir = normalize(lights[i].direction);	
		/*vec3 lightDir = lights[i].direction;*/

		float lambertian = max(dot(lightDir,normal), 0.0);
		float specular = 0.0;

		if(lambertian > 0.0) {
			vec3 viewDir = normalize(-surfacePosition);
			vec3 halfDir = normalize(lightDir + viewDir);
			float specAngle = max(dot(halfDir, normal), 0.0);
			specular = pow(specAngle, shininess);
		}


		vec3 specColor = vec3(1.0);
		colorLinear += modelAmbientColor + lambertian * modelDiffuseColor * lights[i].color * lights[i].intensity +	specular * specColor * lights[i].intensity;
	}
	vec3 colorGammaCorrected = pow(colorLinear, vec3(1.0/screenGamma));
	fragColor = vec4(colorGammaCorrected, 1.0);
}
