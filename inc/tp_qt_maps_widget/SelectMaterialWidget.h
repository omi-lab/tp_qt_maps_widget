#ifndef tp_qt_maps_widget_SelectMaterialWidget_h
#define tp_qt_maps_widget_SelectMaterialWidget_h

#include "tp_qt_maps_widget/Globals.h"

#include "tp_maps/Globals.h"

#include <QWidget>

namespace tp_qt_maps_widget
{
class TP_QT_MAPS_WIDGET_SHARED_EXPORT SelectMaterialWidget : public QWidget
{
  Q_OBJECT
public:
  //################################################################################################
  SelectMaterialWidget(QWidget* parent = nullptr);

  //################################################################################################
  ~SelectMaterialWidget() override;

  //################################################################################################
  void setIconSize(const QSize& iconSize);

  //################################################################################################
  void setMaterials(const std::vector<tp_math_utils::Material>& materials);

  //################################################################################################
  void setMaterial(const tp_math_utils::Material& material);

  //################################################################################################
  tp_math_utils::Material material() const;

  //################################################################################################
  //! Shows a dialog to select a material and returns true if accepted.
  static bool selectMaterialDialog(QWidget* parent, const std::vector<tp_math_utils::Material>& materials, tp_math_utils::Material& material);

Q_SIGNALS:
  //################################################################################################
  //! Emitted when the user selects a material.
  void selectionChanged();

private:
  struct Private;
  Private* d;
  friend struct Private;
};
}
#endif

