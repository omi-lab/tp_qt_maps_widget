#ifndef tp_qt_maps_widget_ProgressEventsGraph_h
#define tp_qt_maps_widget_ProgressEventsGraph_h

#include "tp_qt_maps_widget/Globals.h"

#include "tp_utils/Progress.h"

#include <QWidget>

namespace tp_qt_maps_widget
{
//##################################################################################################
class TP_QT_MAPS_WIDGET_SHARED_EXPORT ProgressEventsGraph : public QWidget
{
  Q_OBJECT
public:
  //################################################################################################
  ProgressEventsGraph(QWidget* parent = nullptr);

  //################################################################################################
  ~ProgressEventsGraph() override;

  //################################################################################################
  void setProgressEvents(const std::vector<tp_utils::ProgressEvent>& progressEvents);

private:
  struct Private;
  Private* d;
  friend struct Private;
};
}
#endif

