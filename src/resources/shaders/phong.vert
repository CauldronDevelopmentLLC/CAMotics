attribute vec3 position;
attribute vec3 normal;
attribute vec4 color;
attribute vec4 picking;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat3 normalMat; // mat3(transpose(inverse(model)))

varying vec3 fNormal;
varying vec3 fPosition;
varying vec4 fColor;
varying vec4 fPicking;


void main() {
  fPosition   = vec3(model * vec4(position, 1.0));
  fNormal     = normalMat * normal;
  fColor      = color;
  fPicking    = picking;
  gl_Position = projection * view * vec4(fPosition, 1.0);
}
