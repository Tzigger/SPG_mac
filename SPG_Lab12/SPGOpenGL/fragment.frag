#version 400
out vec4 FragColor;

in vec3 fragPos;
in vec3 normalVS;
in vec2 texCoord;
in vec3 tangentLightPos;
in vec3 tangentViewPos;
in vec3 tangentFragPos;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform int sceneType;
uniform bool useNormalMap;
uniform sampler2D colorTex;
uniform sampler2D normalMapTex;
uniform sampler2D noiseTex;
uniform float squareMix;
uniform float noiseDelta;
uniform float noiseStrength;

vec2 noiseGradient(vec2 coords, float delta)
{
    // Gradient aproximat cu diferente finite (dx, dy) pe textura de noise
    float sample1x = texture(noiseTex, vec2(coords.s + delta, coords.t)).r;
    float sample2x = texture(noiseTex, vec2(coords.s - delta, coords.t)).r;
    float sample1y = texture(noiseTex, vec2(coords.s, coords.t + delta)).r;
    float sample2y = texture(noiseTex, vec2(coords.s, coords.t - delta)).r;
    return vec2(sample1x - sample2x, sample1y - sample2y);
}

void main()
{
    vec3 N = normalize(normalVS);
    vec3 baseColor = (sceneType == 0) ? texture(colorTex, texCoord).rgb : vec3(0.75, 0.78, 0.82);
    vec3 L = normalize(lightPos - fragPos);
    vec3 V = normalize(viewPos - fragPos);

    if (sceneType == 0) {
        // Ex.1: citim normala din normal map (RGB -> XYZ)
        vec3 normalFromMap = texture(normalMapTex, texCoord).rgb;
        normalFromMap.g = 1.0 - normalFromMap.g; // corectie pe Y (green channel flip)
        normalFromMap = normalize(normalFromMap * 2.0 - 1.0);

        // Interpolare intre normala clasica si normala din map 
        vec3 mappedN = useNormalMap ? normalize(mix(vec3(0.0, 0.0, 1.0), normalFromMap, squareMix)) : vec3(0.0, 0.0, 1.0);
        N = mappedN;
        // Pentru patrat, iluminarea se face in tangent space
        L = normalize(tangentLightPos - tangentFragPos);
        V = normalize(tangentViewPos - tangentFragPos);
    } else if (useNormalMap) {
        // Ex.2: perturbam normala sferei cu gradientul noise
        vec2 grad = noiseGradient(texCoord, noiseDelta);
        N.xy += grad * noiseStrength;
        N = normalize(N);
    }

    // Iluminare Phong/Blinn simplificata (ambient + diffuse + specular)
    vec3 R = reflect(-L, N);

    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * baseColor;

    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = diff * baseColor;

    float specularStrength = 0.35;
    float shininess = 32.0;
    float spec = pow(max(dot(V, R), 0.0), shininess);
    vec3 specular = specularStrength * spec * vec3(1.0);

    FragColor = vec4(ambient + diffuse + specular, 1.0);
}
