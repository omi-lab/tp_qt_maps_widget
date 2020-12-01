#include "tp_qt_maps_widget/EditMaterialWidget.h"
#include "tp_qt_maps_widget/SelectMaterialWidget.h"

#include "tp_utils/JSONUtils.h"

#include <QDialog>
#include <QBoxLayout>
#include <QGridLayout>
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
#include <QGuiApplication>
#include <QClipboard>
#include <QPainter>

namespace tp_qt_maps_widget
{

//##################################################################################################
struct EditMaterialWidget::Private
{
  tp_maps::Material material;

  QLineEdit* nameEdit{nullptr};

  QPushButton* ambientColorButton {nullptr};
  QPushButton* diffuseColorButton {nullptr};
  QPushButton* specularColorButton{nullptr};

  QSlider* ambientScaleSlider{nullptr};
  QSlider* diffuseScaleSlider{nullptr};
  QSlider* specularScaleSlider{nullptr};

  QSlider* shininess{nullptr};
  QSlider* alpha    {nullptr};

  QSlider* roughness{nullptr};
  QSlider* metalness{nullptr};

  QSlider* useDiffuse    {nullptr};
  QSlider* useNdotL      {nullptr};
  QSlider* useAttenuation{nullptr};
  QSlider* useShadow     {nullptr};
  QSlider* useLightMask  {nullptr};
  QSlider* useReflection {nullptr};

  QLineEdit* ambientTexture {nullptr}; //!< mtl: map_Ka
  QLineEdit* diffuseTexture {nullptr}; //!< mtl: map_Kd
  QLineEdit* specularTexture{nullptr}; //!< mtl: map_Ks
  QLineEdit* alphaTexture   {nullptr}; //!< mtl: map_d
  QLineEdit* bumpTexture    {nullptr}; //!< mtl: map_Bump

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

    ambientColorButton ->setIcon(makeIcon(material.ambient ));
    diffuseColorButton ->setIcon(makeIcon(material.diffuse ));
    specularColorButton->setIcon(makeIcon(material.specular));
  }
};

//##################################################################################################
EditMaterialWidget::EditMaterialWidget(QWidget* parent):
  QWidget(parent),
  d(new Private())
{
  auto l = new QVBoxLayout(this);
  l->setContentsMargins(0,0,0,0);

  l->addWidget(new QLabel("Name"));
  d->nameEdit = new QLineEdit();
  l->addWidget(d->nameEdit);
  connect(d->nameEdit, &QLineEdit::editingFinished, this, &EditMaterialWidget::materialEdited);

  {
    l->addWidget(new QLabel("Colors"));

    auto ll = new QHBoxLayout();
    ll->setContentsMargins(0,0,0,0);
    l->addLayout(ll);

    auto make = [&](const QString& text, const std::function<glm::vec3&()>& getColor, QSlider*& slider)
    {
      auto vLayout = new QVBoxLayout();
      vLayout->setContentsMargins(0,0,0,0);
      ll->addLayout(vLayout);

      auto button = new QPushButton(text);
      button->setStyleSheet("text-align:left; padding-left:2;");
      vLayout->addWidget(button);

      slider = new QSlider(Qt::Horizontal);
      slider->setRange(0, 1000);
      vLayout->addWidget(slider);
      connect(slider, &QSlider::valueChanged, this, &EditMaterialWidget::materialEdited);

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
          emit materialEdited();
        }
      });

      return button;
    };

    d->ambientColorButton  = make("Ambient" , [&]()->glm::vec3&{return d->material.ambient;} , d->ambientScaleSlider );
    d->diffuseColorButton  = make("Diffuse" , [&]()->glm::vec3&{return d->material.diffuse;} , d->diffuseScaleSlider );
    d->specularColorButton = make("Specular", [&]()->glm::vec3&{return d->material.specular;}, d->specularScaleSlider);
  }

  {
    l->addWidget(new QLabel("Shininess and alpha"));
    d->shininess = new QSlider(Qt::Horizontal);
    l->addWidget(d->shininess);
    d->shininess->setRange(0, 12800);
    d->shininess->setSingleStep(1);
    connect(d->shininess, &QSlider::valueChanged, this, &EditMaterialWidget::materialEdited);

    d->alpha = new QSlider(Qt::Horizontal);
    l->addWidget(d->alpha);
    d->alpha->setRange(0, 255);
    d->alpha->setSingleStep(1);
    connect(d->alpha, &QSlider::valueChanged, this, &EditMaterialWidget::materialEdited);
  }

  auto sliderLayout = new QGridLayout();
  sliderLayout->setContentsMargins(0,0,0,0);
  l->addLayout(sliderLayout);

  auto addSlider = [&](const QString& name)
  {
    int row = sliderLayout->rowCount();
    sliderLayout->addWidget(new QLabel(name), row, 0, Qt::AlignLeft);
    auto s = new QSlider(Qt::Horizontal);
    sliderLayout->addWidget(s, row, 1);
    s->setRange(0, 255000);
    s->setSingleStep(1);
    connect(s, &QSlider::valueChanged, this, &EditMaterialWidget::materialEdited);
    return s;
  };

  d->roughness = addSlider("Roughness");
  d->metalness = addSlider("Metalness");

  sliderLayout->addWidget(new QLabel("Use..."), sliderLayout->rowCount(), 0, Qt::AlignLeft);

  d->useDiffuse     = addSlider("Use diffuse");
  d->useNdotL       = addSlider("Use N dot L");
  d->useAttenuation = addSlider("Use attenuation");
  d->useShadow      = addSlider("Use shadow");
  d->useLightMask   = addSlider("Light mask");
  d->useReflection  = addSlider("Use reflection");

  auto addTextureEdit = [&](const auto& name)
  {
    l->addWidget(new QLabel(name));
    auto edit = new QLineEdit();
    l->addWidget(edit);
    connect(edit, &QLineEdit::editingFinished, this, &EditMaterialWidget::materialEdited);
    return edit;
  };

  d->ambientTexture  = addTextureEdit("Ambient texture");
  d->diffuseTexture  = addTextureEdit("Diffuse texture");
  d->specularTexture = addTextureEdit("Specular texture");
  d->alphaTexture    = addTextureEdit("Alpha texture");
  d->bumpTexture     = addTextureEdit("Bump texture");

  {
    auto hLayout = new QHBoxLayout();
    hLayout->setContentsMargins(0,0,0,0);
    hLayout->addStretch();
    l->addLayout(hLayout);
    {
      auto button = new QPushButton("Copy");
      hLayout->addWidget(button);
      connect(button, &QPushButton::clicked, this, [=]
      {
        QGuiApplication::clipboard()->setText(QString::fromStdString(material().saveState().dump(2)));
      });
    }
    {
      auto button = new QPushButton("Paste");
      hLayout->addWidget(button);
      connect(button, &QPushButton::clicked, this, [=]
      {
        tp_maps::Material material;
        material.loadState(tp_utils::jsonFromString(QGuiApplication::clipboard()->text().toStdString()));
        setMaterial(material);
        emit materialEdited();
      });
    }
    {
      auto button = new QPushButton("Library");
      hLayout->addWidget(button);
      connect(button, &QPushButton::clicked, this, [=]
      {
        tp_maps::Material mat = material();
        if(SelectMaterialWidget::selectMaterialDialog(this, materialLibrary(), mat))
        {
          setMaterial(mat);
          emit materialEdited();
        }
      });
    }
  }
}

//##################################################################################################
EditMaterialWidget::~EditMaterialWidget()
{
  delete d;
}

//##################################################################################################
void EditMaterialWidget::setMaterial(const tp_maps::Material& material)
{
  blockSignals(true);
  TP_CLEANUP([&]{blockSignals(false);});

  d->material = material;

  d->nameEdit->setText(QString::fromStdString(material.name.keyString()));

  d->updateColors();

  d->shininess->setValue(int(material.shininess*100.0f));
  d->alpha    ->setValue(int(material.alpha    *255.0f));

  d->roughness     ->setValue(int(material.roughness      * 255000.0f));
  d->metalness     ->setValue(int(material.metalness      * 255000.0f));

  d->useDiffuse    ->setValue(int(material.useDiffuse     * 255000.0f));
  d->useNdotL      ->setValue(int(material.useNdotL       * 255000.0f));
  d->useAttenuation->setValue(int(material.useAttenuation * 255000.0f));
  d->useShadow     ->setValue(int(material.useShadow      * 255000.0f));
  d->useLightMask  ->setValue(int(material.useLightMask   * 255000.0f));
  d->useReflection ->setValue(int(material.useReflection  * 255000.0f));

  auto calculateScale = [](float scale)
  {
    float scaleMax = 4.0f;
    auto s = std::sqrt(scale/scaleMax);
    return int(s*1000.0f);
  };

  d->ambientScaleSlider->setValue(calculateScale(material.ambientScale));
  d->diffuseScaleSlider->setValue(calculateScale(material.diffuseScale));
  d->specularScaleSlider->setValue(calculateScale(material.specularScale));

  d->ambientTexture ->setText(QString::fromStdString(d->material.ambientTexture .keyString()));
  d->diffuseTexture ->setText(QString::fromStdString(d->material.diffuseTexture .keyString()));
  d->specularTexture->setText(QString::fromStdString(d->material.specularTexture.keyString()));
  d->alphaTexture   ->setText(QString::fromStdString(d->material.alphaTexture   .keyString()));
  d->bumpTexture    ->setText(QString::fromStdString(d->material.bumpTexture    .keyString()));
}

//##################################################################################################
tp_maps::Material EditMaterialWidget::material() const
{
  d->material.name = d->nameEdit->text().toStdString();

  d->material.shininess = float(d->shininess->value()) / 100.0f;
  d->material.alpha     = float(d->alpha    ->value()) / 255.0f;

  d->material.roughness = float(d->roughness->value()) / 255000.0f;
  d->material.metalness = float(d->metalness->value()) / 255000.0f;

  d->material.useDiffuse     = float(d->useDiffuse    ->value()) / 255000.0f;
  d->material.useNdotL       = float(d->useNdotL      ->value()) / 255000.0f;
  d->material.useAttenuation = float(d->useAttenuation->value()) / 255000.0f;
  d->material.useShadow      = float(d->useShadow     ->value()) / 255000.0f;
  d->material.useLightMask   = float(d->useLightMask  ->value()) / 255000.0f;
  d->material.useReflection  = float(d->useReflection ->value()) / 255000.0f;

  auto calculateScale = [](int scale)
  {
    float scaleMax = 4.0f;
    auto s = float(scale) / 1000.0f;
    return s*s*scaleMax;
  };

  d->material.ambientScale = calculateScale(d->ambientScaleSlider->value());
  d->material.diffuseScale = calculateScale(d->diffuseScaleSlider->value());
  d->material.specularScale = calculateScale(d->specularScaleSlider->value());

  d->material.ambientTexture  = d->ambientTexture ->text().toStdString();
  d->material.diffuseTexture  = d->diffuseTexture ->text().toStdString();
  d->material.specularTexture = d->specularTexture->text().toStdString();
  d->material.alphaTexture    = d->alphaTexture   ->text().toStdString();
  d->material.bumpTexture     = d->bumpTexture    ->text().toStdString();

  return d->material;
}

//##################################################################################################
bool EditMaterialWidget::editMaterialDialog(QWidget* parent, tp_maps::Material& material)
{
  QPointer<QDialog> dialog = new QDialog(parent);
  TP_CLEANUP([&]{delete dialog;});

  dialog->setWindowTitle("Edit Material");

  auto l = new QVBoxLayout(dialog);

  auto editMaterialWidget = new EditMaterialWidget();
  l->addWidget(editMaterialWidget);
  editMaterialWidget->setMaterial(material);

  auto buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  l->addWidget(buttons);

  connect(buttons, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, dialog, &QDialog::reject);

  if(dialog->exec() == QDialog::Accepted)
  {
    material = editMaterialWidget->material();
    return true;
  }

  return false;
}

}
