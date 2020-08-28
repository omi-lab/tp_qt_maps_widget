#include "tp_qt_maps_widget/Globals.h"

#include <QSurfaceFormat>

namespace tp_qt_maps_widget
{

//##################################################################################################
int staticInit()
{
  QSurfaceFormat format;

  //format.setOption(QSurfaceFormat::DebugContext);

#ifdef TP_OSX
  format.setProfile(QSurfaceFormat::CoreProfile);
  format.setMajorVersion(4);
  format.setMinorVersion(1);
#else
  format.setMajorVersion(3);
  format.setMinorVersion(2);
  format.setSamples(4);
#endif
  QSurfaceFormat::setDefaultFormat(format);

  return 0;
}

}
