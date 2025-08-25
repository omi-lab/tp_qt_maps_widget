#include "tp_qt_maps_widget/EditLightSwapParametersWidget.h"
#include "tp_qt_maps_widget/EditSwapParametersWidget.h"

#include <QBoxLayout>
#include <QCheckBox>
#include <QLabel>

namespace tp_qt_maps_widget
{

//##################################################################################################
struct EditLightSwapParametersWidget::Private
{
  QWidget* scrollContents{nullptr};

  EditVec3SwapParametersWidget* diffuse {nullptr};
  EditVec3SwapParametersWidget* ambient {nullptr};
  EditVec3SwapParametersWidget* specular{nullptr};

  const float powerScale{830.0f};
  EditFloatSwapParametersWidget* diffuseScale{nullptr};
  EditFloatSwapParametersWidget* power{nullptr};

  EditFloatSwapParametersWidget* spotLightBlend{nullptr};
  EditFloatSwapParametersWidget* fov{nullptr};

  EditVec3SwapParametersWidget* offsetScale{nullptr};
  EditVec3SwapParametersWidget* offsetTranslation{nullptr};

  QCheckBox* excludeFromAngleX{nullptr};
  QCheckBox* excludeFromAngleY{nullptr};

  //################################################################################################
  void toPower()
  {
    tp_math_utils::FloatSwapParameters p = diffuseScale->floatSwapParameters();
    p.scale *= powerScale;
    p.bias *= powerScale;
    power->setFloatSwapParameters(p);
  }

  //################################################################################################
  void fromPower()
  {
    tp_math_utils::FloatSwapParameters p = power->floatSwapParameters();
    p.scale /= powerScale;
    p.bias /= powerScale;
    diffuseScale->setFloatSwapParameters(p);
  }
};

//##################################################################################################
EditLightSwapParametersWidget::EditLightSwapParametersWidget(Visibility visibility, QWidget* parent):
  QWidget(parent),
  d(new Private())
{
  auto l = new QVBoxLayout(this);
  l->setContentsMargins(0,0,0,0);

  {
    auto ll = new QHBoxLayout();
    l->addLayout(ll);
    ll->setContentsMargins(0,0,0,0);
  }

  auto addOtherSection = [&](auto title, bool visible, auto widget)
  {
    if(visible)
      l->addWidget(new QLabel(QString("<h3>%1</h3>").arg(title)), 2, Qt::AlignLeft);

    l->addWidget(widget);
    widget->setVisible(visible);
  };

  auto addSection = [&](auto title, bool visible, auto widget)
  {
    addOtherSection(title, visible, widget);
    widget->edited.addCallback([&]{Q_EMIT lightSwapParametersEdited();});
  };

  l->addWidget(new QLabel("<h2>Light Swap Properties</h2>"), 2, Qt::AlignLeft);

  d->diffuse = new EditVec3SwapParametersWidget(VectorComponents::RGB, HelperButtons::Color, 0.0f, 4.0f, 0.0f, 1.0f);
  addSection("Diffuse", visibility.diffuse, d->diffuse);

  d->ambient = new EditVec3SwapParametersWidget(VectorComponents::RGB, HelperButtons::Color, 0.0f, 4.0f, 0.0f, 1.0f);
  addSection("Ambient", visibility.ambient, d->ambient);

  d->specular = new EditVec3SwapParametersWidget(VectorComponents::RGB, HelperButtons::Color, 0.0f, 4.0f, 0.0f, 1.0f);
  addSection("Specular", visibility.specular, d->specular);

  d->diffuseScale = new EditFloatSwapParametersWidget(HelperButtons::Range, 0.0f, 1000.0f, 0.0f, 1000.0f);
  d->diffuseScale->edited.addCallback([&]{d->toPower();});
  addSection("Diffuse scale", visibility.diffuseScale, d->diffuseScale);

  d->power = new EditFloatSwapParametersWidget(HelperButtons::Range, 0.0f, 1000.0f*d->powerScale, 0.0f, 1000.0f*d->powerScale);
  d->power->edited.addCallback([&]{d->fromPower();});
  addSection("Power", visibility.power, d->power);

  d->spotLightBlend = new EditFloatSwapParametersWidget(HelperButtons::Default, 0.0f, 4.0f, 0.0f, 1.0f);
  addSection("Spot light blend", visibility.spotLightBlend, d->spotLightBlend);

  d->fov = new EditFloatSwapParametersWidget(HelperButtons::Default, 0.0f, 4.0f, 0.0f, 1.0f);
  addSection("FOV", visibility.fov, d->fov);

  d->offsetScale = new EditVec3SwapParametersWidget(VectorComponents::XYZ, HelperButtons::Default, 0.0f, 40.0f, 0.0f, 10.0f);
  addSection("Offset scale (radius of the bulb)", visibility.offsetScale, d->offsetScale);

  d->offsetTranslation = new EditVec3SwapParametersWidget(VectorComponents::XRotYRot, HelperButtons::Default, -180.0f, 180.0f, 0.0f, 0.0f);
  addSection("Offset translation (rotate around the focal point to move the origin)", visibility.offsetTranslation, d->offsetTranslation);

  {
    auto w = new QWidget();
    auto l = new QVBoxLayout(w);
    l->setContentsMargins(0,0,0,0);
    d->excludeFromAngleX = new QCheckBox("Exclude from angle X translation");
    d->excludeFromAngleY = new QCheckBox("Exclude from angle Y translation");
    l->addWidget(d->excludeFromAngleX);
    l->addWidget(d->excludeFromAngleY);
    addOtherSection("Translation", visibility.translation, w);

    connect(d->excludeFromAngleX, &QCheckBox::clicked, this, &EditLightSwapParametersWidget::lightSwapParametersEdited);
    connect(d->excludeFromAngleY, &QCheckBox::clicked, this, &EditLightSwapParametersWidget::lightSwapParametersEdited);
  }
}

//##################################################################################################
EditLightSwapParametersWidget::~EditLightSwapParametersWidget()
{
  delete d;
}

//##################################################################################################
void EditLightSwapParametersWidget::setLightSwapParameters(const tp_math_utils::LightSwapParameters& lightSwapParameters)
{
  blockSignals(true);
  TP_CLEANUP([&]{blockSignals(false);});

  d->diffuse ->setVec3SwapParameters(lightSwapParameters.diffuse );
  d->ambient ->setVec3SwapParameters(lightSwapParameters.ambient );
  d->specular->setVec3SwapParameters(lightSwapParameters.specular);

  d->diffuseScale->setFloatSwapParameters(lightSwapParameters.diffuseScale);
  d->toPower();

  d->spotLightBlend->setFloatSwapParameters(lightSwapParameters.spotLightBlend);
  d->fov->setFloatSwapParameters(lightSwapParameters.fov);

  d->offsetScale->setVec3SwapParameters(lightSwapParameters.offsetScale);
  d->offsetTranslation->setVec3SwapParameters(lightSwapParameters.offsetTranslation);

  d->excludeFromAngleX->setChecked(lightSwapParameters.excludeFromAngleX);
  d->excludeFromAngleY->setChecked(lightSwapParameters.excludeFromAngleY);
}

//##################################################################################################
tp_math_utils::LightSwapParameters EditLightSwapParametersWidget::lightSwapParameters() const
{
  tp_math_utils::LightSwapParameters lightSwapParameters;

  lightSwapParameters.diffuse  = d->diffuse ->vec3SwapParameters();
  lightSwapParameters.ambient  = d->ambient ->vec3SwapParameters();
  lightSwapParameters.specular = d->specular->vec3SwapParameters();

  lightSwapParameters.diffuseScale = d->diffuseScale->floatSwapParameters();

  lightSwapParameters.spotLightBlend = d->spotLightBlend->floatSwapParameters();
  lightSwapParameters.fov            = d->fov           ->floatSwapParameters();

  lightSwapParameters.offsetScale = d->offsetScale->vec3SwapParameters();
  lightSwapParameters.offsetTranslation = d->offsetTranslation->vec3SwapParameters();

  lightSwapParameters.excludeFromAngleX = d->excludeFromAngleX->isChecked();
  lightSwapParameters.excludeFromAngleY = d->excludeFromAngleY->isChecked();

  return lightSwapParameters;
}

}
