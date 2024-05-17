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
  tp_utils::CallbackCollection<void(tp_maps::GizmoPlaneParameters& gizmoPlaneParameters)> fromUI;

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
EditGizmoPlaneWidget::EditGizmoPlaneWidget(bool optionalFields, QWidget* parent):
  QWidget(parent),
  d(new Private(this))
{
  auto l = new QVBoxLayout(this);
  l->setContentsMargins(0,0,0,0);

  {
    auto r = OptionalEditRow::init(optionalFields, l);

    auto checkBox = new QCheckBox("Enabled");
    r.l->addWidget(checkBox);

    connect(checkBox, &QCheckBox::clicked, this, [&]{edited();});

    d->toUI.addCallback([=]
    {
      checkBox->setChecked(d->gizmoPlaneParameters.enable);
    });

    d->fromUI.addCallback([=](tp_maps::GizmoPlaneParameters& gizmoPlaneParameters)
    {
      if(r.enabled())
        gizmoPlaneParameters.enable = checkBox->isChecked();
    });
  }

  {
    auto r = OptionalEditRow::init(optionalFields, l);

    auto button = new tp_qt_widgets::ColorButton("Color");
    r.l->addWidget(button);
    d->edited.connect(button->edited);

    d->toUI.addCallback([=]
    {
      button->setColor<glm::vec3>(d->gizmoPlaneParameters.color);
    });

    d->fromUI.addCallback([=](tp_maps::GizmoPlaneParameters& gizmoPlaneParameters)
    {
      if(r.enabled())
        gizmoPlaneParameters.color = button->toFloat3<glm::vec3>();
    });
  }

  {
    auto r = OptionalEditRow::init(optionalFields, l);

    auto checkBox = new QCheckBox("Use selected color");
    r.l->addWidget(checkBox);

    connect(checkBox, &QCheckBox::clicked, this, [&]{edited();});

    d->toUI.addCallback([=]
    {
      checkBox->setChecked(d->gizmoPlaneParameters.useSelectedColor);
    });

    d->fromUI.addCallback([=](tp_maps::GizmoPlaneParameters& gizmoPlaneParameters)
    {
      if(r.enabled())
        gizmoPlaneParameters.useSelectedColor = checkBox->isChecked();
    });
  }

  {
    auto r = OptionalEditRow::init(optionalFields, l);

    auto button = new tp_qt_widgets::ColorButton("Selected color");
    r.l->addWidget(button);
    d->edited.connect(button->edited);

    d->toUI.addCallback([=]
    {
      button->setColor<glm::vec3>(d->gizmoPlaneParameters.selectedColor);
    });

    d->fromUI.addCallback([=](tp_maps::GizmoPlaneParameters& gizmoPlaneParameters)
    {
      if(r.enabled())
        gizmoPlaneParameters.selectedColor = button->toFloat3<glm::vec3>();
    });
  }

  {
    auto r = OptionalEditRow::init(optionalFields, l);

    auto spin = new QDoubleSpinBox();
    r.l->addWidget(new QLabel("Size"));
    r.l->addWidget(spin);

    connect(spin, &QDoubleSpinBox::valueChanged, this, [&]{edited();});

    spin->setRange(0.0, 1.0);
    spin->setSingleStep(0.01);

    d->toUI.addCallback([=]
    {
      spin->setValue(double(d->gizmoPlaneParameters.size));
    });

    d->fromUI.addCallback([=](tp_maps::GizmoPlaneParameters& gizmoPlaneParameters)
    {
      if(r.enabled())
        gizmoPlaneParameters.size = float(spin->value());
    });
  }

  {
    auto r = OptionalEditRow::init(optionalFields, l);

    auto spin = new QDoubleSpinBox();
    r.l->addWidget(new QLabel("Radius"));
    r.l->addWidget(spin);

    connect(spin, &QDoubleSpinBox::valueChanged, this, [&]{edited();});

    spin->setRange(0.0, 1.0);
    spin->setSingleStep(0.01);

    d->toUI.addCallback([=]
    {
      spin->setValue(double(d->gizmoPlaneParameters.radius));
    });

    d->fromUI.addCallback([=](tp_maps::GizmoPlaneParameters& gizmoPlaneParameters)
    {
      if(r.enabled())
        gizmoPlaneParameters.radius = float(spin->value());
    });
  }

  {
    auto r = OptionalEditRow::init(optionalFields, l);

    auto spin = new QDoubleSpinBox();
    r.l->addWidget(new QLabel("Padding"));
    r.l->addWidget(spin);

    connect(spin, &QDoubleSpinBox::valueChanged, this, [&]{edited();});

    spin->setRange(0.0, 1.0);
    spin->setSingleStep(0.01);

    d->toUI.addCallback([=]
    {
      spin->setValue(double(d->gizmoPlaneParameters.padding));
    });

    d->fromUI.addCallback([=](tp_maps::GizmoPlaneParameters& gizmoPlaneParameters)
    {
      if(r.enabled())
        gizmoPlaneParameters.padding = float(spin->value());
    });
  }

  {
    auto r = OptionalEditRow::init(optionalFields, l);

    auto checkBox = new QCheckBox("Center");
    r.l->addWidget(checkBox);

    connect(checkBox, &QCheckBox::clicked, this, [&]{edited();});

    d->toUI.addCallback([=]
    {
      checkBox->setChecked(d->gizmoPlaneParameters.center);
    });

    d->fromUI.addCallback([=](tp_maps::GizmoPlaneParameters& gizmoPlaneParameters)
    {
      if(r.enabled())
        gizmoPlaneParameters.center = checkBox->isChecked();
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
  d->fromUI(d->gizmoPlaneParameters);
  return d->gizmoPlaneParameters;
}

//##################################################################################################
void EditGizmoPlaneWidget::updateGizmoPlaneParameters(tp_maps::GizmoPlaneParameters& gizmoPlaneParameters) const
{
  d->fromUI(gizmoPlaneParameters);
}

}
