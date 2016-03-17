uniform sampler2D nmap;
uniform mat4 normalMatrix;

uniform vec4 color;
uniform bool useUniformColor = false;

in vec4 vertColor;
in vec2 texCoordVar;
in vec3 objectSpaceNormal;
in vec3 objectSpaceTangent;
in vec3 objectSpaceBitangent;
in vec3 cameraPosition;
in vec3 surfacePosition;

struct DirectionalLight {
	vec3 direction;
	vec3 color;
	float intensity;
};

const int nbLights = 4;

out vec4 FragColor;
vec4 mixV4(vec4 v1, vec4 v2, float i) { return v1 + (v2 - v1) * i; }
vec3 mixV3(vec3 v1, vec3 v2, float i) { return v1 + (v2 - v1) * i; }

vec3 superpose(vec3 v1, vec4 v2, float opacity) {
	return mixV3(v1, vec3(v2), v2.a * opacity);
}

vec3 eclaircirSeulement(vec3 v1, vec3 v2) {
	return vec3(max(v1.r, v2.r), max(v1.g, v2.g), max(v1.b, v2.b));
}

void main() {
	DirectionalLight lights[nbLights];

	lights[0].direction = vec3(0.661438,-0.75,0);
	lights[0].color = vec3(1.0);
	lights[0].intensity = 0.7;
	lights[1].direction = vec3(-0.841789,-0.297529,0.450408);
	lights[1].color = vec3(1.0);
	lights[1].intensity = 0.5;
	lights[2].direction = vec3(-0.2011,0.264352,-0.943227);
	lights[2].color = vec3(1.0);
	lights[2].intensity = 0.7;
	lights[3].direction = vec3(0.381145,0.782907,0.491717);
	lights[3].color = vec3(1.0);
	lights[3].intensity = 0.52;

	vec3 surfaceToCamera = normalize(cameraPosition - surfacePosition);  // world space
	vec3 tangentSpaceNormal = texture(nmap, texCoordVar).yxz * 2.0 - 1.0;
	tangentSpaceNormal = normalize(tangentSpaceNormal);

	vec3 n = normalize(objectSpaceNormal);              // Z
	vec3 t = normalize(cross(n, vec3(0.0, 0.0, 1.0)));  // X
	vec3 b = cross(n, t);                               // Y

	mat3 basis = mat3(t, b, n);
	vec3 eyespaceNormal1 = basis * tangentSpaceNormal;  // world -> object space
	eyespaceNormal1 = normalize(mat3(normalMatrix) * eyespaceNormal1);  // object -> eye
	vec3 eyespaceNormal0 = normalize(mat3(normalMatrix) * objectSpaceNormal);

	float diffuseCoef =
		min(1.0, max(0.0, abs(dot(eyespaceNormal0,
							surfaceToCamera))));  // bords noirds, centre blanc
	float diffuseCoefMap = min(
			1.0, max(0.0, abs(dot(eyespaceNormal1, surfaceToCamera))));  // bords noirds, centre
	// blanc, normales
	// perturbées
	// (éclairage caméra)

	vec4 verticeColor = useUniformColor ? color : vertColor;
	vec3 surfaceColor = verticeColor.rgb;

	/*[>vec3 reducedColor = mixV3(surfaceColor,surfaceColor*surfaceColor -
	 * vec3(0.15),max(0.0,min(0.8,diffuseCoef))) ; // dégradé couleur clair sur les bords ->
	 * sombre au centre. Effet "translucide"<]*/
	vec3 reducedColor =
		mix(surfaceColor + vec3(0.1), vec3((surfaceColor - 0.63) * 1.15 + 0.5),
				max(0.0, min(0.8, diffuseCoef * diffuseCoef)));  // dégradé couleur clair sur
	// les bords -> sombre au
	// centre. Effet "translucide"

	vec3 normalColor = mixV3(
			vec3(1.0), vec3(0.0),
			max(0.0,
				min(1.0,
					diffuseCoefMap *
					diffuseCoefMap)));  // éclairage caméra n&b avec normales perturbées

	vec4 whiteHalo = mixV4(
			vec4(1.0), vec4(1.0, 1.0, 1.0, 0.0),
			diffuseCoef * diffuseCoef);  // blanc opaque sur les bords, transparent au milieu
	whiteHalo *= whiteHalo;

	vec3 final = mixV3(reducedColor, (vec3(0.2) + reducedColor) * normalColor.r, 0.1);

	final = superpose(final, whiteHalo, 0.05);
	final = superpose(final, whiteHalo * vec4(normalColor, 1.0), 0.32);
	vec4 colorLinear;

	/*final = vec3(0.2);*/
	vec3 normal = eyespaceNormal1;
	vec4 specColor = vec4(vec3(0.5),1.0);
	for (int i = 0; i < nbLights; ++i) {
		vec3 lightDir = normalize(lights[i].direction);
		float lambertian = max(dot(lightDir, normal), 0.0);
		colorLinear +=
			lambertian * specColor * vec4(lights[i].color, 1.0) *
			lights[i].intensity;
	}

	/*FragColor =  vec4(final,0.0 + diffuseCoef*diffuseCoef) ;*/
	FragColor = vec4(mixV3(final , final+ final*colorLinear.rgb, 0.5), verticeColor.a);

	/*FragColor = vec4(1.0,0.2,0.3,1.0);*/
}
