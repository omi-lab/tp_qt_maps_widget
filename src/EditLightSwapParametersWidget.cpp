#include "tp_qt_maps_widget/EditLightSwapParametersWidget.h"

#include "tp_qt_maps/ConvertTexture.h"

#include <algorithm>

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
  QDoubleSpinBox* spin;
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
struct EditLightSwapParametersWidget::Private
{
  QScrollArea* scroll{nullptr};
  QWidget* scrollContents{nullptr};

  FloatEditor ambientUse;
  FloatEditor ambientUseR;
  FloatEditor ambientUseG;
  FloatEditor ambientUseB;

  FloatEditor ambientScale;
  FloatEditor ambientScaleR;
  FloatEditor ambientScaleG;
  FloatEditor ambientScaleB;

  FloatEditor ambientBias;
  FloatEditor ambientBiasR;
  FloatEditor ambientBiasG;
  FloatEditor ambientBiasB;

  FloatEditor diffuseUse;
  FloatEditor diffuseUseR;
  FloatEditor diffuseUseG;
  FloatEditor diffuseUseB;

  FloatEditor diffuseScale;
  FloatEditor diffuseScaleR;
  FloatEditor diffuseScaleG;
  FloatEditor diffuseScaleB;

  FloatEditor diffuseBias;
  FloatEditor diffuseBiasR;
  FloatEditor diffuseBiasG;
  FloatEditor diffuseBiasB;

  FloatEditor specularUse;
  FloatEditor specularUseR;
  FloatEditor specularUseG;
  FloatEditor specularUseB;

  FloatEditor specularScale;
  FloatEditor specularScaleR;
  FloatEditor specularScaleG;
  FloatEditor specularScaleB;

  FloatEditor specularBias;
  FloatEditor specularBiasR;
  FloatEditor specularBiasG;
  FloatEditor specularBiasB;

  FloatEditor offsetScaleUse;
  FloatEditor offsetScaleUseR;
  FloatEditor offsetScaleUseG;
  FloatEditor offsetScaleUseB;

  FloatEditor offsetScaleScale;
  FloatEditor offsetScaleScaleR;
  FloatEditor offsetScaleScaleG;
  FloatEditor offsetScaleScaleB;

  FloatEditor offsetScaleBias;
  FloatEditor offsetScaleBiasR;
  FloatEditor offsetScaleBiasG;
  FloatEditor offsetScaleBiasB;

  BoolEditor ambientHueCheckbox;

  FloatEditor diffuseScaleUse;
  FloatEditor diffuseScaleScale;
  FloatEditor diffuseScaleBias;

  FloatEditor spotLightBlendUse;
  FloatEditor spotLightBlendScale;
  FloatEditor spotLightBlendBias;

  FloatEditor fovUse;
  FloatEditor fovScale;
  FloatEditor fovBias;
};

//##################################################################################################
EditLightSwapParametersWidget::EditLightSwapParametersWidget( QWidget* parent):
  QWidget(parent),
  d(new Private())
{
  auto l = new QVBoxLayout(this);
  l->setContentsMargins(0,0,0,0);

  {
    auto ll = new QHBoxLayout();
    l->addLayout(ll);
    ll->setContentsMargins(0,0,0,0);
  }

  {

    auto makeFloatSlider = [&](QBoxLayout* layout, const QString& title, float min, float scaleMax, bool linear, int labelWidth = 120, Qt::Alignment labelAlign = Qt::AlignLeft,  QAbstractButton* button = nullptr, bool blockSignal = false)
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
      connect(slider, &QSlider::valueChanged, this, &EditLightSwapParametersWidget::lightSwapParametersEdited);

      connect(slider, &QSlider::valueChanged, this, [=]
      {
        float v = float(slider->value()) / 100000.0f;
        v = linear?((v*scale) + min):(v*v*scale);
        if(std::fabs(v-float(spin->value())) > 0.000001f)
        {
          spin->blockSignals(true);
          spin->setValue(double(v));
          spin->blockSignals(false);
          if(!blockSignal)
            Q_EMIT lightSwapParametersEdited();
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
          if(!blockSignal)
            Q_EMIT lightSwapParametersEdited();
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
      floatEditor.spin = spin;

      return floatEditor;
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

    auto emitParametersChangedAfter = [&](const std::function<void()>& closure)
    {
      blockSignals(true);
      closure();
      blockSignals(false);
      Q_EMIT lightSwapParametersEdited();
    };

    addTitle(l, "Light Swap Properties");
    addSubTitle(l, "Ambient");
    //--------------------------------AMBIENT USE--------------------------------------
    auto ambientUseExpandButton = addExpandIcon(l);
    d->ambientUse = makeFloatSlider( l, "Use", 0.0f, 1.0f, true, 50, Qt::AlignLeft, ambientUseExpandButton, true);

    auto ambientUseHLayout = new QHBoxLayout();
    ambientUseHLayout->setContentsMargins(0,0,0,0);
    l->addLayout(ambientUseHLayout);

    QWidget* ambientUseRGBGroup = new QWidget(this);
    auto ambientUseVLayout = new QVBoxLayout(ambientUseRGBGroup);
    ambientUseVLayout->setContentsMargins(0,0,0,0);
    d->ambientUseR = makeFloatSlider( ambientUseVLayout, "R", 0.0f, 1.0f, true, 72, Qt::AlignRight);
    d->ambientUseG = makeFloatSlider( ambientUseVLayout, "G", 0.0f, 1.0f, true, 72, Qt::AlignRight);
    d->ambientUseB = makeFloatSlider( ambientUseVLayout, "B", 0.0f, 1.0f, true, 72, Qt::AlignRight);

    // connect use slider to the set function of all the sliders
    connect( d->ambientUse.slider, &QSlider::valueChanged, ambientUseRGBGroup, [=] 
    {
      emitParametersChangedAfter([&]()
      {
        d->ambientUseR.set( d->ambientUse.get() );
        d->ambientUseG.set( d->ambientUse.get() );
        d->ambientUseB.set( d->ambientUse.get() );
      });
    });

    connect( d->ambientUse.spin, &QDoubleSpinBox::valueChanged, ambientUseRGBGroup, [=] 
    {
      emitParametersChangedAfter([&]()
      {
        d->ambientUseR.set( d->ambientUse.get() );
        d->ambientUseG.set( d->ambientUse.get() );
        d->ambientUseB.set( d->ambientUse.get() );
      });
    });

    ambientUseHLayout->addWidget(ambientUseRGBGroup);

    connect(ambientUseExpandButton, &QAbstractButton::toggled, ambientUseRGBGroup, [=] {
      ambientUseRGBGroup->setVisible( ambientUseExpandButton->isChecked() );
    });

    ambientUseRGBGroup->setVisible(false);

    //--------------------------------AMBIENT SCALE------------------------------------
    auto ambientScaleExpandButton = addExpandIcon(l);
    d->ambientScale = makeFloatSlider( l, "Scale", 0.0f, 1.0f, true, 50, Qt::AlignLeft, ambientScaleExpandButton, true);
    d->ambientScale.set(1.0f);

    auto ambientScaleHLayout = new QHBoxLayout();
    ambientScaleHLayout->setContentsMargins(0,0,0,0);
    l->addLayout(ambientScaleHLayout);

    QWidget* ambientScaleRGBGroup = new QWidget(this);
    auto ambientScaleVLayout = new QVBoxLayout(ambientScaleRGBGroup);
    ambientScaleVLayout->setContentsMargins(0,0,0,0);
    d->ambientScaleR = makeFloatSlider( ambientScaleVLayout, "R", 0.0f, 1.0f, true, 72, Qt::AlignRight);
    d->ambientScaleG = makeFloatSlider( ambientScaleVLayout, "G", 0.0f, 1.0f, true, 72, Qt::AlignRight);
    d->ambientScaleB = makeFloatSlider( ambientScaleVLayout, "B", 0.0f, 1.0f, true, 72, Qt::AlignRight);

    // connect use slider to the set function of all the sliders
    connect( d->ambientScale.slider, &QSlider::valueChanged, ambientScaleRGBGroup, [=] {
      emitParametersChangedAfter([&]()
      {
        d->ambientScaleR.set( d->ambientScale.get() );
        d->ambientScaleG.set( d->ambientScale.get() );
        d->ambientScaleB.set( d->ambientScale.get() );
      });
    } );

    connect( d->ambientScale.spin, &QDoubleSpinBox::valueChanged, ambientScaleRGBGroup, [=] 
    {
      emitParametersChangedAfter([&]()
      {
        d->ambientScaleR.set( d->ambientScale.get() );
        d->ambientScaleG.set( d->ambientScale.get() );
        d->ambientScaleB.set( d->ambientScale.get() );
      });
    } );

    ambientScaleHLayout->addWidget(ambientScaleRGBGroup);

    connect(ambientScaleExpandButton, &QAbstractButton::toggled, ambientScaleRGBGroup, [=] {
      ambientScaleRGBGroup->setVisible(ambientScaleExpandButton->isChecked());
    });

    ambientScaleRGBGroup->setVisible(false);

    //--------------------------------AMBIENT BIAS------------------------------------
    auto ambientBiasExpandButton = addExpandIcon(l);
    d->ambientBias = makeFloatSlider( l, "Bias", 0.0f, 1.0f, true, 50, Qt::AlignLeft, ambientBiasExpandButton, true);
    d->ambientBias.set(0.0f);

    auto ambientBiasHLayout = new QHBoxLayout();
    ambientBiasHLayout->setContentsMargins(0,0,0,0);
    l->addLayout(ambientBiasHLayout);

    QWidget* ambientBiasRGBGroup = new QWidget(this);
    auto ambientBiasVLayout = new QVBoxLayout(ambientBiasRGBGroup);
    ambientBiasVLayout->setContentsMargins(0,0,0,0);
    d->ambientBiasR = makeFloatSlider( ambientBiasVLayout, "R", 0.0f, 1.0f, true, 72, Qt::AlignRight);
    d->ambientBiasG = makeFloatSlider( ambientBiasVLayout, "G", 0.0f, 1.0f, true, 72, Qt::AlignRight);
    d->ambientBiasB = makeFloatSlider( ambientBiasVLayout, "B", 0.0f, 1.0f, true, 72, Qt::AlignRight);

    // connect use slider to the set function of all the sliders
    connect( d->ambientBias.slider, &QSlider::valueChanged, ambientBiasRGBGroup, [=] {
      emitParametersChangedAfter([&]()
      {
        d->ambientBiasR.set( d->ambientBias.get() );
        d->ambientBiasG.set( d->ambientBias.get() );
        d->ambientBiasB.set( d->ambientBias.get() );
      });
    } );

    connect( d->ambientBias.spin, &QDoubleSpinBox::valueChanged, ambientBiasRGBGroup, [=] {
      emitParametersChangedAfter([&]()
      {
        d->ambientBiasR.set( d->ambientBias.get() );
        d->ambientBiasG.set( d->ambientBias.get() );
        d->ambientBiasB.set( d->ambientBias.get() );
      });
    } );

    ambientBiasHLayout->addWidget(ambientBiasRGBGroup);

    connect(ambientBiasExpandButton, &QAbstractButton::toggled, ambientBiasRGBGroup, [=] {
      ambientBiasRGBGroup->setVisible(ambientBiasExpandButton->isChecked());
    });

    ambientBiasRGBGroup->setVisible(false);

    addSubTitle(l, "Diffuse");
    //--------------------------------DIFFUSE USE--------------------------------------
    auto diffuseUseExpandButton = addExpandIcon(l);
    d->diffuseUse = makeFloatSlider( l, "Use", 0.0f, 1.0f, true, 50, Qt::AlignLeft, diffuseUseExpandButton, true);

    auto diffuseUseHLayout = new QHBoxLayout();
    diffuseUseHLayout->setContentsMargins(0,0,0,0);
    l->addLayout(diffuseUseHLayout);

    QWidget* diffuseUseRGBGroup = new QWidget(this);
    auto diffuseUseVLayout = new QVBoxLayout(diffuseUseRGBGroup);
    diffuseUseVLayout->setContentsMargins(0,0,0,0);
    d->diffuseUseR = makeFloatSlider( diffuseUseVLayout, "R", 0.0f, 1.0f, true, 72, Qt::AlignRight);
    d->diffuseUseG = makeFloatSlider( diffuseUseVLayout, "G", 0.0f, 1.0f, true, 72, Qt::AlignRight);
    d->diffuseUseB = makeFloatSlider( diffuseUseVLayout, "B", 0.0f, 1.0f, true, 72, Qt::AlignRight);

    // connect use slider to the set function of all the sliders
    connect( d->diffuseUse.slider, &QSlider::valueChanged, diffuseUseRGBGroup, [=] {
      emitParametersChangedAfter([&]()
      {
        d->diffuseUseR.set( d->diffuseUse.get() );
        d->diffuseUseG.set( d->diffuseUse.get() );
        d->diffuseUseB.set( d->diffuseUse.get() );
      });
    } );

    connect( d->diffuseUse.spin, &QDoubleSpinBox::valueChanged, diffuseUseRGBGroup, [=] {
      emitParametersChangedAfter([&]()
      {
        d->diffuseUseR.set( d->diffuseUse.get() );
        d->diffuseUseG.set( d->diffuseUse.get() );
        d->diffuseUseB.set( d->diffuseUse.get() );
      });
    } );

    diffuseUseHLayout->addWidget(diffuseUseRGBGroup);

    connect( diffuseUseExpandButton, &QAbstractButton::toggled, diffuseUseRGBGroup, [=] {
      diffuseUseRGBGroup->setVisible(diffuseUseExpandButton->isChecked());
    });

    diffuseUseRGBGroup->setVisible(false);

    //--------------------------------DIFFUSE SCALE------------------------------------
    auto diffuseScaleExpandButton = addExpandIcon(l);
    d->diffuseScale = makeFloatSlider( l, "Scale", 0.0f, 1.0f, true, 50, Qt::AlignLeft, diffuseScaleExpandButton, true );
    d->diffuseScale.set(1.0f);

    auto diffuseScaleHLayout = new QHBoxLayout();
    diffuseScaleHLayout->setContentsMargins(0,0,0,0);
    l->addLayout(diffuseScaleHLayout);

    QWidget* diffuseScaleRGBGroup = new QWidget(this);
    auto diffuseScaleVLayout = new QVBoxLayout(diffuseScaleRGBGroup);
    diffuseScaleVLayout->setContentsMargins(0,0,0,0);
    d->diffuseScaleR = makeFloatSlider( diffuseScaleVLayout, "R", 0.0f, 1.0f, true, 72, Qt::AlignRight);
    d->diffuseScaleG = makeFloatSlider( diffuseScaleVLayout, "G", 0.0f, 1.0f, true, 72, Qt::AlignRight);
    d->diffuseScaleB = makeFloatSlider( diffuseScaleVLayout, "B", 0.0f, 1.0f, true, 72, Qt::AlignRight);

    // connect use slider to the set function of all the sliders
    connect( d->diffuseScale.slider, &QSlider::valueChanged, diffuseScaleRGBGroup, [=] {
      emitParametersChangedAfter([&]()
      {
        d->diffuseScaleR.set( d->diffuseScale.get() );
        d->diffuseScaleG.set( d->diffuseScale.get() );
        d->diffuseScaleB.set( d->diffuseScale.get() );
      });
    } );

    connect( d->diffuseScale.spin, &QDoubleSpinBox::valueChanged, diffuseScaleRGBGroup, [=] {
      emitParametersChangedAfter([&]()
      {
        d->diffuseScaleR.set( d->diffuseScale.get() );
        d->diffuseScaleG.set( d->diffuseScale.get() );
        d->diffuseScaleB.set( d->diffuseScale.get() );
      });
    } );

    diffuseScaleHLayout->addWidget(diffuseScaleRGBGroup);

    connect( diffuseScaleExpandButton, &QAbstractButton::toggled, diffuseScaleRGBGroup, [=] {
      diffuseScaleRGBGroup->setVisible(diffuseScaleExpandButton->isChecked());
    });

    diffuseScaleRGBGroup->setVisible(false);

    //--------------------------------DIFFUSE BIAS------------------------------------
    auto diffuseBiasExpandButton = addExpandIcon(l);
    d->diffuseBias = makeFloatSlider( l, "Bias", 0.0f, 1.0f, true, 50, Qt::AlignLeft, diffuseBiasExpandButton, true);
    d->diffuseBias.set(0.0f);

    auto diffuseBiasHLayout = new QHBoxLayout();
    diffuseBiasHLayout->setContentsMargins(0,0,0,0);
    l->addLayout(diffuseBiasHLayout);

    QWidget* diffuseBiasRGBGroup = new QWidget(this);
    auto diffuseBiasVLayout = new QVBoxLayout(diffuseBiasRGBGroup);
    diffuseBiasVLayout->setContentsMargins(0,0,0,0);
    d->diffuseBiasR = makeFloatSlider( diffuseBiasVLayout, "R", 0.0f, 1.0f, true, 72, Qt::AlignRight);
    d->diffuseBiasG = makeFloatSlider( diffuseBiasVLayout, "G", 0.0f, 1.0f, true, 72, Qt::AlignRight);
    d->diffuseBiasB = makeFloatSlider( diffuseBiasVLayout, "B", 0.0f, 1.0f, true, 72, Qt::AlignRight);

    // connect use slider to the set function of all the sliders
    connect( d->diffuseBias.slider, &QSlider::valueChanged, diffuseBiasRGBGroup, [=] {
      emitParametersChangedAfter([&]()
      {
        d->diffuseBiasR.set( d->diffuseBias.get() );
        d->diffuseBiasG.set( d->diffuseBias.get() );
        d->diffuseBiasB.set( d->diffuseBias.get() );
      });
    } );

    connect( d->diffuseBias.spin, &QDoubleSpinBox::valueChanged, diffuseBiasRGBGroup, [=] {
      emitParametersChangedAfter([&]()
      {
        d->diffuseBiasR.set( d->diffuseBias.get() );
        d->diffuseBiasG.set( d->diffuseBias.get() );
        d->diffuseBiasB.set( d->diffuseBias.get() );
      });
    } );

    diffuseBiasHLayout->addWidget(diffuseBiasRGBGroup);

    connect( diffuseBiasExpandButton, &QAbstractButton::toggled, diffuseBiasRGBGroup, [=] {
      diffuseBiasRGBGroup->setVisible(diffuseBiasExpandButton->isChecked());
    });

    diffuseBiasRGBGroup->setVisible(false);

    addSubTitle(l, "Specular");
    //--------------------------------SPECULAR USE--------------------------------------
    auto specularUseExpandButton = addExpandIcon(l);
    d->specularUse = makeFloatSlider( l, "Use", 0.0f, 1.0f, true, 50, Qt::AlignLeft, specularUseExpandButton, true );

    auto specularUseHLayout = new QHBoxLayout();
    specularUseHLayout->setContentsMargins(0,0,0,0);
    l->addLayout(specularUseHLayout);

    QWidget* specularUseRGBGroup = new QWidget(this);
    auto specularUseVLayout = new QVBoxLayout(specularUseRGBGroup);
    specularUseVLayout->setContentsMargins(0,0,0,0);
    d->specularUseR = makeFloatSlider( specularUseVLayout, "R", 0.0f, 1.0f, true, 72, Qt::AlignRight);
    d->specularUseG = makeFloatSlider( specularUseVLayout, "G", 0.0f, 1.0f, true, 72, Qt::AlignRight);
    d->specularUseB = makeFloatSlider( specularUseVLayout, "B", 0.0f, 1.0f, true, 72, Qt::AlignRight);

    // connect use slider to the set function of all the sliders
    connect( d->specularUse.slider, &QSlider::valueChanged, specularUseRGBGroup, [=] {
      emitParametersChangedAfter([&]()
      {
        d->specularUseR.set( d->specularUse.get() );
        d->specularUseG.set( d->specularUse.get() );
        d->specularUseB.set( d->specularUse.get() );
      });
    } );

    connect( d->specularUse.spin, &QDoubleSpinBox::valueChanged, specularUseRGBGroup, [=] {
      emitParametersChangedAfter([&]()
      {
        d->specularUseR.set( d->specularUse.get() );
        d->specularUseG.set( d->specularUse.get() );
        d->specularUseB.set( d->specularUse.get() );
      });
    } );

    specularUseHLayout->addWidget(specularUseRGBGroup);

    connect( specularUseExpandButton, &QAbstractButton::toggled, specularUseRGBGroup, [=] {
      specularUseRGBGroup->setVisible(specularUseExpandButton->isChecked());
    });

    specularUseRGBGroup->setVisible(false);

    //--------------------------------SPECULAR SCALE------------------------------------
    auto specularScaleExpandButton = addExpandIcon(l);
    d->specularScale = makeFloatSlider( l, "Scale", 0.0f, 1.0f, true, 50, Qt::AlignLeft, specularScaleExpandButton, true );
    d->specularScale.set(1.0f);

    auto specularScaleHLayout = new QHBoxLayout();
    specularScaleHLayout->setContentsMargins(0,0,0,0);
    l->addLayout(specularScaleHLayout);

    QWidget* specularScaleRGBGroup = new QWidget(this);
    auto specularScaleVLayout = new QVBoxLayout(specularScaleRGBGroup);
    specularScaleVLayout->setContentsMargins(0,0,0,0);
    d->specularScaleR = makeFloatSlider( specularScaleVLayout, "R", 0.0f, 1.0f, true, 72, Qt::AlignRight);
    d->specularScaleG = makeFloatSlider( specularScaleVLayout, "G", 0.0f, 1.0f, true, 72, Qt::AlignRight);
    d->specularScaleB = makeFloatSlider( specularScaleVLayout, "B", 0.0f, 1.0f, true, 72, Qt::AlignRight);

    // connect use slider to the set function of all the sliders
    connect( d->specularScale.slider, &QSlider::valueChanged, specularScaleRGBGroup, [=] {
      emitParametersChangedAfter([&]()
      {
        d->specularScaleR.set( d->specularScale.get() );
        d->specularScaleG.set( d->specularScale.get() );
        d->specularScaleB.set( d->specularScale.get() );
      });
    } );

    connect( d->specularScale.spin, &QDoubleSpinBox::valueChanged, specularScaleRGBGroup, [=] {
      emitParametersChangedAfter([&]()
      {
        d->specularScaleR.set( d->specularScale.get() );
        d->specularScaleG.set( d->specularScale.get() );
        d->specularScaleB.set( d->specularScale.get() );
      });
    } );

    specularScaleHLayout->addWidget(specularScaleRGBGroup);

    connect( specularScaleExpandButton, &QAbstractButton::toggled, specularScaleRGBGroup, [=] {
      specularScaleRGBGroup->setVisible(specularScaleExpandButton->isChecked());
    });

    specularScaleRGBGroup->setVisible(false);

    //--------------------------------SPECULAR BIAS------------------------------------
    auto specularBiasExpandButton = addExpandIcon(l);
    d->specularBias = makeFloatSlider( l, "Bias", 0.0f, 1.0f, true, 50, Qt::AlignLeft, specularBiasExpandButton, true );
    d->specularBias.set(0.0f);

    auto specularBiasHLayout = new QHBoxLayout();
    specularBiasHLayout->setContentsMargins(0,0,0,0);
    l->addLayout(specularBiasHLayout);

    QWidget* specularBiasRGBGroup = new QWidget(this);
    auto specularBiasVLayout = new QVBoxLayout(specularBiasRGBGroup);
    specularBiasVLayout->setContentsMargins(0,0,0,0);
    d->specularBiasR = makeFloatSlider( specularBiasVLayout, "R", 0.0f, 1.0f, true, 72, Qt::AlignRight);
    d->specularBiasG = makeFloatSlider( specularBiasVLayout, "G", 0.0f, 1.0f, true, 72, Qt::AlignRight);
    d->specularBiasB = makeFloatSlider( specularBiasVLayout, "B", 0.0f, 1.0f, true, 72, Qt::AlignRight);

    // connect use slider to the set function of all the sliders
    connect( d->specularBias.slider, &QSlider::valueChanged, specularBiasRGBGroup, [=] {
      emitParametersChangedAfter([&]()
      {
        d->specularBiasR.set( d->specularBias.get() );
        d->specularBiasG.set( d->specularBias.get() );
        d->specularBiasB.set( d->specularBias.get() );
      });
    } );

    connect( d->specularBias.spin, &QDoubleSpinBox::valueChanged, specularBiasRGBGroup, [=] {
      emitParametersChangedAfter([&]()
      {
        d->specularBiasR.set( d->specularBias.get() );
        d->specularBiasG.set( d->specularBias.get() );
        d->specularBiasB.set( d->specularBias.get() );
      });
    } );

    specularBiasHLayout->addWidget(specularBiasRGBGroup);

    connect( specularBiasExpandButton, &QAbstractButton::toggled, specularBiasRGBGroup, [=] {
      specularBiasRGBGroup->setVisible(specularBiasExpandButton->isChecked());
    });

    specularBiasRGBGroup->setVisible(false);

    addSubTitle(l, "OffsetScale");
    //--------------------------------OFFSETSCALE USE--------------------------------------
    auto offsetScaleUseExpandButton = addExpandIcon(l);
    d->offsetScaleUse = makeFloatSlider( l, "Use", 0.0f, 1.0f, true, 50, Qt::AlignLeft, offsetScaleUseExpandButton, true );

    auto offsetScaleUseHLayout = new QHBoxLayout();
    offsetScaleUseHLayout->setContentsMargins(0,0,0,0);
    l->addLayout(offsetScaleUseHLayout);

    QWidget* offsetScaleUseRGBGroup = new QWidget(this);
    auto offsetScaleUseVLayout = new QVBoxLayout(offsetScaleUseRGBGroup);
    offsetScaleUseVLayout->setContentsMargins(0,0,0,0);
    d->offsetScaleUseR = makeFloatSlider( offsetScaleUseVLayout, "R", 0.0f, 1.0f, true, 72, Qt::AlignRight);
    d->offsetScaleUseG = makeFloatSlider( offsetScaleUseVLayout, "G", 0.0f, 1.0f, true, 72, Qt::AlignRight);
    d->offsetScaleUseB = makeFloatSlider( offsetScaleUseVLayout, "B", 0.0f, 1.0f, true, 72, Qt::AlignRight);

    // connect use slider to the set function of all the sliders
    connect( d->offsetScaleUse.slider, &QSlider::valueChanged, offsetScaleUseRGBGroup, [=] {
      emitParametersChangedAfter([&]()
      {
        d->offsetScaleUseR.set( d->offsetScaleUse.get() );
        d->offsetScaleUseG.set( d->offsetScaleUse.get() );
        d->offsetScaleUseB.set( d->offsetScaleUse.get() );
      });
    } );

    connect( d->offsetScaleUse.spin, &QDoubleSpinBox::valueChanged, offsetScaleUseRGBGroup, [=] {
      emitParametersChangedAfter([&]()
      {
        d->offsetScaleUseR.set( d->offsetScaleUse.get() );
        d->offsetScaleUseG.set( d->offsetScaleUse.get() );
        d->offsetScaleUseB.set( d->offsetScaleUse.get() );
      });
    } );

    offsetScaleUseHLayout->addWidget(offsetScaleUseRGBGroup);

    connect( offsetScaleUseExpandButton, &QAbstractButton::toggled, offsetScaleUseRGBGroup, [=] {
      offsetScaleUseRGBGroup->setVisible(offsetScaleUseExpandButton->isChecked());
    });

    offsetScaleUseRGBGroup->setVisible(false);

    //--------------------------------OFFSETSCALE SCALE------------------------------------
    auto offsetScaleScaleExpandButton = addExpandIcon(l);
    d->offsetScaleScale = makeFloatSlider( l, "Scale", 0.0f, 1000.0f, true, 50, Qt::AlignLeft, offsetScaleScaleExpandButton, true );
    d->offsetScaleScale.set(1.0f);

    auto offsetScaleScaleHLayout = new QHBoxLayout();
    offsetScaleScaleHLayout->setContentsMargins(0,0,0,0);
    l->addLayout(offsetScaleScaleHLayout);

    QWidget* offsetScaleScaleRGBGroup = new QWidget(this);
    auto offsetScaleScaleVLayout = new QVBoxLayout(offsetScaleScaleRGBGroup);
    offsetScaleScaleVLayout->setContentsMargins(0,0,0,0);
    d->offsetScaleScaleR = makeFloatSlider( offsetScaleScaleVLayout, "R", 0.0f, 1000.0f, true, 72, Qt::AlignRight);
    d->offsetScaleScaleG = makeFloatSlider( offsetScaleScaleVLayout, "G", 0.0f, 1000.0f, true, 72, Qt::AlignRight);
    d->offsetScaleScaleB = makeFloatSlider( offsetScaleScaleVLayout, "B", 0.0f, 1000.0f, true, 72, Qt::AlignRight);

    // connect use slider to the set function of all the sliders
    connect( d->offsetScaleScale.slider, &QSlider::valueChanged, offsetScaleScaleRGBGroup, [=] {
      emitParametersChangedAfter([&]()
      {
        d->offsetScaleScaleR.set( d->offsetScaleScale.get() );
        d->offsetScaleScaleG.set( d->offsetScaleScale.get() );
        d->offsetScaleScaleB.set( d->offsetScaleScale.get() );
      });
    } );

    connect( d->offsetScaleScale.spin, &QDoubleSpinBox::valueChanged, offsetScaleScaleRGBGroup, [=] {
      emitParametersChangedAfter([&]()
      {
        d->offsetScaleScaleR.set( d->offsetScaleScale.get() );
        d->offsetScaleScaleG.set( d->offsetScaleScale.get() );
        d->offsetScaleScaleB.set( d->offsetScaleScale.get() );
      });
    } );

    offsetScaleScaleHLayout->addWidget(offsetScaleScaleRGBGroup);

    connect(offsetScaleScaleExpandButton, &QAbstractButton::toggled, offsetScaleScaleRGBGroup, [=] {
      offsetScaleScaleRGBGroup->setVisible(offsetScaleScaleExpandButton->isChecked());
    });

    offsetScaleScaleRGBGroup->setVisible(false);

    //--------------------------------OFFSETSCALE BIAS------------------------------------
    auto offsetScaleBiasExpandButton = addExpandIcon(l);
    d->offsetScaleBias = makeFloatSlider( l, "Bias", 0.0f, 1000.0f, true, 50, Qt::AlignLeft, offsetScaleBiasExpandButton, true);
    d->offsetScaleBias.set(0.0f);

    auto offsetScaleBiasHLayout = new QHBoxLayout();
    offsetScaleBiasHLayout->setContentsMargins(0,0,0,0);
    l->addLayout(offsetScaleBiasHLayout);

    QWidget* offsetScaleBiasRGBGroup = new QWidget(this);
    auto offsetScaleBiasVLayout = new QVBoxLayout(offsetScaleBiasRGBGroup);
    offsetScaleBiasVLayout->setContentsMargins(0,0,0,0);
    d->offsetScaleBiasR = makeFloatSlider( offsetScaleBiasVLayout, "R", 0.0f, 1000.0f, true, 72, Qt::AlignRight);
    d->offsetScaleBiasG = makeFloatSlider( offsetScaleBiasVLayout, "G", 0.0f, 1000.0f, true, 72, Qt::AlignRight);
    d->offsetScaleBiasB = makeFloatSlider( offsetScaleBiasVLayout, "B", 0.0f, 1000.0f, true, 72, Qt::AlignRight);

    // connect use slider to the set function of all the sliders
    connect( d->offsetScaleBias.slider, &QSlider::valueChanged, offsetScaleBiasRGBGroup, [=] {
      emitParametersChangedAfter([&]()
      {
        d->offsetScaleBiasR.set( d->offsetScaleBias.get() );
        d->offsetScaleBiasG.set( d->offsetScaleBias.get() );
        d->offsetScaleBiasB.set( d->offsetScaleBias.get() );
      });
    } );

    connect( d->offsetScaleBias.spin, &QDoubleSpinBox::valueChanged, offsetScaleBiasRGBGroup, [=] {
      emitParametersChangedAfter([&]()
      {
        d->offsetScaleBiasR.set( d->offsetScaleBias.get() );
        d->offsetScaleBiasG.set( d->offsetScaleBias.get() );
        d->offsetScaleBiasB.set( d->offsetScaleBias.get() );
      });
    } );

    offsetScaleBiasHLayout->addWidget(offsetScaleBiasRGBGroup);

    connect( offsetScaleBiasExpandButton, &QCheckBox::toggled, offsetScaleBiasRGBGroup, [=] {
      offsetScaleBiasRGBGroup->setVisible(offsetScaleBiasExpandButton->isChecked());
    });

    offsetScaleBiasRGBGroup->setVisible(false);

    //--------------------------------DIFFUSE SCALE------------------------------------
    addSubTitle(l, "Diffuse Scale");
    d->diffuseScaleUse = makeFloatSlider( l, "Use", 0.0f, 1.0f, true, 50, Qt::AlignLeft );
    d->diffuseScaleScale = makeFloatSlider( l, "Scale", 0.0f, 1000.0f, true, 50, Qt::AlignLeft );
    d->diffuseScaleBias = makeFloatSlider( l, "Bias", 0.0f, 1000.0f, true, 50, Qt::AlignLeft );

    //--------------------------------SPOT LIGHT BLEND------------------------------------------
    addSubTitle(l, "Spot Light Blend");
    d->spotLightBlendUse = makeFloatSlider( l, "Use", 0.0f, 1.0f, true, 50, Qt::AlignLeft );
    d->spotLightBlendScale = makeFloatSlider( l, "Scale", 0.0f, 1000.0f, true, 50, Qt::AlignLeft );
    d->spotLightBlendBias = makeFloatSlider( l, "Bias", 0.0f, 1000.0f, true, 50, Qt::AlignLeft );

    //--------------------------------FOV------------------------------------------
    addSubTitle(l, "FOV");
    d->fovUse = makeFloatSlider( l, "Use", 0.0f, 1.0f, true, 50, Qt::AlignLeft );
    d->fovScale = makeFloatSlider( l, "Scale", 0.0f, 1000.0f, true, 50, Qt::AlignLeft );
    d->fovBias = makeFloatSlider( l, "Bias", 0.0f, 1000.0f, true, 50, Qt::AlignLeft );
  }
}

//##################################################################################################
EditLightSwapParametersWidget::~EditLightSwapParametersWidget()
{
  delete d;
}

//################################################################################################
void EditLightSwapParametersWidget::setLightSwapParameters(const tp_math_utils::LightSwapParameters& lightSwapParameters)
{
  blockSignals(true);
  TP_CLEANUP([&]{blockSignals(false);});

  auto setMasterToMaxOf = [&](FloatEditor& master, const float& x, const float& y, const float& z)
  {
    float max = std::max(x, y);
    max = std::max(max, z);
    if(master.get() != max)
      master.set(max);
  };

  setMasterToMaxOf(d->ambientUse, lightSwapParameters.ambientUse.x, lightSwapParameters.ambientUse.y, lightSwapParameters.ambientUse.z);
  d->ambientUseR         .set(lightSwapParameters.ambientUse.x);
  d->ambientUseG         .set(lightSwapParameters.ambientUse.y);
  d->ambientUseB         .set(lightSwapParameters.ambientUse.z);

  setMasterToMaxOf(d->ambientScale, lightSwapParameters.ambientScale.x, lightSwapParameters.ambientScale.y, lightSwapParameters.ambientScale.z);
  d->ambientScaleR       .set(lightSwapParameters.ambientScale.x);
  d->ambientScaleG       .set(lightSwapParameters.ambientScale.y);
  d->ambientScaleB       .set(lightSwapParameters.ambientScale.z);

  setMasterToMaxOf(d->ambientBias, lightSwapParameters.ambientBias.x, lightSwapParameters.ambientBias.y, lightSwapParameters.ambientBias.z);
  d->ambientBiasR        .set(lightSwapParameters.ambientBias.x);
  d->ambientBiasG        .set(lightSwapParameters.ambientBias.y);
  d->ambientBiasB        .set(lightSwapParameters.ambientBias.z);

  setMasterToMaxOf(d->diffuseUse, lightSwapParameters.diffuseUse.x, lightSwapParameters.diffuseUse.y, lightSwapParameters.diffuseUse.z);
  d->diffuseUseR         .set(lightSwapParameters.diffuseUse.x);
  d->diffuseUseG         .set(lightSwapParameters.diffuseUse.y);
  d->diffuseUseB         .set(lightSwapParameters.diffuseUse.z);

  setMasterToMaxOf(d->diffuseScale, lightSwapParameters.diffuseScale.x, lightSwapParameters.diffuseScale.y, lightSwapParameters.diffuseScale.z);
  d->diffuseScaleR       .set(lightSwapParameters.diffuseScale.x);
  d->diffuseScaleG       .set(lightSwapParameters.diffuseScale.y);
  d->diffuseScaleB       .set(lightSwapParameters.diffuseScale.z);

  setMasterToMaxOf(d->diffuseBias, lightSwapParameters.diffuseBias.x, lightSwapParameters.diffuseBias.y, lightSwapParameters.diffuseBias.z);
  d->diffuseBiasR        .set(lightSwapParameters.diffuseBias.x);
  d->diffuseBiasG        .set(lightSwapParameters.diffuseBias.y);
  d->diffuseBiasB        .set(lightSwapParameters.diffuseBias.z);

  setMasterToMaxOf(d->specularUse, lightSwapParameters.specularUse.x, lightSwapParameters.specularUse.y, lightSwapParameters.specularUse.z);
  d->specularUseR        .set(lightSwapParameters.specularUse.x);
  d->specularUseG        .set(lightSwapParameters.specularUse.y);
  d->specularUseB        .set(lightSwapParameters.specularUse.z);

  setMasterToMaxOf(d->specularScale, lightSwapParameters.specularScale.x, lightSwapParameters.specularScale.y, lightSwapParameters.specularScale.z);
  d->specularScaleR      .set(lightSwapParameters.specularScale.x);
  d->specularScaleG      .set(lightSwapParameters.specularScale.y);
  d->specularScaleB      .set(lightSwapParameters.specularScale.z);

  setMasterToMaxOf(d->specularBias, lightSwapParameters.specularBias.x, lightSwapParameters.specularBias.y, lightSwapParameters.specularBias.z);
  d->specularBiasR       .set(lightSwapParameters.specularBias.x);
  d->specularBiasG       .set(lightSwapParameters.specularBias.y);
  d->specularBiasB       .set(lightSwapParameters.specularBias.z);

  setMasterToMaxOf(d->offsetScaleUse, lightSwapParameters.offsetScaleUse.x, lightSwapParameters.offsetScaleUse.y, lightSwapParameters.offsetScaleUse.z);
  d->offsetScaleUseR     .set(lightSwapParameters.offsetScaleUse.x);
  d->offsetScaleUseG     .set(lightSwapParameters.offsetScaleUse.y);
  d->offsetScaleUseB     .set(lightSwapParameters.offsetScaleUse.z);

  setMasterToMaxOf(d->offsetScaleScale, lightSwapParameters.offsetScaleScale.x, lightSwapParameters.offsetScaleScale.y, lightSwapParameters.offsetScaleScale.z);
  d->offsetScaleScaleR   .set(lightSwapParameters.offsetScaleScale.x);
  d->offsetScaleScaleG   .set(lightSwapParameters.offsetScaleScale.y);
  d->offsetScaleScaleB   .set(lightSwapParameters.offsetScaleScale.z);

  setMasterToMaxOf(d->offsetScaleBias, lightSwapParameters.offsetScaleBias.x, lightSwapParameters.offsetScaleBias.y, lightSwapParameters.offsetScaleBias.z);
  d->offsetScaleBiasR    .set(lightSwapParameters.offsetScaleBias.x);
  d->offsetScaleBiasG    .set(lightSwapParameters.offsetScaleBias.y);
  d->offsetScaleBiasB    .set(lightSwapParameters.offsetScaleBias.z);


  d->diffuseScaleUse     .set(lightSwapParameters.diffuseScaleUse);
  d->diffuseScaleScale   .set(lightSwapParameters.diffuseScaleScale);
  d->diffuseScaleBias    .set(lightSwapParameters.diffuseScaleBias);

  d->spotLightBlendUse   .set(lightSwapParameters.spotLightBlendUse);
  d->spotLightBlendScale .set(lightSwapParameters.spotLightBlendScale);
  d->spotLightBlendBias  .set(lightSwapParameters.spotLightBlendBias);

  d->fovUse              .set(lightSwapParameters.fovUse);
  d->fovScale            .set(lightSwapParameters.fovScale);
  d->fovBias             .set(lightSwapParameters.fovBias);
}

//##################################################################################################
tp_math_utils::LightSwapParameters EditLightSwapParametersWidget::lightSwapParameters() const
{
  tp_math_utils::LightSwapParameters lightSwapParameters;

  lightSwapParameters.ambientUse.x        = d->ambientUseR.get();
  lightSwapParameters.ambientUse.y        = d->ambientUseG.get();
  lightSwapParameters.ambientUse.z        = d->ambientUseB.get();
  lightSwapParameters.ambientScale.x      = d->ambientScaleR.get();
  lightSwapParameters.ambientScale.y      = d->ambientScaleG.get();
  lightSwapParameters.ambientScale.z      = d->ambientScaleB.get();
  lightSwapParameters.ambientBias.x       = d->ambientBiasR.get();
  lightSwapParameters.ambientBias.y       = d->ambientBiasG.get();
  lightSwapParameters.ambientBias.z       = d->ambientBiasB.get();

  lightSwapParameters.diffuseUse.x        = d->diffuseUseR.get();
  lightSwapParameters.diffuseUse.y        = d->diffuseUseG.get();
  lightSwapParameters.diffuseUse.z        = d->diffuseUseB.get();
  lightSwapParameters.diffuseScale.x      = d->diffuseScaleR.get();
  lightSwapParameters.diffuseScale.y      = d->diffuseScaleG.get();
  lightSwapParameters.diffuseScale.z      = d->diffuseScaleB.get();
  lightSwapParameters.diffuseBias.x       = d->diffuseBiasR.get();
  lightSwapParameters.diffuseBias.y       = d->diffuseBiasG.get();
  lightSwapParameters.diffuseBias.z       = d->diffuseBiasB.get();

  lightSwapParameters.specularUse.x       = d->specularUseR.get();
  lightSwapParameters.specularUse.y       = d->specularUseG.get();
  lightSwapParameters.specularUse.z       = d->specularUseB.get();
  lightSwapParameters.specularScale.x     = d->specularScaleR.get();
  lightSwapParameters.specularScale.y     = d->specularScaleG.get();
  lightSwapParameters.specularScale.z     = d->specularScaleB.get();
  lightSwapParameters.specularBias.x      = d->specularBiasR.get();
  lightSwapParameters.specularBias.y      = d->specularBiasG.get();
  lightSwapParameters.specularBias.z      = d->specularBiasB.get();

  lightSwapParameters.offsetScaleUse.x    = d->offsetScaleUseR.get();
  lightSwapParameters.offsetScaleUse.y    = d->offsetScaleUseG.get();
  lightSwapParameters.offsetScaleUse.z    = d->offsetScaleUseB.get();
  lightSwapParameters.offsetScaleScale.x  = d->offsetScaleScaleR.get();
  lightSwapParameters.offsetScaleScale.y  = d->offsetScaleScaleG.get();
  lightSwapParameters.offsetScaleScale.z  = d->offsetScaleScaleB.get();
  lightSwapParameters.offsetScaleBias.x   = d->offsetScaleBiasR.get();
  lightSwapParameters.offsetScaleBias.y   = d->offsetScaleBiasG.get();
  lightSwapParameters.offsetScaleBias.z   = d->offsetScaleBiasB.get();

  lightSwapParameters.diffuseScaleUse     = d->diffuseScaleUse.get();
  lightSwapParameters.diffuseScaleScale   = d->diffuseScaleScale.get();
  lightSwapParameters.diffuseScaleBias    = d->diffuseScaleBias.get();

  lightSwapParameters.spotLightBlendUse   = d->spotLightBlendUse.get();
  lightSwapParameters.spotLightBlendScale = d->spotLightBlendScale.get();
  lightSwapParameters.spotLightBlendBias  = d->spotLightBlendBias.get();

  lightSwapParameters.fovUse              = d->fovUse.get();
  lightSwapParameters.fovScale            = d->fovScale.get();
  lightSwapParameters.fovBias             = d->fovBias.get();

  return lightSwapParameters;
}

}
