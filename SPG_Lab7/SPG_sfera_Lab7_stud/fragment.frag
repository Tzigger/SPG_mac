#version 410

// Pozitia interpolata a fragmentului, primita de la vertex shader
// Pe sfera unitara, aceasta este normala la suprafata
in vec3 vPosition;

// Uniform pentru selectia exercitiului:
//   mode = 1 => sfera jumatate-jumatate (verde/albastru, split dupa X=0)
//   mode = 2 => culoare in functie de unghiul normalei cu axa OX
uniform int mode;

out vec4 fragColor;

void main()
{
    if (mode == 1) {
        // ------------------------------------------------
        // Exercitiul 3: Sfera colorata jumatate-jumatate
        //   - Normala sferei = vPosition (normalizata)
        //   - componenta X > 0  => albastru (jumatatea dreapta)
        //   - componenta X <= 0 => verde   (jumatatea stanga)
        // ------------------------------------------------
        if (vPosition.x > 0.0) {
            fragColor = vec4(0.0, 0.0, 1.0, 1.0);  // albastru
        } else {
            fragColor = vec4(0.0, 1.0, 0.0, 1.0);  // verde
        }

    } else {
        vec3 normal = normalize(vPosition);

        vec3 axaOX = vec3(1.0, 0.0, 0.0);

        float cosUnghi = dot(normal, axaOX);

        float t = (cosUnghi + 1.0) / 2.0;

        fragColor = vec4(t * 0.2, t * 0.2, t, 1.0);
    }
}
