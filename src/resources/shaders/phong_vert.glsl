attribute vec3 position;
attribute vec3 normal;

uniform mat4 projection, modelView, normalMat;

varying vec3 normalInterp;
varying vec3 vertPos;


void main() {
  vec4 vertPos4 = modelView * vec4(position, 1.0);
  vertPos = vec3(vertPos4) / vertPos4.w;
  normalInterp = vec3(normalMat * vec4(normal, 0.0));
  gl_Position = projection * vertPos4;
}
