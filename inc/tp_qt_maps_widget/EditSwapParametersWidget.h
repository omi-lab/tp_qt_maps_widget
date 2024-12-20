#ifndef tp_qt_maps_widget_EditSwapParametersWidget_h
#define tp_qt_maps_widget_EditSwapParametersWidget_h

#include "tp_qt_maps_widget/Globals.h"

#include "tp_math_utils/SwapParameters.h"

#include "tp_qt_widgets/SpinSlider.h"

#include "tp_utils/CallbackCollection.h"

#include <QWidget>

namespace tp_qt_maps_widget
{

//##################################################################################################
enum class HelperButtons
{
  Default,
  Color,
  Range
};

//##################################################################################################
enum class VectorComponents
{
  RGB,
  XYZ
};

//##################################################################################################
class TP_QT_MAPS_WIDGET_SHARED_EXPORT EditVec3ComponentWidget : public QWidget
{
  Q_OBJECT
  TP_DQ;
public:
  //################################################################################################
  EditVec3ComponentWidget(VectorComponents vectorComponents,
                          tp_qt_widgets::SliderMode sliderMode,
                          float min,
                          float max,
                          const QString& title,
                          QWidget* parent = nullptr);

  //################################################################################################
  ~EditVec3ComponentWidget() override;

  //################################################################################################
  void setVec3(const glm::vec3& vec3);

  //################################################################################################
  glm::vec3 vec3() const;

  //################################################################################################
  tp_utils::CallbackCollection<void()> edited;
};

//##################################################################################################
class TP_QT_MAPS_WIDGET_SHARED_EXPORT EditVec3SwapParametersWidget : public QWidget
{
  Q_OBJECT
  TP_DQ;
public:
  //################################################################################################
  EditVec3SwapParametersWidget(VectorComponents vectorComponents,
                               HelperButtons helperButtons,
                               float scaleMin,
                               float scaleMax,
                               float biasMin,
                               float biasMax,
                               QWidget* parent = nullptr);

  //################################################################################################
  ~EditVec3SwapParametersWidget() override;

  //################################################################################################
  void setVec3SwapParameters(const tp_math_utils::Vec3SwapParameters& vec3SwapParameters);

  //################################################################################################
  tp_math_utils::Vec3SwapParameters vec3SwapParameters() const;

  //################################################################################################
  tp_utils::CallbackCollection<void()> edited;
};

//##################################################################################################
class TP_QT_MAPS_WIDGET_SHARED_EXPORT EditFloatSwapParametersWidget : public QWidget
{
  Q_OBJECT
  TP_DQ;
public:
  //################################################################################################
  EditFloatSwapParametersWidget(HelperButtons helperButtons,
                               float scaleMin,
                               float scaleMax,
                               float biasMin,
                               float biasMax,
                               QWidget* parent = nullptr);

  //################################################################################################
  ~EditFloatSwapParametersWidget() override;

  //################################################################################################
  void setFloatSwapParameters(const tp_math_utils::FloatSwapParameters& floatSwapParameters);

  //################################################################################################
  tp_math_utils::FloatSwapParameters floatSwapParameters() const;

  //################################################################################################
  tp_utils::CallbackCollection<void()> edited;
};

}
#endif
