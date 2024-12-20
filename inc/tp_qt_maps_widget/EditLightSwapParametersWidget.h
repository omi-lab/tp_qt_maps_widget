#ifndef tp_qt_maps_widget_EditLightSwapParametersWidget_h
#define tp_qt_maps_widget_EditLightSwapParametersWidget_h

#include "tp_qt_maps_widget/Globals.h"

#include "tp_math_utils/LightSwapParameters.h"

#include <QWidget>

namespace tp_qt_maps_widget
{

//##################################################################################################
class TP_QT_MAPS_WIDGET_SHARED_EXPORT EditLightSwapParametersWidget : public QWidget
{
  Q_OBJECT
  TP_DQ;
public:

  //################################################################################################
  struct Visibility
  {
    bool diffuse{true};
    bool ambient{true};
    bool specular{true};
    bool diffuseScale{true};
    bool spotLightBlend{true};
    bool fov{true};
    bool offsetScale{true};
  };

  //################################################################################################
  EditLightSwapParametersWidget(Visibility visibility, QWidget* parent = nullptr);

  //################################################################################################
  ~EditLightSwapParametersWidget() override;

  //################################################################################################
  void setLightSwapParameters(const tp_math_utils::LightSwapParameters& lightSwapParameters);

  //################################################################################################
  tp_math_utils::LightSwapParameters lightSwapParameters() const;

Q_SIGNALS:
  //################################################################################################
  //! Emitted when material swap parameters are edited by the user.
  void lightSwapParametersEdited();
};
}
#endif
