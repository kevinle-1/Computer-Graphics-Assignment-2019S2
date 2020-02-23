/*
 * File: box.fs
 * Project: CCSEP 2019 S2 Assignment
 * Created Date: 23/10/19 - 16:09:14
 * Author: Kevin Le - 19472960
 * Contact: kevin.le2@student.curtin.edu.au
 * -----
 * Purpose: Box Fragment Shader 
 */

#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    sampler2D specular;    
    float shininess;
}; 

struct Light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

in vec3 FragPos;  
in vec3 Normal;  
in vec2 TexCoords;
  
uniform vec3 viewPos;
uniform Material material;
uniform Light light;

void main()
{
    if(texture(material.diffuse, TexCoords).a != 1.0f)
    {
        discard;
    }

	vec3 lightDir = normalize(light.position - FragPos); //LightDir = surfaceToLight

    //Ambient 
    vec3 ambient = light.ambient * texture(material.diffuse, TexCoords).rgb;
  	
    //Diffuse
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoords).rgb;  
    
    //Specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * texture(material.specular, TexCoords).rgb;  

	//Attenuation 
	float lightDist = length(light.position - FragPos);
	float attenuation = 1.0 / (light.constant + light.linear * lightDist + light.quadratic * (lightDist * lightDist));   
        
	//ambient *= attenuation;  
    diffuse *= attenuation;
    specular *= attenuation;   

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
} 
