#ifndef tp_qt_maps_widget_EditLightWidget_h
#define tp_qt_maps_widget_EditLightWidget_h

#include "tp_qt_maps_widget/Globals.h"

#include "tp_math_utils/LightSwapParameters.h"
#include "tp_math_utils/Light.h"

#include <QWidget>
#include <optional>

namespace tp_qt_maps_widget
{
class TP_QT_MAPS_WIDGET_SHARED_EXPORT EditLightWidget : public QWidget
{
  Q_OBJECT
  TP_DQ;
public:
  //################################################################################################
  EditLightWidget(QWidget* parent = nullptr);

  //################################################################################################
  ~EditLightWidget() override;

  //################################################################################################
  void setLight(const tp_math_utils::Light& light);

  //################################################################################################
  tp_math_utils::Light light() const;

Q_SIGNALS:
  //################################################################################################
  //! Emitted when a light is edited by the user.
  void lightEdited();
};
}
#endif

