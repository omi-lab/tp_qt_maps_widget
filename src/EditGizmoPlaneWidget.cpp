#include "tp_qt_maps_widget/EditGizmoPlaneWidget.h"

#include "tp_qt_widgets/ColorButton.h"

#include <QBoxLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QCheckBox>

namespace tp_qt_maps_widget
{

//##################################################################################################
struct EditGizmoPlaneWidget::Private
{
  Q* q;
  tp_maps::GizmoPlaneParameters gizmoPlaneParameters;

  tp_utils::CallbackCollection<void()> toUI;
  tp_utils::CallbackCollection<void()> fromUI;

  //################################################################################################
  Private(Q* q_):
    q(q_)
  {

  }

  //################################################################################################
  tp_utils::Callback<void()> edited = [&]
  {
    q->edited();
  };
};

//##################################################################################################
EditGizmoPlaneWidget::EditGizmoPlaneWidget(QWidget* parent):
  QWidget(parent),
  d(new Private(this))
{
  auto l = new QVBoxLayout(this);
  l->setContentsMargins(0,0,0,0);

  {
    auto checkBox = new QCheckBox("Enabled");
    l->addWidget(checkBox);

    connect(checkBox, &QCheckBox::clicked, this, [&]{edited();});

    d->toUI.addCallback([=]
    {
      checkBox->setChecked(d->gizmoPlaneParameters.enable);
    });

    d->fromUI.addCallback([=]
    {
      d->gizmoPlaneParameters.enable = checkBox->isChecked();
    });
  }

  {
    auto button = new tp_qt_widgets::ColorButton("Color");
    l->addWidget(button);
    d->edited.connect(button->edited);

    d->toUI.addCallback([=]
    {
      button->setColor<glm::vec3>(d->gizmoPlaneParameters.color);
    });

    d->fromUI.addCallback([=]
    {
      d->gizmoPlaneParameters.color = button->toFloat3<glm::vec3>();
    });
  }

  {
    auto checkBox = new QCheckBox("Use selected color");
    l->addWidget(checkBox);

    connect(checkBox, &QCheckBox::clicked, this, [&]{edited();});

    d->toUI.addCallback([=]
    {
      checkBox->setChecked(d->gizmoPlaneParameters.useSelectedColor);
    });

    d->fromUI.addCallback([=]
    {
      d->gizmoPlaneParameters.useSelectedColor = checkBox->isChecked();
    });
  }

  {
    auto button = new tp_qt_widgets::ColorButton("Selected color");
    l->addWidget(button);
    d->edited.connect(button->edited);

    d->toUI.addCallback([=]
    {
      button->setColor<glm::vec3>(d->gizmoPlaneParameters.selectedColor);
    });

    d->fromUI.addCallback([=]
    {
      d->gizmoPlaneParameters.selectedColor = button->toFloat3<glm::vec3>();
    });
  }

  {
    auto spin = new QDoubleSpinBox();
    l->addWidget(new QLabel("Size"));
    l->addWidget(spin);

    connect(spin, &QDoubleSpinBox::valueChanged, this, [&]{edited();});

    spin->setRange(0.0, 1.0);
    spin->setSingleStep(0.01);

    d->toUI.addCallback([=]
    {
      spin->setValue(double(d->gizmoPlaneParameters.size));
    });

    d->fromUI.addCallback([=]
    {
      d->gizmoPlaneParameters.size = float(spin->value());
    });
  }

  {
    auto spin = new QDoubleSpinBox();
    l->addWidget(new QLabel("Radius"));
    l->addWidget(spin);

    connect(spin, &QDoubleSpinBox::valueChanged, this, [&]{edited();});

    spin->setRange(0.0, 1.0);
    spin->setSingleStep(0.01);

    d->toUI.addCallback([=]
    {
      spin->setValue(double(d->gizmoPlaneParameters.radius));
    });

    d->fromUI.addCallback([=]
    {
      d->gizmoPlaneParameters.radius = float(spin->value());
    });
  }

  {
    auto spin = new QDoubleSpinBox();
    l->addWidget(new QLabel("Padding"));
    l->addWidget(spin);

    connect(spin, &QDoubleSpinBox::valueChanged, this, [&]{edited();});

    spin->setRange(0.0, 1.0);
    spin->setSingleStep(0.01);

    d->toUI.addCallback([=]
    {
      spin->setValue(double(d->gizmoPlaneParameters.padding));
    });

    d->fromUI.addCallback([=]
    {
      d->gizmoPlaneParameters.padding = float(spin->value());
    });
  }

  {
    auto checkBox = new QCheckBox("Center");
    l->addWidget(checkBox);

    connect(checkBox, &QCheckBox::clicked, this, [&]{edited();});

    d->toUI.addCallback([=]
    {
      checkBox->setChecked(d->gizmoPlaneParameters.center);
    });

    d->fromUI.addCallback([=]
    {
      d->gizmoPlaneParameters.center = checkBox->isChecked();
    });
  }

  d->toUI();
}

//##################################################################################################
EditGizmoPlaneWidget::~EditGizmoPlaneWidget()
{
  delete d;
}

//##################################################################################################
void EditGizmoPlaneWidget::setGizmoPlaneParameters(const tp_maps::GizmoPlaneParameters& gizmoPlaneParameters)
{
  blockSignals(true);
  TP_CLEANUP([&]{blockSignals(false);});

  d->gizmoPlaneParameters = gizmoPlaneParameters;
  d->toUI();
}

//##################################################################################################
const tp_maps::GizmoPlaneParameters& EditGizmoPlaneWidget::gizmoPlaneParameters() const
{
  d->fromUI();
  return d->gizmoPlaneParameters;
}

}
