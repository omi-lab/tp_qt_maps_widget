#ifndef tp_qt_maps_widget_MapWidget_h
#define tp_qt_maps_widget_MapWidget_h

#include "tp_qt_maps_widget/Globals.h"
#include "tp_maps/Map.h"

#include <QOpenGLWidget>
#include <memory>

namespace tp_qt_maps_widget
{
class TP_QT_MAPS_WIDGET_SHARED_EXPORT MapWidget : public QOpenGLWidget
{
  Q_OBJECT
public:
  //################################################################################################
  MapWidget(QWidget* parent = nullptr);

  //################################################################################################
  ~MapWidget() override;

  //################################################################################################
  tp_maps::Map* map();

  //################################################################################################
  QSize minimumSizeHint() const override;

  //################################################################################################
  QSize sizeHint() const override;

  //################################################################################################
  void setAnimationInterval(int64_t interval);

Q_SIGNALS:
  //################################################################################################
  void initialized();

protected:
  //################################################################################################
  void initializeGL() override;

  //################################################################################################
  void resizeGL(int width, int height) override;

  //################################################################################################
  void paintGL() override;

  //################################################################################################
  void mousePressEvent(QMouseEvent* event) override;

  //################################################################################################
  void mouseMoveEvent(QMouseEvent* event) override;

  //################################################################################################
  void mouseReleaseEvent(QMouseEvent* event) override;

  //################################################################################################
  void wheelEvent(QWheelEvent* event) override;

  //################################################################################################
  void mouseDoubleClickEvent(QMouseEvent* event) override;

  //################################################################################################
  void keyPressEvent(QKeyEvent *event) override;

  //################################################################################################
  void keyReleaseEvent(QKeyEvent *event) override;

  //################################################################################################
  void timerEvent(QTimerEvent *event) override;

  //################################################################################################
  void hideEvent(QHideEvent* event) override;

  //################################################################################################
  void showEvent(QShowEvent* event) override;

private:
  struct Private;
  Private* d;
  friend struct Private;
};
}
#endif

