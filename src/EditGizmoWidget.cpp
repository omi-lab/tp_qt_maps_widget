#include "tp_qt_maps_widget/EditGizmoWidget.h"
#include "tp_qt_maps_widget/EditGizmoRingWidget.h"
#include "tp_qt_maps_widget/EditGizmoArrowWidget.h"
#include "tp_qt_maps_widget/EditGizmoPlaneWidget.h"

#include "tp_qt_utils/Globals.h"

#include "tp_utils/FileUtils.h"

#include <QDialog>
#include <QBoxLayout>
#include <QLabel>
#include <QTabWidget>
#include <QPushButton>
#include <QPointer>
#include <QDialogButtonBox>
#include <QScrollArea>
#include <QSettings>
#include <QFileDialog>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QCheckBox>

namespace tp_qt_maps_widget
{

//##################################################################################################
struct EditGizmoWidget::Private
{
  Q* q;
  tp_maps::GizmoParameters gizmoParameters;

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

  //################################################################################################
  void addTitle(QVBoxLayout* l, const QString& title)
  {
    l->addWidget(new QLabel(QString("<b>%1</b>").arg(title)));

    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    l->addWidget(line);
  }

  //################################################################################################
  void addRing(QVBoxLayout* l,
               const QString& title,
               tp_maps::GizmoRingParameters tp_maps::GizmoParameters::* field)
  {
    addTitle(l, title);

    auto widget = new EditGizmoRingWidget();
    l->addWidget(widget);
    edited.connect(widget->edited);

    toUI.addCallback([=]
    {
      widget->setGizmoRingParameters(gizmoParameters.*field);
    });

    fromUI.addCallback([=]
    {
      gizmoParameters.*field = widget->gizmoRingParameters();
    });
  }

  //################################################################################################
  void addGroupRing(QVBoxLayout* l,
                    const QString& title,
                    std::vector<tp_maps::GizmoRingParameters tp_maps::GizmoParameters::*> fields)
  {
    addTitle(l, title);

    auto widget = new EditGizmoRingWidget(true);
    l->addWidget(widget);
    edited.connect(widget->edited);

    toUI.addCallback([=]
    {
      auto field = fields.front();
      widget->setGizmoRingParameters(gizmoParameters.*field);
    });

    fromUI.addCallback([=]
    {
      for(auto field : fields)
        widget->updateGizmoRingParameters(gizmoParameters.*field);
    });
  }

  //################################################################################################
  void addArrow(QVBoxLayout* l,
                const QString& title,
                tp_maps::GizmoArrowParameters tp_maps::GizmoParameters::* field)
  {
    addTitle(l, title);

    auto widget = new EditGizmoArrowWidget();
    l->addWidget(widget);
    edited.connect(widget->edited);

    toUI.addCallback([=]
    {
      widget->setGizmoArrowParameters(gizmoParameters.*field);
    });

    fromUI.addCallback([=]
    {
      gizmoParameters.*field = widget->gizmoArrowParameters();
    });
  }

  //################################################################################################
  void addGroupArrow(QVBoxLayout* l,
                     const QString& title,
                     std::vector<tp_maps::GizmoArrowParameters tp_maps::GizmoParameters::*> fields)
  {
    addTitle(l, title);

    auto widget = new EditGizmoArrowWidget(true);
    l->addWidget(widget);
    edited.connect(widget->edited);

    toUI.addCallback([=]
    {
      auto field = fields.front();
      widget->setGizmoArrowParameters(gizmoParameters.*field);
    });

    fromUI.addCallback([=]
    {
      for(auto field : fields)
        widget->updateGizmoArrowParameters(gizmoParameters.*field);
    });
  }

  //################################################################################################
  void addPlane(QVBoxLayout* l,
                const QString& title,
                tp_maps::GizmoPlaneParameters tp_maps::GizmoParameters::* field)
  {
    addTitle(l, title);

    auto widget = new EditGizmoPlaneWidget();
    l->addWidget(widget);
    edited.connect(widget->edited);

    toUI.addCallback([=]
    {
      widget->setGizmoPlaneParameters(gizmoParameters.*field);
    });

    fromUI.addCallback([=]
    {
      gizmoParameters.*field = widget->gizmoPlaneParameters();
    });
  }

  //################################################################################################
  void addGroupPlane(QVBoxLayout* l,
                     const QString& title,
                     std::vector<tp_maps::GizmoPlaneParameters tp_maps::GizmoParameters::*> fields)
  {
    addTitle(l, title);

    auto widget = new EditGizmoPlaneWidget(true);
    l->addWidget(widget);
    edited.connect(widget->edited);

    toUI.addCallback([=]
    {
      auto field = fields.front();
      widget->setGizmoPlaneParameters(gizmoParameters.*field);
    });

    fromUI.addCallback([=]
    {
      for(auto field : fields)
        widget->updateGizmoPlaneParameters(gizmoParameters.*field);
    });
  }
};

//##################################################################################################
EditGizmoWidget::EditGizmoWidget(const std::function<void(QVBoxLayout*)>& populateLoadAndSave,
                                 QWidget* parent):
  QWidget(parent),
  d(new Private(this))
{
  auto mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0,0,0,0);

  auto tabWidget = new QTabWidget();
  mainLayout->addWidget(tabWidget);

  auto addTab = [&](const QString& title)
  {
    auto scroll = new QScrollArea();
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll->setWidgetResizable(true);
    tabWidget->addTab(scroll, title);

    QWidget* scrollContents = new QWidget();
    scroll->setWidget(scrollContents);
    scrollContents->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);

    auto l = new QVBoxLayout(scrollContents);
    l->setContentsMargins(4,4,4,4);

    return l;
  };

  {
    auto l = addTab("Load && Save");

    {
      auto button = new QPushButton("Save");
      l->addWidget(button);
      connect(button, &QPushButton::clicked, this, [&]
      {
        QString dir = QSettings().value("EditGizmoWidget").toString();
        QString path = QFileDialog::getSaveFileName(this, "Save Gizmo Parameters", dir, "*.json");

        if(path.isEmpty())
          return;

        QSettings().setValue("EditGizmoWidget", QFileInfo(path).dir().absolutePath());

        nlohmann::json j;
        d->gizmoParameters.saveState(j);
        tp_utils::writeJSONFile(path.toStdString(), j, 2);
      });
    }

    {
      auto button = new QPushButton("Load");
      l->addWidget(button);
      connect(button, &QPushButton::clicked, this, [&]
      {
        QString dir = QSettings().value("EditGizmoWidget").toString();
        QString path = QFileDialog::getOpenFileName(this, "Load Gizmo Parameters", dir, "*.json");

        if(path.isEmpty())
          return;

        QSettings().setValue("EditGizmoWidget", QFileInfo(path).dir().absolutePath());

        d->gizmoParameters.loadState(tp_utils::readJSONFile(path.toStdString()));
        d->toUI();
        edited();
      });
    }

    if(populateLoadAndSave)
      populateLoadAndSave(l);

    l->addStretch();
  }

  {
    auto l = addTab("Common");

    {
      auto combo = new QComboBox();
      l->addWidget(new QLabel("Gizmo render pass"));
      l->addWidget(combo);

      connect(combo, &QComboBox::activated, this, [&]{edited();});

      combo->addItems(tp_qt_utils::convertStringList(tp_maps::gizmoRenderPasses()));

      d->toUI.addCallback([=]
      {
        combo->setCurrentText(QString::fromStdString(tp_maps::gizmoRenderPassToString(d->gizmoParameters.gizmoRenderPass)));
      });

      d->fromUI.addCallback([=]
      {
        d->gizmoParameters.gizmoRenderPass = tp_maps::gizmoRenderPassFromString(combo->currentText().toStdString());
      });
    }

    {
      auto combo = new QComboBox();
      l->addWidget(new QLabel("Reference lines render pass"));
      l->addWidget(combo);

      connect(combo, &QComboBox::activated, this, [&]{edited();});

      combo->addItems(tp_qt_utils::convertStringList(tp_maps::gizmoRenderPasses()));

      d->toUI.addCallback([=]
      {
        combo->setCurrentText(QString::fromStdString(tp_maps::gizmoRenderPassToString(d->gizmoParameters.referenceLinesRenderPass)));
      });

      d->fromUI.addCallback([=]
      {
        d->gizmoParameters.referenceLinesRenderPass = tp_maps::gizmoRenderPassFromString(combo->currentText().toStdString());
      });
    }

    {
      auto combo = new QComboBox();
      l->addWidget(new QLabel("Shader selection"));
      l->addWidget(combo);

      connect(combo, &QComboBox::activated, this, [&]{edited();});

      combo->addItems(tp_qt_utils::convertStringList(tp_maps::Geometry3DLayer::shaderSelections()));

      d->toUI.addCallback([=]
      {
        combo->setCurrentText(QString::fromStdString(tp_maps::Geometry3DLayer::shaderSelectionToString(d->gizmoParameters.shaderSelection)));
      });

      d->fromUI.addCallback([=]
      {
        d->gizmoParameters.shaderSelection = tp_maps::Geometry3DLayer::shaderSelectionFromString(combo->currentText().toStdString());
      });
    }

    {
      auto combo = new QComboBox();
      l->addWidget(new QLabel("Scale mode"));
      l->addWidget(combo);

      connect(combo, &QComboBox::activated, this, [&]{edited();});

      combo->addItems(tp_qt_utils::convertStringList(tp_maps::gizmoScaleModes()));

      d->toUI.addCallback([=]
      {
        combo->setCurrentText(QString::fromStdString(tp_maps::gizmoScaleModeToString(d->gizmoParameters.gizmoScaleMode)));
      });

      d->fromUI.addCallback([=]
      {
        d->gizmoParameters.gizmoScaleMode = tp_maps::gizmoScaleModeFromString(combo->currentText().toStdString());
      });
    }

    {
      auto spin = new QDoubleSpinBox();
      l->addWidget(new QLabel("Scale"));
      l->addWidget(spin);

      connect(spin, &QDoubleSpinBox::valueChanged, this, [&]{edited();});

      spin->setRange(0.1, 1000.0);
      spin->setSingleStep(0.01);

      d->toUI.addCallback([=]
      {
        spin->setValue(double(d->gizmoParameters.gizmoScale));
      });

      d->fromUI.addCallback([=]
      {
        d->gizmoParameters.gizmoScale = float(spin->value());
      });
    }

    {
      auto checkBox = new QCheckBox("only render selected axis");
      l->addWidget(checkBox);

      connect(checkBox, &QCheckBox::clicked, this, [&]{edited();});

      d->toUI.addCallback([=]
      {
        checkBox->setChecked(d->gizmoParameters.onlyRenderSelectedAxis);
      });

      d->fromUI.addCallback([=]
      {
        d->gizmoParameters.onlyRenderSelectedAxis = checkBox->isChecked();
      });
    }

    l->addStretch();
  }

  {
    auto l = addTab("Rotation");

    d->addRing(l, "Rotation ring X"     , &tp_maps::GizmoParameters::rotationX     );
    d->addRing(l, "Rotation ring Y"     , &tp_maps::GizmoParameters::rotationY     );
    d->addRing(l, "Rotation ring Z"     , &tp_maps::GizmoParameters::rotationZ     );
    d->addRing(l, "Rotation ring screen", &tp_maps::GizmoParameters::rotationScreen);

    d->addGroupRing(l, "Common rotation ring XYZ",
                    {
                      &tp_maps::GizmoParameters::rotationX,
                      &tp_maps::GizmoParameters::rotationY,
                      &tp_maps::GizmoParameters::rotationZ
                    });

    l->addStretch();
  }

  {
    auto l = addTab("Translation");

    d->addArrow(l, "Translation arrow X", &tp_maps::GizmoParameters::translationArrowX);
    d->addArrow(l, "Translation arrow Y", &tp_maps::GizmoParameters::translationArrowY);
    d->addArrow(l, "Translation arrow Z", &tp_maps::GizmoParameters::translationArrowZ);

    d->addPlane(l, "Translation plane X", &tp_maps::GizmoParameters::translationPlaneX);
    d->addPlane(l, "Translation plane Y", &tp_maps::GizmoParameters::translationPlaneY);
    d->addPlane(l, "Translation plane Z", &tp_maps::GizmoParameters::translationPlaneZ);

    d->addPlane(l, "Translation plane screen", &tp_maps::GizmoParameters::translationPlaneScreen);

    d->addGroupArrow(l, "Common translation arrow XYZ",
                     {
                       &tp_maps::GizmoParameters::translationArrowX,
                       &tp_maps::GizmoParameters::translationArrowY,
                       &tp_maps::GizmoParameters::translationArrowZ
                     });

    d->addGroupPlane(l, "Common translation plane XYZ",
                     {
                       &tp_maps::GizmoParameters::translationPlaneX,
                       &tp_maps::GizmoParameters::translationPlaneY,
                       &tp_maps::GizmoParameters::translationPlaneZ
                     });

    l->addStretch();
  }

  {
    auto l = addTab("Scale");

    d->addArrow(l, "Scale arrow X", &tp_maps::GizmoParameters::scaleArrowX);
    d->addArrow(l, "Scale arrow Y", &tp_maps::GizmoParameters::scaleArrowY);
    d->addArrow(l, "Scale arrow Z", &tp_maps::GizmoParameters::scaleArrowZ);

    d->addArrow(l, "Scale arrow screen", &tp_maps::GizmoParameters::scaleArrowScreen);

    d->addGroupArrow(l, "Common scale arrow XYZ",
                     {
                       &tp_maps::GizmoParameters::scaleArrowX,
                       &tp_maps::GizmoParameters::scaleArrowY,
                       &tp_maps::GizmoParameters::scaleArrowZ
                     });
    l->addStretch();
  }

  d->toUI();
}

//##################################################################################################
EditGizmoWidget::~EditGizmoWidget()
{
  delete d;
}

//##################################################################################################
void EditGizmoWidget::setGizmoParameters(const tp_maps::GizmoParameters& gizmoParameters)
{
  blockSignals(true);
  TP_CLEANUP([&]{blockSignals(false);});

  d->gizmoParameters = gizmoParameters;
  d->toUI();
}

//##################################################################################################
const tp_maps::GizmoParameters& EditGizmoWidget::gizmoParameters() const
{
  d->fromUI();
  return d->gizmoParameters;
}

//##################################################################################################
bool EditGizmoWidget::editGizmoDialog(QWidget* parent, tp_maps::GizmoParameters& gizmoParameters)
{
  QPointer<QDialog> dialog = new QDialog(parent);
  TP_CLEANUP([&]{delete dialog;});

  dialog->setWindowTitle("Edit Gizmo");

  auto l = new QVBoxLayout(dialog);

  auto widget = new EditGizmoWidget();
  l->addWidget(widget);
  widget->setGizmoParameters(gizmoParameters);

  auto buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  l->addWidget(buttons);

  connect(buttons, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, dialog, &QDialog::reject);

  if(dialog->exec() == QDialog::Accepted)
  {
    gizmoParameters = widget->gizmoParameters();
    return true;
  }

  return false;
}

}
