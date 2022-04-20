#include "include/glm/glm.hpp"
#include <vector>

struct Ray {};

struct Triangle {
  struct {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
  } vertexes[3];
};

struct Sphere {
  glm::vec3 pos;
  float radius;
};

struct Material {};

struct Intersection {};

struct Scene {
  
};
