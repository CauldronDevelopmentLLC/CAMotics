#ifdef GL_ES
precision mediump float;
#endif

varying vec3 fNormal;
varying vec3 fPosition;
varying vec4 fColor;

struct Light {
  bool enabled;
  vec3 direction;
  vec4 ambient;
  vec4 diffuse;
};

uniform Light light;


vec4 phong_color() {
  vec4 ambient  = fColor * light.ambient;
  vec4 diffuse  = fColor * light.diffuse;

  // Diffuse
  vec3 norm     = normalize(fNormal);
  vec3 lightDir = normalize(-light.direction);
  diffuse *= max(dot(norm, lightDir), 0.0);

  return ambient + diffuse;
}


void main() {
  if (light.enabled) gl_FragColor = phong_color();
  else gl_FragColor = fColor;
}
