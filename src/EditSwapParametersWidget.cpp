#include "tp_qt_maps_widget/EditSwapParametersWidget.h"

#include "tp_qt_widgets/SpinSlider.h"

#include "tp_qt_maps/ConvertTexture.h"

#include "tp_math_utils/SwapParameters.h"

#include <QBoxLayout>
#include <QAbstractButton>
#include <QLabel>
#include <QPushButton>

namespace tp_qt_maps_widget
{

//##################################################################################################
struct EditVec3ComponentWidget::Private
{
  tp_qt_widgets::SpinSlider* shared{nullptr};
  tp_qt_widgets::SpinSlider* x{nullptr};
  tp_qt_widgets::SpinSlider* y{nullptr};
  tp_qt_widgets::SpinSlider* z{nullptr};
};

//##################################################################################################
EditVec3ComponentWidget::EditVec3ComponentWidget(VectorComponents vectorComponents,
                                                 tp_qt_widgets::SliderMode sliderMode,
                                                 float min,
                                                 float max,
                                                 const QString& title,
                                                 QWidget* parent):
  QWidget(parent),
  d(new Private())
{
  auto addExpandIcon = [&](QBoxLayout* layout)
  {
    QIcon normalIcon = tp_qt_maps::loadIconFromResource("/omi_scene_builder/right_chevron.png");
    QIcon expandedIcon = tp_qt_maps::loadIconFromResource("/omi_scene_builder/down_chevron.png");

    QPushButton* button = new QPushButton(normalIcon, "");
    QString buttonStyle = "QPushButton{border:none;background-color:rgba(255, 255, 255,0);}";
    button->setStyleSheet(buttonStyle);
    int size = 16;
    button->setIconSize(QSize(size,size));
    button->setMinimumSize(size,size);
    button->setMaximumSize(size,size);
    layout->addWidget(button);

    button->setCheckable(true);

    connect( button, &QAbstractButton::toggled, button, [=](bool b)
    {
      b ? button->setIcon(expandedIcon) : button->setIcon(normalIcon);
    });

    return button;
  };

  auto makeSlider = [&](QBoxLayout* l, const QString& title, int labelWidth, Qt::Alignment labelAlign, QAbstractButton* button = nullptr)
  {
    auto hLayout = new QHBoxLayout();
    hLayout->setContentsMargins(0, 0, 0, 0);
    l->addLayout(hLayout);

    QLabel* label = new QLabel(title);
    label->setFixedWidth(labelWidth);
    label->setAlignment( labelAlign | Qt::AlignVCenter);
    hLayout->addWidget(label, 0, Qt::AlignLeft);

    if(button)
      hLayout->addWidget(button);

    auto slider = new tp_qt_widgets::SpinSlider(sliderMode);
    slider->setRange(min, max);
    hLayout->addWidget(slider);

    return slider;
  };

  auto l = new QVBoxLayout(this);
  l->setContentsMargins(0, 0, 0, 0);

  auto expandButton = addExpandIcon(l);
  d->shared = makeSlider(l, title, 50, Qt::AlignLeft, expandButton);

  auto hLayout = new QHBoxLayout();
  hLayout->setContentsMargins(0, 0, 0, 0);
  l->addLayout(hLayout);

  QWidget* group = new QWidget(this);
  auto vLayout = new QVBoxLayout(group);
  vLayout->setContentsMargins(0, 0, 0, 0);

  const char* c;
  switch(vectorComponents)
  {
    case VectorComponents::RGB: c="RGB"; break;
    case VectorComponents::XYZ: c="XYZ"; break;
  }

  d->x = makeSlider(vLayout, QString(c[0]), 72, Qt::AlignRight);
  d->y = makeSlider(vLayout, QString(c[1]), 72, Qt::AlignRight);
  d->z = makeSlider(vLayout, QString(c[2]), 72, Qt::AlignRight);

  d->shared->edited.addCallback([this](float)
  {
    d->x->setValue(d->shared->value());
    d->y->setValue(d->shared->value());
    d->z->setValue(d->shared->value());
    edited();
  });

  d->x->edited.addCallback([this](float){edited();});
  d->y->edited.addCallback([this](float){edited();});
  d->z->edited.addCallback([this](float){edited();});

  hLayout->addWidget(group);

  connect(expandButton, &QAbstractButton::toggled, group, [=]
  {
    group->setVisible(expandButton->isChecked());
  });

  group->setVisible(false);
}

//##################################################################################################
EditVec3ComponentWidget::~EditVec3ComponentWidget()
{
  delete d;
}

//##################################################################################################
void EditVec3ComponentWidget::setVec3(const glm::vec3& vec3)
{
  d->shared->setValue(vec3.x);
  d->x->setValue(vec3.x);
  d->y->setValue(vec3.y);
  d->z->setValue(vec3.z);
}

//##################################################################################################
glm::vec3 EditVec3ComponentWidget::vec3() const
{
  return {d->x->value(), d->x->value(), d->x->value()};
}

//##################################################################################################
struct EditVec3SwapParametersWidget::Private
{
  VectorComponents vectorComponents;
  HelperButtons helperButtons;

  EditVec3ComponentWidget* use  {nullptr};
  EditVec3ComponentWidget* scale{nullptr};
  EditVec3ComponentWidget* bias {nullptr};

  //################################################################################################
  Private(VectorComponents vectorComponents_, HelperButtons helperButtons_):
    vectorComponents(vectorComponents_),
    helperButtons(helperButtons_)
  {

  }
};

//##################################################################################################
EditVec3SwapParametersWidget::EditVec3SwapParametersWidget(VectorComponents vectorComponents,
                                                           HelperButtons helperButtons,
                                                           float scaleMin,
                                                           float scaleMax,
                                                           float biasMin,
                                                           float biasMax,
                                                           QWidget* parent):
  QWidget(parent),
  d(new Private(vectorComponents, helperButtons))
{
  auto l = new QVBoxLayout(this);
  l->setContentsMargins(0, 0, 0, 0);

  d->use   = new EditVec3ComponentWidget(vectorComponents, tp_qt_widgets::SliderMode::Linear, 0.0f    , 1.0f    , "Use"  );
  d->scale = new EditVec3ComponentWidget(vectorComponents, tp_qt_widgets::SliderMode::Linear, scaleMin, scaleMax, "Scale");
  d->bias  = new EditVec3ComponentWidget(vectorComponents, tp_qt_widgets::SliderMode::Linear, biasMin , biasMax , "Bias" );

  l->addWidget(d->use  );
  l->addWidget(d->scale);
  l->addWidget(d->bias );

  d->use  ->edited.addCallback([&]{edited();});
  d->scale->edited.addCallback([&]{edited();});
  d->bias ->edited.addCallback([&]{edited();});
}

//##################################################################################################
EditVec3SwapParametersWidget::~EditVec3SwapParametersWidget()
{
  delete d;
}

//##################################################################################################
void EditVec3SwapParametersWidget::setVec3SwapParameters(const tp_math_utils::Vec3SwapParameters& vec3SwapParameters)
{
  d->use  ->setVec3(vec3SwapParameters.use  );
  d->scale->setVec3(vec3SwapParameters.scale);
  d->bias ->setVec3(vec3SwapParameters.bias );
}

//##################################################################################################
tp_math_utils::Vec3SwapParameters EditVec3SwapParametersWidget::vec3SwapParameters() const
{
  tp_math_utils::Vec3SwapParameters vec3SwapParameters;
  vec3SwapParameters.use   = d->use  ->vec3();
  vec3SwapParameters.scale = d->scale->vec3();
  vec3SwapParameters.bias  = d->bias ->vec3();
  return vec3SwapParameters;
}





//##################################################################################################
struct EditFloatSwapParametersWidget::Private
{
  HelperButtons helperButtons;

  tp_qt_widgets::SpinSlider* use  {nullptr};
  tp_qt_widgets::SpinSlider* scale{nullptr};
  tp_qt_widgets::SpinSlider* bias {nullptr};

  //################################################################################################
  Private(HelperButtons helperButtons_):
    helperButtons(helperButtons_)
  {

  }
};

//##################################################################################################
EditFloatSwapParametersWidget::EditFloatSwapParametersWidget(HelperButtons helperButtons,
                                                             float scaleMin,
                                                             float scaleMax,
                                                             float biasMin,
                                                             float biasMax,
                                                             QWidget* parent):
  QWidget(parent),
  d(new Private(helperButtons))
{
  auto l = new QVBoxLayout(this);
  l->setContentsMargins(0, 0, 0, 0);

  auto addLine = [&](tp_qt_widgets::SliderMode sliderMode, float min, float max, const QString& title)
  {
    auto ll = new QHBoxLayout();
    ll->setContentsMargins(0, 0, 0, 0);
    l->addLayout(ll);

    auto label = new QLabel(title);
    label->setFixedWidth(72);
    ll->addWidget(label, 0, Qt::AlignLeft);

    auto spinSlider = new tp_qt_widgets::SpinSlider(sliderMode);
    spinSlider->setRange(min, max);
    ll->addWidget(spinSlider);

    spinSlider->edited.addCallback([&](float){edited();});

    return spinSlider;
  };

  d->use   = addLine(tp_qt_widgets::SliderMode::Linear, 0.0f    , 1.0f    , "Use"  );
  d->scale = addLine(tp_qt_widgets::SliderMode::Linear, scaleMin, scaleMax, "Scale");
  d->bias  = addLine(tp_qt_widgets::SliderMode::Linear, biasMin , biasMax , "Bias" );
}

//##################################################################################################
EditFloatSwapParametersWidget::~EditFloatSwapParametersWidget()
{
  delete d;
}

//##################################################################################################
void EditFloatSwapParametersWidget::setFloatSwapParameters(const tp_math_utils::FloatSwapParameters& floatSwapParameters)
{
  d->use  ->setValue(floatSwapParameters.use  );
  d->scale->setValue(floatSwapParameters.scale);
  d->bias ->setValue(floatSwapParameters.bias );
}

//##################################################################################################
tp_math_utils::FloatSwapParameters EditFloatSwapParametersWidget::floatSwapParameters() const
{
  tp_math_utils::FloatSwapParameters floatSwapParameters;
  floatSwapParameters.use   = d->use  ->value();
  floatSwapParameters.scale = d->scale->value();
  floatSwapParameters.bias  = d->bias ->value();
  return floatSwapParameters;
}



}
