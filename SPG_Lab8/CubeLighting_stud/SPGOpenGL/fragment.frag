#version 400

out vec4 fragColor;

in vec3 normal;
in vec3 pos;

uniform vec3 lightPos;
uniform vec3 viewPos;

// Modelul de iluminare Phong: ambient + difuz + specular
vec3 lighting(vec3 pos, vec3 normal, vec3 lightPos, vec3 viewPos,
              vec3 ambient, vec3 diffuse, vec3 specular, float specPower)
{
    vec3 N = normalize(normal);
    vec3 L = normalize(lightPos - pos);       // directia spre sursa de lumina
    vec3 V = normalize(viewPos  - pos);       // directia spre observator
    vec3 R = reflect(-L, N);                  // directia razei reflectate

    
    vec3 ambientComp = ambient;

    float diff       = max(dot(L, N), 0.0);
    vec3 diffuseComp = diffuse * diff;

    float spec        = pow(max(dot(R, V), 0.0), specPower);
    vec3 specularComp = specular * spec;

    return ambientComp + diffuseComp + specularComp;
}

void main()
{
    vec3  ambient   = vec3(0.2);
    vec3  diffuse   = vec3(1.0, 0.0, 0.0);
    vec3  specular  = vec3(0.8);
    float specPower = 32.0;

    vec3 color = lighting(pos, normal, lightPos, viewPos,
                          ambient, diffuse, specular, specPower);

    fragColor = vec4(color, 1.0);
}
