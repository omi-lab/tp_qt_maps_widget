#ifndef tp_qt_maps_widget_global_h
#define tp_qt_maps_widget_global_h

#include "tp_math_utils/Material.h"

#if defined(TP_QT_MAPS_WIDGET_LIBRARY)
#  define TP_QT_MAPS_WIDGET_SHARED_EXPORT TP_EXPORT
#else
#  define TP_QT_MAPS_WIDGET_SHARED_EXPORT TP_IMPORT
#endif

class QVBoxLayout;

//##################################################################################################
//! A simple 3D engine for widget based applications.
namespace tp_qt_maps_widget
{

//##################################################################################################
int staticInit();

//##################################################################################################
std::vector<tp_math_utils::Material> materialLibrary();

//##################################################################################################
struct OptionalEditRow
{
  //################################################################################################
  static OptionalEditRow init(bool optionalFields, QVBoxLayout* l);

  QVBoxLayout* l;
  std::function<bool()> enabled;
};

}

#endif
