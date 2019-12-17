#ifndef tp_qt_maps_widget_global_h
#define tp_qt_maps_widget_global_h

#include "tp_utils/StringID.h"

#if defined(TP_QT_MAPS_WIDGET_LIBRARY)
#  define TP_QT_MAPS_WIDGET_SHARED_EXPORT TP_EXPORT
#else
#  define TP_QT_MAPS_WIDGET_SHARED_EXPORT TP_IMPORT
#endif

//##################################################################################################
//! A simple 3D engine for widget based applications.
namespace tp_qt_maps_widget
{

//##################################################################################################
int staticInit();

}

#endif
