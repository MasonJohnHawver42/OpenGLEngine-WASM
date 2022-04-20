#version 300 es
precision highp float;

out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;

vec3 lightPos = vec3(0.0, 0.0, 0.0);
vec3 lightColor = vec3(1.0, 1.0, 1.0);

uniform vec3 ambientLight;
uniform vec3 objectColor;

void main()
{

  float ambientStrength = 0.2;
  vec3 ambient = ambientStrength * ambientLight;

  // diffuse
  vec3 norm = normalize(Normal);
  float diff = max(dot(norm, vec3(0.0, 0.0, 1.0)), 0.0);
  vec3 diffuse = diff * lightColor;

  vec3 result = (ambient + diffuse) * objectColor;
  FragColor = vec4(result, 1.0);
}
