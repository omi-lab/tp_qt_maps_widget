#ifndef tp_qt_maps_widget_ConnectContext_h
#define tp_qt_maps_widget_ConnectContext_h

#include "tp_qt_maps_widget/ConnectContext.h"

#include <QOpenGLContext>
#include <QWidget>

namespace tp_qt_maps_widget
{

//##################################################################################################
QMetaObject::Connection connectContext(QOpenGLContext* context, QWidget* parent, const std::function<void()>& aboutToBeDestroyed)
{
  return QObject::connect(context, &QOpenGLContext::aboutToBeDestroyed, parent, [=]
  {
    aboutToBeDestroyed();
  });
}

}

#endif

