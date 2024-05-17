#include "tp_qt_maps_widget/EditGizmoArrowWidget.h"

#include "tp_qt_widgets/ColorButton.h"

#include <QBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QCheckBox>

namespace tp_qt_maps_widget
{

//##################################################################################################
struct EditGizmoArrowWidget::Private
{
  Q* q;
  tp_maps::GizmoArrowParameters gizmoArrowParameters;

  tp_utils::CallbackCollection<void()> toUI;
  tp_utils::CallbackCollection<void(tp_maps::GizmoArrowParameters& gizmoArrowParameters)> fromUI;

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
EditGizmoArrowWidget::EditGizmoArrowWidget(bool optionalFields, QWidget* parent):
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
      checkBox->setChecked(d->gizmoArrowParameters.enable);
    });

    d->fromUI.addCallback([=](tp_maps::GizmoArrowParameters& gizmoArrowParameters)
    {
      if(r.enabled())
        gizmoArrowParameters.enable = checkBox->isChecked();
    });
  }

  {
    auto r = OptionalEditRow::init(optionalFields, l);

    auto button = new tp_qt_widgets::ColorButton("Color");
    r.l->addWidget(button);
    d->edited.connect(button->edited);

    d->toUI.addCallback([=]
    {
      button->setColor<glm::vec3>(d->gizmoArrowParameters.color);
    });

    d->fromUI.addCallback([=](tp_maps::GizmoArrowParameters& gizmoArrowParameters)
    {
      if(r.enabled())
        gizmoArrowParameters.color = button->toFloat3<glm::vec3>();
    });
  }

  {
    auto r = OptionalEditRow::init(optionalFields, l);

    auto checkBox = new QCheckBox("Use selected color");
    r.l->addWidget(checkBox);

    connect(checkBox, &QCheckBox::clicked, this, [&]{edited();});

    d->toUI.addCallback([=]
    {
      checkBox->setChecked(d->gizmoArrowParameters.useSelectedColor);
    });

    d->fromUI.addCallback([=](tp_maps::GizmoArrowParameters& gizmoArrowParameters)
    {
      if(r.enabled())
        gizmoArrowParameters.useSelectedColor = checkBox->isChecked();
    });
  }

  {
    auto r = OptionalEditRow::init(optionalFields, l);

    auto button = new tp_qt_widgets::ColorButton("Selected color");
    r.l->addWidget(button);
    d->edited.connect(button->edited);

    d->toUI.addCallback([=]
    {
      button->setColor<glm::vec3>(d->gizmoArrowParameters.selectedColor);
    });

    d->fromUI.addCallback([=](tp_maps::GizmoArrowParameters& gizmoArrowParameters)
    {
      if(r.enabled())
        gizmoArrowParameters.selectedColor = button->toFloat3<glm::vec3>();
    });
  }

  {
    auto r = OptionalEditRow::init(optionalFields, l);

    auto spin = new QDoubleSpinBox();
    r.l->addWidget(new QLabel("Stem start"));
    r.l->addWidget(spin);

    connect(spin, &QDoubleSpinBox::valueChanged, this, [&]{edited();});

    spin->setRange(0.0, 1.0);
    spin->setSingleStep(0.01);

    d->toUI.addCallback([=]
    {
      spin->setValue(double(d->gizmoArrowParameters.stemStart));
    });

    d->fromUI.addCallback([=](tp_maps::GizmoArrowParameters& gizmoArrowParameters)
    {
      if(r.enabled())
        gizmoArrowParameters.stemStart = float(spin->value());
    });
  }

  {
    auto r = OptionalEditRow::init(optionalFields, l);

    auto spin = new QDoubleSpinBox();
    r.l->addWidget(new QLabel("Stem length"));
    r.l->addWidget(spin);

    connect(spin, &QDoubleSpinBox::valueChanged, this, [&]{edited();});

    spin->setRange(0.0, 2.0);
    spin->setSingleStep(0.01);

    d->toUI.addCallback([=]
    {
      spin->setValue(double(d->gizmoArrowParameters.stemLength));
    });

    d->fromUI.addCallback([=](tp_maps::GizmoArrowParameters& gizmoArrowParameters)
    {
      if(r.enabled())
        gizmoArrowParameters.stemLength = float(spin->value());
    });
  }

  {
    auto r = OptionalEditRow::init(optionalFields, l);

    auto spin = new QDoubleSpinBox();
    r.l->addWidget(new QLabel("Stem radius"));
    r.l->addWidget(spin);

    connect(spin, &QDoubleSpinBox::valueChanged, this, [&]{edited();});

    spin->setRange(0.0, 1.0);
    spin->setSingleStep(0.01);

    d->toUI.addCallback([=]
    {
      spin->setValue(double(d->gizmoArrowParameters.stemRadius));
    });

    d->fromUI.addCallback([=](tp_maps::GizmoArrowParameters& gizmoArrowParameters)
    {
      if(r.enabled())
        gizmoArrowParameters.stemRadius = float(spin->value());
    });
  }

  {
    auto r = OptionalEditRow::init(optionalFields, l);

    auto spin = new QDoubleSpinBox();
    r.l->addWidget(new QLabel("Cone radius"));
    r.l->addWidget(spin);

    connect(spin, &QDoubleSpinBox::valueChanged, this, [&]{edited();});

    spin->setRange(0.0, 1.0);
    spin->setSingleStep(0.01);

    d->toUI.addCallback([=]
    {
      spin->setValue(double(d->gizmoArrowParameters.coneRadius));
    });

    d->fromUI.addCallback([=](tp_maps::GizmoArrowParameters& gizmoArrowParameters)
    {
      if(r.enabled())
        gizmoArrowParameters.coneRadius = float(spin->value());
    });
  }

  {
    auto r = OptionalEditRow::init(optionalFields, l);

    auto spin = new QDoubleSpinBox();
    r.l->addWidget(new QLabel("Cone length"));
    r.l->addWidget(spin);

    connect(spin, &QDoubleSpinBox::valueChanged, this, [&]{edited();});

    spin->setRange(0.0, 1.0);
    spin->setSingleStep(0.01);

    d->toUI.addCallback([=]
    {
      spin->setValue(double(d->gizmoArrowParameters.coneLength));
    });

    d->fromUI.addCallback([=](tp_maps::GizmoArrowParameters& gizmoArrowParameters)
    {
      if(r.enabled())
        gizmoArrowParameters.coneLength = float(spin->value());
    });
  }

  {
    auto r = OptionalEditRow::init(optionalFields, l);

    auto combo = new QComboBox();
    r.l->addWidget(new QLabel("Positive style"));
    r.l->addWidget(combo);

    connect(combo, &QComboBox::activated, this, [&]{edited();});

    combo->addItems(tp_qt_utils::convertStringList(tp_maps::gizmoArrowStyles()));

    d->toUI.addCallback([=]
    {
      combo->setCurrentText(QString::fromStdString(tp_maps::gizmoArrowStyleToString(d->gizmoArrowParameters.positiveArrowStyle)));
    });

    d->fromUI.addCallback([=](tp_maps::GizmoArrowParameters& gizmoArrowParameters)
    {
      if(r.enabled())
        gizmoArrowParameters.positiveArrowStyle = tp_maps::gizmoArrowStyleFromString(combo->currentText().toStdString());
    });
  }

  {
    auto r = OptionalEditRow::init(optionalFields, l);

    auto combo = new QComboBox();
    r.l->addWidget(new QLabel("Negative style"));
    r.l->addWidget(combo);

    connect(combo, &QComboBox::activated, this, [&]{edited();});

    combo->addItems(tp_qt_utils::convertStringList(tp_maps::gizmoArrowStyles()));

    d->toUI.addCallback([=]
    {
      combo->setCurrentText(QString::fromStdString(tp_maps::gizmoArrowStyleToString(d->gizmoArrowParameters.negativeArrowStyle)));
    });

    d->fromUI.addCallback([=](tp_maps::GizmoArrowParameters& gizmoArrowParameters)
    {
      if(r.enabled())
        gizmoArrowParameters.negativeArrowStyle = tp_maps::gizmoArrowStyleFromString(combo->currentText().toStdString());
    });
  }

  d->toUI();
}

//##################################################################################################
EditGizmoArrowWidget::~EditGizmoArrowWidget()
{
  delete d;
}

//##################################################################################################
void EditGizmoArrowWidget::setGizmoArrowParameters(const tp_maps::GizmoArrowParameters& gizmoArrowParameters)
{
  blockSignals(true);
  TP_CLEANUP([&]{blockSignals(false);});

  d->gizmoArrowParameters = gizmoArrowParameters;
  d->toUI();
}

//##################################################################################################
const tp_maps::GizmoArrowParameters& EditGizmoArrowWidget::gizmoArrowParameters() const
{
  d->fromUI(d->gizmoArrowParameters);
  return d->gizmoArrowParameters;
}

//##################################################################################################
void EditGizmoArrowWidget::updateGizmoArrowParameters(tp_maps::GizmoArrowParameters& gizmoArrowParameters) const
{
  d->fromUI(gizmoArrowParameters);
}

}
