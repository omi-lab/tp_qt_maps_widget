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
  void setLight(const tp_math_utils::Light& light, std::optional<tp_math_utils::LightSwapParameters> lightSwapParamsOpt);

  //################################################################################################
  tp_math_utils::Light light() const;

  //################################################################################################
  bool isTemplate() const;

  //################################################################################################
  std::optional<tp_math_utils::LightSwapParameters> lightSwapParameters() const;

  //################################################################################################
  //! Shows a dialog to edit the light and returns true if accepted.
  static bool editLightDialog(QWidget* parent, tp_math_utils::Light& light, std::optional<tp_math_utils::LightSwapParameters> lightSwapParamsOpt);

Q_SIGNALS:
  //################################################################################################
  //! Emitted when a light is edited by the user.
  void lightEdited();
};
}
#endif

