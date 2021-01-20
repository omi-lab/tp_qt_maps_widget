#include "tp_qt_maps_widget/EditMaterialWidget.h"
#include "tp_qt_maps_widget/SelectMaterialWidget.h"

#include "tp_qt_widgets/FileDialogLineEdit.h"

#include "tp_utils/JSONUtils.h"

#include <QDialog>
#include <QBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QRadioButton>
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

namespace
{

//##################################################################################################
int colorScaleToInt(float scale)
{
  float scaleMax = 4.0f;
  auto s = std::sqrt(scale/scaleMax);
  return int(s*100000.0f);
}

//##################################################################################################
float colorScaleFromInt(int scale)
{
  float scaleMax = 4.0f;
  auto s = float(scale) / 100000.0f;
  return s*s*scaleMax;
}
}

//##################################################################################################
struct EditMaterialWidget::Private
{
  tp_math_utils::Material material;
  std::function<std::vector<tp_utils::StringID>()> getExistingTextures;
  std::function<tp_utils::StringID(const std::string&)> loadTexture;

  QLineEdit* nameEdit{nullptr};

  QPushButton* albedoColorButton {nullptr};
  QPushButton* specularColorButton{nullptr};

  QSlider* albedoScaleSlider{nullptr};
  QSlider* specularScaleSlider{nullptr};

  QSlider* alpha    {nullptr};

  QSlider* roughness{nullptr};
  QSlider* metalness{nullptr};

  QSlider* useAmbient    {nullptr};
  QSlider* useDiffuse    {nullptr};
  QSlider* useNdotL      {nullptr};
  QSlider* useAttenuation{nullptr};
  QSlider* useShadow     {nullptr};
  QSlider* useLightMask  {nullptr};
  QSlider* useReflection {nullptr};

  QLineEdit*    albedoTexture{nullptr};  //!< mtl: map_Kd or map_Ka
  QLineEdit*  specularTexture{nullptr};  //!< mtl: map_Ks
  QLineEdit*     alphaTexture{nullptr};  //!< mtl: map_d
  QLineEdit*   normalsTexture{nullptr};  //!< mtl: map_Bump
  QLineEdit* roughnessTexture{nullptr};  //!<
  QLineEdit* metalnessTexture{nullptr};  //!<
  QLineEdit*        aoTexture{nullptr};  //!<

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

    albedoColorButton  ->setIcon(makeIcon(material.albedo  ));
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

      auto hLayout = new QHBoxLayout();
      hLayout->setContentsMargins(0,0,0,0);
      vLayout->addLayout(hLayout);

      auto spin = new QDoubleSpinBox();
      spin->setRange(0.0, 4.0);
      spin->setDecimals(3);
      hLayout->addWidget(spin);

      slider = new QSlider(Qt::Horizontal);
      slider->setRange(0, 100000);
      hLayout->addWidget(slider);
      connect(slider, &QSlider::valueChanged, this, &EditMaterialWidget::materialEdited);

      connect(slider, &QSlider::valueChanged, this, [=]
      {
        double v = colorScaleFromInt(slider->value());
        if(std::fabs(v-spin->value()) > 0.0001)
          spin->setValue(v);
      });

      connect(spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [=]
      {
        slider->setValue(colorScaleToInt(spin->value()));
      });


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

    d->albedoColorButton   = make("Albedo"  , [&]()->glm::vec3&{return d->material.albedo;}  , d->albedoScaleSlider  );
    d->specularColorButton = make("Specular", [&]()->glm::vec3&{return d->material.specular;}, d->specularScaleSlider);
  }

  auto gridLayout = new QGridLayout();
  gridLayout->setContentsMargins(0,0,0,0);
  l->addLayout(gridLayout);

  {
    int row = gridLayout->rowCount();
    gridLayout->addWidget(new QLabel("Alpha"), row, 0, Qt::AlignLeft);
    d->alpha = new QSlider(Qt::Horizontal);
    gridLayout->addWidget(d->alpha, row, 1);
    d->alpha->setRange(0, 255);
    d->alpha->setSingleStep(1);
    connect(d->alpha, &QSlider::valueChanged, this, &EditMaterialWidget::materialEdited);
  }

  auto addSlider = [&](const QString& name)
  {
    int row = gridLayout->rowCount();
    gridLayout->addWidget(new QLabel(name), row, 0, Qt::AlignLeft);
    auto s = new QSlider(Qt::Horizontal);
    gridLayout->addWidget(s, row, 1);
    s->setRange(0, 255000);
    s->setSingleStep(1);
    connect(s, &QSlider::valueChanged, this, &EditMaterialWidget::materialEdited);
    return s;
  };

  d->roughness      = addSlider("Roughness");
  d->metalness      = addSlider("Metalness");

  d->useAmbient     = addSlider("Use ambient");
  d->useDiffuse     = addSlider("Use diffuse");
  d->useNdotL       = addSlider("Use N dot L");
  d->useAttenuation = addSlider("Use attenuation");
  d->useShadow      = addSlider("Use shadow");
  d->useLightMask   = addSlider("Light mask");
  d->useReflection  = addSlider("Use reflection");

  auto addTextureEdit = [&](const auto& name)
  {
    int row = gridLayout->rowCount();
    gridLayout->addWidget(new QLabel(name), row, 0, Qt::AlignLeft);

    auto ll = new QHBoxLayout();
    gridLayout->addLayout(ll, row, 1);

    auto edit = new QLineEdit();
    ll->addWidget(edit);
    connect(edit, &QLineEdit::editingFinished, this, &EditMaterialWidget::materialEdited);

    auto button = new QPushButton("Load");
    ll->addWidget(button);
    connect(button, &QPushButton::clicked, this, [=]
    {
      QPointer<QDialog> dialog = new QDialog(this);
      TP_CLEANUP([&]{delete dialog;});

      auto l = new QVBoxLayout(dialog);

      auto existingRadio = new QRadioButton("Existing");
      l->addWidget(existingRadio);
      existingRadio->setChecked(true);
      auto existing = new QComboBox();
      l->addWidget(existing);

      if(d->getExistingTextures)
      {
        for(const auto& name : d->getExistingTextures())
          existing->addItem(QString::fromStdString(name.keyString()));
      }

      auto loadRadio = new QRadioButton("Load");
      l->addWidget(loadRadio);
      auto load = new tp_qt_widgets::FileDialogLineEdit();
      load->setMode(tp_qt_widgets::FileDialogLineEdit::OpenFileMode);
      load->setFilter("*.png *.jpg *.jpeg *.bmp");
      l->addWidget(load);
      load->setEnabled(false);

      connect(loadRadio, &QRadioButton::toggled, this, [=]
      {
        existing->setEnabled(existingRadio->isChecked());
        load->setEnabled(loadRadio->isChecked());
      });

      l->addStretch();
      auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
      l->addWidget(buttonBox);
      connect(buttonBox, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
      connect(buttonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);

      if(dialog->exec() == QDialog::Accepted)
      {
        if(existingRadio->isChecked() && !existing->currentText().isEmpty())
        {
          edit->setText(existing->currentText());
          emit materialEdited();
        }
        else if(loadRadio->isChecked() && d->loadTexture)
        {
          auto text = d->loadTexture(load->text().toStdString());
          if(text.isValid())
          {
            edit->setText(QString::fromStdString(text.keyString()));
            emit materialEdited();
          }
        }
      }
    });
    return edit;
  };

  d->   albedoTexture  = addTextureEdit("Albedo map"   ); //!< mtl: map_Kd or map_Ka
  d-> specularTexture  = addTextureEdit("Specular map" ); //!< mtl: map_Ks
  d->    alphaTexture  = addTextureEdit("Alpha map"    ); //!< mtl: map_d
  d->  normalsTexture  = addTextureEdit("Normals map"  ); //!< mtl: map_Bump
  d->roughnessTexture  = addTextureEdit("Roughness map"); //!<
  d->metalnessTexture  = addTextureEdit("Metalness map"); //!<
  d->       aoTexture  = addTextureEdit("AO map"       ); //!<

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
        tp_math_utils::Material material;
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
        tp_math_utils::Material mat = material();
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
void EditMaterialWidget::setMaterial(const tp_math_utils::Material& material)
{
  blockSignals(true);
  TP_CLEANUP([&]{blockSignals(false);});

  d->material = material;

  d->nameEdit->setText(QString::fromStdString(material.name.keyString()));

  d->updateColors();

  d->alpha    ->setValue(int(material.alpha    *255.0f));

  d->roughness     ->setValue(int(material.roughness      * 255000.0f));
  d->metalness     ->setValue(int(material.metalness      * 255000.0f));

  d->useAmbient    ->setValue(int(material.useAmbient     * 255000.0f));
  d->useDiffuse    ->setValue(int(material.useDiffuse     * 255000.0f));
  d->useNdotL      ->setValue(int(material.useNdotL       * 255000.0f));
  d->useAttenuation->setValue(int(material.useAttenuation * 255000.0f));
  d->useShadow     ->setValue(int(material.useShadow      * 255000.0f));
  d->useLightMask  ->setValue(int(material.useLightMask   * 255000.0f));
  d->useReflection ->setValue(int(material.useReflection  * 255000.0f));

  d->albedoScaleSlider  ->setValue(colorScaleToInt(material.albedoScale  ));
  d->specularScaleSlider->setValue(colorScaleToInt(material.specularScale));

  d->   albedoTexture ->setText(QString::fromStdString(d->material.   albedoTexture.keyString())); //!< mtl: map_Kd or map_Ka
  d-> specularTexture ->setText(QString::fromStdString(d->material. specularTexture.keyString())); //!< mtl: map_Ks
  d->    alphaTexture ->setText(QString::fromStdString(d->material.    alphaTexture.keyString())); //!< mtl: map_d
  d->  normalsTexture ->setText(QString::fromStdString(d->material.  normalsTexture.keyString())); //!< mtl: map_Bump
  d->roughnessTexture ->setText(QString::fromStdString(d->material.roughnessTexture.keyString())); //!<
  d->metalnessTexture ->setText(QString::fromStdString(d->material.metalnessTexture.keyString())); //!<
  d->       aoTexture ->setText(QString::fromStdString(d->material.       aoTexture.keyString())); //!<
}

//##################################################################################################
tp_math_utils::Material EditMaterialWidget::material() const
{
  d->material.name = d->nameEdit->text().toStdString();

  d->material.alpha     = float(d->alpha    ->value()) / 255.0f;

  d->material.roughness = float(d->roughness->value()) / 255000.0f;
  d->material.metalness = float(d->metalness->value()) / 255000.0f;

  d->material.useAmbient     = float(d->useAmbient    ->value()) / 255000.0f;
  d->material.useDiffuse     = float(d->useDiffuse    ->value()) / 255000.0f;
  d->material.useNdotL       = float(d->useNdotL      ->value()) / 255000.0f;
  d->material.useAttenuation = float(d->useAttenuation->value()) / 255000.0f;
  d->material.useShadow      = float(d->useShadow     ->value()) / 255000.0f;
  d->material.useLightMask   = float(d->useLightMask  ->value()) / 255000.0f;
  d->material.useReflection  = float(d->useReflection ->value()) / 255000.0f;

  d->material.albedoScale   = colorScaleFromInt(d->albedoScaleSlider->value());
  d->material.specularScale = colorScaleFromInt(d->specularScaleSlider->value());

  d->material.   albedoTexture = d->   albedoTexture->text().toStdString(); //!< mtl: map_Kd or map_Ka
  d->material. specularTexture = d-> specularTexture->text().toStdString(); //!< mtl: map_Ks
  d->material.    alphaTexture = d->    alphaTexture->text().toStdString(); //!< mtl: map_d
  d->material.  normalsTexture = d->  normalsTexture->text().toStdString(); //!< mtl: map_Bump
  d->material.roughnessTexture = d->roughnessTexture->text().toStdString(); //!<
  d->material.metalnessTexture = d->metalnessTexture->text().toStdString(); //!<
  d->material.       aoTexture = d->       aoTexture->text().toStdString(); //!<

  return d->material;
}

//##################################################################################################
void EditMaterialWidget::setGetExistingTextures(const std::function<std::vector<tp_utils::StringID>()>& getExistingTextures)
{
  d->getExistingTextures = getExistingTextures;
}

//##################################################################################################
void EditMaterialWidget::setLoadTexture(const std::function<tp_utils::StringID(const std::string&)>& loadTexture)
{
  d->loadTexture = loadTexture;
}

//##################################################################################################
bool EditMaterialWidget::editMaterialDialog(QWidget* parent,
                                            tp_math_utils::Material& material,
                                            const std::function<std::vector<tp_utils::StringID>()>& getExistingTextures,
                                            const std::function<tp_utils::StringID(const std::string&)>& loadTexture)
{
  QPointer<QDialog> dialog = new QDialog(parent);
  TP_CLEANUP([&]{delete dialog;});

  dialog->setWindowTitle("Edit Material");

  auto l = new QVBoxLayout(dialog);

  auto editMaterialWidget = new EditMaterialWidget();
  l->addWidget(editMaterialWidget);
  editMaterialWidget->setMaterial(material);
  editMaterialWidget->setGetExistingTextures(getExistingTextures);
  editMaterialWidget->setLoadTexture(loadTexture);

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
