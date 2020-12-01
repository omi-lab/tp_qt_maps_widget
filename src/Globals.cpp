#include "tp_qt_maps_widget/Globals.h"

#include <QSurfaceFormat>

namespace tp_qt_maps_widget
{

//##################################################################################################
int staticInit()
{
  QSurfaceFormat format;

#ifdef TP_OSX
  format.setProfile(QSurfaceFormat::CoreProfile);
  format.setMajorVersion(4);
  format.setMinorVersion(1);
#else
  format.setMajorVersion(3);
  format.setMinorVersion(2);
  format.setSamples(4);
#endif

  format.setOption(QSurfaceFormat::DebugContext);
  QSurfaceFormat::setDefaultFormat(format);

  return 0;
}

//##################################################################################################
std::vector<tp_maps::Material> materialLibrary()
{
  std::vector<tp_maps::Material> materials;

  auto toF = [](int r, int g, int b)
  {
    return glm::vec3(float(r)/255.0f, float(g)/255.0f, float(b)/255.0f);
  };

  {
    auto& material = materials.emplace_back();
    material.name          = "Silver polished";
    material.diffuse       = toF(252, 250, 249);
    material.specular      = toF(255, 255, 255);
    material.roughness     = 0.1f;
    material.metalness     = 1.0f;
    material.alpha         = 1.0f;
    material.diffuseScale  = 1.0f;
    material.specularScale = 1.0f;
  }

  {
    auto& material = materials.emplace_back();
    material.name          = "Silver satin";
    material.diffuse       = toF(252, 250, 249);
    material.specular      = toF(255, 255, 255);
    material.roughness     = 0.4f;
    material.metalness     = 1.0f;
    material.alpha         = 1.0f;
    material.diffuseScale  = 1.0f;
    material.specularScale = 1.0f;
  }

  {
    auto& material = materials.emplace_back();
    material.name          = "Silver matte";
    material.diffuse       = toF(252, 250, 249);
    material.specular      = toF(255, 255, 255);
    material.roughness     = 0.7f;
    material.metalness     = 1.0f;
    material.alpha         = 1.0f;
    material.diffuseScale  = 1.0f;
    material.specularScale = 1.0f;
  }

  {
    auto& material = materials.emplace_back();
    material.name          = "Gold polished";
    material.diffuse       = toF(243, 201, 104);
    material.specular      = toF(255, 255, 255);
    material.roughness     = 0.1f;
    material.metalness     = 1.0f;
    material.alpha         = 1.0f;
    material.diffuseScale  = 1.0f;
    material.specularScale = 1.0f;
  }

  {
    auto& material = materials.emplace_back();
    material.name          = "Gold satin";
    material.diffuse       = toF(243, 201, 104);
    material.specular      = toF(255, 255, 255);
    material.roughness     = 0.4f;
    material.metalness     = 1.0f;
    material.alpha         = 1.0f;
    material.diffuseScale  = 1.0f;
    material.specularScale = 1.0f;
  }

  {
    auto& material = materials.emplace_back();
    material.name          = "Gold matte";
    material.diffuse       = toF(243, 201, 104);
    material.specular      = toF(255, 255, 255);
    material.roughness     = 0.7f;
    material.metalness     = 1.0f;
    material.alpha         = 1.0f;
    material.diffuseScale  = 1.0f;
    material.specularScale = 1.0f;
  }

  {
    auto& material = materials.emplace_back();
    material.name          = "Copper polished";
    material.diffuse       = toF(238, 158, 137);
    material.specular      = toF(255, 255, 255);
    material.roughness     = 0.1f;
    material.metalness     = 1.0f;
    material.alpha         = 1.0f;
    material.diffuseScale  = 1.0f;
    material.specularScale = 1.0f;
  }

  {
    auto& material = materials.emplace_back();
    material.name          = "Copper satin";
    material.diffuse       = toF(238, 158, 137);
    material.specular      = toF(255, 255, 255);
    material.roughness     = 0.4f;
    material.metalness     = 1.0f;
    material.alpha         = 1.0f;
    material.diffuseScale  = 1.0f;
    material.specularScale = 1.0f;
  }

  {
    auto& material = materials.emplace_back();
    material.name          = "Copper matte";
    material.diffuse       = toF(238, 158, 137);
    material.specular      = toF(255, 255, 255);
    material.roughness     = 0.7f;
    material.metalness     = 1.0f;
    material.alpha         = 1.0f;
    material.diffuseScale  = 1.0f;
    material.specularScale = 1.0f;
  }

  return materials;
}

}
