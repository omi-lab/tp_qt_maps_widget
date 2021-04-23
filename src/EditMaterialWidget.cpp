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
  FloatEditor transmission;
  FloatEditor transmissionRoughness;
  FloatEditor sheen;
  FloatEditor sheenTint;
  FloatEditor clearCoat;
  FloatEditor clearCoatRoughness;
  FloatEditor heightScale;
  FloatEditor heightMidlevel;
  FloatEditor ior;
  QDoubleSpinBox* sssRadiusR{nullptr};
  QDoubleSpinBox* sssRadiusG{nullptr};
  QDoubleSpinBox* sssRadiusB{nullptr};
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
        p.setBrush(QColor::fromRgbF(c.x, c.y, c.z));
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
EditMaterialWidget::EditMaterialWidget(const std::function<void(QLayout*)>& addButtons, QWidget* parent):
  QWidget(parent),
  d(new Private())
{
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
  auto makeFloatEditor = [&](float scaleMax, int row, const bool linear)
  {
    FloatEditor floatEditor;

    auto hLayout = new QHBoxLayout();
    hLayout->setContentsMargins(0,0,0,0);
    gridLayout->addLayout(hLayout, row, 1);

    auto spin = new QDoubleSpinBox();
    spin->setRange(0.0, double(scaleMax));
    spin->setDecimals(3);
    hLayout->addWidget(spin, 1);

    auto slider = new QSlider(Qt::Horizontal);
    slider->setRange(0, 100000);
    hLayout->addWidget(slider, 3);
    connect(slider, &QSlider::valueChanged, this, &EditMaterialWidget::materialEdited);

    connect(slider, &QSlider::valueChanged, this, [=]
    {
      float v = float(slider->value()) / 100000.0f;
      v = linear?(v*scaleMax):(v*v*scaleMax);
      if(std::fabs(v-float(spin->value())) > 0.0001f)
      {
        spin->setValue(double(v));
        emit materialEdited();
      }
    });

    connect(spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [=]
    {
      auto s = float(spin->value())/scaleMax;
      slider->setValue(int( (linear?s:std::sqrt(s))*100000.0f));
    });

    floatEditor.get = [=]{return spin->value();};
    floatEditor.set = [=](float v){spin->setValue(double(v));};

    return floatEditor;
  };

  //------------------------------------------------------------------------------------------------
  auto makeColorEdit = [&](const QString& text, const std::function<glm::vec3&()>& getColor, FloatEditor& slider, float scaleMax, const bool linear)
  {
    int row = gridLayout->rowCount();

    auto button = new QPushButton(text);
    button->setStyleSheet("text-align:left; padding-left:2;");
    gridLayout->addWidget(button, row, 0);

    slider = makeFloatEditor(scaleMax, row, linear);

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

  //------------------------------------------------------------------------------------------------
  auto makeFloatEditorRow = [&](const QString& name, float scaleMax, bool linear)
  {
    int row = gridLayout->rowCount();
    gridLayout->addWidget(new QLabel(name), row, 0, Qt::AlignLeft);
    return makeFloatEditor(scaleMax, row, linear);
  };

  d->albedoColorButton   = makeColorEdit("Albedo"    , [&]()->glm::vec3&{return d->material.albedo;}  , d->albedoScaleSlider  ,   4.0f, false);
  d->sssColorButton      = makeColorEdit("Subsurface", [&]()->glm::vec3&{return d->material.sss;}     , d->sssSlider          ,   1.0f, false);
  d->emissionColorButton = makeColorEdit("Emission"  , [&]()->glm::vec3&{return d->material.emission;}, d->emissionSlider     , 100.0f, false);
  d->velvetColorButton   = makeColorEdit("Velvet"    , [&]()->glm::vec3&{return d->material.velvet;}  , d->velvetSlider       ,   1.0f, false);


  d->alpha                 = makeFloatEditorRow("Alpha"                 , 1.0f, true);
  d->roughness             = makeFloatEditorRow("Roughness"             , 1.0f, true);
  d->metalness             = makeFloatEditorRow("Metalness"             , 1.0f, true);
  d->transmission          = makeFloatEditorRow("Transmission"          , 1.0f, true);
  d->transmissionRoughness = makeFloatEditorRow("Transmission roughness", 1.0f, true);
  d->ior                   = makeFloatEditorRow("IOR"                   , 6.0f, true);
  d->sheen                 = makeFloatEditorRow("Sheen"                 , 1.0f, true);
  d->sheenTint             = makeFloatEditorRow("Sheen tint"            , 1.0f, true);
  d->clearCoat             = makeFloatEditorRow("Clear coat"            , 1.0f, true);
  d->clearCoatRoughness    = makeFloatEditorRow("Clear coat roughness"  , 1.0f, true);

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

  d->heightScale    = makeFloatEditorRow("Height scale"   , 1.0f, true);
  d->heightMidlevel = makeFloatEditorRow("Height midlevel", 1.0f, true);

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

    edit->installEventFilter(this);

    return edit;
  };

  d->material.viewTypedTextures([&](const auto& type, const auto&, const auto& pretty)
  {
    d->textureLineEdits[type] = addTextureEdit(pretty);
  });

  d->useAmbient     = makeFloatEditorRow("Use ambient"    , 1.0f, true);
  d->useDiffuse     = makeFloatEditorRow("Use diffuse"    , 1.0f, true);
  d->useNdotL       = makeFloatEditorRow("Use N dot L"    , 1.0f, true);
  d->useAttenuation = makeFloatEditorRow("Use attenuation", 1.0f, true);
  d->useShadow      = makeFloatEditorRow("Use shadow"     , 1.0f, true);
  d->useLightMask   = makeFloatEditorRow("Light mask"     , 1.0f, true);
  d->useReflection  = makeFloatEditorRow("Use reflection" , 1.0f, true);

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
        emit materialEdited();
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
  d->transmission         .set(material.transmission         );
  d->transmissionRoughness.set(material.transmissionRoughness);
  d->heightScale          .set(material.heightScale          );
  d->heightMidlevel       .set(material.heightMidlevel       );
  d->ior                  .set(material.ior                  );
  d->sheen                .set(material.sheen                );
  d->sheenTint            .set(material.sheenTint            );
  d->clearCoat            .set(material.clearCoat            );
  d->clearCoatRoughness   .set(material.clearCoatRoughness   );

  d->sssRadiusR->setValue(material.sssRadius.x);
  d->sssRadiusG->setValue(material.sssRadius.y);
  d->sssRadiusB->setValue(material.sssRadius.z);

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
  d->material.transmission          = d->transmission         .get();
  d->material.transmissionRoughness = d->transmissionRoughness.get();
  d->material.heightScale           = d->heightScale          .get();
  d->material.heightMidlevel        = d->heightMidlevel       .get();
  d->material.ior                   = d->ior                  .get();
  d->material.sheen                 = d->sheen                .get();
  d->material.sheenTint             = d->sheenTint            .get();
  d->material.clearCoat             = d->clearCoat            .get();
  d->material.clearCoatRoughness    = d->clearCoatRoughness   .get();

  d->material.sssRadius.x = d->sssRadiusR->value();
  d->material.sssRadius.y = d->sssRadiusG->value();
  d->material.sssRadius.z = d->sssRadiusB->value();

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
        emit materialEdited();
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
