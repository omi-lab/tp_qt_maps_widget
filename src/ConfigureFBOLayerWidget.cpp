#include "tp_qt_maps_widget/ConfigureFBOLayerWidget.h"

#include "tp_qt_utils/Globals.h"

#include "tp_maps/Map.h"
#include "tp_maps/layers/FBOLayer.h"
#include "tp_maps/subsystems/open_gl/OpenGLBuffers.h"

#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QBoxLayout>
#include <QGridLayout>
#include <QComboBox>
#include <QListWidget>
#include <QPushButton>
#include <QGroupBox>
#include <QSplitter>

namespace tp_qt_maps_widget
{

//##################################################################################################
struct ConfigureFBOLayerWidget::Private
{
  Q* q;

  tp_maps::FBOLayer* fboLayer;

  QCheckBox* enableCheckBox{nullptr};
  QCheckBox* pollPickingCheckBox{nullptr};

  QListWidget* windowsList{nullptr};
  QPushButton* addButton{nullptr};
  QPushButton* removeButton{nullptr};

  QGroupBox* groupBox{nullptr};
  QDoubleSpinBox* positionX{nullptr};
  QDoubleSpinBox* positionY{nullptr};
  QDoubleSpinBox* sizeX{nullptr};
  QDoubleSpinBox* sizeY{nullptr};

  QComboBox* fboNames{nullptr};
  QComboBox* source{nullptr};

  //################################################################################################
  Private(Q* q_, tp_maps::FBOLayer* fboLayer_):
    q(q_),
    fboLayer(fboLayer_)
  {

  }

  //################################################################################################
  size_t selectedItem()
  {
    if(auto items = windowsList->selectedItems(); !items.empty())
      return size_t(windowsList->row(items.front()));

    return 0;
  }

  //################################################################################################
  void addClicked()
  {
    std::vector<tp_maps::FBOWindow> windows = fboLayer->windows();
    tp_maps::FBOWindow& window = windows.emplace_back();

    if(auto fbos = fboLayer->map()->buffers().storedBuffers(); !fbos.empty())
      window.fboName = fbos.begin()->first;

    fboLayer->setWindows(windows);
    q->update();
  }

  //################################################################################################
  void removeClicked()
  {
    size_t selectedIndex = selectedItem();
    std::vector<tp_maps::FBOWindow> windows = fboLayer->windows();

    if(selectedIndex<windows.size())
    {
      tpRemoveAt(windows, selectedIndex);

      fboLayer->setWindows(windows);
      q->update();
    }
  }

  //################################################################################################
  void edited()
  {
    size_t selectedIndex = selectedItem();
    std::vector<tp_maps::FBOWindow> windows = fboLayer->windows();

    if(selectedIndex<windows.size())
    {
      tp_maps::FBOWindow& window = windows.at(selectedIndex);

      window.fboName = fboNames->currentText().toStdString();
      window.source  = tp_maps::fboLayerSourceFromString(source->currentText().toStdString());
      window.origin  = {float(positionX->value()), float(positionY->value())};
      window.size    = {float(sizeX->value()), float(sizeY->value())};

      fboLayer->setWindows(windows);
      q->update();
    }
  }

  //################################################################################################
  void updateSelection()
  {
    size_t selectedIndex = selectedItem();
    const auto& windows = fboLayer->windows();

    if(selectedIndex >= windows.size())
    {
      groupBox->setEnabled(false);
      return;
    }

    groupBox->setEnabled(true);

    const auto& window = windows.at(selectedIndex);

    positionX->blockSignals(true);
    positionY->blockSignals(true);
    sizeX    ->blockSignals(true);
    sizeY    ->blockSignals(true);

    positionX->setValue(double(window.origin.x));
    positionY->setValue(double(window.origin.y));
    sizeX    ->setValue(double(window.size.x));
    sizeY    ->setValue(double(window.size.y));

    positionX->blockSignals(false);
    positionY->blockSignals(false);
    sizeX    ->blockSignals(false);
    sizeY    ->blockSignals(false);

    {
      fboNames->blockSignals(true);
      fboNames->clear();
      for(auto& i : fboLayer->map()->buffers().storedBuffers())
        fboNames->addItem(QString::fromStdString(i.first.toString()));
//      for(auto& i : fboLayer->map()->intermediateBuffers())
//        fboNames->addItem(QString::fromStdString(i.first));
      fboNames->setCurrentText(QString::fromStdString(window.fboName.toString()));
      fboNames->blockSignals(false);
    }

    source->blockSignals(true);
    source->setCurrentText(QString::fromStdString(tp_maps::fboLayerSourceToString(window.source)));
    source->blockSignals(false);
  }
};

//##################################################################################################
ConfigureFBOLayerWidget::ConfigureFBOLayerWidget(tp_maps::FBOLayer* fboLayer):
  d(new Private(this, fboLayer))
{
  auto l = new QVBoxLayout(this);

  d->enableCheckBox = new QCheckBox("Enable FBO layer.");
  l->addWidget(d->enableCheckBox);

  connect(d->enableCheckBox, &QCheckBox::clicked, this, [&]
  {
    d->fboLayer->setVisible(d->enableCheckBox->isChecked());
  });

  d->pollPickingCheckBox = new QCheckBox("Poll picking.");
  l->addWidget(d->pollPickingCheckBox);

  connect(d->pollPickingCheckBox, &QCheckBox::clicked, this, [&]
  {
    d->fboLayer->setPollPicking(d->pollPickingCheckBox->isChecked());
  });

  auto mainSplitter = new QSplitter();
  l->addWidget(mainSplitter);

  {
    auto w = new QWidget();
    auto ll = new QVBoxLayout(w);
    mainSplitter->addWidget(w);

    ll->addWidget(new QLabel("FBO windows"));

    d->windowsList = new QListWidget();
    ll->addWidget(d->windowsList);
    connect(d->windowsList->selectionModel(), &QItemSelectionModel::selectionChanged, this, [&]
    {
      d->updateSelection();
    });

    auto buttonLayout = new QHBoxLayout();
    ll->addLayout(buttonLayout);
    buttonLayout->setContentsMargins(0,0,0,0);

    d->addButton = new QPushButton("Add");
    buttonLayout->addWidget(d->addButton);
    connect(d->addButton, &QPushButton::clicked, this, [&]{d->addClicked();});

    d->removeButton = new QPushButton("Remove");
    buttonLayout->addWidget(d->removeButton);
    connect(d->removeButton, &QPushButton::clicked, this, [&]{d->removeClicked();});
  }

  {
    d->groupBox = new QGroupBox();
    auto ll = new QGridLayout(d->groupBox);
    mainSplitter->addWidget(d->groupBox);

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

    ll->addWidget(new QLabel("FBO"), 2, 0, 1, 4);
    d->fboNames = new QComboBox();
    ll->addWidget(d->fboNames, 3, 0, 1, 4);

    ll->addWidget(new QLabel("Texture"), 4, 0, 1, 4);
    d->source = new QComboBox();
    ll->addWidget(d->source, 5, 0, 1, 4);
    d->source->addItems(tp_qt_utils::convertStringList(tp_maps::fboLayerSources()));

    connect(d->positionX, &QDoubleSpinBox::valueChanged , this, [&]{d->edited();});
    connect(d->positionY, &QDoubleSpinBox::valueChanged , this, [&]{d->edited();});
    connect(d->sizeX    , &QDoubleSpinBox::valueChanged , this, [&]{d->edited();});
    connect(d->sizeY    , &QDoubleSpinBox::valueChanged , this, [&]{d->edited();});
    connect(d->fboNames , &QComboBox::currentTextChanged, this, [&]{d->edited();});
    connect(d->source   , &QComboBox::currentTextChanged, this, [&]{d->edited();});
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
  d->pollPickingCheckBox->setChecked(d->fboLayer->pollPicking());

  //------------------------------------------------------------------------------------------------
  // Update the list of windows.
  int selectedIndex=0;
  {
    if(auto items = d->windowsList->selectedItems(); !items.empty())
      selectedIndex = d->windowsList->row(items.front());

    d->windowsList->blockSignals(true);
    d->windowsList->clear();
    for(const auto& window : d->fboLayer->windows())
      d->windowsList->addItem(QString::fromStdString(window.fboName.toString()));

    d->windowsList->clearSelection();
    if(auto item = d->windowsList->item(selectedIndex); item)
      item->setSelected(true);

    d->windowsList->blockSignals(false);
  }

  d->updateSelection();
}

}
