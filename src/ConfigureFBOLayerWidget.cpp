#include "tp_qt_maps_widget/ConfigureFBOLayerWidget.h"

#include "tp_qt_utils/Globals.h"

#include "tp_maps/layers/FBOLayer.h"

#include "tp_maps/Map.h"
#include "tp_maps/Buffers.h"

#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QBoxLayout>
#include <QGridLayout>
#include <QComboBox>

namespace tp_qt_maps_widget
{

//##################################################################################################
struct ConfigureFBOLayerWidget::Private
{
  tp_maps::FBOLayer* fboLayer;

  QCheckBox* enableCheckBox{nullptr};

  QDoubleSpinBox* positionX{nullptr};
  QDoubleSpinBox* positionY{nullptr};
  QDoubleSpinBox* sizeX{nullptr};
  QDoubleSpinBox* sizeY{nullptr};

  QComboBox* source{nullptr};
  QSpinBox* index{nullptr};


  QComboBox* storedBuffers{nullptr};

  //################################################################################################
  Private(tp_maps::FBOLayer* fboLayer_):
    fboLayer(fboLayer_)
  {

  }
};

//##################################################################################################
ConfigureFBOLayerWidget::ConfigureFBOLayerWidget(tp_maps::FBOLayer* fboLayer):
  d(new Private(fboLayer))
{
  auto l = new QVBoxLayout(this);

  d->enableCheckBox = new QCheckBox("Enable FBO layer.");
  l->addWidget(d->enableCheckBox);

  connect(d->enableCheckBox, &QCheckBox::clicked, this, [&]
  {
    d->fboLayer->setVisible(d->enableCheckBox->isChecked());
  });

  {
    auto ll = new QGridLayout();
    ll->setContentsMargins(0,0,0,0);
    l->addLayout(ll);

    ll->addWidget(new QLabel("Position X"), 0, 0);
    d->positionX = new QDoubleSpinBox();
    d->positionX->setRange(0.0, 1.0);
    d->positionX->setDecimals(3);
    d->positionX->setSingleStep(0.04);
    ll->addWidget(d->positionX, 0, 1);

    ll->addWidget(new QLabel("Position Y"), 1, 0);
    d->positionY = new QDoubleSpinBox();
    d->positionY->setRange(0.0, 1.0);
    d->positionY->setDecimals(3);
    d->positionY->setSingleStep(0.04);
    ll->addWidget(d->positionY, 1, 1);

    ll->addWidget(new QLabel("Size X"), 0, 2);
    d->sizeX = new QDoubleSpinBox();
    d->sizeX->setRange(0.05, 1.0);
    d->sizeX->setDecimals(3);
    d->sizeX->setSingleStep(0.04);
    ll->addWidget(d->sizeX, 0, 3);

    ll->addWidget(new QLabel("Size Y"), 1, 2);
    d->sizeY = new QDoubleSpinBox();
    d->sizeY->setRange(0.05, 1.0);
    d->sizeY->setDecimals(3);
    d->sizeY->setSingleStep(0.04);
    ll->addWidget(d->sizeY, 1, 3);

    auto edited = [&]
    {
      glm::vec2 position = {float(d->positionX->value()), float(d->positionY->value())};
      glm::vec2 size = {float(d->sizeX->value()), float(d->sizeY->value())};
      d->fboLayer->setImageCoords(position, size);
    };

    connect(d->positionX, &QDoubleSpinBox::valueChanged, this, edited);
    connect(d->positionY, &QDoubleSpinBox::valueChanged, this, edited);
    connect(d->sizeX, &QDoubleSpinBox::valueChanged, this, edited);
    connect(d->sizeY, &QDoubleSpinBox::valueChanged, this, edited);
  }

  {
    l->addWidget(new QLabel("Source"));
    d->source = new QComboBox();
    l->addWidget(d->source);
    d->source->addItems(tp_qt_utils::convertStringList(tp_maps::fboLayerSources()));

    l->addWidget(new QLabel("Light index"));
    d->index = new QSpinBox();
    l->addWidget(d->index);
    d->index->setRange(0, 4);
    auto edited = [&]
    {
      d->fboLayer->setSource(tp_maps::fboLayerSourceFromString(d->source->currentText().toStdString()), d->index->value());
    };

    connect(d->source, &QComboBox::currentTextChanged, this, edited);
    connect(d->index, &QSpinBox::valueChanged, this, edited);
  }

  {
    QStringList dst;
    std::vector<std::string> keys;

    for( auto& i : fboLayer->map()->buffers().storedBuffers() ) {
      keys.push_back( i.first );
    }

    auto src = keys;
    dst.reserve(src.size());
    for(const auto& i : src)
      dst.push_back(QString::fromStdString(i));


    l->addWidget(new QLabel("Stored buffers"));
    d->source = new QComboBox();
    l->addWidget(d->source);
    d->source->addItems( dst );
  }

  update();
}

//##################################################################################################
ConfigureFBOLayerWidget::~ConfigureFBOLayerWidget()
{
  delete d;
}

//##################################################################################################
void ConfigureFBOLayerWidget::update()
{
  d->enableCheckBox->setChecked(d->fboLayer->visible());

  d->positionX->blockSignals(true);
  d->positionY->blockSignals(true);
  d->sizeX    ->blockSignals(true);
  d->sizeY    ->blockSignals(true);

  d->positionX->setValue(double(d->fboLayer->origin().x));
  d->positionY->setValue(double(d->fboLayer->origin().y));
  d->sizeX    ->setValue(double(d->fboLayer->size().x));
  d->sizeY    ->setValue(double(d->fboLayer->size().y));

  d->positionX->blockSignals(false);
  d->positionY->blockSignals(false);
  d->sizeX    ->blockSignals(false);
  d->sizeY    ->blockSignals(false);

  d->source->blockSignals(true);
  d->source->setCurrentText(QString::fromStdString(tp_maps::fboLayerSourceToString(d->fboLayer->source())));
  d->source->blockSignals(false);

  d->index->blockSignals(true);
  d->index->setValue(int(d->fboLayer->index()));
  d->index->blockSignals(false);
}

}
