#ifndef tp_qt_maps_widget_ProgressEventsGraphWidget_h
#define tp_qt_maps_widget_ProgressEventsGraphWidget_h

#include "tp_qt_maps_widget/Globals.h"

#include "tp_utils/Progress.h"

#include <QWidget>

namespace tp_qt_maps_widget
{
//##################################################################################################
class TP_QT_MAPS_WIDGET_SHARED_EXPORT ProgressEventsGraphWidget : public QWidget
{
  Q_OBJECT
  TP_DQ;
public:
  //################################################################################################
  ProgressEventsGraphWidget(QWidget* parent = nullptr);

  //################################################################################################
  ~ProgressEventsGraphWidget() override;

  //################################################################################################
  void setProgressEvents(const std::vector<tp_utils::ProgressEvent>& progressEvents);
};
}
#endif

