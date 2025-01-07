#include "tp_qt_maps_widget/EditMaterialSwapParametersWidget.h"
#include "tp_qt_maps_widget/EditSwapParametersWidget.h"

#include "tp_qt_widgets/ColorButton.h"

#include <QBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QCheckBox>

namespace tp_qt_maps_widget
{

namespace
{
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

  tp_qt_widgets::ColorButton* colorButton{nullptr};

  EditVec3SwapParametersWidget* albedo  {nullptr};
  EditVec3SwapParametersWidget* sss     {nullptr};
  EditVec3SwapParametersWidget* emission{nullptr};
  EditVec3SwapParametersWidget* velvet  {nullptr};

  BoolEditor albedoHueCheckbox;

  EditFloatSwapParametersWidget* albedoSaturation{nullptr};
  EditFloatSwapParametersWidget* albedoValue     {nullptr};
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

    auto addTitle = [&](QBoxLayout* layout, const QString& name)
    {
      layout->addWidget(new QLabel(QString("<h2>%1</h2>").arg(name)), 2, Qt::AlignLeft);
    };

    auto addSubTitle = [&](QBoxLayout* layout, const QString& name)
    {
      layout->addWidget(new QLabel(QString("<h3>%1</h3>").arg(name)), 2, Qt::AlignLeft);
    };

    addTitle(l, "Material Swap Properties");

    d->colorButton   = new tp_qt_widgets::ColorButton("Color");
    d->colorButton->edited.addCallback([&]{Q_EMIT materialSwapParametersEdited();});
    l->addWidget(d->colorButton);


    addSubTitle(l, "Albedo");
    d->albedo = new EditVec3SwapParametersWidget(VectorComponents::RGB, HelperButtons::Color, 0.0f, 4.0f, 0.0f, 1.0f);
    d->albedo->edited.addCallback([&]{Q_EMIT materialSwapParametersEdited();});
    l->addWidget(d->albedo);


    addSubTitle(l, "SSS");
    d->sss = new EditVec3SwapParametersWidget(VectorComponents::RGB, HelperButtons::Color, 0.0f, 4.0f, 0.0f, 1.0f);
    d->sss->edited.addCallback([&]{Q_EMIT materialSwapParametersEdited();});
    l->addWidget(d->sss);


    addSubTitle(l, "Emission");
    d->emission = new EditVec3SwapParametersWidget(VectorComponents::RGB, HelperButtons::Color, 0.0f, 4.0f, 0.0f, 1.0f);
    d->emission->edited.addCallback([&]{Q_EMIT materialSwapParametersEdited();});
    l->addWidget(d->emission);


    addSubTitle(l, "Velvet");
    d->velvet = new EditVec3SwapParametersWidget(VectorComponents::RGB, HelperButtons::Color, 0.0f, 4.0f, 0.0f, 1.0f);
    d->velvet->edited.addCallback([&]{Q_EMIT materialSwapParametersEdited();});
    l->addWidget(d->velvet);


    addSubTitle(l, "HSV");
    d->albedoHueCheckbox = makeCheckbox( l, "Use Albedo Hue", true);
    connect(d->albedoHueCheckbox.checkbox, &QCheckBox::toggled, this, &EditMaterialSwapParametersWidget::materialSwapParametersEdited);


    addSubTitle(l, "Albedo saturation");
    d->albedoSaturation = new EditFloatSwapParametersWidget(HelperButtons::Default, 0.0f, 4.0f, 0.0f, 1.0f);
    d->albedoSaturation->edited.addCallback([&]{Q_EMIT materialSwapParametersEdited();});
    l->addWidget(d->albedoSaturation);


    addSubTitle(l, "Albedo value");
    d->albedoValue = new EditFloatSwapParametersWidget(HelperButtons::Default, 0.0f, 4.0f, 0.0f, 1.0f);
    d->albedoValue->edited.addCallback([&]{Q_EMIT materialSwapParametersEdited();});
    l->addWidget(d->albedoValue);
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

  d->albedo  ->setVec3SwapParameters(materialSwapParameters.albedo  );
  d->sss     ->setVec3SwapParameters(materialSwapParameters.sss     );
  d->emission->setVec3SwapParameters(materialSwapParameters.emission);
  d->velvet  ->setVec3SwapParameters(materialSwapParameters.velvet  );

  d->albedoHueCheckbox.set(materialSwapParameters.useAlbedoHue);

  d->colorButton->setColor(materialSwapParameters.initialColor);

  d->albedoSaturation->setFloatSwapParameters(materialSwapParameters.albedoSaturation);
  d->albedoValue->setFloatSwapParameters(materialSwapParameters.albedoValue);
}

//##################################################################################################
tp_math_utils::MaterialSwapParameters EditMaterialSwapParametersWidget::materialSwapParameters() const
{
  tp_math_utils::MaterialSwapParameters materialSwapParameters;

  materialSwapParameters.albedo   = d->albedo  ->vec3SwapParameters();
  materialSwapParameters.sss      = d->sss     ->vec3SwapParameters();
  materialSwapParameters.emission = d->emission->vec3SwapParameters();
  materialSwapParameters.velvet   = d->velvet  ->vec3SwapParameters();

  materialSwapParameters.useAlbedoHue = d->albedoHueCheckbox.get();

  materialSwapParameters.initialColor = d->colorButton->toFloat3<glm::vec3>();

  materialSwapParameters.albedoSaturation   = d->albedoSaturation->floatSwapParameters();
  materialSwapParameters.albedoValue   = d->albedoValue->floatSwapParameters();

  return materialSwapParameters;
}

}
