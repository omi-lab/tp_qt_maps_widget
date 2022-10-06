#include "tp_qt_maps_widget/EditMaterialSwapParametersWidget.h"

#include "tp_qt_maps/ConvertTexture.h"

#include <QBoxLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QPushButton>
#include <QColorDialog>
#include <QPainter>
#include <QScrollArea>
#include <QScrollBar>
#include <QCheckBox>

namespace tp_qt_maps_widget
{

namespace
{
//##################################################################################################
struct FloatEditor
{
  std::function<float()> get;
  std::function<void(float)> set;
  QSlider* slider;
};

//##################################################################################################
struct BoolEditor
{
  std::function<bool()> get;
  std::function<void(bool)> set;
  QCheckBox* checkbox;
};
}

//##################################################################################################
struct EditMaterialSwapParametersWidget::Private
{
  QScrollArea* scroll{nullptr};
  QWidget* scrollContents{nullptr};

  QPushButton* colorButton  {nullptr};

  glm::vec3 initialColor{1.0f, 1.0f, 1.0f};

  FloatEditor albedoUse;
  FloatEditor albedoUseR;
  FloatEditor albedoUseG;
  FloatEditor albedoUseB;

  FloatEditor albedoScale;
  FloatEditor albedoScaleR;
  FloatEditor albedoScaleG;
  FloatEditor albedoScaleB;

  FloatEditor albedoBias;
  FloatEditor albedoBiasR;
  FloatEditor albedoBiasG;
  FloatEditor albedoBiasB;

  FloatEditor sssUse;
  FloatEditor sssUseR;
  FloatEditor sssUseG;
  FloatEditor sssUseB;

  FloatEditor sssScale;
  FloatEditor sssScaleR;
  FloatEditor sssScaleG;
  FloatEditor sssScaleB;

  FloatEditor sssBias;
  FloatEditor sssBiasR;
  FloatEditor sssBiasG;
  FloatEditor sssBiasB;

  FloatEditor emissionUse;
  FloatEditor emissionUseR;
  FloatEditor emissionUseG;
  FloatEditor emissionUseB;

  FloatEditor emissionScale;
  FloatEditor emissionScaleR;
  FloatEditor emissionScaleG;
  FloatEditor emissionScaleB;

  FloatEditor emissionBias;
  FloatEditor emissionBiasR;
  FloatEditor emissionBiasG;
  FloatEditor emissionBiasB;

  FloatEditor velvetUse;
  FloatEditor velvetUseR;
  FloatEditor velvetUseG;
  FloatEditor velvetUseB;

  FloatEditor velvetScale;
  FloatEditor velvetScaleR;
  FloatEditor velvetScaleG;
  FloatEditor velvetScaleB;

  FloatEditor velvetBias;
  FloatEditor velvetBiasR;
  FloatEditor velvetBiasG;
  FloatEditor velvetBiasB;

  BoolEditor albedoHueCheckbox;

  FloatEditor albedoSaturationUse;
  FloatEditor albedoSaturationScale;
  FloatEditor albedoSaturationBias;

  FloatEditor albedoValueUse;
  FloatEditor albedoValueScale;
  FloatEditor albedoValueBias;

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

    colorButton->setIcon(makeIcon(initialColor));
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

  {

    auto makeFloatSlider = [&](QBoxLayout* layout, const QString& title, float min, float scaleMax, bool linear, int labelWidth = 120, Qt::Alignment labelAlign = Qt::AlignLeft,  QAbstractButton* button = nullptr)
    {
      float scale = scaleMax - min;

      FloatEditor floatEditor;

      auto hLayout = new QHBoxLayout();
      hLayout->setContentsMargins(0,0,0,0);
      layout->addLayout(hLayout);

      QLabel* label = new QLabel(title);
      label->setFixedWidth(labelWidth);
      label->setAlignment( labelAlign | Qt::AlignVCenter);

      hLayout->addWidget(label, 0, Qt::AlignLeft);

      if( button ) {
        hLayout->addWidget(button);
      }

      auto spin = new QDoubleSpinBox();
      spin->setRange(double(min), double(scaleMax));
      spin->setDecimals(3);
      spin->setSingleStep(0.01);
      spin->setFixedWidth(100);
      hLayout->addWidget(spin, 1);

      auto slider = new QSlider(Qt::Horizontal);
      slider->setRange(0, 100000);
      hLayout->addWidget(slider, Qt::AlignRight);
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

      floatEditor.slider = slider;

      return floatEditor;
    };

    auto makeCheckbox =[&](QBoxLayout* layout, const QString& name, bool checked)
    {
      BoolEditor boolEditor;

      boolEditor.checkbox = new QCheckBox(name, this);
      boolEditor.checkbox->setChecked(checked);

      layout->addWidget(boolEditor.checkbox, 0, Qt::AlignTop);

      boolEditor.get = [=]{return boolEditor.checkbox->isChecked();};
      boolEditor.set = [=](bool b)
      {
        boolEditor.checkbox->setChecked(b);
      };

      return boolEditor;
    };

    auto makeColorEdit = [&](QBoxLayout* layout, const QString& text, const std::function<glm::vec3&()>& getColor )
    {
      auto button = new QPushButton(text);
      button->setStyleSheet("text-align:left; padding-left:2;");
      layout->addWidget(button, 2);

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

    auto addTitle = [&](QBoxLayout* layout, const QString& name)
    {
      layout->addWidget(new QLabel(QString("<h2>%1</h2>").arg(name)), 2, Qt::AlignLeft);
    };

    auto addSubTitle = [&](QBoxLayout* layout, const QString& name)
    {
      layout->addWidget(new QLabel(QString("<h3>%1</h3>").arg(name)), 2, Qt::AlignLeft);
    };

    auto addExpandIcon = [&](QBoxLayout* layout)
    {
      QIcon normalIcon = tp_qt_maps::loadIconFromResource("/omi_scene_builder/right_chevron.png");
      QIcon expandedIcon = tp_qt_maps::loadIconFromResource("/omi_scene_builder/down_chevron.png");

      QPushButton* button = new QPushButton(normalIcon,"");
      QString buttonStyle = "QPushButton{border:none;background-color:rgba(255, 255, 255,0);}";
      button->setStyleSheet(buttonStyle); // Style sheet
      int size = 16;
      button->setIconSize(QSize(size,size));
      button->setMinimumSize(size,size);
      button->setMaximumSize(size,size);
      layout->addWidget(button);// The horizontal layout

      button->setCheckable(true);

      connect( button, &QAbstractButton::toggled, button, [=]( bool b) {
        b ? button->setIcon(expandedIcon) : button->setIcon(normalIcon);
      });

      return button;
    };

    addTitle(l, "Material Swap Properties");

    d->colorButton   = makeColorEdit(l, "Color", [&]()->glm::vec3&{return d->initialColor;});
    d->updateColors();

    addSubTitle(l, "Albedo");
    //--------------------------------ALBEDO USE--------------------------------------
    auto albedoUseExpandButton = addExpandIcon(l);
    d->albedoUse = makeFloatSlider( l, "Use", 0.0f, 1.0f, true, 50, Qt::AlignLeft, albedoUseExpandButton );

    auto albedoUseHLayout = new QHBoxLayout();
    albedoUseHLayout->setContentsMargins(0,0,0,0);
    l->addLayout(albedoUseHLayout);

    QWidget* albedoUseRGBGroup = new QWidget(this);
    auto albedoUseVLayout = new QVBoxLayout(albedoUseRGBGroup);
    albedoUseVLayout->setContentsMargins(0,0,0,0);
    d->albedoUseR = makeFloatSlider( albedoUseVLayout, "R", 0.0f, 1.0f, true, 72, Qt::AlignRight);
    d->albedoUseG = makeFloatSlider( albedoUseVLayout, "G", 0.0f, 1.0f, true, 72, Qt::AlignRight);
    d->albedoUseB = makeFloatSlider( albedoUseVLayout, "B", 0.0f, 1.0f, true, 72, Qt::AlignRight);

    // connect use slider to the set function of all the sliders
    connect( d->albedoUse.slider, &QSlider::valueChanged, albedoUseRGBGroup, [=] {
      d->albedoUseR.set( d->albedoUse.get() );
      d->albedoUseG.set( d->albedoUse.get() );
      d->albedoUseB.set( d->albedoUse.get() );
    } );

    albedoUseHLayout->addWidget(albedoUseRGBGroup);

    connect(albedoUseExpandButton, &QAbstractButton::toggled, albedoUseRGBGroup, [=] {
      albedoUseRGBGroup->setVisible( albedoUseExpandButton->isChecked() );
    });

    albedoUseRGBGroup->setVisible(false);

    //--------------------------------ALBEDO SCALE------------------------------------
    auto albedoScaleExpandButton = addExpandIcon(l);
    d->albedoScale = makeFloatSlider( l, "Scale", 0.0f, 1.0f, true, 50, Qt::AlignLeft, albedoScaleExpandButton );
    d->albedoScale.set(1.0f);

    auto albedoScaleHLayout = new QHBoxLayout();
    albedoScaleHLayout->setContentsMargins(0,0,0,0);
    l->addLayout(albedoScaleHLayout);

    QWidget* albedoScaleRGBGroup = new QWidget(this);
    auto albedoScaleVLayout = new QVBoxLayout(albedoScaleRGBGroup);
    albedoScaleVLayout->setContentsMargins(0,0,0,0);
    d->albedoScaleR = makeFloatSlider( albedoScaleVLayout, "R", 0.0f, 1.0f, true, 72, Qt::AlignRight);
    d->albedoScaleG = makeFloatSlider( albedoScaleVLayout, "G", 0.0f, 1.0f, true, 72, Qt::AlignRight);
    d->albedoScaleB = makeFloatSlider( albedoScaleVLayout, "B", 0.0f, 1.0f, true, 72, Qt::AlignRight);

    // connect use slider to the set function of all the sliders
    connect( d->albedoScale.slider, &QSlider::valueChanged, albedoScaleRGBGroup, [=] {
      d->albedoScaleR.set( d->albedoScale.get() );
      d->albedoScaleG.set( d->albedoScale.get() );
      d->albedoScaleB.set( d->albedoScale.get() );
    } );

    albedoScaleHLayout->addWidget(albedoScaleRGBGroup);

    connect(albedoScaleExpandButton, &QAbstractButton::toggled, albedoScaleRGBGroup, [=] {
      albedoScaleRGBGroup->setVisible(albedoScaleExpandButton->isChecked());
    });

    albedoScaleRGBGroup->setVisible(false);

    //--------------------------------ALBEDO BIAS------------------------------------
    auto albedoBiasExpandButton = addExpandIcon(l);
    d->albedoBias = makeFloatSlider( l, "Bias", 0.0f, 1.0f, true, 50, Qt::AlignLeft, albedoBiasExpandButton );
    d->albedoBias.set(0.0f);

    auto albedoBiasHLayout = new QHBoxLayout();
    albedoBiasHLayout->setContentsMargins(0,0,0,0);
    l->addLayout(albedoBiasHLayout);

    QWidget* albedoBiasRGBGroup = new QWidget(this);
    auto albedoBiasVLayout = new QVBoxLayout(albedoBiasRGBGroup);
    albedoBiasVLayout->setContentsMargins(0,0,0,0);
    d->albedoBiasR = makeFloatSlider( albedoBiasVLayout, "R", 0.0f, 1.0f, true, 72, Qt::AlignRight);
    d->albedoBiasG = makeFloatSlider( albedoBiasVLayout, "G", 0.0f, 1.0f, true, 72, Qt::AlignRight);
    d->albedoBiasB = makeFloatSlider( albedoBiasVLayout, "B", 0.0f, 1.0f, true, 72, Qt::AlignRight);

    // connect use slider to the set function of all the sliders
    connect( d->albedoBias.slider, &QSlider::valueChanged, albedoBiasRGBGroup, [=] {
      d->albedoBiasR.set( d->albedoBias.get() );
      d->albedoBiasG.set( d->albedoBias.get() );
      d->albedoBiasB.set( d->albedoBias.get() );
    } );

    albedoBiasHLayout->addWidget(albedoBiasRGBGroup);

    connect(albedoBiasExpandButton, &QAbstractButton::toggled, albedoBiasRGBGroup, [=] {
      albedoBiasRGBGroup->setVisible(albedoBiasExpandButton->isChecked());
    });

    albedoBiasRGBGroup->setVisible(false);

    addSubTitle(l, "SSS");
    //--------------------------------SSS USE--------------------------------------
    auto sssUseExpandButton = addExpandIcon(l);
    d->sssUse = makeFloatSlider( l, "Use", 0.0f, 1.0f, true, 50, Qt::AlignLeft, sssUseExpandButton );

    auto sssUseHLayout = new QHBoxLayout();
    sssUseHLayout->setContentsMargins(0,0,0,0);
    l->addLayout(sssUseHLayout);

    QWidget* sssUseRGBGroup = new QWidget(this);
    auto sssUseVLayout = new QVBoxLayout(sssUseRGBGroup);
    sssUseVLayout->setContentsMargins(0,0,0,0);
    d->sssUseR = makeFloatSlider( sssUseVLayout, "R", 0.0f, 1.0f, true, 72, Qt::AlignRight);
    d->sssUseG = makeFloatSlider( sssUseVLayout, "G", 0.0f, 1.0f, true, 72, Qt::AlignRight);
    d->sssUseB = makeFloatSlider( sssUseVLayout, "B", 0.0f, 1.0f, true, 72, Qt::AlignRight);

    // connect use slider to the set function of all the sliders
    connect( d->sssUse.slider, &QSlider::valueChanged, sssUseRGBGroup, [=] {
      d->sssUseR.set( d->sssUse.get() );
      d->sssUseG.set( d->sssUse.get() );
      d->sssUseB.set( d->sssUse.get() );
    } );

    sssUseHLayout->addWidget(sssUseRGBGroup);

    connect( sssUseExpandButton, &QAbstractButton::toggled, sssUseRGBGroup, [=] {
      sssUseRGBGroup->setVisible(sssUseExpandButton->isChecked());
    });

    sssUseRGBGroup->setVisible(false);

    //--------------------------------SSS SCALE------------------------------------
    auto sssScaleExpandButton = addExpandIcon(l);
    d->sssScale = makeFloatSlider( l, "Scale", 0.0f, 1.0f, true, 50, Qt::AlignLeft, sssScaleExpandButton );
    d->sssScale.set(1.0f);

    auto sssScaleHLayout = new QHBoxLayout();
    sssScaleHLayout->setContentsMargins(0,0,0,0);
    l->addLayout(sssScaleHLayout);

    QWidget* sssScaleRGBGroup = new QWidget(this);
    auto sssScaleVLayout = new QVBoxLayout(sssScaleRGBGroup);
    sssScaleVLayout->setContentsMargins(0,0,0,0);
    d->sssScaleR = makeFloatSlider( sssScaleVLayout, "R", 0.0f, 1.0f, true, 72, Qt::AlignRight);
    d->sssScaleG = makeFloatSlider( sssScaleVLayout, "G", 0.0f, 1.0f, true, 72, Qt::AlignRight);
    d->sssScaleB = makeFloatSlider( sssScaleVLayout, "B", 0.0f, 1.0f, true, 72, Qt::AlignRight);

    // connect use slider to the set function of all the sliders
    connect( d->sssScale.slider, &QSlider::valueChanged, sssScaleRGBGroup, [=] {
      d->sssScaleR.set( d->sssScale.get() );
      d->sssScaleG.set( d->sssScale.get() );
      d->sssScaleB.set( d->sssScale.get() );
    } );

    sssScaleHLayout->addWidget(sssScaleRGBGroup);

    connect( sssScaleExpandButton, &QAbstractButton::toggled, sssScaleRGBGroup, [=] {
      sssScaleRGBGroup->setVisible(sssScaleExpandButton->isChecked());
    });

    sssScaleRGBGroup->setVisible(false);

    //--------------------------------SSS BIAS------------------------------------
    auto sssBiasExpandButton = addExpandIcon(l);
    d->sssBias = makeFloatSlider( l, "Bias", 0.0f, 1.0f, true, 50, Qt::AlignLeft, sssBiasExpandButton );
    d->sssBias.set(0.0f);

    auto sssBiasHLayout = new QHBoxLayout();
    sssBiasHLayout->setContentsMargins(0,0,0,0);
    l->addLayout(sssBiasHLayout);

    QWidget* sssBiasRGBGroup = new QWidget(this);
    auto sssBiasVLayout = new QVBoxLayout(sssBiasRGBGroup);
    sssBiasVLayout->setContentsMargins(0,0,0,0);
    d->sssBiasR = makeFloatSlider( sssBiasVLayout, "R", 0.0f, 1.0f, true, 72, Qt::AlignRight);
    d->sssBiasG = makeFloatSlider( sssBiasVLayout, "G", 0.0f, 1.0f, true, 72, Qt::AlignRight);
    d->sssBiasB = makeFloatSlider( sssBiasVLayout, "B", 0.0f, 1.0f, true, 72, Qt::AlignRight);

    // connect use slider to the set function of all the sliders
    connect( d->sssBias.slider, &QSlider::valueChanged, sssBiasRGBGroup, [=] {
      d->sssBiasR.set( d->sssBias.get() );
      d->sssBiasG.set( d->sssBias.get() );
      d->sssBiasB.set( d->sssBias.get() );
    } );

    sssBiasHLayout->addWidget(sssBiasRGBGroup);

    connect( sssBiasExpandButton, &QAbstractButton::toggled, sssBiasRGBGroup, [=] {
      sssBiasRGBGroup->setVisible(sssBiasExpandButton->isChecked());
    });

    sssBiasRGBGroup->setVisible(false);

    addSubTitle(l, "Emission");
    //--------------------------------EMISSION USE--------------------------------------
    auto emissionUseExpandButton = addExpandIcon(l);
    d->emissionUse = makeFloatSlider( l, "Use", 0.0f, 1.0f, true, 50, Qt::AlignLeft, emissionUseExpandButton );

    auto emissionUseHLayout = new QHBoxLayout();
    emissionUseHLayout->setContentsMargins(0,0,0,0);
    l->addLayout(emissionUseHLayout);

    QWidget* emissionUseRGBGroup = new QWidget(this);
    auto emissionUseVLayout = new QVBoxLayout(emissionUseRGBGroup);
    emissionUseVLayout->setContentsMargins(0,0,0,0);
    d->emissionUseR = makeFloatSlider( emissionUseVLayout, "R", 0.0f, 1.0f, true, 72, Qt::AlignRight);
    d->emissionUseG = makeFloatSlider( emissionUseVLayout, "G", 0.0f, 1.0f, true, 72, Qt::AlignRight);
    d->emissionUseB = makeFloatSlider( emissionUseVLayout, "B", 0.0f, 1.0f, true, 72, Qt::AlignRight);

    // connect use slider to the set function of all the sliders
    connect( d->emissionUse.slider, &QSlider::valueChanged, emissionUseRGBGroup, [=] {
      d->emissionUseR.set( d->emissionUse.get() );
      d->emissionUseG.set( d->emissionUse.get() );
      d->emissionUseB.set( d->emissionUse.get() );
    } );

    emissionUseHLayout->addWidget(emissionUseRGBGroup);

    connect( emissionUseExpandButton, &QAbstractButton::toggled, emissionUseRGBGroup, [=] {
      emissionUseRGBGroup->setVisible(emissionUseExpandButton->isChecked());
    });

    emissionUseRGBGroup->setVisible(false);

    //--------------------------------EMISSION SCALE------------------------------------
    auto emissionScaleExpandButton = addExpandIcon(l);
    d->emissionScale = makeFloatSlider( l, "Scale", 0.0f, 1.0f, true, 50, Qt::AlignLeft, emissionScaleExpandButton );
    d->emissionScale.set(1.0f);

    auto emissionScaleHLayout = new QHBoxLayout();
    emissionScaleHLayout->setContentsMargins(0,0,0,0);
    l->addLayout(emissionScaleHLayout);

    QWidget* emissionScaleRGBGroup = new QWidget(this);
    auto emissionScaleVLayout = new QVBoxLayout(emissionScaleRGBGroup);
    emissionScaleVLayout->setContentsMargins(0,0,0,0);
    d->emissionScaleR = makeFloatSlider( emissionScaleVLayout, "R", 0.0f, 1.0f, true, 72, Qt::AlignRight);
    d->emissionScaleG = makeFloatSlider( emissionScaleVLayout, "G", 0.0f, 1.0f, true, 72, Qt::AlignRight);
    d->emissionScaleB = makeFloatSlider( emissionScaleVLayout, "B", 0.0f, 1.0f, true, 72, Qt::AlignRight);

    // connect use slider to the set function of all the sliders
    connect( d->emissionScale.slider, &QSlider::valueChanged, emissionScaleRGBGroup, [=] {
      d->emissionScaleR.set( d->emissionScale.get() );
      d->emissionScaleG.set( d->emissionScale.get() );
      d->emissionScaleB.set( d->emissionScale.get() );
    } );

    emissionScaleHLayout->addWidget(emissionScaleRGBGroup);

    connect( emissionScaleExpandButton, &QAbstractButton::toggled, emissionScaleRGBGroup, [=] {
      emissionScaleRGBGroup->setVisible(emissionScaleExpandButton->isChecked());
    });

    emissionScaleRGBGroup->setVisible(false);

    //--------------------------------EMISSION BIAS------------------------------------
    auto emissionBiasExpandButton = addExpandIcon(l);
    d->emissionBias = makeFloatSlider( l, "Bias", 0.0f, 1.0f, true, 50, Qt::AlignLeft, emissionBiasExpandButton );
    d->emissionBias.set(0.0f);

    auto emissionBiasHLayout = new QHBoxLayout();
    emissionBiasHLayout->setContentsMargins(0,0,0,0);
    l->addLayout(emissionBiasHLayout);

    QWidget* emissionBiasRGBGroup = new QWidget(this);
    auto emissionBiasVLayout = new QVBoxLayout(emissionBiasRGBGroup);
    emissionBiasVLayout->setContentsMargins(0,0,0,0);
    d->emissionBiasR = makeFloatSlider( emissionBiasVLayout, "R", 0.0f, 1.0f, true, 72, Qt::AlignRight);
    d->emissionBiasG = makeFloatSlider( emissionBiasVLayout, "G", 0.0f, 1.0f, true, 72, Qt::AlignRight);
    d->emissionBiasB = makeFloatSlider( emissionBiasVLayout, "B", 0.0f, 1.0f, true, 72, Qt::AlignRight);

    // connect use slider to the set function of all the sliders
    connect( d->emissionBias.slider, &QSlider::valueChanged, emissionBiasRGBGroup, [=] {
      d->emissionBiasR.set( d->emissionBias.get() );
      d->emissionBiasG.set( d->emissionBias.get() );
      d->emissionBiasB.set( d->emissionBias.get() );
    } );

    emissionBiasHLayout->addWidget(emissionBiasRGBGroup);

    connect( emissionBiasExpandButton, &QAbstractButton::toggled, emissionBiasRGBGroup, [=] {
      emissionBiasRGBGroup->setVisible(emissionBiasExpandButton->isChecked());
    });

    emissionBiasRGBGroup->setVisible(false);

    addSubTitle(l, "Velvet");
    //--------------------------------VELVET USE--------------------------------------
    auto velvetUseExpandButton = addExpandIcon(l);
    d->velvetUse = makeFloatSlider( l, "Use", 0.0f, 1.0f, true, 50, Qt::AlignLeft, velvetUseExpandButton );

    auto velvetUseHLayout = new QHBoxLayout();
    velvetUseHLayout->setContentsMargins(0,0,0,0);
    l->addLayout(velvetUseHLayout);

    QWidget* velvetUseRGBGroup = new QWidget(this);
    auto velvetUseVLayout = new QVBoxLayout(velvetUseRGBGroup);
    velvetUseVLayout->setContentsMargins(0,0,0,0);
    d->velvetUseR = makeFloatSlider( velvetUseVLayout, "R", 0.0f, 1.0f, true, 72, Qt::AlignRight);
    d->velvetUseG = makeFloatSlider( velvetUseVLayout, "G", 0.0f, 1.0f, true, 72, Qt::AlignRight);
    d->velvetUseB = makeFloatSlider( velvetUseVLayout, "B", 0.0f, 1.0f, true, 72, Qt::AlignRight);

    // connect use slider to the set function of all the sliders
    connect( d->velvetUse.slider, &QSlider::valueChanged, velvetUseRGBGroup, [=] {
      d->velvetUseR.set( d->velvetUse.get() );
      d->velvetUseG.set( d->velvetUse.get() );
      d->velvetUseB.set( d->velvetUse.get() );
    } );

    velvetUseHLayout->addWidget(velvetUseRGBGroup);

    connect( velvetUseExpandButton, &QAbstractButton::toggled, velvetUseRGBGroup, [=] {
      velvetUseRGBGroup->setVisible(velvetUseExpandButton->isChecked());
    });

    velvetUseRGBGroup->setVisible(false);

    //--------------------------------VELVET SCALE------------------------------------
    auto velvetScaleExpandButton = addExpandIcon(l);
    d->velvetScale = makeFloatSlider( l, "Scale", 0.0f, 1.0f, true, 50, Qt::AlignLeft, velvetScaleExpandButton );
    d->velvetScale.set(1.0f);

    auto velvetScaleHLayout = new QHBoxLayout();
    velvetScaleHLayout->setContentsMargins(0,0,0,0);
    l->addLayout(velvetScaleHLayout);

    QWidget* velvetScaleRGBGroup = new QWidget(this);
    auto velvetScaleVLayout = new QVBoxLayout(velvetScaleRGBGroup);
    velvetScaleVLayout->setContentsMargins(0,0,0,0);
    d->velvetScaleR = makeFloatSlider( velvetScaleVLayout, "R", 0.0f, 1.0f, true, 72, Qt::AlignRight);
    d->velvetScaleG = makeFloatSlider( velvetScaleVLayout, "G", 0.0f, 1.0f, true, 72, Qt::AlignRight);
    d->velvetScaleB = makeFloatSlider( velvetScaleVLayout, "B", 0.0f, 1.0f, true, 72, Qt::AlignRight);

    // connect use slider to the set function of all the sliders
    connect( d->velvetScale.slider, &QSlider::valueChanged, velvetScaleRGBGroup, [=] {
      d->velvetScaleR.set( d->velvetScale.get() );
      d->velvetScaleG.set( d->velvetScale.get() );
      d->velvetScaleB.set( d->velvetScale.get() );
    } );

    velvetScaleHLayout->addWidget(velvetScaleRGBGroup);

    connect(velvetScaleExpandButton, &QAbstractButton::toggled, velvetScaleRGBGroup, [=] {
      velvetScaleRGBGroup->setVisible(velvetScaleExpandButton->isChecked());
    });

    velvetScaleRGBGroup->setVisible(false);

    //--------------------------------VELVET BIAS------------------------------------
    auto velvetBiasExpandButton = addExpandIcon(l);
    d->velvetBias = makeFloatSlider( l, "Bias", 0.0f, 1.0f, true, 50, Qt::AlignLeft, velvetBiasExpandButton );
    d->velvetBias.set(0.0f);

    auto velvetBiasHLayout = new QHBoxLayout();
    velvetBiasHLayout->setContentsMargins(0,0,0,0);
    l->addLayout(velvetBiasHLayout);

    QWidget* velvetBiasRGBGroup = new QWidget(this);
    auto velvetBiasVLayout = new QVBoxLayout(velvetBiasRGBGroup);
    velvetBiasVLayout->setContentsMargins(0,0,0,0);
    d->velvetBiasR = makeFloatSlider( velvetBiasVLayout, "R", 0.0f, 1.0f, true, 72, Qt::AlignRight);
    d->velvetBiasG = makeFloatSlider( velvetBiasVLayout, "G", 0.0f, 1.0f, true, 72, Qt::AlignRight);
    d->velvetBiasB = makeFloatSlider( velvetBiasVLayout, "B", 0.0f, 1.0f, true, 72, Qt::AlignRight);

    // connect use slider to the set function of all the sliders
    connect( d->velvetBias.slider, &QSlider::valueChanged, velvetBiasRGBGroup, [=] {
      d->velvetBiasR.set( d->velvetBias.get() );
      d->velvetBiasG.set( d->velvetBias.get() );
      d->velvetBiasB.set( d->velvetBias.get() );
    } );

    velvetBiasHLayout->addWidget(velvetBiasRGBGroup);

    connect( velvetBiasExpandButton, &QCheckBox::toggled, velvetBiasRGBGroup, [=] {
      velvetBiasRGBGroup->setVisible(velvetBiasExpandButton->isChecked());
    });

    velvetBiasRGBGroup->setVisible(false);

    //--------------------------------HSV--------------------------------------------
    addTitle(l, "HSV");
    //--------------------------------ALBEDO HUE ------------------------------------
    d->albedoHueCheckbox = makeCheckbox( l, "Use Albedo Hue", true);
    connect(d->albedoHueCheckbox.checkbox, &QCheckBox::toggled, this, &EditMaterialSwapParametersWidget::materialSwapParametersEdited);

    //--------------------------------ALBEDO SATURATION ------------------------------------
    addSubTitle(l, "Albedo saturation");
    d->albedoSaturationUse = makeFloatSlider( l, "Use", 0.0f, 1.0f, true, 50, Qt::AlignLeft );
    d->albedoSaturationScale = makeFloatSlider( l, "Scale", 0.0f, 1.0f, true, 50, Qt::AlignLeft );
    d->albedoSaturationBias = makeFloatSlider( l, "Bias", 0.0f, 1.0f, true, 50, Qt::AlignLeft );

    //--------------------------------ALBEDO VALUE------------------------------------------
    addSubTitle(l, "Albedo value");
    d->albedoValueUse = makeFloatSlider( l, "Use", 0.0f, 1.0f, true, 50, Qt::AlignLeft );
    d->albedoValueScale = makeFloatSlider( l, "Scale", 0.0f, 1.0f, true, 50, Qt::AlignLeft );
    d->albedoValueBias = makeFloatSlider( l, "Bias", 0.0f, 1.0f, true, 50, Qt::AlignLeft );
  }

  d->scroll->setMinimumWidth(d->scrollContents->minimumSizeHint().width() + d->scroll->verticalScrollBar()->width());
}

//##################################################################################################
EditMaterialSwapParametersWidget::~EditMaterialSwapParametersWidget()
{
  delete d;
}

//################################################################################################
void EditMaterialSwapParametersWidget::setMaterialSwapParameters(const tp_math_utils::MaterialSwapParameters& materialSwapParameters)
{
  blockSignals(true);
  TP_CLEANUP([&]{blockSignals(false);});

  d->albedoUseR.set(materialSwapParameters.albedoUse.x);
  d->albedoUseG.set(materialSwapParameters.albedoUse.y);
  d->albedoUseB.set(materialSwapParameters.albedoUse.z);
  d->albedoScaleR.set(materialSwapParameters.albedoScale.x);
  d->albedoScaleG.set(materialSwapParameters.albedoScale.y);
  d->albedoScaleB.set(materialSwapParameters.albedoScale.z);
  d->albedoBiasR.set(materialSwapParameters.albedoBias.x);
  d->albedoBiasG.set(materialSwapParameters.albedoBias.y);
  d->albedoBiasB.set(materialSwapParameters.albedoBias.z);

  d->sssUseR.set(materialSwapParameters.sssUse.x);
  d->sssUseG.set(materialSwapParameters.sssUse.y);
  d->sssUseB.set(materialSwapParameters.sssUse.z);
  d->sssScaleR.set(materialSwapParameters.sssScale.x);
  d->sssScaleG.set(materialSwapParameters.sssScale.y);
  d->sssScaleB.set(materialSwapParameters.sssScale.z);
  d->sssBiasR.set(materialSwapParameters.sssBias.x);
  d->sssBiasG.set(materialSwapParameters.sssBias.y);
  d->sssBiasB.set(materialSwapParameters.sssBias.z);

  d->emissionUseR.set(materialSwapParameters.emissionUse.x);
  d->emissionUseG.set(materialSwapParameters.emissionUse.y);
  d->emissionUseB.set(materialSwapParameters.emissionUse.z);
  d->emissionScaleR.set(materialSwapParameters.emissionScale.x);
  d->emissionScaleG.set(materialSwapParameters.emissionScale.y);
  d->emissionScaleB.set(materialSwapParameters.emissionScale.z);
  d->emissionBiasR.set(materialSwapParameters.emissionBias.x);
  d->emissionBiasG.set(materialSwapParameters.emissionBias.y);
  d->emissionBiasB.set(materialSwapParameters.emissionBias.z);

  d->velvetUseR.set(materialSwapParameters.velvetUse.x);
  d->velvetUseG.set(materialSwapParameters.velvetUse.y);
  d->velvetUseB.set(materialSwapParameters.velvetUse.z);
  d->velvetScaleR.set(materialSwapParameters.velvetScale.x);
  d->velvetScaleG.set(materialSwapParameters.velvetScale.y);
  d->velvetScaleB.set(materialSwapParameters.velvetScale.z);
  d->velvetBiasR.set(materialSwapParameters.velvetBias.x);
  d->velvetBiasG.set(materialSwapParameters.velvetBias.y);
  d->velvetBiasB.set(materialSwapParameters.velvetBias.z);

  d->albedoHueCheckbox.set(materialSwapParameters.useAlbedoHue);

  d->initialColor = materialSwapParameters.initialColor;

  d->albedoSaturationUse   .set(materialSwapParameters.albedoSaturationUse);
  d->albedoSaturationScale .set(materialSwapParameters.albedoSaturationScale);
  d->albedoSaturationBias  .set(materialSwapParameters.albedoSaturationBias);

  d->albedoValueUse   .set(materialSwapParameters.albedoValueUse);
  d->albedoValueScale .set(materialSwapParameters.albedoValueScale);
  d->albedoValueBias  .set(materialSwapParameters.albedoValueBias);
  d->updateColors();
}

//##################################################################################################
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

  materialSwapParameters.useAlbedoHue     = d->albedoHueCheckbox.get();

  materialSwapParameters.initialColor     = d->initialColor;

  materialSwapParameters.albedoSaturationUse       = d->albedoSaturationUse.get();
  materialSwapParameters.albedoSaturationScale     = d->albedoSaturationScale.get();
  materialSwapParameters.albedoSaturationBias      = d->albedoSaturationBias.get();

  materialSwapParameters.albedoValueUse        = d->albedoValueUse.get();
  materialSwapParameters.albedoValueScale      = d->albedoValueScale.get();
  materialSwapParameters.albedoValueBias       = d->albedoValueBias.get();

  return materialSwapParameters;
}

}
