#include "tp_qt_maps_widget/EditMaterialSwapParametersWidget.h"
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
struct EditMaterialSwapParametersWidget::Private
{
  QScrollArea* scroll{nullptr};
  QWidget* scrollContents{nullptr};

  QPushButton* colorButton  {nullptr};

  glm::vec3 swapColor {1.0f, 1.0f, 1.0f};

  FloatEditor albedoUseR;
  FloatEditor albedoUseG;
  FloatEditor albedoUseB;
  FloatEditor albedoScaleR;
  FloatEditor albedoScaleG;
  FloatEditor albedoScaleB;
  FloatEditor albedoBiasR;
  FloatEditor albedoBiasG;
  FloatEditor albedoBiasB;
  FloatEditor sssUseR;
  FloatEditor sssUseG;
  FloatEditor sssUseB;
  FloatEditor sssScaleR;
  FloatEditor sssScaleG;
  FloatEditor sssScaleB;
  FloatEditor sssBiasR;
  FloatEditor sssBiasG;
  FloatEditor sssBiasB;
  FloatEditor emissionUseR;
  FloatEditor emissionUseG;
  FloatEditor emissionUseB;
  FloatEditor emissionScaleR;
  FloatEditor emissionScaleG;
  FloatEditor emissionScaleB;
  FloatEditor emissionBiasR;
  FloatEditor emissionBiasG;
  FloatEditor emissionBiasB;
  FloatEditor velvetUseR;
  FloatEditor velvetUseG;
  FloatEditor velvetUseB;
  FloatEditor velvetScaleR;
  FloatEditor velvetScaleG;
  FloatEditor velvetScaleB;
  FloatEditor velvetBiasR;
  FloatEditor velvetBiasG;
  FloatEditor velvetBiasB;

  FloatEditor albedoHue;

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

    colorButton  ->setIcon(makeIcon(swapColor));
  }
};

//##################################################################################################
EditMaterialSwapParametersWidget::EditMaterialSwapParametersWidget( QWidget* parent):
  QWidget(parent),
  d(new Private())
{
  auto mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0,0,0,0);

  {
    auto ll = new QHBoxLayout();
    mainLayout->addLayout(ll);
    ll->setContentsMargins(0,0,0,0);
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
    connect(slider, &QSlider::valueChanged, this, &EditMaterialSwapParametersWidget::materialSwapParametersEdited);

    connect(slider, &QSlider::valueChanged, this, [=]
    {
      float v = float(slider->value()) / 100000.0f;
      v = linear?((v*scale) + min):(v*v*scale);
      if(std::fabs(v-float(spin->value())) > 0.000001f)
      {
        spin->blockSignals(true);
        spin->setValue(double(v));
        spin->blockSignals(false);
        Q_EMIT materialSwapParametersEdited();
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
        Q_EMIT materialSwapParametersEdited();
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
  auto makeColorEdit = [&](const QString& text, const std::function<glm::vec3&()>& getColor )
  {
    int row = gridLayout->rowCount();

    auto button = new QPushButton(text);
    button->setStyleSheet("text-align:left; padding-left:2;");
    gridLayout->addWidget(button, row, 0);

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
        Q_EMIT materialSwapParametersEdited();
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
  auto addTitle = [&](const QString& name)
  {
    int row = gridLayout->rowCount();
    gridLayout->addWidget(new QLabel(QString("<h3>%1</h3>").arg(name)), row, 0, 1, 2, Qt::AlignLeft);
  };

  d->colorButton   = makeColorEdit("Color"    , [&]()->glm::vec3&{return d->swapColor;} );
  d->updateColors();

  addTitle("Material Swap Properties");
  d->albedoUseR         = makeFloatEditorRow("Albedo Use R"       , 0.0f, 1.0f, true);
  d->albedoUseG         = makeFloatEditorRow("Albedo Use G"       , 0.0f, 1.0f, true);
  d->albedoUseB         = makeFloatEditorRow("Albedo Use B"       , 0.0f, 1.0f, true);
  d->albedoScaleR       = makeFloatEditorRow("Albedo Scale R"     , 0.0f, 1.0f, true);
  d->albedoScaleG       = makeFloatEditorRow("Albedo Scale G"     , 0.0f, 1.0f, true);
  d->albedoScaleB       = makeFloatEditorRow("Albedo Scale B"     , 0.0f, 1.0f, true);
  d->albedoBiasR        = makeFloatEditorRow("Albedo Bias R"      , 0.0f, 1.0f, true);
  d->albedoBiasG        = makeFloatEditorRow("Albedo Bias G"      , 0.0f, 1.0f, true);
  d->albedoBiasB        = makeFloatEditorRow("Albedo Bias B"      , 0.0f, 1.0f, true);

  d->sssUseR            = makeFloatEditorRow("SSS Use R"          , 0.0f, 1.0f, true);
  d->sssUseG            = makeFloatEditorRow("SSS Use G"          , 0.0f, 1.0f, true);
  d->sssUseB            = makeFloatEditorRow("SSS Use B"          , 0.0f, 1.0f, true);
  d->sssScaleR          = makeFloatEditorRow("SSS Scale R"        , 0.0f, 1.0f, true);
  d->sssScaleG          = makeFloatEditorRow("SSS Scale G"        , 0.0f, 1.0f, true);
  d->sssScaleB          = makeFloatEditorRow("SSS Scale B"        , 0.0f, 1.0f, true);
  d->sssBiasR           = makeFloatEditorRow("SSS Bias R"         , 0.0f, 1.0f, true);
  d->sssBiasG           = makeFloatEditorRow("SSS Bias G"         , 0.0f, 1.0f, true);
  d->sssBiasB           = makeFloatEditorRow("SSS Bias B"         , 0.0f, 1.0f, true);

  d->emissionUseR       = makeFloatEditorRow("Emission Use R"     , 0.0f, 1.0f, true);
  d->emissionUseG       = makeFloatEditorRow("Emission Use G"     , 0.0f, 1.0f, true);
  d->emissionUseB       = makeFloatEditorRow("Emission Use B"     , 0.0f, 1.0f, true);
  d->emissionScaleR     = makeFloatEditorRow("Emission Scale R"   , 0.0f, 1.0f, true);
  d->emissionScaleG     = makeFloatEditorRow("Emission Scale G"   , 0.0f, 1.0f, true);
  d->emissionScaleB     = makeFloatEditorRow("Emission Scale B"   , 0.0f, 1.0f, true);
  d->emissionBiasR      = makeFloatEditorRow("Emission Bias R"    , 0.0f, 1.0f, true);
  d->emissionBiasG      = makeFloatEditorRow("Emission Bias G"    , 0.0f, 1.0f, true);
  d->emissionBiasB      = makeFloatEditorRow("Emission Bias B"    , 0.0f, 1.0f, true);

  d->velvetUseR         = makeFloatEditorRow("Velvet Use R"       , 0.0f, 1.0f, true);
  d->velvetUseG         = makeFloatEditorRow("Velvet Use G"       , 0.0f, 1.0f, true);
  d->velvetUseB         = makeFloatEditorRow("Velvet Use B"       , 0.0f, 1.0f, true);
  d->velvetScaleR       = makeFloatEditorRow("Velvet Scale R"     , 0.0f, 1.0f, true);
  d->velvetScaleG       = makeFloatEditorRow("Velvet Scale G"     , 0.0f, 1.0f, true);
  d->velvetScaleB       = makeFloatEditorRow("Velvet Scale B"     , 0.0f, 1.0f, true);
  d->velvetBiasR        = makeFloatEditorRow("Velvet Bias R"      , 0.0f, 1.0f, true);
  d->velvetBiasG        = makeFloatEditorRow("Velvet Bias G"      , 0.0f, 1.0f, true);
  d->velvetBiasB        = makeFloatEditorRow("Velvet Bias B"      , 0.0f, 1.0f, true);

  d->albedoHue          = makeFloatEditorRow("Albdeo Hue"         , 0.0f, 1.0f, true);

  d->scroll->setMinimumWidth(d->scrollContents->minimumSizeHint().width() + d->scroll->verticalScrollBar()->width());
}

//##################################################################################################
EditMaterialSwapParametersWidget::~EditMaterialSwapParametersWidget()
{
  delete d;
}

tp_math_utils::MaterialSwapParameters EditMaterialSwapParametersWidget::materialSwapParameters() const
{
  tp_math_utils::MaterialSwapParameters materialSwapParameters;

  materialSwapParameters.albedoUse.x      = d->albedoUseR.get();
  materialSwapParameters.albedoUse.y      = d->albedoUseG.get();
  materialSwapParameters.albedoUse.z      = d->albedoUseB.get();
  materialSwapParameters.albedoScale.x    = d->albedoScaleR.get();
  materialSwapParameters.albedoScale.y    = d->albedoScaleG.get();
  materialSwapParameters.albedoScale.z    = d->albedoScaleB.get();
  materialSwapParameters.albedoBias.x     = d->albedoBiasR.get();
  materialSwapParameters.albedoBias.y     = d->albedoBiasG.get();
  materialSwapParameters.albedoBias.z     = d->albedoBiasB.get();

  materialSwapParameters.sssUse.x         = d->sssUseR.get();
  materialSwapParameters.sssUse.y         = d->sssUseG.get();
  materialSwapParameters.sssUse.z         = d->sssUseB.get();
  materialSwapParameters.sssScale.x       = d->sssScaleR.get();
  materialSwapParameters.sssScale.y       = d->sssScaleG.get();
  materialSwapParameters.sssScale.z       = d->sssScaleB.get();
  materialSwapParameters.sssBias.x        = d->sssBiasR.get();
  materialSwapParameters.sssBias.y        = d->sssBiasG.get();
  materialSwapParameters.sssBias.z        = d->sssBiasB.get();

  materialSwapParameters.emissionUse.x    = d->emissionUseR.get();
  materialSwapParameters.emissionUse.y    = d->emissionUseG.get();
  materialSwapParameters.emissionUse.z    = d->emissionUseB.get();
  materialSwapParameters.emissionScale.x  = d->emissionScaleR.get();
  materialSwapParameters.emissionScale.y  = d->emissionScaleG.get();
  materialSwapParameters.emissionScale.z  = d->emissionScaleB.get();
  materialSwapParameters.emissionBias.x   = d->emissionBiasR.get();
  materialSwapParameters.emissionBias.y   = d->emissionBiasG.get();
  materialSwapParameters.emissionBias.z   = d->emissionBiasB.get();

  materialSwapParameters.velvetUse.x      = d->velvetUseR.get();
  materialSwapParameters.velvetUse.y      = d->velvetUseG.get();
  materialSwapParameters.velvetUse.z      = d->velvetUseB.get();
  materialSwapParameters.velvetScale.x    = d->velvetScaleR.get();
  materialSwapParameters.velvetScale.y    = d->velvetScaleG.get();
  materialSwapParameters.velvetScale.z    = d->velvetScaleB.get();
  materialSwapParameters.velvetBias.x     = d->velvetBiasR.get();
  materialSwapParameters.velvetBias.y     = d->velvetBiasG.get();
  materialSwapParameters.velvetBias.z     = d->velvetBiasB.get();

  materialSwapParameters.albedoHue        = d->albedoHue.get();

  return materialSwapParameters;
}

glm::vec3 EditMaterialSwapParametersWidget::swapColor() const
{
  return d->swapColor;
}

}
