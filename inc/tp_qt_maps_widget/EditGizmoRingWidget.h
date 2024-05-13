#ifndef tp_qt_maps_widget_EditGizmoRingWidget_h
#define tp_qt_maps_widget_EditGizmoRingWidget_h

#include "tp_qt_maps_widget/Globals.h"

#include "tp_maps/layers/GizmoLayer.h"

#include <QWidget>

namespace tp_qt_maps_widget
{
class TP_QT_MAPS_WIDGET_SHARED_EXPORT EditGizmoRingWidget : public QWidget
{
  Q_OBJECT
  TP_DQ;
public:
  //################################################################################################
  EditGizmoRingWidget(QWidget* parent = nullptr);

  //################################################################################################
  ~EditGizmoRingWidget() override;

  //################################################################################################
  void setGizmoRingParameters(const tp_maps::GizmoRingParameters& gizmoRingParameters);

  //################################################################################################
  const tp_maps::GizmoRingParameters& gizmoRingParameters() const;

  //################################################################################################
  tp_utils::CallbackCollection<void()> edited;
};
}
#endif

