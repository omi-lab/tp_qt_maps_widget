#ifndef tdp_maps_widget_global_h
#define tdp_maps_widget_global_h

#include <QtCore/qglobal.h>

#if defined(TDP_MAPS_WIDGET_LIBRARY)
#  define TDP_MAPS_WIDGET_SHARED_EXPORT Q_DECL_EXPORT
#else
#  define TDP_MAPS_WIDGET_SHARED_EXPORT Q_DECL_IMPORT
#endif

#include "tp_maps/textures/BasicTexture.h"

class QImage;

//##################################################################################################
//! A simple 3D engine for widget based applications
namespace tdp_maps_widget
{

//##################################################################################################
tp_maps::TextureData convertTexture(const QImage& image);

}
#endif
