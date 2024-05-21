#include "tp_qt_maps_widget/EditGizmoRingWidget.h"

#include "tp_qt_widgets/ColorButton.h"

#include "tp_qt_utils/Globals.h"

#include <QBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QDoubleSpinBox>

namespace tp_qt_maps_widget
{

//##################################################################################################
struct EditGizmoRingWidget::Private
{
  Q* q;
  tp_maps::GizmoRingParameters gizmoRingParameters;

  tp_utils::CallbackCollection<void()> toUI;
  tp_utils::CallbackCollection<void(tp_maps::GizmoRingParameters& gizmoRingParameters)> fromUI;

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
EditGizmoRingWidget::EditGizmoRingWidget(bool optionalFields, QWidget* parent):
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
      checkBox->setChecked(d->gizmoRingParameters.enable);
    });

    d->fromUI.addCallback([=](tp_maps::GizmoRingParameters& gizmoRingParameters)
    {
      if(r.enabled())
        gizmoRingParameters.enable = checkBox->isChecked();
    });
  }

  {
    auto r = OptionalEditRow::init(optionalFields, l);

    auto button = new tp_qt_widgets::ColorButton("Color");
    r.l->addWidget(button);
    d->edited.connect(button->edited);

    d->toUI.addCallback([=]
    {
      button->setColor<glm::vec3>(d->gizmoRingParameters.color);
    });

    d->fromUI.addCallback([=](tp_maps::GizmoRingParameters& gizmoRingParameters)
    {
      if(r.enabled())
        gizmoRingParameters.color = button->toFloat3<glm::vec3>();
    });
  }

  {
    auto r = OptionalEditRow::init(optionalFields, l);

    auto checkBox = new QCheckBox("Use selected color");
    r.l->addWidget(checkBox);

    connect(checkBox, &QCheckBox::clicked, this, [&]{edited();});

    d->toUI.addCallback([=]
    {
      checkBox->setChecked(d->gizmoRingParameters.useSelectedColor);
    });

    d->fromUI.addCallback([=](tp_maps::GizmoRingParameters& gizmoRingParameters)
    {
      if(r.enabled())
        gizmoRingParameters.useSelectedColor = checkBox->isChecked();
    });
  }

  {
    auto r = OptionalEditRow::init(optionalFields, l);

    auto button = new tp_qt_widgets::ColorButton("Selected color");
    r.l->addWidget(button);
    d->edited.connect(button->edited);

    d->toUI.addCallback([=]
    {
      button->setColor<glm::vec3>(d->gizmoRingParameters.selectedColor);
    });

    d->fromUI.addCallback([=](tp_maps::GizmoRingParameters& gizmoRingParameters)
    {
      if(r.enabled())
        gizmoRingParameters.selectedColor = button->toFloat3<glm::vec3>();
    });
  }

  {
    auto r = OptionalEditRow::init(optionalFields, l);

    auto combo = new QComboBox();
    r.l->addWidget(new QLabel("Style"));
    r.l->addWidget(combo);

    connect(combo, &QComboBox::activated, this, [&]{edited();});

    combo->addItems(tp_qt_utils::convertStringList(tp_maps::gizmoRingStyles()));

    d->toUI.addCallback([=]
    {
      combo->setCurrentText(QString::fromStdString(tp_maps::gizmoRingStyleToString(d->gizmoRingParameters.style)));
    });

    d->fromUI.addCallback([=](tp_maps::GizmoRingParameters& gizmoRingParameters)
    {
      if(r.enabled())
        gizmoRingParameters.style = tp_maps::gizmoRingStyleFromString(combo->currentText().toStdString());
    });
  }

  {
    auto r = OptionalEditRow::init(optionalFields, l);

    auto spin = new QDoubleSpinBox();
    r.l->addWidget(new QLabel("Ring height"));
    r.l->addWidget(spin);

    connect(spin, &QDoubleSpinBox::valueChanged, this, [&]{edited();});

    spin->setRange(0.0, 2.0);
    spin->setSingleStep(0.01);
    spin->setDecimals(3);

    d->toUI.addCallback([=]
    {
      spin->setValue(double(d->gizmoRingParameters.ringHeight));
    });

    d->fromUI.addCallback([=](tp_maps::GizmoRingParameters& gizmoRingParameters)
    {
      if(r.enabled())
        gizmoRingParameters.ringHeight = float(spin->value());
    });
  }

  {
    auto r = OptionalEditRow::init(optionalFields, l);

    auto spin = new QDoubleSpinBox();
    r.l->addWidget(new QLabel("Outer radius"));
    r.l->addWidget(spin);

    connect(spin, &QDoubleSpinBox::valueChanged, this, [&]{edited();});

    spin->setRange(0.0, 2.0);
    spin->setSingleStep(0.01);
    spin->setDecimals(3);

    d->toUI.addCallback([=]
    {
      spin->setValue(double(d->gizmoRingParameters.outerRadius));
    });

    d->fromUI.addCallback([=](tp_maps::GizmoRingParameters& gizmoRingParameters)
    {
      if(r.enabled())
        gizmoRingParameters.outerRadius = float(spin->value());
    });
  }

  {
    auto r = OptionalEditRow::init(optionalFields, l);

    auto spin = new QDoubleSpinBox();
    r.l->addWidget(new QLabel("Inner radius"));
    r.l->addWidget(spin);

    connect(spin, &QDoubleSpinBox::valueChanged, this, [&]{edited();});

    spin->setRange(0.0, 2.0);
    spin->setSingleStep(0.01);
    spin->setDecimals(3);

    d->toUI.addCallback([=]
    {
      spin->setValue(double(d->gizmoRingParameters.innerRadius));
    });

    d->fromUI.addCallback([=](tp_maps::GizmoRingParameters& gizmoRingParameters)
    {
      if(r.enabled())
        gizmoRingParameters.innerRadius = float(spin->value());
    });
  }

  {
    auto r = OptionalEditRow::init(optionalFields, l);

    auto spin = new QDoubleSpinBox();
    r.l->addWidget(new QLabel("Spike radius"));
    r.l->addWidget(spin);

    connect(spin, &QDoubleSpinBox::valueChanged, this, [&]{edited();});

    spin->setRange(0.0, 2.0);
    spin->setSingleStep(0.01);
    spin->setDecimals(3);

    d->toUI.addCallback([=]
    {
      spin->setValue(double(d->gizmoRingParameters.spikeRadius));
    });

    d->fromUI.addCallback([=](tp_maps::GizmoRingParameters& gizmoRingParameters)
    {
      if(r.enabled())
        gizmoRingParameters.spikeRadius = float(spin->value());
    });
  }

  {
    auto r = OptionalEditRow::init(optionalFields, l);

    auto spin = new QDoubleSpinBox();
    r.l->addWidget(new QLabel("Arrow inner radius"));
    r.l->addWidget(spin);

    connect(spin, &QDoubleSpinBox::valueChanged, this, [&]{edited();});

    spin->setRange(0.0, 2.0);
    spin->setSingleStep(0.01);
    spin->setDecimals(3);

    d->toUI.addCallback([=]
    {
      spin->setValue(double(d->gizmoRingParameters.arrowInnerRadius));
    });

    d->fromUI.addCallback([=](tp_maps::GizmoRingParameters& gizmoRingParameters)
    {
      if(r.enabled())
        gizmoRingParameters.arrowInnerRadius = float(spin->value());
    });
  }

  {
    auto r = OptionalEditRow::init(optionalFields, l);

    auto spin = new QDoubleSpinBox();
    r.l->addWidget(new QLabel("Arrow outer radius"));
    r.l->addWidget(spin);

    connect(spin, &QDoubleSpinBox::valueChanged, this, [&]{edited();});

    spin->setRange(0.0, 2.0);
    spin->setSingleStep(0.01);
    spin->setDecimals(3);

    d->toUI.addCallback([=]
    {
      spin->setValue(double(d->gizmoRingParameters.arrowOuterRadius));
    });

    d->fromUI.addCallback([=](tp_maps::GizmoRingParameters& gizmoRingParameters)
    {
      if(r.enabled())
        gizmoRingParameters.arrowOuterRadius = float(spin->value());
    });
  }

  d->toUI();
}

//##################################################################################################
EditGizmoRingWidget::~EditGizmoRingWidget()
{
  delete d;
}

//##################################################################################################
void EditGizmoRingWidget::setGizmoRingParameters(const tp_maps::GizmoRingParameters& gizmoRingParameters)
{
  blockSignals(true);
  TP_CLEANUP([&]{blockSignals(false);});

  d->gizmoRingParameters = gizmoRingParameters;
  d->toUI();
}

//##################################################################################################
const tp_maps::GizmoRingParameters& EditGizmoRingWidget::gizmoRingParameters() const
{
  d->fromUI(d->gizmoRingParameters);
  return d->gizmoRingParameters;
}

//##################################################################################################
void EditGizmoRingWidget::updateGizmoRingParameters(tp_maps::GizmoRingParameters& gizmoRingParameters) const
{
  d->fromUI(gizmoRingParameters);
}

}
