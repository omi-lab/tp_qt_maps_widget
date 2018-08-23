QT += core gui widgets opengl

include(../tp_maps/dependencies.pri)

!equals(TARGET, "tdp_maps_widget"){
LIBS += -ltdp_maps_widget
}

INCLUDEPATH  += $$PWD/../tdp_maps_widget/inc/

