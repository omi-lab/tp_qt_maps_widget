#ifndef tp_qt_maps_widget_EditGizmoWidget_h
#define tp_qt_maps_widget_EditGizmoWidget_h

#include "tp_qt_maps_widget/Globals.h"

#include "tp_maps/layers/GizmoLayer.h"

#include <QWidget>

class QVBoxLayout;

namespace tp_qt_maps_widget
{
class TP_QT_MAPS_WIDGET_SHARED_EXPORT EditGizmoWidget : public QWidget
{
  Q_OBJECT
  TP_DQ;
public:
  //################################################################################################
  EditGizmoWidget(const std::function<void(QVBoxLayout*)>& populateLoadAndSave={},
                  QWidget* parent = nullptr);

  //################################################################################################
  ~EditGizmoWidget() override;

  //################################################################################################
  void setGizmoParameters(const tp_maps::GizmoParameters& gizmoParameters);

  //################################################################################################
  const tp_maps::GizmoParameters& gizmoParameters() const;

  //################################################################################################
  //! Shows a dialog to edit the gizmo parameters and returns true if accepted.
  static bool editGizmoDialog(QWidget* parent, tp_maps::GizmoParameters& gizmoParameters);

  //################################################################################################
  tp_utils::CallbackCollection<void()> edited;
};
}
#endif

