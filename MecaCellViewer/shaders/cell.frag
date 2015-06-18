#version 330 core
uniform sampler2D nmap;
uniform vec3 color;
uniform mat4 normalMatrix;

in vec2 texCoordVar;
in  vec3 objectSpaceNormal;
in vec3 objectSpaceTangent;
in vec3 objectSpaceBitangent;
in vec3 cameraPosition;
in  vec3 surfacePosition;

out vec4 FragColor;
vec4 mixV4(vec4 v1, vec4 v2, float i){
	return v1+(v2-v1)*i;
}
vec3 mixV3(vec3 v1, vec3 v2, float i){
	return v1+(v2-v1)*i;
}


vec3 superpose(vec3 v1, vec4 v2, float opacity){
	return mixV3(v1,vec3(v2),v2.a*opacity);
}

vec3 eclaircirSeulement(vec3 v1, vec3 v2){
	return vec3(max(v1.r,v2.r),max(v1.g,v2.g),max(v1.b,v2.b));
}

void main(){
	vec3 surfaceToCamera = normalize(cameraPosition - surfacePosition); //world space
	vec3 tangentSpaceNormal = texture(nmap, texCoordVar).yxz * 2.0 - 1.0;
	tangentSpaceNormal = normalize(tangentSpaceNormal);

	vec3 n = normalize(objectSpaceNormal); //Z
	vec3 t = normalize(cross(n,vec3(0.0,0.0,1.0))); //X
	vec3 b = cross(n,t); //Y

	mat3 basis = mat3(t,b,n );
	vec3 eyespaceNormal1 = basis * tangentSpaceNormal; // world -> object space
	eyespaceNormal1 = normalize( mat3(normalMatrix)*eyespaceNormal1); // object -> eye
	vec3 eyespaceNormal0 = normalize(mat3(normalMatrix) * objectSpaceNormal);

	float diffuseCoef = min(1.0,max(0.0, abs(dot(eyespaceNormal0, surfaceToCamera)))); //bords noirds, centre blanc
	float diffuseCoefMap =  min(1.0,max(0.0, abs(dot(eyespaceNormal1, surfaceToCamera)))); //bords noirds, centre blanc, normales perturbées (éclairage caméra)

	vec3 surfaceColor =  color;

	/*vec3 reducedColor = mixV3(surfaceColor,surfaceColor*surfaceColor - vec3(0.15),max(0.0,min(0.8,diffuseCoef))) ; // dégradé couleur clair sur les bords -> sombre au centre. Effet "translucide"*/
	vec3 reducedColor = mix(surfaceColor,surfaceColor*surfaceColor - vec3(0.1),max(0.0,min(0.8,diffuseCoef))) ; // dégradé couleur clair sur les bords -> sombre au centre. Effet "translucide"

	vec3 normalColor = mixV3(vec3(1.0),vec3(0.0),max(0.0,min(1.0, diffuseCoefMap*diffuseCoefMap))) ; // éclairage caméra n&b avec normales perturbées

	vec4 whiteHalo = mixV4(vec4(1.0),vec4(1.0,1.0,1.0,0.0),diffuseCoef*diffuseCoef); // blanc opaque sur les bords, transparent au milieu
	whiteHalo *= whiteHalo;

	vec3 final = mixV3(reducedColor,(vec3(0.2)+reducedColor)*normalColor.r,0.1);

	final = superpose(final,whiteHalo,0.06);
	final = superpose(final,whiteHalo*vec4(normalColor,1.0),0.3);

	FragColor =  vec4(final,0.82 + diffuseCoef*diffuseCoef) ;
	/*FragColor =  vec4(final,1.0) ;*/
}
