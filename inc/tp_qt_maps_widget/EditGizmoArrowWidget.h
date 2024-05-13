#ifndef tp_qt_maps_widget_EditGizmoArrowWidget_h
#define tp_qt_maps_widget_EditGizmoArrowWidget_h

#include "tp_qt_maps_widget/Globals.h"

#include "tp_maps/layers/GizmoLayer.h"

#include <QWidget>

namespace tp_qt_maps_widget
{
class TP_QT_MAPS_WIDGET_SHARED_EXPORT EditGizmoArrowWidget : public QWidget
{
  Q_OBJECT
  TP_DQ;
public:
  //################################################################################################
  EditGizmoArrowWidget(QWidget* parent = nullptr);

  //################################################################################################
  ~EditGizmoArrowWidget() override;

  //################################################################################################
  void setGizmoArrowParameters(const tp_maps::GizmoArrowParameters& gizmoArrowParameters);

  //################################################################################################
  const tp_maps::GizmoArrowParameters& gizmoArrowParameters() const;

  //################################################################################################
  tp_utils::CallbackCollection<void()> edited;
};
}
#endif

