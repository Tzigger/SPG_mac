#version 410 core

out vec4 fragColor;

in vec3 fragPos; // pozitia pixelului in world space
in vec3 normal;  // normala interpolata in world space

uniform vec3 lightPos1; // sursa 1: fixa in scena
uniform vec3 lightPos2; // sursa 2: se misca cu observatorul
uniform vec3 viewPos;   // pozitia camerei (pentru componenta speculara)

vec3 phong(vec3 objColor, vec3 lightPos, vec3 lightColor, vec3 ambientCoef, vec3 specCoef, float specPow)
{
    vec3 N = normalize(normal);
    vec3 L = normalize(lightPos - fragPos);  // directia spre lumina
    vec3 V = normalize(viewPos  - fragPos);  // directia spre observator
    vec3 R = reflect(-L, N);                 // directia de reflexie

    float diff = max(dot(N, L), 0.0);                  // componenta difuza
    float spec = pow(max(dot(R, V), 0.0), specPow);    // componenta speculara

    vec3 ambient  = ambientCoef * lightColor;
    vec3 diffuse  = diff * lightColor;
    vec3 specular = spec * specCoef * lightColor;

    return clamp((ambient + diffuse + specular) * objColor, 0.0, 1.0);
}

void main()
{
    vec3 objColor  = vec3(0.6, 0.3, 0);
    vec3 specCoef  = vec3(0.6);
    float specPow  = 48.0;

    // Contributia sursei 1 (lumina alba calda, cu ambient)
    vec3 c1 = phong(objColor, lightPos1, vec3(1.0, 0.95, 0.85), vec3(0.08), specCoef, specPow);

    // Contributia sursei 2 (lumina albastruie, fara ambient - torta)
    vec3 c2 = phong(objColor, lightPos2, vec3(0.6, 0.75, 1.0),  vec3(0.0),  specCoef * 0.4, specPow);

    fragColor = vec4(clamp(c1 + c2, 0.0, 1.0), 1.0);
}
