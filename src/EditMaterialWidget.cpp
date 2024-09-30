#include "tp_qt_maps_widget/EditMaterialWidget.h"

#include "tp_qt_widgets/FileDialogLineEdit.h"

#include "tp_image_utils/LoadImages.h"

#include "tp_math_utils/materials/OpenGLMaterial.h"
#include "tp_math_utils/materials/LegacyMaterial.h"
#include "tp_math_utils/materials/ExternalMaterial.h"

#include "tp_utils/JSONUtils.h"

#include <QDialog>
#include <QBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
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
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QScrollArea>
#include <QScrollBar>
#include <QMessageBox>

namespace tp_qt_maps_widget
{

namespace
{
//##################################################################################################
struct FloatEditor
{
  std::function<float()> get;
  std::function<void(float)> set;
};

//##################################################################################################
struct BoolEditor
{
  std::function<bool()> get;
  std::function<void(bool)> set;
};
}

//##################################################################################################
struct EditMaterialWidget::Private
{
  tp_math_utils::Material material;
  TextureSupported textureSupported;
  TPGetExistingTexturesCallback getExistingTextures;
  TPLoadTextureCallback loadTexture;
  TPLoadMaterialBlendCallback loadMaterialBlend;

  QLineEdit* nameEdit{nullptr};

  QComboBox* shaderType{nullptr};

  QScrollArea* scroll{nullptr};
  QWidget* scrollContents{nullptr};

  QPushButton* albedoColorButton  {nullptr};
  QPushButton* sssColorButton     {nullptr};
  QPushButton* emissionColorButton{nullptr};
  QPushButton* velvetColorButton  {nullptr};

  FloatEditor albedoScaleSlider;
  FloatEditor sssSlider;
  FloatEditor emissionSlider;
  FloatEditor velvetSlider;

  FloatEditor alpha;
  FloatEditor roughness;
  FloatEditor metalness;
  FloatEditor specular;
  FloatEditor transmission;
  FloatEditor transmissionRoughness;
  FloatEditor sheen;
  FloatEditor sheenTint;
  FloatEditor clearCoat;
  FloatEditor clearCoatRoughness;
  FloatEditor iridescentFactor;
  FloatEditor iridescentOffset;
  FloatEditor iridescentFrequency;
  FloatEditor heightScale;
  FloatEditor heightMidlevel;
  FloatEditor ior;
  FloatEditor normalStrength;
  QDoubleSpinBox* sssRadiusR{nullptr};
  QDoubleSpinBox* sssRadiusG{nullptr};
  QDoubleSpinBox* sssRadiusB{nullptr};

  QComboBox* sssMethod{nullptr};

  FloatEditor albedoBrightness;
  FloatEditor albedoContrast;
  FloatEditor albedoGamma;
  FloatEditor albedoHue;
  FloatEditor albedoSaturation;
  FloatEditor albedoValue;
  FloatEditor albedoFactor;

  FloatEditor skewU;
  FloatEditor skewV;

  FloatEditor scaleU;
  FloatEditor scaleV;

  FloatEditor translateU;
  FloatEditor translateV;

  FloatEditor rotateUV;

  FloatEditor useAmbient;
  FloatEditor useDiffuse;
  FloatEditor useNdotL;
  FloatEditor useAttenuation;
  FloatEditor useShadow;
  FloatEditor useLightMask;
  FloatEditor useReflection;

  BoolEditor rayVisibilityCamera       ;
  BoolEditor rayVisibilityDiffuse      ;
  BoolEditor rayVisibilityGlossy       ;
  BoolEditor rayVisibilityTransmission ;
  BoolEditor rayVisibilityScatter      ;
  BoolEditor rayVisibilityShadow       ;
  BoolEditor rayVisibilityShadowCatcher;

  std::map<std::string, QLineEdit*> textureLineEdits;

  QLineEdit* blendFileLineEdit{nullptr};

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

    auto openGLMaterial = material.findOrAddOpenGL();
    auto legacyMaterial = material.findOrAddLegacy();

    albedoColorButton  ->setIcon(makeIcon(openGLMaterial->albedo  ));
    sssColorButton     ->setIcon(makeIcon(legacyMaterial->sss     ));
    emissionColorButton->setIcon(makeIcon(legacyMaterial->emission));
    velvetColorButton  ->setIcon(makeIcon(legacyMaterial->velvet  ));
  }
};

//##################################################################################################
EditMaterialWidget::EditMaterialWidget(TextureSupported textureSupported,
                                       const std::function<void(QLayout*)>& addButtons, QWidget* parent):
  QWidget(parent),
  d(new Private())
{
  d->textureSupported = textureSupported;

  auto mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0,0,0,0);

  {
    auto ll = new QHBoxLayout();
    mainLayout->addLayout(ll);
    ll->setContentsMargins(0,0,0,0);
    ll->addWidget(new QLabel("Name"));
    d->nameEdit = new QLineEdit();
    ll->addWidget(d->nameEdit);
    connect(d->nameEdit, &QLineEdit::editingFinished, this, &EditMaterialWidget::materialEdited);
  }

  d->scroll = new QScrollArea();
  d->scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  d->scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  d->scroll->setWidgetResizable(true);
  mainLayout->addWidget(d->scroll);

  d->scrollContents = new QWidget();
  d->scroll->setWidget(d->scrollContents);
  d->scrollContents->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
  d->scrollContents->installEventFilter(this);

  auto l = new QVBoxLayout(d->scrollContents);
  l->setContentsMargins(4,4,4,4);

  auto gridLayout = new QGridLayout();
  gridLayout->setContentsMargins(0,0,0,0);
  l->addLayout(gridLayout);

  //------------------------------------------------------------------------------------------------
  auto makeFloatEditor = [&](float min, float max, int row, const bool linear)
  {
    float scale = max - min;

    FloatEditor floatEditor;

    auto hLayout = new QHBoxLayout();
    hLayout->setContentsMargins(0,0,0,0);
    gridLayout->addLayout(hLayout, row, 1);

    auto spin = new QDoubleSpinBox();
    spin->setRange(double(min), double(max));
    spin->setDecimals(3);
    spin->setSingleStep(0.01);
    hLayout->addWidget(spin, 1);

    auto slider = new QSlider(Qt::Horizontal);
    slider->setRange(0, 100000);
    hLayout->addWidget(slider, 3);

    connect(slider, &QSlider::valueChanged, this, [=]
    {
      float v = float(slider->value()) / 100000.0f;
      v = linear?((v*scale) + min):(v*v*scale);
      if(std::fabs(v-float(spin->value())) > 0.000001f)
      {
        spin->blockSignals(true);
        spin->setValue(double(v));
        spin->blockSignals(false);
        Q_EMIT materialEdited();
      }
    });

    auto spinValueChanged = [=]
    {
      if(slider->isSliderDown())
        return;

      auto s = (float(spin->value())-min)/scale;
      int newSliderValue = int((linear?s:std::sqrt(s))*100000.0f);
      if(newSliderValue != slider->value())
      {
        slider->blockSignals(true);
        slider->setValue(newSliderValue);
        slider->blockSignals(false);
        Q_EMIT materialEdited();
      }
    };

    connect(spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, spinValueChanged);

    floatEditor.get = [=]{return spin->value();};
    floatEditor.set = [=](float v)
    {
      spin->setValue(double(v));
      spinValueChanged();
    };

    return floatEditor;
  };

  //------------------------------------------------------------------------------------------------
  auto makeColorEdit = [&](const QString& text, const std::function<glm::vec3&()>& getColor, FloatEditor& slider, float scaleMax, const bool linear)
  {
    int row = gridLayout->rowCount();

    auto button = new QPushButton(text);
    button->setStyleSheet("text-align:left; padding-left:2;");
    gridLayout->addWidget(button, row, 0);

    slider = makeFloatEditor(0.0f, scaleMax, row, linear);

    connect(button, &QAbstractButton::clicked, this, [=]
    {
      glm::vec3& c = getColor();
      QColor color = QColorDialog::getColor(QColor::fromRgbF(qreal(c.x), qreal(c.y), qreal(c.z)), this, "Select " + text + " color", QColorDialog::DontUseNativeDialog);
      if(color.isValid())
      {
        c.x = color.redF();
        c.y = color.greenF();
        c.z = color.blueF();
        d->updateColors();
        Q_EMIT materialEdited();
      }
    });

    return button;
  };

  //------------------------------------------------------------------------------------------------
  auto makeFloatEditorRow = [&](const QString& name, float min, float scaleMax, bool linear)
  {
    int row = gridLayout->rowCount();
    gridLayout->addWidget(new QLabel(name), row, 0, Qt::AlignLeft);
    return makeFloatEditor(min, scaleMax, row, linear);
  };

  //------------------------------------------------------------------------------------------------
  auto makeBoolEditorRow = [&](const QString& name)
  {
    BoolEditor boolEditor;

    int row = gridLayout->rowCount();

    auto check = new QCheckBox(name);
    gridLayout->addWidget(check, row, 1);

    connect(check, &QCheckBox::toggled, this, &EditMaterialWidget::materialEdited);

    boolEditor.get = [=]{return check->isChecked();};
    boolEditor.set = [=](bool v){check->setChecked(v);};

    return boolEditor;
  };

  //------------------------------------------------------------------------------------------------
  auto addTitle = [&](const QString& name)
  {
    int row = gridLayout->rowCount();
    gridLayout->addWidget(new QLabel(QString("<h3>%1</h3>").arg(name)), row, 0, 1, 2, Qt::AlignLeft);
  };


  addTitle("Shader Type");
  {
    d->shaderType = new QComboBox();
    d->shaderType->addItems({"Principled", "None"});
    int row = gridLayout->rowCount();
    gridLayout->addWidget(new QLabel("Shader type"), row, 0, Qt::AlignLeft);
    gridLayout->addWidget(d->shaderType, row, 1);
    connect(d->shaderType, &QComboBox::currentTextChanged, this, [this]{emit materialEdited();});
  }

  addTitle("Colors");
  d->albedoColorButton   = makeColorEdit("Albedo"    , [&]()->glm::vec3&{return d->material.findOrAddOpenGL()->albedo;}  , d->albedoScaleSlider  ,     4.0f, false);
  d->sssColorButton      = makeColorEdit("Subsurface", [&]()->glm::vec3&{return d->material.findOrAddLegacy()->sss;}     , d->sssSlider          ,     1.0f, false);
  d->emissionColorButton = makeColorEdit("Emission"  , [&]()->glm::vec3&{return d->material.findOrAddLegacy()->emission;}, d->emissionSlider     , 50000.0f, false);
  d->velvetColorButton   = makeColorEdit("Velvet"    , [&]()->glm::vec3&{return d->material.findOrAddLegacy()->velvet;}  , d->velvetSlider       ,     1.0f, false);


  addTitle("Material Properties");
  d->alpha                 = makeFloatEditorRow("Alpha"                 , 0.0f,  1.0f, true);
  d->roughness             = makeFloatEditorRow("Roughness"             , 0.0f,  1.0f, true);
  d->metalness             = makeFloatEditorRow("Metalness"             , 0.0f,  1.0f, true);
  d->specular              = makeFloatEditorRow("Specular"              , 0.0f,  1.0f, true);
  d->transmission          = makeFloatEditorRow("Transmission"          , 0.0f,  1.0f, true);
  d->transmissionRoughness = makeFloatEditorRow("Transmission roughness", 0.0f,  1.0f, true);
  d->ior                   = makeFloatEditorRow("IOR"                   , 0.0f,  6.0f, true);
  d->sheen                 = makeFloatEditorRow("Sheen"                 , 0.0f,  1.0f, true);
  d->sheenTint             = makeFloatEditorRow("Sheen tint"            , 0.0f,  1.0f, true);
  d->clearCoat             = makeFloatEditorRow("Clear coat"            , 0.0f,  1.0f, true);
  d->clearCoatRoughness    = makeFloatEditorRow("Clear coat roughness"  , 0.0f,  1.0f, true);
  d->iridescentFactor      = makeFloatEditorRow("Iridescent factor"     , 0.0f,  1.0f, true);
  d->iridescentOffset      = makeFloatEditorRow("Iridescent offset"     , 0.0f,  1.0f, true);
  d->iridescentFrequency   = makeFloatEditorRow("Iridescent frequency"  , 0.0f, 20.0f, false);
  d->normalStrength        = makeFloatEditorRow("Normal strength"       , 0.1f, 10.0f, false);

  {
    int row = gridLayout->rowCount();
    gridLayout->addWidget(new QLabel("Subsurface radius"), row, 0, Qt::AlignLeft);

    auto ll = new QHBoxLayout();
    gridLayout->addLayout(ll, row, 1);

    auto make = [&]
    {
      auto spin = new QDoubleSpinBox();
      ll->addWidget(spin);
      spin->setRange(0.0, 100.0);
      connect(spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EditMaterialWidget::materialEdited);
      return spin;
    };

    d->sssRadiusR = make();
    d->sssRadiusG = make();
    d->sssRadiusB = make();
  }

  {
    d->sssMethod = new QComboBox();
    d->sssMethod->addItems({"ChristensenBurley", "RandomWalk", "RandomWalkFixedRadius"});
    int row = gridLayout->rowCount();
    gridLayout->addWidget(new QLabel("Subsurface method"), row, 0, Qt::AlignLeft);
    gridLayout->addWidget(d->sssMethod, row, 1);
    connect(d->sssMethod, &QComboBox::currentTextChanged, this, [this]{emit materialEdited();});
  }


  addTitle("Albedo Color Modification");
  d->albedoBrightness  = makeFloatEditorRow("Albedo brightness", -50.0f, 50.0f, true);
  d->albedoContrast    = makeFloatEditorRow("Albedo contrast"  , -50.0f, 50.0f, true);
  d->albedoGamma       = makeFloatEditorRow("Albedo gamma"     ,   0.0f, 50.0f, true);
  d->albedoHue         = makeFloatEditorRow("Albedo hue shift" ,   0.0f,  1.0f, true);
  d->albedoSaturation  = makeFloatEditorRow("Albedo saturation",   0.0f, 10.0f, true);
  d->albedoValue       = makeFloatEditorRow("Albedo value"     ,   0.0f, 10.0f, true);
  d->albedoFactor      = makeFloatEditorRow("Albedo factor"    ,   0.0f,  1.0f, true);


  addTitle("Displacement");
  d->heightScale    = makeFloatEditorRow("Height scale"   , 0.0f, 1.0f, true);
  d->heightMidlevel = makeFloatEditorRow("Height midlevel", 0.0f, 1.0f, true);


  addTitle("Texture Transformation");
  d->skewU      = makeFloatEditorRow("Skew U"     ,  -70.00f,  70.0f, true );
  d->skewV      = makeFloatEditorRow("Skew V"     ,  -70.00f,  70.0f, true );

  d->scaleU     = makeFloatEditorRow("Scale U"    ,    0.01f,  10.0f, false);
  d->scaleV     = makeFloatEditorRow("Scale V"    ,    0.01f,  10.0f, false);

  d->translateU = makeFloatEditorRow("Translate U",    0.00f,   5.0f, true );
  d->translateV = makeFloatEditorRow("Translate V",    0.00f,   5.0f, true );

  d->rotateUV   = makeFloatEditorRow("Rotate UV"  , -180.00f, 180.0f, true );


  addTitle("Texture Maps");
  auto addTextureBlendEdit = [&](const auto& name, bool isBlendFile=false)
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
      auto existing = new QComboBox();
      l->addWidget(existing);
      existing->setEnabled(false);

      if(d->getExistingTextures)
      {
        for(const auto& name : d->getExistingTextures())
          existing->addItem(QString::fromStdString(name.toString()));
      }

      auto loadRadio = new QRadioButton("Load");
      loadRadio->setChecked(true);
      l->addWidget(loadRadio);
      auto load = new tp_qt_widgets::FileDialogLineEdit();
      load->setQSettingsPath("EditMaterialWidget");
      load->setMode(tp_qt_widgets::FileDialogLineEdit::OpenFileMode);
      if(isBlendFile)
        load->setFilter(QString::fromStdString("*.blend"));
      else
        load->setFilter(QString::fromStdString(tp_image_utils::imageTypesFilter()));
        
      l->addWidget(load);

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
          Q_EMIT materialEdited();
        }
        else if(loadRadio->isChecked())
        {
          if(isBlendFile)
          {
              std::string error;
              auto text = d->loadMaterialBlend(load->text().toStdString(), error);
              if(text.isValid())
              {
                  edit->setText(QString::fromStdString(text.toString()));
                  Q_EMIT materialEdited();
              }

              if(!error.empty())
                  QMessageBox::critical(this, "Error Loading Blend Material!", QString::fromStdString(error));
          }
          else if(d->loadTexture)
          {
              std::string error;
              auto text = d->loadTexture(load->text().toStdString(), error);
              if(text.isValid())
              {
                  edit->setText(QString::fromStdString(text.toString()));
                  Q_EMIT materialEdited();
              }

              if(!error.empty())
                  QMessageBox::critical(this, "Error Loading Image!", QString::fromStdString(error));
          }
        }

        }
    });

    edit->installEventFilter(this);

    return edit;
  };

  if(textureSupported == TextureSupported::Yes)
  {
    d->material.findOrAddOpenGL()->viewTypedTextures([&](const auto& type, const auto&, const auto& pretty)
    {
      d->textureLineEdits[type] = addTextureBlendEdit(pretty);
    });

    d->material.findOrAddLegacy()->viewTypedTextures([&](const auto& type, const auto&, const auto& pretty)
    {
      d->textureLineEdits[type] = addTextureBlendEdit(pretty);
    });
  }

  addTitle(".Blend Material");
  d->blendFileLineEdit = addTextureBlendEdit(".blend material", true);

  addTitle("OpenGL Shading Calculation");
  d->useAmbient     = makeFloatEditorRow("Use ambient"    , 0.0f, 1.0f, true);
  d->useDiffuse     = makeFloatEditorRow("Use diffuse"    , 0.0f, 1.0f, true);
  d->useNdotL       = makeFloatEditorRow("Use N dot L"    , 0.0f, 1.0f, true);
  d->useAttenuation = makeFloatEditorRow("Use attenuation", 0.0f, 1.0f, true);
  d->useShadow      = makeFloatEditorRow("Use shadow"     , 0.0f, 1.0f, true);
  d->useLightMask   = makeFloatEditorRow("Light mask"     , 0.0f, 1.0f, true);
  d->useReflection  = makeFloatEditorRow("Use reflection" , 0.0f, 1.0f, true);

  addTitle("Ray Visibility");
  d->rayVisibilityCamera        = makeBoolEditorRow("Camera"        );
  d->rayVisibilityDiffuse       = makeBoolEditorRow("Diffuse"       );
  d->rayVisibilityGlossy        = makeBoolEditorRow("Glossy"        );
  d->rayVisibilityTransmission  = makeBoolEditorRow("Transmission"  );
  d->rayVisibilityScatter       = makeBoolEditorRow("Volume scatter");
  d->rayVisibilityShadow        = makeBoolEditorRow("Shadow"        );
  d->rayVisibilityShadowCatcher = makeBoolEditorRow("Shadow catcher");

  {
    auto hLayout = new QHBoxLayout();
    hLayout->setContentsMargins(0,0,0,0);
    hLayout->addStretch();
    mainLayout->addLayout(hLayout);

    if(addButtons)
      addButtons(hLayout);

    {
      auto button = new QPushButton("Copy");
      hLayout->addWidget(button);
      connect(button, &QPushButton::clicked, this, [=]
      {
        nlohmann::json j;
        material().saveState(j);
        QGuiApplication::clipboard()->setText(QString::fromStdString(j.dump(2)));
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
        Q_EMIT materialEdited();
      });
    }
  }

  d->scroll->setMinimumWidth(d->scrollContents->minimumSizeHint().width() + d->scroll->verticalScrollBar()->width());
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
  auto openGLMaterial = d->material.findOrAddOpenGL();
  auto legacyMaterial = d->material.findOrAddLegacy();

  d->nameEdit->setText(QString::fromStdString(material.name.toString()));

  d->shaderType->setCurrentText(QString::fromStdString(tp_math_utils::shaderTypeToString(legacyMaterial->shaderType)));

  d->updateColors();

  d->alpha                .set(openGLMaterial->alpha                );
  d->roughness            .set(openGLMaterial->roughness            );
  d->metalness            .set(openGLMaterial->metalness            );
  d->specular             .set(legacyMaterial->specular             );
  d->transmission         .set(openGLMaterial->transmission         );
  d->transmissionRoughness.set(openGLMaterial->transmissionRoughness);
  d->heightScale          .set(legacyMaterial->heightScale          );
  d->heightMidlevel       .set(legacyMaterial->heightMidlevel       );
  d->ior                  .set(legacyMaterial->ior                  );
  d->sheen                .set(legacyMaterial->sheen                );
  d->sheenTint            .set(legacyMaterial->sheenTint            );
  d->clearCoat            .set(legacyMaterial->clearCoat            );
  d->clearCoatRoughness   .set(legacyMaterial->clearCoatRoughness   );
  d->iridescentFactor     .set(legacyMaterial->iridescentFactor     );
  d->iridescentOffset     .set(legacyMaterial->iridescentOffset     );
  d->iridescentFrequency  .set(legacyMaterial->iridescentFrequency  );
  d->normalStrength       .set(legacyMaterial->normalStrength       );

  d->sssRadiusR->setValue(double(legacyMaterial->sssRadius.x));
  d->sssRadiusG->setValue(double(legacyMaterial->sssRadius.y));
  d->sssRadiusB->setValue(double(legacyMaterial->sssRadius.z));

  d->sssMethod->setCurrentText(QString::fromStdString(tp_math_utils::sssMethodToString(legacyMaterial->sssMethod)));

  d->albedoBrightness     .set(openGLMaterial->albedoBrightness     );
  d->albedoContrast       .set(openGLMaterial->albedoContrast       );
  d->albedoGamma          .set(openGLMaterial->albedoGamma          );
  d->albedoHue            .set(openGLMaterial->albedoHue            );
  d->albedoSaturation     .set(openGLMaterial->albedoSaturation     );
  d->albedoValue          .set(openGLMaterial->albedoValue          );
  d->albedoFactor         .set(openGLMaterial->albedoFactor         );

  d->skewU                .set(material.uvTransformation.skewUV.x     );
  d->skewV                .set(material.uvTransformation.skewUV.y     );

  d->scaleU               .set(material.uvTransformation.scaleUV.x    );
  d->scaleV               .set(material.uvTransformation.scaleUV.y    );

  d->translateU           .set(material.uvTransformation.translateUV.x);
  d->translateV           .set(material.uvTransformation.translateUV.y);

  d->rotateUV             .set(material.uvTransformation.rotateUV     );

  d->useAmbient           .set(openGLMaterial->useAmbient           );
  d->useDiffuse           .set(openGLMaterial->useDiffuse           );
  d->useNdotL             .set(openGLMaterial->useNdotL             );
  d->useAttenuation       .set(openGLMaterial->useAttenuation       );
  d->useShadow            .set(openGLMaterial->useShadow            );
  d->useLightMask         .set(openGLMaterial->useLightMask         );
  d->useReflection        .set(openGLMaterial->useReflection        );

  d->rayVisibilityCamera       .set(legacyMaterial->rayVisibilityCamera       );
  d->rayVisibilityDiffuse      .set(legacyMaterial->rayVisibilityDiffuse      );
  d->rayVisibilityGlossy       .set(legacyMaterial->rayVisibilityGlossy       );
  d->rayVisibilityTransmission .set(legacyMaterial->rayVisibilityTransmission );
  d->rayVisibilityScatter      .set(legacyMaterial->rayVisibilityScatter      );
  d->rayVisibilityShadow       .set(legacyMaterial->rayVisibilityShadow       );
  d->rayVisibilityShadowCatcher.set(openGLMaterial->rayVisibilityShadowCatcher);

  d->albedoScaleSlider    .set(openGLMaterial->albedoScale          );
  d->sssSlider            .set(legacyMaterial->sssScale             );
  d->emissionSlider       .set(legacyMaterial->emissionScale        );
  d->velvetSlider         .set(legacyMaterial->velvetScale          );

  openGLMaterial->viewTypedTextures([&](const auto& type, const auto& value, const auto&)
  {
    if(d->textureSupported == TextureSupported::Yes)
      d->textureLineEdits[type]->setText(QString::fromStdString(value.toString()));
  });

  legacyMaterial->viewTypedTextures([&](const auto& type, const auto& value, const auto&)
  {
    if(d->textureSupported == TextureSupported::Yes)
      d->textureLineEdits[type]->setText(QString::fromStdString(value.toString()));
  });

  d->blendFileLineEdit->setText("");
  d->material.viewExternal("blend", [&](const tp_math_utils::ExternalMaterial& externalMaterial)
  {
    d->blendFileLineEdit->setText(QString::fromStdString(externalMaterial.subPath.toString()));
  });
}

//##################################################################################################
tp_math_utils::Material EditMaterialWidget::material() const
{
  auto openGLMaterial = d->material.findOrAddOpenGL();
  auto legacyMaterial = d->material.findOrAddLegacy();

  d->material.name = d->nameEdit->text().toStdString();

  legacyMaterial->shaderType = tp_math_utils::shaderTypeFromString(d->shaderType->currentText().toStdString());

  openGLMaterial->alpha                 = d->alpha                .get();
  openGLMaterial->roughness             = d->roughness            .get();
  openGLMaterial->metalness             = d->metalness            .get();
  legacyMaterial->specular              = d->specular             .get();
  openGLMaterial->transmission          = d->transmission         .get();
  openGLMaterial->transmissionRoughness = d->transmissionRoughness.get();
  legacyMaterial->heightScale           = d->heightScale          .get();
  legacyMaterial->heightMidlevel        = d->heightMidlevel       .get();
  legacyMaterial->ior                   = d->ior                  .get();
  legacyMaterial->sheen                 = d->sheen                .get();
  legacyMaterial->sheenTint             = d->sheenTint            .get();
  legacyMaterial->clearCoat             = d->clearCoat            .get();
  legacyMaterial->clearCoatRoughness    = d->clearCoatRoughness   .get();
  legacyMaterial->iridescentFactor      = d->iridescentFactor     .get();
  legacyMaterial->iridescentOffset      = d->iridescentOffset     .get();
  legacyMaterial->iridescentFrequency   = d->iridescentFrequency  .get();
  legacyMaterial->normalStrength        = d->normalStrength       .get();

  legacyMaterial->sssRadius.x = d->sssRadiusR->value();
  legacyMaterial->sssRadius.y = d->sssRadiusG->value();
  legacyMaterial->sssRadius.z = d->sssRadiusB->value();

  legacyMaterial->sssMethod = tp_math_utils::sssMethodFromString(d->sssMethod->currentText().toStdString());


  openGLMaterial->albedoBrightness = d->albedoBrightness.get();
  openGLMaterial->albedoContrast   = d->albedoContrast  .get();
  openGLMaterial->albedoGamma      = d->albedoGamma     .get();
  openGLMaterial->albedoHue        = d->albedoHue       .get();
  openGLMaterial->albedoSaturation = d->albedoSaturation.get();
  openGLMaterial->albedoValue      = d->albedoValue     .get();
  openGLMaterial->albedoFactor     = d->albedoFactor    .get();

  d->material.uvTransformation.skewUV.x      = d->skewU     .get();
  d->material.uvTransformation.skewUV.y      = d->skewV     .get();

  d->material.uvTransformation.scaleUV.x     = d->scaleU    .get();
  d->material.uvTransformation.scaleUV.y     = d->scaleV    .get();

  d->material.uvTransformation.translateUV.x = d->translateU.get();
  d->material.uvTransformation.translateUV.y = d->translateV.get();

  d->material.uvTransformation.rotateUV      = d->rotateUV  .get();

  openGLMaterial->useAmbient            = d->useAmbient       .get();
  openGLMaterial->useDiffuse            = d->useDiffuse       .get();
  openGLMaterial->useNdotL              = d->useNdotL         .get();
  openGLMaterial->useAttenuation        = d->useAttenuation   .get();
  openGLMaterial->useShadow             = d->useShadow        .get();
  openGLMaterial->useLightMask          = d->useLightMask     .get();
  openGLMaterial->useReflection         = d->useReflection    .get();

  legacyMaterial->rayVisibilityCamera        = d->rayVisibilityCamera         .get();
  legacyMaterial->rayVisibilityDiffuse       = d->rayVisibilityDiffuse        .get();
  legacyMaterial->rayVisibilityGlossy        = d->rayVisibilityGlossy         .get();
  legacyMaterial->rayVisibilityTransmission  = d->rayVisibilityTransmission   .get();
  legacyMaterial->rayVisibilityScatter       = d->rayVisibilityScatter        .get();
  legacyMaterial->rayVisibilityShadow        = d->rayVisibilityShadow         .get();
  openGLMaterial->rayVisibilityShadowCatcher = d->rayVisibilityShadowCatcher  .get();

  openGLMaterial->albedoScale           = d->albedoScaleSlider.get();
  legacyMaterial->sssScale              = d->sssSlider        .get();
  legacyMaterial->emissionScale         = d->emissionSlider   .get();
  legacyMaterial->velvetScale           = d->velvetSlider     .get();

  openGLMaterial->updateTypedTextures([&](const auto& type, auto& value, const auto&)
  {
    if(d->textureSupported == TextureSupported::Yes)
      value = d->textureLineEdits[type]->text().toStdString();
  });

  legacyMaterial->updateTypedTextures([&](const auto& type, auto& value, const auto&)
  {
    if(d->textureSupported == TextureSupported::Yes)
      value = d->textureLineEdits[type]->text().toStdString();
  });

  {
    tp_utils::StringID subPath = d->blendFileLineEdit->text().toStdString();
    if(subPath.isValid())
    {
      auto externalMaterial = d->material.findOrAddExternal("blend");
      externalMaterial->subPath = d->blendFileLineEdit->text().toStdString();
    }
    else
    {
      d->material.removeExternal("blend");
    }
  }
  return d->material;
}

//##################################################################################################
void EditMaterialWidget::setGetExistingTextures(const TPGetExistingTexturesCallback& getExistingTextures)
{
  d->getExistingTextures = getExistingTextures;
}

//##################################################################################################
void EditMaterialWidget::setLoadTexture(const TPLoadTextureCallback& loadTexture)
{
  d->loadTexture = loadTexture;
}


//################################################################################################
void EditMaterialWidget::setMaterialBlend(const TPLoadMaterialBlendCallback& loadMaterialBlend)
{
  d->loadMaterialBlend = loadMaterialBlend;
}

//##################################################################################################
bool EditMaterialWidget::editMaterialDialog(QWidget* parent,
                                            tp_math_utils::Material& material,
                                            TextureSupported textureSupported,
                                            const TPGetExistingTexturesCallback& getExistingTextures,
                                            const TPLoadTextureCallback& loadTexture,
                                            const TPLoadMaterialBlendCallback& loadMaterialBlend)
{
  QPointer<QDialog> dialog = new QDialog(parent);
  TP_CLEANUP([&]{delete dialog;});

  dialog->setWindowTitle("Edit Material");

  auto l = new QVBoxLayout(dialog);

  auto editMaterialWidget = new EditMaterialWidget(textureSupported);
  l->addWidget(editMaterialWidget);
  editMaterialWidget->setMaterial(material);
  editMaterialWidget->setGetExistingTextures(getExistingTextures);
  editMaterialWidget->setLoadTexture(loadTexture);
  editMaterialWidget->setMaterialBlend(loadMaterialBlend);

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

//##################################################################################################
bool EditMaterialWidget::eventFilter(QObject* watched, QEvent* event)
{
  if(event->type() == QEvent::DragEnter)
  {
    QDragEnterEvent* e = static_cast<QDragEnterEvent*>(event);
    auto urls = e->mimeData()->urls();
    if(urls.size() == 1 && urls.front().isLocalFile())
    {
      e->acceptProposedAction();
      return true;
    }
  }

  else if(event->type() == QEvent::Drop)
  {
    QDropEvent* e = static_cast<QDropEvent*>(event);
    auto urls = e->mimeData()->urls();
    if(urls.size() == 1 && urls.front().isLocalFile())
    {
      std::string error;
      std::string path = urls.front().toLocalFile().toStdString();
      auto text = d->loadTexture(path, error);
      if(text.isValid())
      {
        for(const auto& i: d->textureLineEdits)
        {
          if(watched == i.second)
          {
            i.second->setText(QString::fromStdString(text.toString()));
            break;
          }
        }
        Q_EMIT materialEdited();
      }

      if(!error.empty())
        QMessageBox::critical(this, "Error Loading Image!", QString::fromStdString(error));
    }

    return true;
  }

  else if(event->type() == QEvent::Resize && watched == d->scrollContents)
  {
    d->scroll->setMinimumWidth(d->scrollContents->minimumSizeHint().width() + d->scroll->verticalScrollBar()->width());
  }

  return QWidget::eventFilter(watched, event);
}

}
