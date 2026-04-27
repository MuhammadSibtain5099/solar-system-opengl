#version 330 core

in  vec3 FragPos;
in  vec3 Normal;
in  vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D uTexture;

uniform vec3  uLightPos;          // world-space light position (sun center)
uniform vec3  uViewPos;           // world-space camera position

uniform bool  uIsLightSource;     // if true → render as emissive (the sun)
uniform float uAmbientStrength;   // ka
uniform float uSpecularStrength;  // ks
uniform float uShininess;         // shininess exponent

void main() {
    vec4 texColor = texture(uTexture, TexCoord);

    // ── Sun: render fully bright, no lighting calculation ───────────────────
    if (uIsLightSource) {
        FragColor = texColor;
        return;
    }

    // ── Phong shading (per-fragment) ─────────────────────────────────────────
    vec3 N = normalize(Normal);
    vec3 L = normalize(uLightPos - FragPos);   // light direction
    vec3 V = normalize(uViewPos  - FragPos);   // view direction
    vec3 R = reflect(-L, N);                   // reflection direction

    // Ambient
    vec3 ambient = uAmbientStrength * texColor.rgb;

    // Diffuse  (Lambertian)
    float diff    = max(dot(N, L), 0.0);
    vec3  diffuse = diff * texColor.rgb;

    // Specular (Phong)
    float spec     = pow(max(dot(V, R), 0.0), uShininess);
    vec3  specular = uSpecularStrength * spec * vec3(1.0); // white highlight

    vec3 result = ambient + diffuse + specular;
    FragColor   = vec4(result, texColor.a);
}
