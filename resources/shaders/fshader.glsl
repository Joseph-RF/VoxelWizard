#version 330 core

uniform vec3 viewer_pos;
uniform float shininess;
uniform bool show_normals;

in vec3 normal;
in vec3 fragment_pos;
in vec3 colour;

out vec4 FragColor; // Fragment shader only requires a single output variable

struct DirLight {
    vec3 direction;

    vec3 colour;

    float ambient;
    float diffuse;
    float specular;
};
uniform DirLight dir_light;

void main() {

    if(show_normals) {
        FragColor = vec4(abs(normal), 1.0);
        return;
    }

    // Normalize the vectors that we'll be working with for light calculations
    vec3 n_normal = normalize(normal);
    vec3 n_viwer_direction = normalize(viewer_pos - fragment_pos); // From fragment to viewer
    vec3 n_light_direction = normalize(dir_light.direction);

    // Calculate ambient light
    // --------------------------------
    vec3 ambient = dir_light.ambient * dir_light.colour * colour;

    // Calculate diffuse light
    // --------------------------------
    float diffuse_factor = max(dot(n_light_direction, n_normal), 0.0);
    vec3 diffuse = diffuse_factor * dir_light.diffuse * dir_light.colour * colour;

    // Calculate specular light
    // --------------------------------
    // Using Blinn-Phong, using halfway vector between viewing direction and light direction
    vec3 n_halfway_direction = normalize(n_viwer_direction + n_light_direction);

    float specular_factor = pow(max(dot(n_normal, n_halfway_direction), 0.0), shininess);
    vec3 specular         = specular_factor * dir_light.specular * dir_light.colour * colour;

    FragColor = vec4(ambient + diffuse + specular, 1.0);
};