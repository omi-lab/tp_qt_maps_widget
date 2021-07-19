#include "tp_qt_maps_widget/EditMaterialWidget.h"
#include "tp_qt_maps_widget/SelectMaterialWidget.h"

#include "tp_qt_widgets/FileDialogLineEdit.h"

#include "tp_image_utils/LoadImages.h"

#include "tp_utils/JSONUtils.h"
#include "tp_utils/DebugUtils.h"

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
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QScrollArea>
#include <QScrollBar>

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
}

//##################################################################################################
struct EditMaterialWidget::Private
{
  tp_math_utils::Material material;
  TextureSupported textureSupported;
  std::function<std::vector<tp_utils::StringID>()> getExistingTextures;
  std::function<tp_utils::StringID(const std::string&)> loadTexture;

  QLineEdit* nameEdit{nullptr};

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
  QDoubleSpinBox* sssRadiusR{nullptr};
  QDoubleSpinBox* sssRadiusG{nullptr};
  QDoubleSpinBox* sssRadiusB{nullptr};

  FloatEditor albedoBrightness;
  FloatEditor albedoContrast;
  FloatEditor albedoGamma;
  FloatEditor albedoHue;
  FloatEditor albedoSaturation;
  FloatEditor albedoValue;
  FloatEditor albedoFactor;

  FloatEditor useAmbient;
  FloatEditor useDiffuse;
  FloatEditor useNdotL;
  FloatEditor useAttenuation;
  FloatEditor useShadow;
  FloatEditor useLightMask;
  FloatEditor useReflection;

  std::map<std::string, QLineEdit*> textureLineEdits;

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

    albedoColorButton  ->setIcon(makeIcon(material.albedo  ));
    sssColorButton     ->setIcon(makeIcon(material.sss     ));
    emissionColorButton->setIcon(makeIcon(material.emission));
    velvetColorButton  ->setIcon(makeIcon(material.velvet  ));
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
  auto makeFloatEditor = [&](float min, float scaleMax, int row, const bool linear)
  {
    float scale = scaleMax - min;

    FloatEditor floatEditor;

    auto hLayout = new QHBoxLayout();
    hLayout->setContentsMargins(0,0,0,0);
    gridLayout->addLayout(hLayout, row, 1);

    auto spin = new QDoubleSpinBox();
    spin->setRange(double(min), double(scaleMax));
    spin->setDecimals(3);
    spin->setSingleStep(0.01);
    hLayout->addWidget(spin, 1);

    auto slider = new QSlider(Qt::Horizontal);
    slider->setRange(0, 100000);
    hLayout->addWidget(slider, 3);
    connect(slider, &QSlider::valueChanged, this, &EditMaterialWidget::materialEdited);

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

  d->albedoColorButton   = makeColorEdit("Albedo"    , [&]()->glm::vec3&{return d->material.albedo;}  , d->albedoScaleSlider  ,   4.0f, false);
  d->sssColorButton      = makeColorEdit("Subsurface", [&]()->glm::vec3&{return d->material.sss;}     , d->sssSlider          ,   1.0f, false);
  d->emissionColorButton = makeColorEdit("Emission"  , [&]()->glm::vec3&{return d->material.emission;}, d->emissionSlider     , 100.0f, false);
  d->velvetColorButton   = makeColorEdit("Velvet"    , [&]()->glm::vec3&{return d->material.velvet;}  , d->velvetSlider       ,   1.0f, false);


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

  d->albedoBrightness  = makeFloatEditorRow("Albedo brightness", -1.0f, 1.0f, true);
  d->albedoContrast    = makeFloatEditorRow("Albedo contrast"  , -1.0f, 1.0f, true);
  d->albedoGamma       = makeFloatEditorRow("Albedo gamma"     ,  0.0f, 2.0f, true);
  d->albedoHue         = makeFloatEditorRow("Albedo hue"       ,  0.0f, 1.0f, true);
  d->albedoSaturation  = makeFloatEditorRow("Albedo saturation",  0.0f, 2.0f, true);
  d->albedoValue       = makeFloatEditorRow("Albedo value"     ,  0.0f, 2.0f, true);
  d->albedoFactor      = makeFloatEditorRow("Albedo factor"    ,  0.0f, 1.0f, true);

  d->heightScale    = makeFloatEditorRow("Height scale"   , 0.0f, 1.0f, true);
  d->heightMidlevel = makeFloatEditorRow("Height midlevel", 0.0f, 1.0f, true);

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
      auto existing = new QComboBox();
      l->addWidget(existing);
      existing->setEnabled(false);

      if(d->getExistingTextures)
      {
        for(const auto& name : d->getExistingTextures())
          existing->addItem(QString::fromStdString(name.keyString()));
      }

      auto loadRadio = new QRadioButton("Load");
      loadRadio->setChecked(true);
      l->addWidget(loadRadio);
      auto load = new tp_qt_widgets::FileDialogLineEdit();
      load->setQSettingsPath("EditMaterialWidget");
      load->setMode(tp_qt_widgets::FileDialogLineEdit::OpenFileMode);
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
        else if(loadRadio->isChecked() && d->loadTexture)
        {
          auto text = d->loadTexture(load->text().toStdString());
          if(text.isValid())
          {
            edit->setText(QString::fromStdString(text.keyString()));
            Q_EMIT materialEdited();
          }
        }
      }
    });

    edit->installEventFilter(this);

    return edit;
  };

  d->material.viewTypedTextures([&](const auto& type, const auto&, const auto& pretty)
  {
    if(textureSupported == TextureSupported::Yes)
      d->textureLineEdits[type] = addTextureEdit(pretty);
  });

  d->useAmbient     = makeFloatEditorRow("Use ambient"    , 0.0f, 1.0f, true);
  d->useDiffuse     = makeFloatEditorRow("Use diffuse"    , 0.0f, 1.0f, true);
  d->useNdotL       = makeFloatEditorRow("Use N dot L"    , 0.0f, 1.0f, true);
  d->useAttenuation = makeFloatEditorRow("Use attenuation", 0.0f, 1.0f, true);
  d->useShadow      = makeFloatEditorRow("Use shadow"     , 0.0f, 1.0f, true);
  d->useLightMask   = makeFloatEditorRow("Light mask"     , 0.0f, 1.0f, true);
  d->useReflection  = makeFloatEditorRow("Use reflection" , 0.0f, 1.0f, true);

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

  d->nameEdit->setText(QString::fromStdString(material.name.keyString()));

  d->updateColors();

  d->alpha                .set(material.alpha                );
  d->roughness            .set(material.roughness            );
  d->metalness            .set(material.metalness            );
  d->specular             .set(material.specular             );
  d->transmission         .set(material.transmission         );
  d->transmissionRoughness.set(material.transmissionRoughness);
  d->heightScale          .set(material.heightScale          );
  d->heightMidlevel       .set(material.heightMidlevel       );
  d->ior                  .set(material.ior                  );
  d->sheen                .set(material.sheen                );
  d->sheenTint            .set(material.sheenTint            );
  d->clearCoat            .set(material.clearCoat            );
  d->clearCoatRoughness   .set(material.clearCoatRoughness   );
  d->iridescentFactor     .set(material.iridescentFactor     );
  d->iridescentOffset     .set(material.iridescentOffset     );
  d->iridescentFrequency  .set(material.iridescentFrequency  );

  d->sssRadiusR->setValue(double(material.sssRadius.x));
  d->sssRadiusG->setValue(double(material.sssRadius.y));
  d->sssRadiusB->setValue(double(material.sssRadius.z));

  d->albedoBrightness     .set(material.albedoBrightness     );
  d->albedoContrast       .set(material.albedoContrast       );
  d->albedoGamma          .set(material.albedoGamma          );
  d->albedoHue            .set(material.albedoHue            );
  d->albedoSaturation     .set(material.albedoSaturation     );
  d->albedoValue          .set(material.albedoValue          );
  d->albedoFactor         .set(material.albedoFactor         );

  d->useAmbient           .set(material.useAmbient           );
  d->useDiffuse           .set(material.useDiffuse           );
  d->useNdotL             .set(material.useNdotL             );
  d->useAttenuation       .set(material.useAttenuation       );
  d->useShadow            .set(material.useShadow            );
  d->useLightMask         .set(material.useLightMask         );
  d->useReflection        .set(material.useReflection        );

  d->albedoScaleSlider    .set(material.albedoScale          );
  d->sssSlider            .set(material.sssScale             );
  d->emissionSlider       .set(material.emissionScale        );
  d->velvetSlider         .set(material.velvetScale          );

  d->material.viewTypedTextures([&](const auto& type, const auto& value, const auto&)
  {
    if(d->textureSupported == TextureSupported::Yes)
      d->textureLineEdits[type]->setText(QString::fromStdString(value.keyString()));
  });
}

//##################################################################################################
tp_math_utils::Material EditMaterialWidget::material() const
{
  d->material.name = d->nameEdit->text().toStdString();

  d->material.alpha                 = d->alpha                .get();
  d->material.roughness             = d->roughness            .get();
  d->material.metalness             = d->metalness            .get();
  d->material.specular              = d->specular             .get();
  d->material.transmission          = d->transmission         .get();
  d->material.transmissionRoughness = d->transmissionRoughness.get();
  d->material.heightScale           = d->heightScale          .get();
  d->material.heightMidlevel        = d->heightMidlevel       .get();
  d->material.ior                   = d->ior                  .get();
  d->material.sheen                 = d->sheen                .get();
  d->material.sheenTint             = d->sheenTint            .get();
  d->material.clearCoat             = d->clearCoat            .get();
  d->material.clearCoatRoughness    = d->clearCoatRoughness   .get();
  d->material.iridescentFactor      = d->iridescentFactor     .get();
  d->material.iridescentOffset      = d->iridescentOffset     .get();
  d->material.iridescentFrequency   = d->iridescentFrequency  .get();

  d->material.sssRadius.x = d->sssRadiusR->value();
  d->material.sssRadius.y = d->sssRadiusG->value();
  d->material.sssRadius.z = d->sssRadiusB->value();

  d->material.albedoBrightness = d->albedoBrightness.get();
  d->material.albedoContrast   = d->albedoContrast  .get();
  d->material.albedoGamma      = d->albedoGamma     .get();
  d->material.albedoHue        = d->albedoHue       .get();
  d->material.albedoSaturation = d->albedoSaturation.get();
  d->material.albedoValue      = d->albedoValue     .get();
  d->material.albedoFactor     = d->albedoFactor    .get();

  d->material.useAmbient            = d->useAmbient       .get();
  d->material.useDiffuse            = d->useDiffuse       .get();
  d->material.useNdotL              = d->useNdotL         .get();
  d->material.useAttenuation        = d->useAttenuation   .get();
  d->material.useShadow             = d->useShadow        .get();
  d->material.useLightMask          = d->useLightMask     .get();
  d->material.useReflection         = d->useReflection    .get();

  d->material.albedoScale           = d->albedoScaleSlider.get();
  d->material.sssScale              = d->sssSlider        .get();
  d->material.emissionScale         = d->emissionSlider   .get();
  d->material.velvetScale           = d->velvetSlider     .get();

  d->material.updateTypedTextures([&](const auto& type, auto& value, const auto&)
  {
    if(d->textureSupported == TextureSupported::Yes)
      value = d->textureLineEdits[type]->text().toStdString();
  });

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
                                            TextureSupported textureSupported,
                                            const std::function<std::vector<tp_utils::StringID>()>& getExistingTextures,
                                            const std::function<tp_utils::StringID(const std::string&)>& loadTexture)
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
      std::string path = urls.front().toLocalFile().toStdString();
      auto text = d->loadTexture(path);
      if(text.isValid())
      {
        for(const auto& i: d->textureLineEdits)
        {
          if(watched == i.second)
          {
            i.second->setText(QString::fromStdString(text.keyString()));
            break;
          }
        }
        Q_EMIT materialEdited();
      }
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
