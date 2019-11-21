varying vec3 normalInterp;  // Surface normal
varying vec3 vertPos;       // Vertex position

struct Light {
  float ambient;
  float diffuse;
  float specular;
};


struct Material {
  Light reflection;
  Light color;
  float shininess;
};

// Material
uniform Material material;

// Light position
uniform vec3 lightPos;


void main() {
  vec3 N = normalize(normalInterp);
  vec3 L = normalize(lightPos - vertPos);

  // Lambert's cosine law
  float lambertian = max(dot(N, L), 0.0);
  float specular = 0.0;

  if (0.0 < lambertian) {
    vec3 R = reflect(-L, N);      // Reflected light vector
    vec3 V = normalize(-vertPos); // Vector to viewer

    // Compute the specular term
    float angle = max(dot(R, V), 0.0);
    specular = pow(angle, material.shininess);
  }

  gl_FragColor =
    vec4(material.reflection.ambient * material.color.ambient +
         material.reflection.diffuse * material.color.diffuse * lambertian +
         material.reflection.specular * material.color.specular * specular,
         1.0);
}
