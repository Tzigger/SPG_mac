#version 400
layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform int sceneType;
uniform vec3 lightPos;
uniform vec3 viewPos;

out vec3 fragPos;
out vec3 normalVS;
out vec2 texCoord;
out vec3 tangentLightPos;
out vec3 tangentViewPos;
out vec3 tangentFragPos;

void main()
{
    vec4 worldPos = model * vec4(inPos, 1.0);
    fragPos = worldPos.xyz;

    if (sceneType == 0) {
        // Ex.1: construim baza TBN pentru a lucra corect cu normala din normal map (tangent space)
        vec3 N = normalize(mat3(transpose(inverse(model))) * inNormal);
        vec3 T = normalize(mat3(model) * vec3(1.0, 0.0, 0.0));
        T = normalize(T - dot(T, N) * N); // Gram-Schmidt: T perpendicular pe N
        vec3 B = cross(N, T);
        mat3 TBNinv = transpose(mat3(T, B, N)); // inverse(TBN) pentru world -> tangent

        normalVS = N;
        texCoord = inTexCoord;
        // Mutam vectorii de iluminare in tangent space (mai eficient decat in fragment)
        tangentLightPos = TBNinv * lightPos;
        tangentViewPos = TBNinv * viewPos;
        tangentFragPos = TBNinv * fragPos;
    } else {
        // Ex.2: normala clasica de sfera + UV generate din directia normalei
        vec3 sphereN = normalize(inPos);
        normalVS = normalize(mat3(transpose(inverse(model))) * sphereN);
        float s = atan(sphereN.x, sphereN.z) / 3.141592 + 0.5;
        float t = sphereN.y * 0.5 + 0.5;
        texCoord = vec2(s, t);
        tangentLightPos = vec3(0.0);
        tangentViewPos = vec3(0.0);
        tangentFragPos = vec3(0.0);
    }

    gl_Position = projection * view * worldPos;
}
