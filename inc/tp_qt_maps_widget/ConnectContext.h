#ifndef tp_qt_maps_widget_ConnectContext_h
#define tp_qt_maps_widget_ConnectContext_h

#include <QObject>

class QOpenGLContext;

namespace tp_qt_maps_widget
{

//##################################################################################################
//! This is done like this to hide Qt and GLEW from each other.
QMetaObject::Connection connectContext(QOpenGLContext* context, QWidget* parent, const std::function<void()>& aboutToBeDestroyed);

}

#endif

