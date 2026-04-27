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
        // ------------------------------------------------
        // Exercitiul 4: Culoare in functie de unghiul
        //   dintre normala la suprafata si axa OX
        //
        //   Normala sferei = vPosition (deja normalizata)
        //   Normalizam explicit normala sferei (buna practica, chiar daca e deja unitara)
        vec3 normal = normalize(vPosition);

        // Axa OX are directia (1, 0, 0)
        vec3 axaOX = vec3(1.0, 0.0, 0.0);

        // Produsul scalar (dot) intre normala si axa OX = cos(unghi)
        // rezultat in [-1, 1]: 1 = paralel cu OX, 0 = perpendicular, -1 = opus
        float cosUnghi = dot(normal, axaOX);

        // Remapam [-1, 1] -> [0, 1] pentru a folosi valoarea ca intensitate de culoare
        float t = (cosUnghi + 1.0) / 2.0;

        // Cream un gradient albastru: t=0 -> inchis (unghi=180°), t=1 -> deschis (unghi=0°)
        fragColor = vec4(t * 0.2, t * 0.2, t, 1.0);
        // ------------------------------------------------
    }
}
