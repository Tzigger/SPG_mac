#version 410
// Culoarea per-vertex interpolata – Ex. 0, 1
in vec4 vColor;

// Pozitia interpolata in spatiu obiect – Ex. 2
in vec3 vPosition;

// Uniform pentru selectia exercitiului:
//   mode = 1 sau 2  => culoare per-vertex interpolata
//   mode = 3        => jumatate rosu / jumatate albastru dupa Y
uniform int mode;

out vec4 frag_colour;

void main() {
    if (mode == 3) {
        // ------------------------------------------------
        // Exercitiul 2: triunghi jumatate-jumatate
        //   - vPosition.y este interpolat de la vertex shader
        //   - y > 0 (jumatatea de sus)  => rosu
        //   - y <= 0 (jumatatea de jos) => albastru
        // ------------------------------------------------
        if (vPosition.y > 0.0) {
            frag_colour = vec4(1.0, 0.0, 0.0, 1.0);
        } else {
            frag_colour = vec4(0.0, 0.0, 1.0, 1.0);
        }
    } else {
        // ------------------------------------------------
        // Exercitiul 0: culoare per-vertex (R,G,B -> RGBA)
        //   GPU interpoleaza automat intre culorile vertexurilor
        // Exercitiul 1: culoare per-vertex (R,G,B,A)
        //   Aceeasi logica, culoarea contine si componenta Alpha
        // ------------------------------------------------
        frag_colour = vColor;
    }
}
