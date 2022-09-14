#ifndef tp_qt_maps_widget_EditVec3Widget_h
#define tp_qt_maps_widget_EditVec3Widget_h

#include "tp_qt_maps_widget/Globals.h"

#include <QWidget>

namespace tp_qt_maps_widget
{

class TP_QT_MAPS_WIDGET_SHARED_EXPORT EditVec3Widget : public QWidget
{
  Q_OBJECT
public:
  //################################################################################################
  EditVec3Widget(const QString& label, QWidget* parent = nullptr);

  //################################################################################################
  ~EditVec3Widget() override;

  //################################################################################################
  void setVec3(const glm::vec3& vec);

  //################################################################################################
  glm::vec3 vec() const;

  //################################################################################################
  //! Shows a dialog to edit the vector and returns true if accepted.
  static bool editVec3Dialog(QWidget* parent, const QString& title, const QString& label, glm::vec3& vec);

private:
  struct Private;
  Private* d;
  friend struct Private;
};
}
#endif
