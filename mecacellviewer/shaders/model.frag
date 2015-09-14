uniform vec4 color;

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


void main(){
	/*vec4 modelDiffuseColor = color;*/
	vec4 modelDiffuseColor = vec4(0.9);
	vec4 modelAmbientColor = vec4(modelDiffuseColor.xyz * 0.07, modelDiffuseColor.a); 

	DirectionalLight lights[nbLights];
	lights[0].direction = vec3(-0.10, 1, 0.1);
	lights[0].color = vec3(1.0,1.0,1.0);
	lights[0].intensity = 0.65;
	lights[1].direction = vec3(-0.8,0.2,-0.1);
	lights[1].color = vec3(1.0,0.63,0.57);
	lights[1].intensity = 0.52;
	lights[2].direction = vec3(0.8,0.2,0.0);
	lights[2].color = vec3(0.56,0.65,1.0);
	lights[2].intensity = 0.52;

	vec3 normal = normalize(normalInterp);


	vec4 colorLinear;

	for(int i = 0 ; i< nbLights ; ++i){

		vec3 lightPos = vec3(5000.0,5000.0,-200.0);
		/*vec3 lightDir = normalize(lightPos - surfacePosition);*/
		vec3 lightDir = normalize(lights[i].direction);	
		/*vec3 lightDir = lights[i].direction;*/

		float lambertian = abs(dot(lightDir,normal));
		float specular = 0.0;

		if(lambertian > 0.0) {
			vec3 viewDir = normalize(-surfacePosition);
			vec3 halfDir = normalize(lightDir + viewDir);
			float specAngle = max(dot(halfDir, normal), 0.0);
			specular = pow(specAngle, shininess);
		}


		vec4 specColor = vec4(1.0);
		colorLinear += modelAmbientColor + lambertian * modelDiffuseColor * vec4(lights[i].color,1.0) * lights[i].intensity;// +	specular * specColor * lights[i].intensity;
	}
	fragColor = vec4(pow(colorLinear.rgb, vec3(1.0/screenGamma)),colorLinear.a);
	/*fragColor = vec4(normal,1.0);//colorLinear;*/
}
