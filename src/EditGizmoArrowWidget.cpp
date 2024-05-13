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
EditGizmoArrowWidget::EditGizmoArrowWidget(QWidget* parent):
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
      checkBox->setChecked(d->gizmoArrowParameters.enable);
    });

    d->fromUI.addCallback([=]
    {
      d->gizmoArrowParameters.enable = checkBox->isChecked();
    });
  }

  {
    auto button = new tp_qt_widgets::ColorButton("Color");
    l->addWidget(button);
    d->edited.connect(button->edited);

    d->toUI.addCallback([=]
    {
      button->setColor<glm::vec3>(d->gizmoArrowParameters.color);
    });

    d->fromUI.addCallback([=]
    {
      d->gizmoArrowParameters.color = button->toFloat3<glm::vec3>();
    });
  }

  {
    auto checkBox = new QCheckBox("Use selected color");
    l->addWidget(checkBox);

    connect(checkBox, &QCheckBox::clicked, this, [&]{edited();});

    d->toUI.addCallback([=]
    {
      checkBox->setChecked(d->gizmoArrowParameters.useSelectedColor);
    });

    d->fromUI.addCallback([=]
    {
      d->gizmoArrowParameters.useSelectedColor = checkBox->isChecked();
    });
  }

  {
    auto button = new tp_qt_widgets::ColorButton("Selected color");
    l->addWidget(button);
    d->edited.connect(button->edited);

    d->toUI.addCallback([=]
    {
      button->setColor<glm::vec3>(d->gizmoArrowParameters.selectedColor);
    });

    d->fromUI.addCallback([=]
    {
      d->gizmoArrowParameters.selectedColor = button->toFloat3<glm::vec3>();
    });
  }

  {
    auto spin = new QDoubleSpinBox();
    l->addWidget(new QLabel("Stem start"));
    l->addWidget(spin);

    connect(spin, &QDoubleSpinBox::valueChanged, this, [&]{edited();});

    spin->setRange(0.0, 1.0);
    spin->setSingleStep(0.01);

    d->toUI.addCallback([=]
    {
      spin->setValue(double(d->gizmoArrowParameters.stemStart));
    });

    d->fromUI.addCallback([=]
    {
      d->gizmoArrowParameters.stemStart = float(spin->value());
    });
  }

  {
    auto spin = new QDoubleSpinBox();
    l->addWidget(new QLabel("Stem length"));
    l->addWidget(spin);

    connect(spin, &QDoubleSpinBox::valueChanged, this, [&]{edited();});

    spin->setRange(0.0, 2.0);
    spin->setSingleStep(0.01);

    d->toUI.addCallback([=]
    {
      spin->setValue(double(d->gizmoArrowParameters.stemLength));
    });

    d->fromUI.addCallback([=]
    {
      d->gizmoArrowParameters.stemLength = float(spin->value());
    });
  }

  {
    auto spin = new QDoubleSpinBox();
    l->addWidget(new QLabel("Stem radius"));
    l->addWidget(spin);

    connect(spin, &QDoubleSpinBox::valueChanged, this, [&]{edited();});

    spin->setRange(0.0, 1.0);
    spin->setSingleStep(0.01);

    d->toUI.addCallback([=]
    {
      spin->setValue(double(d->gizmoArrowParameters.stemRadius));
    });

    d->fromUI.addCallback([=]
    {
      d->gizmoArrowParameters.stemRadius = float(spin->value());
    });
  }

  {
    auto spin = new QDoubleSpinBox();
    l->addWidget(new QLabel("Cone radius"));
    l->addWidget(spin);

    connect(spin, &QDoubleSpinBox::valueChanged, this, [&]{edited();});

    spin->setRange(0.0, 1.0);
    spin->setSingleStep(0.01);

    d->toUI.addCallback([=]
    {
      spin->setValue(double(d->gizmoArrowParameters.coneRadius));
    });

    d->fromUI.addCallback([=]
    {
      d->gizmoArrowParameters.coneRadius = float(spin->value());
    });
  }

  {
    auto spin = new QDoubleSpinBox();
    l->addWidget(new QLabel("Cone length"));
    l->addWidget(spin);

    connect(spin, &QDoubleSpinBox::valueChanged, this, [&]{edited();});

    spin->setRange(0.0, 1.0);
    spin->setSingleStep(0.01);

    d->toUI.addCallback([=]
    {
      spin->setValue(double(d->gizmoArrowParameters.coneLength));
    });

    d->fromUI.addCallback([=]
    {
      d->gizmoArrowParameters.coneLength = float(spin->value());
    });
  }

  {
    auto combo = new QComboBox();
    l->addWidget(new QLabel("Positive style"));
    l->addWidget(combo);

    connect(combo, &QComboBox::activated, this, [&]{edited();});

    combo->addItems(tp_qt_utils::convertStringList(tp_maps::gizmoArrowStyles()));

    d->toUI.addCallback([=]
    {
      combo->setCurrentText(QString::fromStdString(tp_maps::gizmoArrowStyleToString(d->gizmoArrowParameters.positiveArrowStyle)));
    });

    d->fromUI.addCallback([=]
    {
      d->gizmoArrowParameters.positiveArrowStyle = tp_maps::gizmoArrowStyleFromString(combo->currentText().toStdString());
    });
  }

  {
    auto combo = new QComboBox();
    l->addWidget(new QLabel("Negative style"));
    l->addWidget(combo);

    connect(combo, &QComboBox::activated, this, [&]{edited();});

    combo->addItems(tp_qt_utils::convertStringList(tp_maps::gizmoArrowStyles()));

    d->toUI.addCallback([=]
    {
      combo->setCurrentText(QString::fromStdString(tp_maps::gizmoArrowStyleToString(d->gizmoArrowParameters.negativeArrowStyle)));
    });

    d->fromUI.addCallback([=]
    {
      d->gizmoArrowParameters.negativeArrowStyle = tp_maps::gizmoArrowStyleFromString(combo->currentText().toStdString());
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
  d->fromUI();
  return d->gizmoArrowParameters;
}

}
