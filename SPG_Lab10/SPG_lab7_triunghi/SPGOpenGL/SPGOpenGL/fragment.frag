#version 410

in vec3 ourColor;

in vec2 TexCoord;

// Uniformul "mode" permite fragment shader-ului sa stie ce exercitiu
// este activ si sa calculeze culoarea corespunzatoare:
//   mode = 1 → textura pura             (Ex1, Ex3)
//   mode = 2 → textura * culoare vertex (Ex2)
//   mode = 4 → mix doua texturi         (Ex4)
uniform int mode;
uniform sampler2D ourTexture;
uniform sampler2D ourTexture2;

out vec4 FragColor;

void main()
{
    if (mode == 2) {
        vec4 texColor = texture(ourTexture, TexCoord);
        FragColor = texColor * vec4(ourColor, 1.0);

    } else if (mode == 4) {
        vec4 tex1 = texture(ourTexture,  TexCoord);
        vec4 tex2 = texture(ourTexture2, TexCoord);
        FragColor = mix(tex1, tex2, 0.5);

    } else {
        FragColor = texture(ourTexture, TexCoord);
    }
}
