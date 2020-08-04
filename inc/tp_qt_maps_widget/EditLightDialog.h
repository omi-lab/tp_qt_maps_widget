#ifndef tp_qt_maps_widget_EditLightDialog_h
#define tp_qt_maps_widget_EditLightDialog_h

#include "tp_qt_maps_widget/Globals.h"

#include "tp_maps/Globals.h"

#include <QWidget>

namespace tp_qt_maps_widget
{
class TP_QT_MAPS_WIDGET_SHARED_EXPORT EditLightDialog : public QWidget
{
  Q_OBJECT
public:
  //################################################################################################
  EditLightDialog(QWidget* parent = nullptr);

  //################################################################################################
  ~EditLightDialog() override;

  //################################################################################################
  void setLight(const tp_maps::Light& light);

  //################################################################################################
  tp_maps::Light light() const;

  //################################################################################################
  //! Shows a dialog to edit the light and returns true if accepted.
  static bool editLight(QWidget* parent, tp_maps::Light& light);

private:
  struct Private;
  Private* d;
  friend struct Private;
};
}
#endif

