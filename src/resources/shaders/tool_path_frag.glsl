//precision mediump float;
attribute vec3 fragColor;

void main() {
  gl_FragColor = vec4(fragColor.r, fragColor.g, fragColor.b, 1.0);
}
