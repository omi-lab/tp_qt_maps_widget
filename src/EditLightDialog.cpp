#include "tp_qt_maps_widget/EditLightDialog.h"

#include <QDialog>
#include <QBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QPointer>
#include <QDialogButtonBox>
#include <QColorDialog>
#include <QImage>
#include <QPixmap>
#include <QPainter>

namespace tp_qt_maps_widget
{

//##################################################################################################
struct EditLightDialog::Private
{
  tp_maps::Light light;

  QComboBox* typeCombo{nullptr};

  QDoubleSpinBox* positionX{nullptr};
  QDoubleSpinBox* positionY{nullptr};
  QDoubleSpinBox* positionZ{nullptr};

  QDoubleSpinBox* directionX{nullptr};
  QDoubleSpinBox* directionY{nullptr};
  QDoubleSpinBox* directionZ{nullptr};

  QPushButton* ambientColorButton {nullptr};
  QPushButton* diffuseColorButton {nullptr};
  QPushButton* specularColorButton{nullptr};

  QDoubleSpinBox* diffuseScale    {nullptr};
  QDoubleSpinBox* diffuseTranslate{nullptr};

  QDoubleSpinBox* spotLightConstant {nullptr};
  QDoubleSpinBox* spotLightLinear   {nullptr};
  QDoubleSpinBox* spotLightQuadratic{nullptr};

  //################################################################################################
  void updateColors()
  {
    auto makeIcon = [](const glm::vec3& c)
    {
      QImage image(24, 24, QImage::Format_ARGB32);
      image.fill(QColor(0,0,0,0));
      {
        QPainter p(&image);
        p.setBrush(QColor::fromRgbF(c.x, c.y, c.z));
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
EditLightDialog::EditLightDialog(QWidget* parent):
  QWidget(parent),
  d(new Private())
{
  auto l = new QVBoxLayout(this);
  l->setContentsMargins(0,0,0,0);

  l->addWidget(new QLabel("Type"));
  d->typeCombo = new QComboBox();
  for(const auto& type : tp_maps::lightTypes())
    d->typeCombo->addItem(QString::fromStdString(type));
  l->addWidget(d->typeCombo);

  {
    l->addWidget(new QLabel("Position"));
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
      return spin;
    };

    d->positionX = make();
    d->positionY = make();
    d->positionZ = make();
  }

  {
    l->addWidget(new QLabel("Direction"));
    auto ll = new QHBoxLayout();
    ll->setContentsMargins(0,0,0,0);
    l->addLayout(ll);

    auto make = [&]()
    {
      auto spin = new QDoubleSpinBox();
      spin->setRange(-1.0, 1.0);
      spin->setDecimals(3);
      spin->setSingleStep(0.001);
      ll->addWidget(spin);
      return spin;
    };

    d->directionX = make();
    d->directionY = make();
    d->directionZ = make();
  }

  {
    l->addWidget(new QLabel("Colors"));

    auto make = [&](const QString& text, const std::function<glm::vec3&()>& getColor)
    {
      auto button = new QPushButton(text);
      button->setStyleSheet("text-align:left; padding-left:2;");
      l->addWidget(button);

      connect(button, &QAbstractButton::clicked, [=]
      {
        glm::vec3& c = getColor();
        QColor color = QColorDialog::getColor(QColor::fromRgbF(c.x, c.y, c.z), this, "Select " + text + " color", QColorDialog::DontUseNativeDialog);
        if(color.isValid())
        {
          c.x = color.redF();
          c.y = color.greenF();
          c.z = color.blueF();
          d->updateColors();
        }
      });

      return button;
    };

    d->ambientColorButton  = make("Ambient" , [&]()->glm::vec3&{return d->light.ambient;});
    d->diffuseColorButton  = make("Diffuse" , [&]()->glm::vec3&{return d->light.diffuse;});
    d->specularColorButton = make("Specular", [&]()->glm::vec3&{return d->light.specular;});
  }

  l->addWidget(new QLabel("Diffuse scale"));
  d->diffuseScale = new QDoubleSpinBox();
  l->addWidget(d->diffuseScale);
  d->diffuseScale->setRange(0.01, 5.0);
  d->diffuseScale->setDecimals(2);
  d->diffuseScale->setSingleStep(0.01);

  l->addWidget(new QLabel("Diffuse translate"));
  d->diffuseTranslate = new QDoubleSpinBox();
  l->addWidget(d->diffuseTranslate);
  d->diffuseTranslate->setRange(-1.0, 1.0);
  d->diffuseTranslate->setDecimals(2);
  d->diffuseTranslate->setSingleStep(0.01);


  QDoubleSpinBox* spotLightConstant {nullptr};
  QDoubleSpinBox* spotLightLinear   {nullptr};
  QDoubleSpinBox* spotLightQuadratic{nullptr};


  l->addWidget(new QLabel("Spot light constant"));
  d->spotLightConstant = new QDoubleSpinBox();
  l->addWidget(d->spotLightConstant);
  d->spotLightConstant->setRange(0.01, 5.0);
  d->spotLightConstant->setDecimals(2);
  d->spotLightConstant->setSingleStep(0.01);

  l->addWidget(new QLabel("Spot light linear"));
  d->spotLightLinear = new QDoubleSpinBox();
  l->addWidget(d->spotLightLinear);
  d->spotLightLinear->setRange(0.01, 5.0);
  d->spotLightLinear->setDecimals(2);
  d->spotLightLinear->setSingleStep(0.01);

  l->addWidget(new QLabel("Spot light quadratic"));
  d->spotLightQuadratic = new QDoubleSpinBox();
  l->addWidget(d->spotLightQuadratic);
  d->spotLightQuadratic->setRange(0.01, 5.0);
  d->spotLightQuadratic->setDecimals(2);
  d->spotLightQuadratic->setSingleStep(0.01);
}

//##################################################################################################
EditLightDialog::~EditLightDialog()
{
  delete d;
}

//##################################################################################################
void EditLightDialog::setLight(const tp_maps::Light& light)
{
  d->light = light;
  d->typeCombo->setCurrentText(QString::fromStdString(tp_maps::lightTypeToString(light.type)));

  d->positionX->setValue(light.position.x);
  d->positionY->setValue(light.position.y);
  d->positionZ->setValue(light.position.z);

  d->directionX->setValue(light.direction.x);
  d->directionY->setValue(light.direction.y);
  d->directionZ->setValue(light.direction.z);

  d->updateColors();

  d->diffuseScale    ->setValue(light.diffuseScale);
  d->diffuseTranslate->setValue(light.diffuseTranslate);

  d->spotLightConstant ->setValue(light.constant);
  d->spotLightLinear   ->setValue(light.linear);
  d->spotLightQuadratic->setValue(light.quadratic);
}

//##################################################################################################
tp_maps::Light EditLightDialog::light() const
{
  d->light.type = tp_maps::lightTypeFromString(d->typeCombo->currentText().toStdString());

  d->light.position.x = d->positionX->value();
  d->light.position.y = d->positionY->value();
  d->light.position.z = d->positionZ->value();

  d->light.direction.x = d->directionX->value();
  d->light.direction.y = d->directionY->value();
  d->light.direction.z = d->directionZ->value();
  d->light.direction = glm::normalize(d->light.direction);

  d->light.diffuseScale     = d->diffuseScale    ->value();
  d->light.diffuseTranslate = d->diffuseTranslate->value();

  d->light.constant  = d->spotLightConstant ->value();
  d->light.linear    = d->spotLightLinear   ->value();
  d->light.quadratic = d->spotLightQuadratic->value();

  return d->light;
}

//##################################################################################################
bool EditLightDialog::editLight(QWidget* parent, tp_maps::Light& light)
{
  QPointer<QDialog> dialog = new QDialog(parent);
  TP_CLEANUP([&]{delete dialog;});

  dialog->setWindowTitle("Edit Light");

  auto l = new QVBoxLayout(dialog);

  auto editLightDialog = new EditLightDialog();
  l->addWidget(editLightDialog);
  editLightDialog->setLight(light);

  auto buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  l->addWidget(buttons);

  connect(buttons, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, dialog, &QDialog::reject);

  if(dialog->exec() == QDialog::Accepted)
  {
    light = editLightDialog->light();
    return true;
  }

  return false;
}

}
