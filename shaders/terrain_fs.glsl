#version 410 core

#define GAMMA 2.2

out vec4	fragColor;

in VS_OUT {
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
} fs_in;

struct	ColorData {
	bool		isTexture;
	vec3		color;
	sampler2D	texture;
};

struct	Material {
	ColorData	diffuse;
	ColorData	specular;
	float		shininess;
};

struct DirLight {
	vec3		direction;

	vec3		ambient;
	vec3		diffuse;
	vec3		specular;
};

uniform vec3 viewPos;
uniform DirLight dirLight;
uniform Material	material;

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
	if (material.diffuse.isTexture) {
		ambient *= vec3(texture(material.diffuse.texture, fs_in.TexCoords));
		diffuse *= diff * vec3(texture(material.diffuse.texture, fs_in.TexCoords));
	}
	else {
		ambient *= pow(material.diffuse.color, vec3(GAMMA));
		diffuse *= diff * pow(material.diffuse.color, vec3(GAMMA));
	}

	// use texture or color for the specular
	vec3 specular = light.specular * GAMMA;
	if (material.specular.isTexture) {
		specular *= spec * vec3(texture(material.specular.texture, fs_in.TexCoords));
	}
	else {
		specular *= spec * pow(material.specular.color, vec3(GAMMA));
	}

	return (ambient + diffuse + specular);
}

void main() {
	// retrieve alpha
	float alpha = 1.0f;
	if (material.diffuse.isTexture) {
		alpha = texture(material.diffuse.texture, fs_in.TexCoords).a;
	}
	// skip pixel if to transparent
	if (alpha < 0.01)
		discard;

	// retrieve normal
	vec3	norm = normalize(fs_in.Normal);

    vec3	viewDir = normalize(viewPos - fs_in.FragPos);

	// Directional lighting
	vec3	result = calcDirLight(dirLight, norm, viewDir);

	fragColor = vec4(result, alpha);

	// apply gamma correction
    fragColor.rgb = pow(fragColor.rgb, vec3(1.0 / GAMMA));
    // fragColor.rgb = norm;
}
