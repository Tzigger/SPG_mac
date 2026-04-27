#version 400

layout(location = 0) in vec3 vPos;

uniform mat4 mvpMatrix;
uniform mat4 normalMatrix;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform int  mode; // 1 = iluminare per-vertex, 2 = iluminare per-pixel

out vec3 normal;      // normala interpolata - folosita in modul per-pixel
out vec3 pos;         // pozitia in spatiul obiectului
out vec3 vertexColor; // culoarea calculata per-vertex - folosita in modul per-vertex

// Functia de iluminare Phong - aceeasi formula ca in fragment shader
vec3 lighting(vec3 p, vec3 n, vec3 lp, vec3 vp,
              vec3 ambient, vec3 diffuse, vec3 specular, float specPower)
{
    vec3 N = normalize(n);
    vec3 L = normalize(lp - p);
    vec3 V = normalize(vp - p);
    vec3 R = reflect(-L, N);

    vec3  ambientComp  = ambient;
    float diff         = max(dot(L, N), 0.0);
    vec3  diffuseComp  = diffuse * diff;
    float spec         = pow(max(dot(R, V), 0.0), specPower);
    vec3  specularComp = specular * spec;

    return ambientComp + diffuseComp + specularComp;
}

void main()
{
    gl_Position = mvpMatrix * vec4(vPos, 1.0);

    // Pentru sfera unitara, normala in spatiul local = pozitia normalizata
    // (vectorul de la centrul sferei spre vertex are exact directia normalei)
    vec3 rawNormal = normalize(vPos);

    // Corectia normalelor: transpusa inversei matricii de modelare
    normal = normalize(vec3(normalMatrix * vec4(rawNormal, 0.0)));
    pos    = vPos;

    if (mode == 1) {
        vec3  ambient   = vec3(0.2);
        vec3  diffuse   = vec3(1.0, 0.0, 0.0);
        vec3  specular  = vec3(0.8);
        float specPower = 32.0;
        vertexColor = lighting(pos, normal, lightPos, viewPos,
                               ambient, diffuse, specular, specPower);
    } else {
        // In modul per-pixel nu folosim vertexColor
        vertexColor = vec3(0.0);
    }
}