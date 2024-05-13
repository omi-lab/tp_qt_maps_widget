#ifndef tp_qt_maps_widget_ConfigureFBOLayerWidget_h
#define tp_qt_maps_widget_ConfigureFBOLayerWidget_h

#include "tp_qt_maps_widget/Globals.h"

#include <QWidget>

namespace tp_maps
{
class FBOLayer;
}

namespace tp_qt_maps_widget
{
//##################################################################################################
class TP_QT_MAPS_WIDGET_SHARED_EXPORT ConfigureFBOLayerWidget : public QWidget
{
  Q_OBJECT
  TP_DQ;
public:
  //################################################################################################
  ConfigureFBOLayerWidget(tp_maps::FBOLayer* fboLayer);

  //################################################################################################
  ~ConfigureFBOLayerWidget() override;

  //################################################################################################
  void update();
};
}
#endif

