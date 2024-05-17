#ifndef tp_qt_maps_widget_EditGizmoPlaneWidget_h
#define tp_qt_maps_widget_EditGizmoPlaneWidget_h

#include "tp_qt_maps_widget/Globals.h"

#include "tp_maps/layers/GizmoLayer.h"

#include <QWidget>

namespace tp_qt_maps_widget
{
class TP_QT_MAPS_WIDGET_SHARED_EXPORT EditGizmoPlaneWidget : public QWidget
{
  Q_OBJECT
  TP_DQ;
public:
  //################################################################################################
  EditGizmoPlaneWidget(bool optionalFields=false, QWidget* parent = nullptr);

  //################################################################################################
  ~EditGizmoPlaneWidget() override;

  //################################################################################################
  void setGizmoPlaneParameters(const tp_maps::GizmoPlaneParameters& gizmoPlaneParameters);

  //################################################################################################
  const tp_maps::GizmoPlaneParameters& gizmoPlaneParameters() const;

  //################################################################################################
  void updateGizmoPlaneParameters(tp_maps::GizmoPlaneParameters& gizmoPlaneParameters) const;

  //################################################################################################
  tp_utils::CallbackCollection<void()> edited;
};
}
#endif

