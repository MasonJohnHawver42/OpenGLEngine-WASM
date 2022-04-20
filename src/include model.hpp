#ifndef MODEL_H
#define MODEL_H

#include <GL/glew.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "stb_image.h"

#include "tiny_obj_loader.h"

#include "mesh.hpp"
#include "shader.hpp"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

static void PrintInfo(const tinyobj::attrib_t& attrib,
                      const std::vector<tinyobj::shape_t>& shapes,
                      const std::vector<tinyobj::material_t>& materials);

class Model {
public:
  std::vector<Mesh*> meshes;
  std::string directory;

  Model(std::string const &path, std::string const &folder)
  {
      loadModel(path, folder);
  }

  Model() {}

  void Draw(Shader &shader)
  {
      for(unsigned int i = 0; i < meshes.size(); i++) {
          meshes[i]->Draw(shader);
      }
  }

private:

  void loadModel(std::string const &path, std::string const &folder)
  {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string warn;
    std::string err;

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, const_cast<char*>(path.c_str()), const_cast<char*>(folder.c_str()), true);

    //if (!warn.empty()) { std::cout << "WARN: " << warn << std::endl; }
    if (!err.empty()) { std::cerr << "ERR: " << err << std::endl; }
    if (!ret) { std::cout << "Failed to load obj: " << path << std::endl; }

    //PrintInfo(attrib, shapes, materials);

//std::cout << "HERE" << '\n';

    for (size_t i = 0; i < shapes.size(); i++) {
      size_t index_offset = 0;

      assert(shapes[i].mesh.num_face_vertices.size() == shapes[i].mesh.material_ids.size());
      assert(shapes[i].mesh.num_face_vertices.size() == shapes[i].mesh.smoothing_group_ids.size());

      std::vector<Vertex> * vertices = new std::vector<Vertex>(0);
      std::vector<unsigned int> * indices = new std::vector<unsigned int>(0);

      std::vector<tinyobj::index_t> vert_indicies;

    //  std::cout << "HERE2" << '\n';

      // For each face
      for (size_t f = 0; f < shapes[i].mesh.num_face_vertices.size(); f++) {
        size_t fnum = shapes[i].mesh.num_face_vertices[f];

      //  std::cout << "HERE3" << '\n';

        if (fnum == 3) {
          // For each vertex in the face
          for (size_t v = 0; v < fnum; v++) {
            //std::cout << "HERE4" << '\n';

            tinyobj::index_t idx = shapes[i].mesh.indices[index_offset + v];
            //printf("    face[%ld].v[%ld].idx = %d/%d/%d\n", static_cast<long>(f), static_cast<long>(v), idx.vertex_index, idx.normal_index, idx.texcoord_index);
            //printf("  v[%ld] = (%f, %f, %f)\n", static_cast<long>(v), static_cast<const double>(attrib.vertices[3 * idx.vertex_index + 0]), static_cast<const double>(attrib.vertices[3 * idx.vertex_index + 1]), static_cast<const double>(attrib.vertices[3 * idx.vertex_index + 2]));

             //std::cout << "here2" << '\n';
            //  vert_indicies.size();
            int vi = 0;
            //  //std::cout << "here1" << '\n';
            while ( vi < vert_indicies.size() ) {
              tinyobj::index_t other = vert_indicies.at(vi);
              if (other.vertex_index == idx.vertex_index && other.normal_index == idx.normal_index && other.texcoord_index == idx.texcoord_index) { break; }
              vi++;
            }
            //
            if ( vi >= vert_indicies.size() ) {
              vert_indicies.push_back(idx);

              Vertex tmp;
              tmp.Position = idx.vertex_index == -1 ? glm::vec3(0.0f) : glm::vec3(attrib.vertices[3 * idx.vertex_index + 0], attrib.vertices[3 * idx.vertex_index + 1], attrib.vertices[3 * idx.vertex_index + 2]);
              tmp.Normal = idx.normal_index == -1 ? glm::vec3(0.0f) : glm::vec3(attrib.normals[3 * idx.normal_index + 0], attrib.normals[3 * idx.normal_index + 1], attrib.normals[3 * idx.normal_index + 2]);
              tmp.TexCoords = idx.texcoord_index == -1 ? glm::vec2(0.0f) : glm::vec2(attrib.texcoords[2 * idx.texcoord_index + 0], attrib.texcoords[2 * idx.texcoord_index + 1]);

              vertices->push_back(tmp);
              indices->push_back(vertices->size() - 1);
            }
            else { indices->push_back(vi); }
          }
        }

        index_offset += fnum;
      }

      // for (int i = 0; i < vertices->size(); i++) {
      //   std::cout << "Vertex: (" << vertices->at(i).Position.x << ", " << vertices->at(i).Position.y << ", " << vertices->at(i).Position.z << ") " << "\n" ;
      // }
      //
      // std::cout << "Indicies: ";
      // for (int i = 0; i < indices->size(); i++) {
      //   std::cout << indices->at(i) << ", ";
      // }

      Mesh * ret = new Mesh(vertices, indices);
      meshes.push_back(ret);
    }

  }

};


static void PrintInfo(const tinyobj::attrib_t& attrib,
                      const std::vector<tinyobj::shape_t>& shapes,
                      const std::vector<tinyobj::material_t>& materials) {
  std::cout << "# of vertices  : " << (attrib.vertices.size() / 3) << std::endl;
  std::cout << "# of normals   : " << (attrib.normals.size() / 3) << std::endl;
  std::cout << "# of texcoords : " << (attrib.texcoords.size() / 2)
            << std::endl;

  std::cout << "# of shapes    : " << shapes.size() << std::endl;
  std::cout << "# of materials : " << materials.size() << std::endl;

  for (size_t v = 0; v < attrib.vertices.size() / 3; v++) {
    printf("  v[%ld] = (%f, %f, %f)\n", static_cast<long>(v),
           static_cast<const double>(attrib.vertices[3 * v + 0]),
           static_cast<const double>(attrib.vertices[3 * v + 1]),
           static_cast<const double>(attrib.vertices[3 * v + 2]));
  }

  for (size_t v = 0; v < attrib.normals.size() / 3; v++) {
    printf("  n[%ld] = (%f, %f, %f)\n", static_cast<long>(v),
           static_cast<const double>(attrib.normals[3 * v + 0]),
           static_cast<const double>(attrib.normals[3 * v + 1]),
           static_cast<const double>(attrib.normals[3 * v + 2]));
  }

  for (size_t v = 0; v < attrib.texcoords.size() / 2; v++) {
    printf("  uv[%ld] = (%f, %f)\n", static_cast<long>(v),
           static_cast<const double>(attrib.texcoords[2 * v + 0]),
           static_cast<const double>(attrib.texcoords[2 * v + 1]));
  }

  // For each shape
  for (size_t i = 0; i < shapes.size(); i++) {
    printf("shape[%ld].name = %s\n", static_cast<long>(i),
           shapes[i].name.c_str());
    printf("Size of shape[%ld].mesh.indices: %lu\n", static_cast<long>(i),
           static_cast<unsigned long>(shapes[i].mesh.indices.size()));
    printf("Size of shape[%ld].lines.indices: %lu\n", static_cast<long>(i),
           static_cast<unsigned long>(shapes[i].lines.indices.size()));
    printf("Size of shape[%ld].points.indices: %lu\n", static_cast<long>(i),
           static_cast<unsigned long>(shapes[i].points.indices.size()));

    size_t index_offset = 0;

    assert(shapes[i].mesh.num_face_vertices.size() ==
           shapes[i].mesh.material_ids.size());

    assert(shapes[i].mesh.num_face_vertices.size() ==
           shapes[i].mesh.smoothing_group_ids.size());

    printf("shape[%ld].num_faces: %lu\n", static_cast<long>(i),
           static_cast<unsigned long>(shapes[i].mesh.num_face_vertices.size()));

    // For each face
    for (size_t f = 0; f < shapes[i].mesh.num_face_vertices.size(); f++) {
      size_t fnum = shapes[i].mesh.num_face_vertices[f];

      printf("  face[%ld].fnum = %ld\n", static_cast<long>(f),
             static_cast<unsigned long>(fnum));

      // For each vertex in the face
      for (size_t v = 0; v < fnum; v++) {
        tinyobj::index_t idx = shapes[i].mesh.indices[index_offset + v];
        printf("    face[%ld].v[%ld].idx = %d/%d/%d\n", static_cast<long>(f),
               static_cast<long>(v), idx.vertex_index, idx.normal_index,
               idx.texcoord_index);
      }

      printf("  face[%ld].material_id = %d\n", static_cast<long>(f),
             shapes[i].mesh.material_ids[f]);
      printf("  face[%ld].smoothing_group_id = %d\n", static_cast<long>(f),
             shapes[i].mesh.smoothing_group_ids[f]);

      index_offset += fnum;
    }

    printf("shape[%ld].num_tags: %lu\n", static_cast<long>(i),
           static_cast<unsigned long>(shapes[i].mesh.tags.size()));
    for (size_t t = 0; t < shapes[i].mesh.tags.size(); t++) {
      printf("  tag[%ld] = %s ", static_cast<long>(t),
             shapes[i].mesh.tags[t].name.c_str());
      printf(" ints: [");
      for (size_t j = 0; j < shapes[i].mesh.tags[t].intValues.size(); ++j) {
        printf("%ld", static_cast<long>(shapes[i].mesh.tags[t].intValues[j]));
        if (j < (shapes[i].mesh.tags[t].intValues.size() - 1)) {
          printf(", ");
        }
      }
      printf("]");

      printf(" floats: [");
      for (size_t j = 0; j < shapes[i].mesh.tags[t].floatValues.size(); ++j) {
        printf("%f", static_cast<const double>(
                         shapes[i].mesh.tags[t].floatValues[j]));
        if (j < (shapes[i].mesh.tags[t].floatValues.size() - 1)) {
          printf(", ");
        }
      }
      printf("]");

      printf(" strings: [");
      for (size_t j = 0; j < shapes[i].mesh.tags[t].stringValues.size(); ++j) {
        printf("%s", shapes[i].mesh.tags[t].stringValues[j].c_str());
        if (j < (shapes[i].mesh.tags[t].stringValues.size() - 1)) {
          printf(", ");
        }
      }
      printf("]");
      printf("\n");
    }
  }

  for (size_t i = 0; i < materials.size(); i++) {
    printf("material[%ld].name = %s\n", static_cast<long>(i),
           materials[i].name.c_str());
    printf("  material.Ka = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].ambient[0]),
           static_cast<const double>(materials[i].ambient[1]),
           static_cast<const double>(materials[i].ambient[2]));
    printf("  material.Kd = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].diffuse[0]),
           static_cast<const double>(materials[i].diffuse[1]),
           static_cast<const double>(materials[i].diffuse[2]));
    printf("  material.Ks = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].specular[0]),
           static_cast<const double>(materials[i].specular[1]),
           static_cast<const double>(materials[i].specular[2]));
    printf("  material.Tr = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].transmittance[0]),
           static_cast<const double>(materials[i].transmittance[1]),
           static_cast<const double>(materials[i].transmittance[2]));
    printf("  material.Ke = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].emission[0]),
           static_cast<const double>(materials[i].emission[1]),
           static_cast<const double>(materials[i].emission[2]));
    printf("  material.Ns = %f\n",
           static_cast<const double>(materials[i].shininess));
    printf("  material.Ni = %f\n", static_cast<const double>(materials[i].ior));
    printf("  material.dissolve = %f\n",
           static_cast<const double>(materials[i].dissolve));
    printf("  material.illum = %d\n", materials[i].illum);
    printf("  material.map_Ka = %s\n", materials[i].ambient_texname.c_str());
    printf("  material.map_Kd = %s\n", materials[i].diffuse_texname.c_str());
    printf("  material.map_Ks = %s\n", materials[i].specular_texname.c_str());
    printf("  material.map_Ns = %s\n",
           materials[i].specular_highlight_texname.c_str());
    printf("  material.map_bump = %s\n", materials[i].bump_texname.c_str());
    printf("    bump_multiplier = %f\n", static_cast<const double>(materials[i].bump_texopt.bump_multiplier));
    printf("  material.map_d = %s\n", materials[i].alpha_texname.c_str());
    printf("  material.disp = %s\n", materials[i].displacement_texname.c_str());
    printf("  <<PBR>>\n");
    printf("  material.Pr     = %f\n", static_cast<const double>(materials[i].roughness));
    printf("  material.Pm     = %f\n", static_cast<const double>(materials[i].metallic));
    printf("  material.Ps     = %f\n", static_cast<const double>(materials[i].sheen));
    printf("  material.Pc     = %f\n", static_cast<const double>(materials[i].clearcoat_thickness));
    printf("  material.Pcr    = %f\n", static_cast<const double>(materials[i].clearcoat_thickness));
    printf("  material.aniso  = %f\n", static_cast<const double>(materials[i].anisotropy));
    printf("  material.anisor = %f\n", static_cast<const double>(materials[i].anisotropy_rotation));
    printf("  material.map_Ke = %s\n", materials[i].emissive_texname.c_str());
    printf("  material.map_Pr = %s\n", materials[i].roughness_texname.c_str());
    printf("  material.map_Pm = %s\n", materials[i].metallic_texname.c_str());
    printf("  material.map_Ps = %s\n", materials[i].sheen_texname.c_str());
    printf("  material.norm   = %s\n", materials[i].normal_texname.c_str());
    std::map<std::string, std::string>::const_iterator it(
        materials[i].unknown_parameter.begin());
    std::map<std::string, std::string>::const_iterator itEnd(
        materials[i].unknown_parameter.end());

    for (; it != itEnd; it++) {
      printf("  material.%s = %s\n", it->first.c_str(), it->second.c_str());
    }
    printf("\n");
  }
}

#endif
