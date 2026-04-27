#version 400

out vec4 fragColor;

in vec3 normal;      // normala interpolata (din vertex shader)
in vec3 pos;         // pozitia interpolata
in vec3 vertexColor; // culoarea pre-calculata per-vertex (pentru mode==1)

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform int  mode; // 1 = per-vertex, 2 = per-pixel

vec3 lighting(vec3 p, vec3 n, vec3 lp, vec3 vp,
              vec3 ambient, vec3 diffuse, vec3 specular, float specPower)
{
    vec3 N = normalize(n);
    vec3 L = normalize(lp - p);       // directia spre sursa de lumina
    vec3 V = normalize(vp - p);       // directia spre observator
    vec3 R = reflect(-L, N);          // directia razei reflectate

    vec3  ambientComp  = ambient;

    float diff         = max(dot(L, N), 0.0);
    vec3  diffuseComp  = diffuse * diff;

    float spec         = pow(max(dot(R, V), 0.0), specPower);
    vec3  specularComp = specular * spec;

    return ambientComp + diffuseComp + specularComp;
}

void main()
{
    vec3 color;

    if (mode == 1) {
        color = vertexColor;
    } else {
        vec3  ambient   = vec3(0.2);
        vec3  diffuse   = vec3(1.0, 0.0, 0.0);
        vec3  specular  = vec3(0.8);
        float specPower = 32.0;
        color = lighting(pos, normal, lightPos, viewPos,
                         ambient, diffuse, specular, specPower);
    }

    fragColor = vec4(color, 1.0);
}
