#include "tp_qt_maps_widget/Globals.h"

#include "tp_math_utils/materials/OpenGLMaterial.h"

#include <QSurfaceFormat>

namespace tp_qt_maps_widget
{

//##################################################################################################
int staticInit()
{
  QSurfaceFormat format;

#ifdef TP_FORCE_ES3
  format.setProfile(QSurfaceFormat::CoreProfile);
  format.setRenderableType(QSurfaceFormat::OpenGLES);
  format.setVersion(3, 0);
#elif defined TP_OSX
  format.setProfile(QSurfaceFormat::CoreProfile);
  format.setVersion(4, 1);
#else
  format.setVersion(3, 2);
  format.setSamples(4);
#endif

  format.setOption(QSurfaceFormat::DebugContext);
  QSurfaceFormat::setDefaultFormat(format);

  return 0;
}

//##################################################################################################
std::vector<tp_math_utils::Material> materialLibrary()
{
  std::vector<tp_math_utils::Material> materials;

  auto toF = [](int r, int g, int b)
  {
    return glm::vec3(float(r)/255.0f, float(g)/255.0f, float(b)/255.0f);
  };

  {
    auto material = materials.emplace_back("Silver polished").findOrAddOpenGL();
    material->albedo        = toF(252, 250, 249);
    material->roughness     = 0.1f;
    material->metalness     = 1.0f;
    material->alpha         = 1.0f;
    material->albedoScale   = 1.0f;
  }

  {
    auto material = materials.emplace_back("Silver satin").findOrAddOpenGL();
    material->albedo        = toF(252, 250, 249);
    material->roughness     = 0.4f;
    material->metalness     = 1.0f;
    material->alpha         = 1.0f;
    material->albedoScale   = 1.0f;
  }

  {
    auto material = materials.emplace_back("Silver matte").findOrAddOpenGL();
    material->albedo        = toF(252, 250, 249);
    material->roughness     = 0.7f;
    material->metalness     = 1.0f;
    material->alpha         = 1.0f;
    material->albedoScale   = 1.0f;
  }

  {
    auto material = materials.emplace_back("Gold polished").findOrAddOpenGL();
    material->albedo        = toF(243, 201, 104);
    material->roughness     = 0.1f;
    material->metalness     = 1.0f;
    material->alpha         = 1.0f;
    material->albedoScale   = 1.0f;
  }

  {
    auto material = materials.emplace_back("Gold satin").findOrAddOpenGL();
    material->albedo        = toF(243, 201, 104);
    material->roughness     = 0.4f;
    material->metalness     = 1.0f;
    material->alpha         = 1.0f;
    material->albedoScale   = 1.0f;
  }

  {
    auto material = materials.emplace_back("Gold matte").findOrAddOpenGL();
    material->albedo        = toF(243, 201, 104);
    material->roughness     = 0.7f;
    material->metalness     = 1.0f;
    material->alpha         = 1.0f;
    material->albedoScale   = 1.0f;
  }

  {
    auto material = materials.emplace_back("Copper polished").findOrAddOpenGL();
    material->albedo        = toF(238, 158, 137);
    material->roughness     = 0.1f;
    material->metalness     = 1.0f;
    material->alpha         = 1.0f;
    material->albedoScale   = 1.0f;
  }

  {
    auto material = materials.emplace_back("Copper satin").findOrAddOpenGL();
    material->albedo        = toF(238, 158, 137);
    material->roughness     = 0.4f;
    material->metalness     = 1.0f;
    material->alpha         = 1.0f;
    material->albedoScale   = 1.0f;
  }

  {
    auto material = materials.emplace_back("Copper matte").findOrAddOpenGL();
    material->albedo        = toF(238, 158, 137);
    material->roughness     = 0.7f;
    material->metalness     = 1.0f;
    material->alpha         = 1.0f;
    material->albedoScale   = 1.0f;
  }

  return materials;
}

}
