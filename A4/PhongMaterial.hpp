// Winter 2020

#pragma once

#include <glm/glm.hpp>

#include "Material.hpp"

class PhongMaterial : public Material {
public:
  PhongMaterial(const glm::vec3& kd, const glm::vec3& ks, double shininess, double reflectivity);
  virtual ~PhongMaterial();
  glm::vec3 get_m_kd();
  glm::vec3 get_m_ks();
  double get_m_shine();
  double get_m_reflect();

private:
  glm::vec3 m_kd;
  glm::vec3 m_ks;

  double m_shininess;
  double m_reflectivity;
};
