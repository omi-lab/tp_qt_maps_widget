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

SOURCES += src/EditVec3Widget.cpp
HEADERS += inc/tp_qt_maps_widget/EditVec3Widget.h

SOURCES += src/SelectMaterialWidget.cpp
HEADERS += inc/tp_qt_maps_widget/SelectMaterialWidget.h

SOURCES += src/ConnectContext.cpp
HEADERS += inc/tp_qt_maps_widget/ConnectContext.h
