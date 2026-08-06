#version 330 core
out vec4 FragColor;

in vec2 texCoords;

uniform sampler2D screenTexture;

void main() {
    float gamma   = 2.2;

    vec3 col  = texture(screenTexture, texCoords).rgb;
    col       = pow(col, vec3(1.0 / gamma));
    FragColor = vec4(col, 1.0);
}