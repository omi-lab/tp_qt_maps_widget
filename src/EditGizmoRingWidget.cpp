#include "tp_qt_maps_widget/EditGizmoRingWidget.h"

#include "tp_qt_widgets/ColorButton.h"

#include "tp_qt_utils/Globals.h"

#include <QBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>

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
