#version 400
out vec4 FragColor;

in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform int mode;
uniform float alphaValue;
uniform int useTexturePair;

void main()
{
    if (mode == 1) {
        FragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), 0.5);
    }
    else if (mode == 2) {
        if (TexCoord.x < 0.5) {
            vec2 uv1 = vec2(TexCoord.x * 2.0, TexCoord.y);
            FragColor = texture(texture1, uv1);
        } else {
            vec2 uv2 = vec2((TexCoord.x - 0.5) * 2.0, TexCoord.y);
            FragColor = texture(texture2, uv2);
        }
    }
    else if (mode == 3) {
        FragColor = vec4(ourColor, alphaValue);
    }
    else {
        vec4 t = (useTexturePair == 0) ? texture(texture1, TexCoord) : texture(texture2, TexCoord);
        t.a = t.a * alphaValue;
        FragColor = t;
    }
}
