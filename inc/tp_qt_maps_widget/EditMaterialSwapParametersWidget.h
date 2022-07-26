#ifndef tp_qt_maps_widget_EditMaterialSwapParametersWidget_h
#define tp_qt_maps_widget_EditMaterialSwapParametersWidget_h

#include "tp_qt_maps_widget/Globals.h"

#include "tp_maps/Globals.h"

#include <QWidget>

#include "tp_math_utils/MaterialSwapParameters.h"

namespace tp_qt_maps_widget
{

//##################################################################################################
class TP_QT_MAPS_WIDGET_SHARED_EXPORT EditMaterialSwapParametersWidget : public QWidget
{
  Q_OBJECT
public:


  //################################################################################################
  EditMaterialSwapParametersWidget(QWidget* parent = nullptr);

  //################################################################################################
  ~EditMaterialSwapParametersWidget() override;

  //################################################################################################
  tp_math_utils::Material swappedMaterial( const tp_math_utils::Material& material ) const;

  //################################################################################################
  tp_math_utils::MaterialSwapParameters materialSwapParameters() const;

  //################################################################################################
  glm::vec3 swapColor() const;

Q_SIGNALS:
  //################################################################################################
  //! Emitted when material swap parameters are edited by the user.
  void materialSwapParametersEdited();

protected:

private:
  struct Private;
  Private* d;
  friend struct Private;
};
}
#endif
