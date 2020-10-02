#version 330 core

#define GAMMA 2.2

out vec4	fragColor;

in VS_OUT {
	vec3 FragPos;
	vec3 Normal;
	float Visible;
} fs_in;

struct	Material {
	vec3	specular;
	float	shininess;
};

struct DirLight {
	vec3		direction;

	vec3		ambient;
	vec3		diffuse;
	vec3		specular;
};

uniform vec4 wColor;
uniform vec3 viewPos;
uniform DirLight dirLight;
uniform Material	material;
uniform samplerCube skybox;
uniform bool skyReflection;

vec3 calcDirLight(DirLight light, vec3 norm, vec3 viewDir) {
	vec3	lightDir = normalize(-dirLight.direction);
	// diffuse
	float	diff = max(dot(lightDir, norm), 0.0);  // result between 0 and 1
	// specular
	vec3	halfwayDir = normalize(lightDir + viewDir);
	float	spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);

	// use texture or color for the diffuse
	vec3	ambient = light.ambient * GAMMA;
	vec3	diffuse = light.diffuse * GAMMA;
	ambient *= pow(wColor.rgb, vec3(GAMMA));
	diffuse *= diff * pow(wColor.rgb, vec3(GAMMA));

	// use texture or color for the specular
	vec3 specular = light.specular * GAMMA;
	specular *= spec * pow(material.specular, vec3(GAMMA));

	return (ambient + diffuse + specular);
}

void main() {
	// retrieve alpha
	float alpha = wColor.a * fs_in.Visible;
	if (alpha < 0.01)
		discard;

	// retrieve normal
	vec3	norm = normalize(fs_in.Normal);

    vec3	viewDir = normalize(viewPos - fs_in.FragPos);

	// Directional lighting
	vec3	result = calcDirLight(dirLight, norm, viewDir);

	if (skyReflection) {
		// reflection
		vec3 I = normalize(fs_in.FragPos - viewPos);
		vec3 R = reflect(I, norm);
		vec3 reflectColor = texture(skybox, R).rgb;

		// fresnel effect (less reflection from above)
		float refFactor = dot(viewDir, norm);
		refFactor = pow(refFactor, 0.6);

		fragColor = vec4(mix(reflectColor, result, refFactor), alpha);
	}
	else {
		fragColor = vec4(result, alpha);
	}

	// apply gamma correction
    fragColor.rgb = pow(fragColor.rgb, vec3(1.0 / GAMMA));
}
