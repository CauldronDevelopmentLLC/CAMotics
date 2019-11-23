uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

attribute vec3 position;
attribute vec3 normal;
attribute vec4 color;

varying vec4 fragColor;


void main() {
  gl_Position = projection * view * model * vec4(position, 1.0);
  fragColor = color;
}
