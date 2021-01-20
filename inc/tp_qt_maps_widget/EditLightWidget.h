#ifndef tp_qt_maps_widget_EditLightWidget_h
#define tp_qt_maps_widget_EditLightWidget_h

#include "tp_qt_maps_widget/Globals.h"

#include "tp_maps/Globals.h"

#include <QWidget>

namespace tp_qt_maps_widget
{
class TP_QT_MAPS_WIDGET_SHARED_EXPORT EditLightWidget : public QWidget
{
  Q_OBJECT
public:
  //################################################################################################
  EditLightWidget(QWidget* parent = nullptr);

  //################################################################################################
  ~EditLightWidget() override;

  //################################################################################################
  void setLight(const tp_math_utils::Light& light);

  //################################################################################################
  tp_math_utils::Light light() const;

  //################################################################################################
  //! Shows a dialog to edit the light and returns true if accepted.
  static bool editLightDialog(QWidget* parent, tp_math_utils::Light& light);

signals:
  //################################################################################################
  //! Emitted when a light is edited by the user.
  void lightEdited();

private:
  struct Private;
  Private* d;
  friend struct Private;
};
}
#endif

