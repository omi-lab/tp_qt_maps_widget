#include "tp_qt_maps_widget/EditLightWidget.h"

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
#include <QPainter>

namespace tp_qt_maps_widget
{

//##################################################################################################
struct EditLightWidget::Private
{
  tp_maps::Light light;

  QLineEdit* nameEdit{nullptr};

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

  QSlider* diffuseScale{nullptr};

  QDoubleSpinBox* spotLightConstant {nullptr};
  QDoubleSpinBox* spotLightLinear   {nullptr};
  QDoubleSpinBox* spotLightQuadratic{nullptr};


  QDoubleSpinBox* spotLightUVX{nullptr};
  QDoubleSpinBox* spotLightUVY{nullptr};

  QDoubleSpinBox* spotLightWHX{nullptr};
  QDoubleSpinBox* spotLightWHY{nullptr};

  QDoubleSpinBox* near      {nullptr};
  QDoubleSpinBox* far       {nullptr};
  QDoubleSpinBox* fov       {nullptr};
  QDoubleSpinBox* orthoRadius{nullptr};

  QSlider* offsetScale{nullptr};

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


  l->addWidget(new QLabel("Type"));
  d->typeCombo = new QComboBox();
  for(const auto& type : tp_maps::lightTypes())
    d->typeCombo->addItem(QString::fromStdString(type));
  l->addWidget(d->typeCombo);
  connect(d->typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EditLightWidget::lightEdited);

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
      connect(spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EditLightWidget::lightEdited);
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
        QColor color = QColorDialog::getColor(QColor::fromRgbF(c.x, c.y, c.z), this, "Select " + text + " color", QColorDialog::DontUseNativeDialog);
        if(color.isValid())
        {
          c.x = color.redF();
          c.y = color.greenF();
          c.z = color.blueF();
          d->updateColors();
          emit lightEdited();
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

  l->addWidget(new QLabel("Spot light constant, linear, and quadratic"));

  auto ll = new QHBoxLayout();
  ll->setContentsMargins(0,0,0,0);
  l->addLayout(ll);

  d->spotLightConstant = new QDoubleSpinBox();
  ll->addWidget(d->spotLightConstant);
  d->spotLightConstant->setRange(0.01, 5.0);
  d->spotLightConstant->setDecimals(2);
  d->spotLightConstant->setSingleStep(0.01);
  connect(d->spotLightConstant, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EditLightWidget::lightEdited);

  d->spotLightLinear = new QDoubleSpinBox();
  ll->addWidget(d->spotLightLinear);
  d->spotLightLinear->setRange(0.01, 5.0);
  d->spotLightLinear->setDecimals(2);
  d->spotLightLinear->setSingleStep(0.01);
  connect(d->spotLightLinear, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EditLightWidget::lightEdited);

  d->spotLightQuadratic = new QDoubleSpinBox();
  ll->addWidget(d->spotLightQuadratic);
  d->spotLightQuadratic->setRange(0.01, 5.0);
  d->spotLightQuadratic->setDecimals(2);
  d->spotLightQuadratic->setSingleStep(0.01);
  connect(d->spotLightQuadratic, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EditLightWidget::lightEdited);


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

    addRow("Spot light UV");
    d->spotLightUVX = make();
    d->spotLightUVY = make();

    addRow("Spot light WH");
    d->spotLightWHX = make();
    d->spotLightWHY = make();
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
    l->addWidget(new QLabel("Offset scale"));
    d->offsetScale = new QSlider(Qt::Horizontal);
    l->addWidget(d->offsetScale);
    d->offsetScale->setRange(1, 10000);
    d->offsetScale->setSingleStep(1);
    connect(d->offsetScale, &QSlider::valueChanged, this, &EditLightWidget::lightEdited);
  }
}

//##################################################################################################
EditLightWidget::~EditLightWidget()
{
  delete d;
}

//##################################################################################################
void EditLightWidget::setLight(const tp_maps::Light& light)
{
  blockSignals(true);
  TP_CLEANUP([&]{blockSignals(false);});

  d->light = light;

  d->nameEdit->setText(QString::fromStdString(light.name.keyString()));

  d->typeCombo->setCurrentText(QString::fromStdString(tp_maps::lightTypeToString(light.type)));

  d->positionX->setValue(light.position().x);
  d->positionY->setValue(light.position().y);
  d->positionZ->setValue(light.position().z);

  d->directionX->setValue(light.direction().x);
  d->directionY->setValue(light.direction().y);
  d->directionZ->setValue(light.direction().z);

  d->updateColors();

  {
    float scaled = light.diffuseScale*100.0f;
    scaled*=float(d->diffuseScale->maximum());
    scaled = std::sqrt(scaled);
    d->diffuseScale    ->setValue(int(scaled));
  }

  d->spotLightConstant ->setValue(light.constant);
  d->spotLightLinear   ->setValue(light.linear);
  d->spotLightQuadratic->setValue(light.quadratic);

  d->spotLightUVX->setValue(light.spotLightUV.x);
  d->spotLightUVY->setValue(light.spotLightUV.y);

  d->spotLightWHX->setValue(light.spotLightWH.x);
  d->spotLightWHY->setValue(light.spotLightWH.y);

  d->near->setValue(light.near);
  d->far->setValue(light.far);
  d->fov->setValue(light.fov);
  d->orthoRadius->setValue(light.orthoRadius);

  {
    float scaled = light.offsetScale.x*5000.0f;
    d->offsetScale->setValue(int(scaled));
  }
}

//##################################################################################################
tp_maps::Light EditLightWidget::light() const
{
  d->light.name = d->nameEdit->text().toStdString();

  d->light.type = tp_maps::lightTypeFromString(d->typeCombo->currentText().toStdString());

  d->light.setPosition({d->positionX->value(), d->positionY->value(), d->positionZ->value()});

  //d->light.direction.x = d->directionX->value();
  //d->light.direction.y = d->directionY->value();
  //d->light.direction.z = d->directionZ->value();
  //d->light.direction = glm::normalize(d->light.direction);

  {
    float scaled = float(d->diffuseScale->value());
    scaled*=scaled;
    scaled/=float(d->diffuseScale->maximum());
    scaled/=100.0f;
    d->light.diffuseScale     = scaled;
  }

  d->light.constant  = d->spotLightConstant ->value();
  d->light.linear    = d->spotLightLinear   ->value();
  d->light.quadratic = d->spotLightQuadratic->value();

  d->light.spotLightUV.x = d->spotLightUVX->value();
  d->light.spotLightUV.y = d->spotLightUVY->value();

  d->light.spotLightWH.x = d->spotLightWHX->value();
  d->light.spotLightWH.y = d->spotLightWHY->value();

  d->light.near        = d->near->value();
  d->light.far         = d->far->value();
  d->light.fov         = d->fov->value();
  d->light.orthoRadius = d->orthoRadius->value();

  {
    float s = float(d->offsetScale->value());
    s/=5000.0f;
    d->light.offsetScale = {s,s,s};
  }

  return d->light;
}

//##################################################################################################
bool EditLightWidget::editLightDialog(QWidget* parent, tp_maps::Light& light)
{
  QPointer<QDialog> dialog = new QDialog(parent);
  TP_CLEANUP([&]{delete dialog;});

  dialog->setWindowTitle("Edit Light");

  auto l = new QVBoxLayout(dialog);

  auto editLightWidget = new EditLightWidget();
  l->addWidget(editLightWidget);
  editLightWidget->setLight(light);

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
