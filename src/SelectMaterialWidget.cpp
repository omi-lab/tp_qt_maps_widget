#include "tp_qt_maps_widget/SelectMaterialWidget.h"
#include "tp_qt_maps_widget/MapWidget.h"

#include "tp_qt_maps/ConvertTexture.h"

#include "tp_maps/layers/Geometry3DLayer.h"
#include "tp_maps/Geometry3DPool.h"

#include "tp_math_utils/Sphere.h"

#include "tp_utils/JSONUtils.h"

#include <QDialog>
#include <QBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPointer>

namespace tp_qt_maps_widget
{

//##################################################################################################
struct SelectMaterialWidget::Private
{
  SelectMaterialWidget* q;
  std::vector<tp_math_utils::Material> materials;

  QSize iconSize{64, 64};

  MapWidget* preview{nullptr};
  QListWidget* thumbnails{nullptr};

  tp_maps::Geometry3DLayer* geometryLayer{nullptr};
  tp_math_utils::Geometry3D geometry;

  //################################################################################################
  Private(SelectMaterialWidget* q_):
    q(q_)
  {

  }

  //################################################################################################
  void addItem(const tp_math_utils::Material& material)
  {
    QIcon thumbnail;
    tp_image_utils::ColorMap image;

    if(preview->map()->initialized())
    {
      setPreviewMaterial(material);
      preview->map()->renderToImage(size_t(iconSize.width()), size_t(iconSize.height()), image);
      setPreviewMaterial(q->material());
    }

    thumbnail.addPixmap(QPixmap::fromImage(tp_qt_maps::convertTexture(image)));
    thumbnails->addItem(new QListWidgetItem(thumbnail, QString::fromStdString(material.name.keyString())));

    materials.push_back(material);
  }

  //################################################################################################
  void setPreviewMaterial(const tp_math_utils::Material& material)
  {
    geometry.material = material;
    geometryLayer->setGeometry({geometry});
  }

  //################################################################################################
  void regenerateItems()
  {
    std::vector<tp_math_utils::Material> m;
    m.swap(materials);
    q->setMaterials(m);
  }
};

//##################################################################################################
SelectMaterialWidget::SelectMaterialWidget(QWidget* parent):
  QWidget(parent),
  d(new Private(this))
{
  auto l = new QVBoxLayout(this);
  l->setContentsMargins(0,0,0,0);

  l->addWidget(new QLabel("Preview"));
  d->preview = new MapWidget();
  l->addWidget(d->preview);

  l->addWidget(new QLabel("Gallery"));
  d->thumbnails = new QListWidget();
  l->addWidget(d->thumbnails);

  d->thumbnails->setViewMode(QListWidget::IconMode);
  d->thumbnails->setIconSize(d->iconSize);
  d->thumbnails->setResizeMode(QListWidget::Adjust);
  connect(d->thumbnails, &QListWidget::currentRowChanged, this, [&]{d->setPreviewMaterial(material()); emit selectionChanged();});

  d->geometryLayer = new tp_maps::Geometry3DLayer();
  d->preview->map()->addLayer(d->geometryLayer);

  d->geometry = tp_math_utils::Sphere::octahedralClass1(1.0f, 6, GL_TRIANGLE_FAN, GL_TRIANGLE_STRIP, GL_TRIANGLES);
  d->geometryLayer->setGeometry({d->geometry});

  connect(d->preview, &MapWidget::initialized, this, [&]{d->regenerateItems();});
}

//##################################################################################################
SelectMaterialWidget::~SelectMaterialWidget()
{
  delete d;
}

//##################################################################################################
void SelectMaterialWidget::setIconSize(const QSize& iconSize)
{
  d->iconSize = iconSize;
  d->regenerateItems();
}

//##################################################################################################
void SelectMaterialWidget::setMaterials(const std::vector<tp_math_utils::Material>& materials)
{
  d->materials.clear();
  d->thumbnails->clear();

  for(const auto& material : materials)
    d->addItem(material);

  if(d->thumbnails->count()>0)
    d->thumbnails->setCurrentRow(0);
}

//##################################################################################################
void SelectMaterialWidget::setMaterial(const tp_math_utils::Material& material)
{
  blockSignals(true);
  TP_CLEANUP([&]{blockSignals(false);});

  for(size_t i=0; i<d->materials.size(); i++)
  {
    const auto& m = d->materials.at(i);
    if(material.name == m.name)
    {
      d->thumbnails->setCurrentRow(int(i));
      return;
    }
  }

  d->addItem(material);

  if(d->thumbnails->count()>0)
    d->thumbnails->setCurrentRow(d->thumbnails->count()-1);
}

//##################################################################################################
tp_math_utils::Material SelectMaterialWidget::material() const
{
  size_t i = d->thumbnails->currentRow();

  if(i<d->materials.size())
    return d->materials.at(i);

  return tp_math_utils::Material();
}

//##################################################################################################
bool SelectMaterialWidget::selectMaterialDialog(QWidget* parent, const std::vector<tp_math_utils::Material>& materials, tp_math_utils::Material& material)
{
  QPointer<QDialog> dialog = new QDialog(parent);
  TP_CLEANUP([&]{delete dialog;});

  dialog->setWindowTitle("Select Material");

  auto l = new QVBoxLayout(dialog);

  auto editMaterialWidget = new SelectMaterialWidget();
  l->addWidget(editMaterialWidget);
  editMaterialWidget->setMaterials(materials);
  editMaterialWidget->setMaterial(material);

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
