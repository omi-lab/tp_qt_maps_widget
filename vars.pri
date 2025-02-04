TARGET = tp_qt_maps_widget
TEMPLATE = lib

DEFINES += TP_QT_MAPS_WIDGET_LIBRARY

SOURCES += src/Globals.cpp
HEADERS += inc/tp_qt_maps_widget/Globals.h

SOURCES += src/MapWidget.cpp
HEADERS += inc/tp_qt_maps_widget/MapWidget.h

SOURCES += src/EditLightWidget.cpp
HEADERS += inc/tp_qt_maps_widget/EditLightWidget.h

SOURCES += src/EditMaterialWidget.cpp
HEADERS += inc/tp_qt_maps_widget/EditMaterialWidget.h

SOURCES += src/EditSwapParametersWidget.cpp
HEADERS += inc/tp_qt_maps_widget/EditSwapParametersWidget.h

SOURCES += src/EditMaterialSwapParametersWidget.cpp
HEADERS += inc/tp_qt_maps_widget/EditMaterialSwapParametersWidget.h

SOURCES += src/EditLightSwapParametersWidget.cpp
HEADERS += inc/tp_qt_maps_widget/EditLightSwapParametersWidget.h

SOURCES += src/EditVec3Widget.cpp
HEADERS += inc/tp_qt_maps_widget/EditVec3Widget.h

SOURCES += src/EditGizmoWidget.cpp
HEADERS += inc/tp_qt_maps_widget/EditGizmoWidget.h

SOURCES += src/EditGizmoArrowWidget.cpp
HEADERS += inc/tp_qt_maps_widget/EditGizmoArrowWidget.h

SOURCES += src/EditGizmoPlaneWidget.cpp
HEADERS += inc/tp_qt_maps_widget/EditGizmoPlaneWidget.h

SOURCES += src/EditGizmoRingWidget.cpp
HEADERS += inc/tp_qt_maps_widget/EditGizmoRingWidget.h

SOURCES += src/SelectMaterialWidget.cpp
HEADERS += inc/tp_qt_maps_widget/SelectMaterialWidget.h

SOURCES += src/ConnectContext.cpp
HEADERS += inc/tp_qt_maps_widget/ConnectContext.h

SOURCES += src/ConfigureFBOLayerWidget.cpp
HEADERS += inc/tp_qt_maps_widget/ConfigureFBOLayerWidget.h

SOURCES += src/ProgressEventsGraphWidget.cpp
HEADERS += inc/tp_qt_maps_widget/ProgressEventsGraphWidget.h
