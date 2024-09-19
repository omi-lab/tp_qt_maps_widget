#include "tp_qt_maps_widget/EditLightWidget.h"
#include "tp_qt_maps_widget/EditLightSwapParametersWidget.h"

#include "tp_utils/JSONUtils.h"
#include "tp_utils/DebugUtils.h"

#include "glm/gtx/norm.hpp" // IWYU pragma: keep

#include <QDialog>
#include <QBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QPushButton>
#include <QPointer>
#include <QDialogButtonBox>
#include <QColorDialog>
#include <QImage>
#include <QPixmap>
#include <QLineEdit>
#include <QCheckBox>
#include <QPainter>
#include <QToolButton>
#include <QClipboard>
#include <QGuiApplication>

namespace tp_qt_maps_widget
{

//##################################################################################################
struct EditLightWidget::Private
{
  tp_math_utils::Light light;
  tp_math_utils::LightSwapParameters lightSwapParameters;

  QLineEdit* nameEdit{nullptr};

  QComboBox* typeCombo{nullptr};
  QLabel* animatedLightWarning{nullptr};
  QDoubleSpinBox* positionX{nullptr};
  QDoubleSpinBox* positionY{nullptr};
  QDoubleSpinBox* positionZ{nullptr};

  QDoubleSpinBox* directionX{nullptr};
  QDoubleSpinBox* directionY{nullptr};
  QDoubleSpinBox* directionZ{nullptr};

  QPushButton* ambientColorButton {nullptr};
  QPushButton* diffuseColorButton {nullptr};
  QPushButton* specularColorButton{nullptr};

  QSlider* diffuseScale{nullptr};

  QDoubleSpinBox* spotLightConstant {nullptr};
  QDoubleSpinBox* spotLightLinear   {nullptr};
  QDoubleSpinBox* spotLightQuadratic{nullptr};

  QDoubleSpinBox* spotLightBlend{nullptr};

  QDoubleSpinBox* near      {nullptr};
  QDoubleSpinBox* far       {nullptr};
  QDoubleSpinBox* fov       {nullptr};
  QDoubleSpinBox* orthoRadius{nullptr};

  QDoubleSpinBox* offsetScale{nullptr};

  QCheckBox* castShadows{nullptr};

  QCheckBox* isTemplateCheckbox{nullptr};
  tp_qt_maps_widget::EditLightSwapParametersWidget* editLightSwapParametersWidget{nullptr};

  //################################################################################################
  void updateColors()
  {
    auto makeIcon = [](const glm::vec3& c)
    {
      QImage image(24, 24, QImage::Format_ARGB32);
      image.fill(QColor(0,0,0,0));
      {
        QPainter p(&image);
        p.setBrush(QColor::fromRgbF(qreal(c.x), qreal(c.y), qreal(c.z)));
        p.setPen(Qt::black);
        p.drawRoundedRect(2,2,20,20,2.0, 2.0);
      }
      return QIcon(QPixmap::fromImage(image));
    };

    ambientColorButton ->setIcon(makeIcon(light.ambient ));
    diffuseColorButton ->setIcon(makeIcon(light.diffuse ));
    specularColorButton->setIcon(makeIcon(light.specular));
  }
};

//##################################################################################################
EditLightWidget::EditLightWidget(QWidget* parent):
  QWidget(parent),
  d(new Private())
{
  auto l = new QVBoxLayout(this);
  l->setContentsMargins(0,0,0,0);

  l->addWidget(new QLabel("Name"));
  d->nameEdit = new QLineEdit();
  l->addWidget(d->nameEdit);
  connect(d->nameEdit, &QLineEdit::editingFinished, this, &EditLightWidget::lightEdited);

  d->animatedLightWarning = new QLabel("Note: Animated lights cannot be edited");
  d->animatedLightWarning->setStyleSheet("QLabel { color : red; }");
  d->animatedLightWarning->setVisible(false);
  l->addWidget(d->animatedLightWarning);

  l->addWidget(new QLabel("Type"));
  d->typeCombo = new QComboBox();
  for(const auto& type : tp_math_utils::lightTypes())
    d->typeCombo->addItem(QString::fromStdString(type));
  l->addWidget(d->typeCombo);
  connect(d->typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EditLightWidget::lightEdited);

  auto copyPasteTitle = [&](auto title, auto copy, auto paste)
  {
    auto ll = new QHBoxLayout();
    ll->setContentsMargins(0,0,0,0);
    l->addLayout(ll);
    ll->addWidget(new QLabel(title));

    ll->addStretch();

    auto copyButton = new QToolButton();
    copyButton->setIcon(QIcon(":/tp_qt_icons_technical/copy.png"));
    copyButton->setFixedSize(18, 18);
    copyButton->setIconSize(QSize(12, 12));
    copyButton->setToolTip("Copy");
    ll->addWidget(copyButton);
    connect(copyButton, &QToolButton::clicked, this, copy);

    auto pasteButton = new QToolButton();
    pasteButton->setIcon(QIcon(":/tp_qt_icons_technical/paste.png"));
    pasteButton->setFixedSize(18, 18);
    pasteButton->setIconSize(QSize(12, 12));
    pasteButton->setToolTip("Paste");
    ll->addWidget(pasteButton);
    connect(pasteButton, &QToolButton::clicked, this, paste);
  };

  {
    copyPasteTitle("Position", [=]
    {
      QGuiApplication::clipboard()->setText(
            QString::fromStdString(nlohmann::json::array(
                                     {
                                       d->positionX->value(),
                                       d->positionY->value(),
                                       d->positionZ->value()
                                     }).dump(2)
                                   ));
    }, [=]
    {
      nlohmann::json j = tp_utils::jsonFromString(QGuiApplication::clipboard()->text().toStdString());

      if(j.is_array() &&
         j.size()==3 &&
         j.at(0).is_number() &&
         j.at(1).is_number() &&
         j.at(2).is_number())
      {
        d->positionX->blockSignals(true);
        d->positionY->blockSignals(true);
        d->positionZ->blockSignals(true);

        d->positionX->setValue(j.at(0).get<double>());
        d->positionY->setValue(j.at(1).get<double>());
        d->positionZ->setValue(j.at(2).get<double>());

        d->positionX->blockSignals(false);
        d->positionY->blockSignals(false);
        d->positionZ->blockSignals(false);

        emit lightEdited();
      }
    });

    auto ll = new QHBoxLayout();
    ll->setContentsMargins(0,0,0,0);
    l->addLayout(ll);

    auto make = [&]()
    {
      auto spin = new QDoubleSpinBox();
      spin->setRange(-10000.0, 10000.0);
      spin->setDecimals(3);
      spin->setSingleStep(0.001);
      ll->addWidget(spin);
      connect(spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EditLightWidget::lightEdited);
      return spin;
    };

    d->positionX = make();
    d->positionY = make();
    d->positionZ = make();
  }

  {
    copyPasteTitle("Direction", [=]
    {
      QGuiApplication::clipboard()->setText(
            QString::fromStdString(nlohmann::json::array(
                                     {
                                       d->directionX->value(),
                                       d->directionY->value(),
                                       d->directionZ->value()
                                     }).dump(2)
                                   ));
    }, [=]
    {
      nlohmann::json j = tp_utils::jsonFromString(QGuiApplication::clipboard()->text().toStdString());

      if(j.is_array() &&
         j.size()==3 &&
         j.at(0).is_number() &&
         j.at(1).is_number() &&
         j.at(2).is_number())
      {
        d->directionX->blockSignals(true);
        d->directionY->blockSignals(true);
        d->directionZ->blockSignals(true);

        glm::vec3 v{j.at(0).get<float>(), j.at(1).get<float>(), j.at(2).get<float>()};
        if(glm::length2(v) > 0.1f)
        {
          v = glm::normalize(v);

          d->directionX->setValue(double(v.x));
          d->directionY->setValue(double(v.y));
          d->directionZ->setValue(double(v.z));

          d->light.setDirection(v);
        }

        d->directionX->blockSignals(false);
        d->directionY->blockSignals(false);
        d->directionZ->blockSignals(false);

        emit lightEdited();
      }
    });

    auto ll = new QHBoxLayout();
    ll->setContentsMargins(0,0,0,0);
    l->addLayout(ll);

    auto make = [&]()
    {
      auto spin = new QDoubleSpinBox();
      spin->setReadOnly(true);
      spin->setRange(-1.0, 1.0);
      spin->setDecimals(3);
      spin->setSingleStep(0.001);
      ll->addWidget(spin);
      connect(spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EditLightWidget::lightEdited);
      return spin;
    };

    d->directionX = make();
    d->directionY = make();
    d->directionZ = make();
  }

  {
    l->addWidget(new QLabel("Colors"));

    auto ll = new QHBoxLayout();
    ll->setContentsMargins(0,0,0,0);
    l->addLayout(ll);

    auto make = [&](const QString& text, const std::function<glm::vec3&()>& getColor)
    {
      auto button = new QPushButton(text);
      button->setStyleSheet("text-align:left; padding-left:2;");
      ll->addWidget(button);

      connect(button, &QAbstractButton::clicked, this, [=]
      {
        glm::vec3& c = getColor();
        QColor color = QColorDialog::getColor(QColor::fromRgbF(qreal(c.x), qreal(c.y), qreal(c.z)), this, "Select " + text + " color", QColorDialog::DontUseNativeDialog);
        if(color.isValid())
        {
          c.x = float(color.redF());
          c.y = float(color.greenF());
          c.z = float(color.blueF());
          d->updateColors();
          Q_EMIT lightEdited();
        }
      });

      return button;
    };

    d->ambientColorButton  = make("Ambient" , [&]()->glm::vec3&{return d->light.ambient;});
    d->diffuseColorButton  = make("Diffuse" , [&]()->glm::vec3&{return d->light.diffuse;});
    d->specularColorButton = make("Specular", [&]()->glm::vec3&{return d->light.specular;});
  }

  {
    l->addWidget(new QLabel("Diffuse scale"));
    d->diffuseScale = new QSlider(Qt::Horizontal);
    l->addWidget(d->diffuseScale);
    d->diffuseScale->setRange(1, 10000);
    d->diffuseScale->setSingleStep(1);
    connect(d->diffuseScale, &QSlider::valueChanged, this, &EditLightWidget::lightEdited);
  }

  l->addWidget(new QLabel("Spot light constant, linear and quadratic attenuation coefficients"));

  auto ll = new QHBoxLayout();
  ll->setContentsMargins(0,0,0,0);
  l->addLayout(ll);

  d->spotLightConstant = new QDoubleSpinBox();
  ll->addWidget(d->spotLightConstant);
  d->spotLightConstant->setRange(0.0, 5.0);
  d->spotLightConstant->setDecimals(2);
  d->spotLightConstant->setSingleStep(0.01);
  connect(d->spotLightConstant, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EditLightWidget::lightEdited);

  d->spotLightLinear = new QDoubleSpinBox();
  ll->addWidget(d->spotLightLinear);
  d->spotLightLinear->setRange(0.0, 5.0);
  d->spotLightLinear->setDecimals(2);
  d->spotLightLinear->setSingleStep(0.01);
  connect(d->spotLightLinear, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EditLightWidget::lightEdited);

  d->spotLightQuadratic = new QDoubleSpinBox();
  ll->addWidget(d->spotLightQuadratic);
  d->spotLightQuadratic->setRange(0.0, 5.0);
  d->spotLightQuadratic->setDecimals(2);
  d->spotLightQuadratic->setSingleStep(0.01);
  connect(d->spotLightQuadratic, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EditLightWidget::lightEdited);

  // A fixed formula is used for light attenuation model.
  d->spotLightConstant->setDisabled(true);
  d->spotLightConstant->setToolTip("A fixed formula is used");
  d->spotLightLinear->setDisabled(true);
  d->spotLightLinear->setToolTip("A fixed formula is used");
  d->spotLightQuadratic->setDisabled(true);
  d->spotLightQuadratic->setToolTip("A fixed formula is used");

  {
    QHBoxLayout* ll{nullptr};

    auto addRow = [&](auto title)
    {
      l->addWidget(new QLabel(title));
      ll = new QHBoxLayout();
      ll->setContentsMargins(0,0,0,0);
      l->addLayout(ll);
    };

    auto make = [&]()
    {
      auto spin = new QDoubleSpinBox();
      spin->setRange(0.0, 1.0);
      spin->setDecimals(3);
      spin->setSingleStep(0.001);
      ll->addWidget(spin);
      connect(spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EditLightWidget::lightEdited);
      return spin;
    };

    addRow("Spot light blend");
    d->spotLightBlend = make();
  }

  {
    l->addWidget(new QLabel("Near and far plane"));

    auto ll = new QHBoxLayout();
    ll->setContentsMargins(0,0,0,0);
    l->addLayout(ll);

    d->near = new QDoubleSpinBox();
    ll->addWidget(d->near);
    d->near->setRange(0.1, 1000.0);
    d->near->setDecimals(1);
    d->near->setSingleStep(0.1);
    connect(d->near, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EditLightWidget::lightEdited);

    d->far = new QDoubleSpinBox();
    ll->addWidget(d->far);
    d->far->setRange(0.1, 10000.0);
    d->far->setDecimals(1);
    d->far->setSingleStep(0.1);
    connect(d->far, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EditLightWidget::lightEdited);
  }

  {
    l->addWidget(new QLabel("FOV and ortho radius"));

    auto ll = new QHBoxLayout();
    ll->setContentsMargins(0,0,0,0);
    l->addLayout(ll);

    d->fov = new QDoubleSpinBox();
    ll->addWidget(d->fov);
    d->fov->setRange(2.0, 140.0);
    d->fov->setDecimals(1);
    d->fov->setSingleStep(1.0);
    connect(d->fov, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EditLightWidget::lightEdited);

    d->orthoRadius = new QDoubleSpinBox();
    ll->addWidget(d->orthoRadius);
    d->orthoRadius->setRange(0.1, 1000.0);
    d->orthoRadius->setDecimals(1);
    d->orthoRadius->setSingleStep(0.1);
    connect(d->orthoRadius, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EditLightWidget::lightEdited);
  }

  {
    d->offsetScale = new QDoubleSpinBox();
    l->addWidget(new QLabel("Offset scale (Meters)"));
    l->addWidget(d->offsetScale);
    d->offsetScale->setRange(0.0002, 100.0);
    d->offsetScale->setDecimals(4);
    d->offsetScale->setSingleStep(0.01);
    connect(d->offsetScale, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EditLightWidget::lightEdited);
  }

  {
    d->castShadows = new QCheckBox("Cast shadows");
    l->addWidget(d->castShadows);
    connect(d->castShadows, &QCheckBox::clicked, this, &EditLightWidget::lightEdited);
  }

  d->isTemplateCheckbox  = new QCheckBox("Is template");
  d->isTemplateCheckbox->setChecked(false);
  l->addWidget(d->isTemplateCheckbox);

  connect(d->isTemplateCheckbox, &QCheckBox::toggled, this, [=]
  {
    Q_EMIT lightEdited();
  });

  d->editLightSwapParametersWidget = new tp_qt_maps_widget::EditLightSwapParametersWidget();
  connect(d->editLightSwapParametersWidget, &tp_qt_maps_widget::EditLightSwapParametersWidget::lightSwapParametersEdited, this, [=]
  {
    Q_EMIT lightEdited();
  });
  l->addWidget(d->editLightSwapParametersWidget);
}

//##################################################################################################
EditLightWidget::~EditLightWidget()
{
  delete d;
}

//##################################################################################################
void EditLightWidget::setLight(const tp_math_utils::Light& light, std::optional<tp_math_utils::LightSwapParameters> lightSwapParamsOpt)
{
  blockSignals(true);
  TP_CLEANUP([&]{blockSignals(false);});

  auto setValue=[](QDoubleSpinBox* s, float v)
  {
    if(std::fabs(float(s->value())-v)>0.0001f)
      s->setValue(double(v));
  };

  d->light = light;
  d->animatedLightWarning->setVisible(d->light.animation.isAnimated());
  d->typeCombo->setEnabled(!d->light.animation.isAnimated());
  d->positionX->setEnabled(!d->light.animation.isAnimated());
  d->positionY->setEnabled(!d->light.animation.isAnimated());
  d->positionZ->setEnabled(!d->light.animation.isAnimated());
  d->directionX->setEnabled(!d->light.animation.isAnimated());
  d->directionY->setEnabled(!d->light.animation.isAnimated());
  d->directionZ->setEnabled(!d->light.animation.isAnimated());
  d->ambientColorButton->setEnabled(!d->light.animation.isAnimated());
  d->diffuseColorButton->setEnabled(!d->light.animation.isAnimated());
  d->specularColorButton->setEnabled(!d->light.animation.isAnimated());
  d->diffuseScale->setEnabled(!d->light.animation.isAnimated());
  d->spotLightConstant->setEnabled(!d->light.animation.isAnimated());
  d->spotLightLinear->setEnabled(!d->light.animation.isAnimated());
  d->spotLightQuadratic->setEnabled(!d->light.animation.isAnimated());
  d->spotLightBlend->setEnabled(!d->light.animation.isAnimated());
  d->near->setEnabled(!d->light.animation.isAnimated());
  d->far->setEnabled(!d->light.animation.isAnimated());
  d->fov->setEnabled(!d->light.animation.isAnimated());
  d->orthoRadius->setEnabled(!d->light.animation.isAnimated());
  d->offsetScale->setEnabled(!d->light.animation.isAnimated());

  if(lightSwapParamsOpt)
    d->lightSwapParameters = (*lightSwapParamsOpt);

  d->nameEdit->setText(QString::fromStdString(light.name.toString()));

  d->typeCombo->setCurrentText(QString::fromStdString(tp_math_utils::lightTypeToString(light.type)));

  setValue(d->positionX, light.position().x);
  setValue(d->positionY, light.position().y);
  setValue(d->positionZ, light.position().z);

  setValue(d->directionX, light.direction().x);
  setValue(d->directionY, light.direction().y);
  setValue(d->directionZ, light.direction().z);

  d->updateColors();

  {
    float scaled = light.diffuseScale*10.0f;
    scaled*=float(d->diffuseScale->maximum());
    scaled = std::sqrt(scaled);
    d->diffuseScale    ->setValue(int(scaled));
  }

  setValue(d->spotLightConstant , light.constant );
  setValue(d->spotLightLinear   , light.linear   );
  setValue(d->spotLightQuadratic, light.quadratic);

  setValue(d->spotLightBlend, light.spotLightBlend);

  setValue(d->near       , light.near       );
  setValue(d->far        , light.far        );
  setValue(d->fov        , light.fov        );
  setValue(d->orthoRadius, light.orthoRadius);

  setValue(d->offsetScale, light.offsetScale.x);

  d->castShadows->setChecked(light.castShadows);
  d->isTemplateCheckbox->setChecked(lightSwapParamsOpt.has_value());
  d->editLightSwapParametersWidget->setVisible(d->isTemplateCheckbox->isChecked());
  if(lightSwapParamsOpt.has_value())
  {
    lightSwapParamsOpt->diffuseScaleDefault = light.diffuseScale;
    d->editLightSwapParametersWidget->setLightSwapParameters(lightSwapParamsOpt.value());
  }

}

//##################################################################################################
tp_math_utils::Light EditLightWidget::light() const
{
  d->light.name = d->nameEdit->text().toStdString();

  d->light.type = tp_math_utils::lightTypeFromString(d->typeCombo->currentText().toStdString());

  d->light.setPosition({d->positionX->value(), d->positionY->value(), d->positionZ->value()});

  {
    float scaled = float(d->diffuseScale->value());
    scaled*=scaled;
    scaled/=float(d->diffuseScale->maximum());
    scaled/=10.0f;
    d->light.diffuseScale     = scaled;
  }

  d->light.constant  = float(d->spotLightConstant ->value());
  d->light.linear    = float(d->spotLightLinear   ->value());
  d->light.quadratic = float(d->spotLightQuadratic->value());

  d->light.spotLightBlend = float(d->spotLightBlend->value());

  d->light.near        = float(d->near->value());
  d->light.far         = float(d->far->value());
  d->light.fov         = float(d->fov->value());
  d->light.orthoRadius = float(d->orthoRadius->value());

  {
    float s = float(d->offsetScale->value());
    d->light.offsetScale = {s,s,s};
  }

  d->light.castShadows = d->castShadows->isChecked();

  return d->light;
}

//################################################################################################
bool EditLightWidget::isTemplate() const
{
  return d->isTemplateCheckbox->isChecked();
}


//################################################################################################
std::optional<tp_math_utils::LightSwapParameters> EditLightWidget::lightSwapParameters() const
{
  if(isTemplate())
    return d->editLightSwapParametersWidget->lightSwapParameters();

  return std::nullopt;
}

//##################################################################################################
bool EditLightWidget::editLightDialog(QWidget* parent, tp_math_utils::Light& light, std::optional<tp_math_utils::LightSwapParameters> lightSwapParamsOpt)
{
  QPointer<QDialog> dialog = new QDialog(parent);
  TP_CLEANUP([&]{delete dialog;});

  dialog->setWindowTitle("Edit Light");

  auto l = new QVBoxLayout(dialog);

  auto editLightWidget = new EditLightWidget();
  l->addWidget(editLightWidget);
  editLightWidget->setLight(light, lightSwapParamsOpt);

  auto buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  l->addWidget(buttons);

  connect(buttons, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, dialog, &QDialog::reject);

  if(dialog->exec() == QDialog::Accepted)
  {
    light = editLightWidget->light();
    return true;
  }

  return false;
}

}
