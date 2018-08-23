#include "tdp_maps_widget/MapWidget.h"

#include "tp_maps/MouseEvent.h"

#include "tp_utils/DebugUtils.h"

#include <QGLFramebufferObject>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QTimer>

namespace tdp_maps_widget
{
namespace
{
class Map_lt : public tp_maps::Map
{
public:
  //################################################################################################
  Map_lt(MapWidget* mapWidget_):
    tp_maps::Map(false)
  {
    mapWidget = mapWidget_;
  }

  //################################################################################################
  ~Map_lt()
  {
    preDelete();
  }

  // GL functions
  //################################################################################################
  virtual void makeCurrent()
  {
    mapWidget->makeCurrent();
  }

  //################################################################################################
  virtual void update()
  {
    mapWidget->update();
  }

private:
  MapWidget* mapWidget;
};
}

struct MapWidget::Private
{
  MapWidget* q;
  Map_lt* map;

  //################################################################################################
  Private(MapWidget* q_):
    q(q_)
  {
    map = new Map_lt(q);
  }

  //################################################################################################
  ~Private()
  {
    map->clearLayers();
    delete map;
  }

  //################################################################################################
  static QGLFormat glFormat()
  {
    QGLFormat glf = QGLFormat::defaultFormat();

#ifdef TDP_OSX
    glf.setProfile(QGLFormat::CompatibilityProfile);
    glf.setVersion(2, 1);
    glf.setSampleBuffers(true);
    glf.setSamples(4);
#else
    glf.setSampleBuffers(true);
    glf.setSamples(4);
#endif
    return glf;
  }

  //################################################################################################
  static tp_maps::Button convertMouseButton(Qt::MouseButton button)
  {
    switch(button)
    {
    case Qt::RightButton: return tp_maps::Button::RightButton;
    case Qt::LeftButton:  return tp_maps::Button::LeftButton;
    default:              return tp_maps::Button::NoButton;
    }
  }
};


//##################################################################################################
MapWidget::MapWidget(QWidget *parent):
  QOpenGLWidget(parent),
  d(new Private(this))
{
  setFocusPolicy(Qt::StrongFocus);

  QSurfaceFormat format;
  format.setMajorVersion(3);
  format.setMinorVersion(2);
  QSurfaceFormat::setDefaultFormat(format);
  setFormat(format);
}

//##################################################################################################
MapWidget::~MapWidget()
{
  delete d;
}

//##################################################################################################
tp_maps::Map* MapWidget::map()
{
  return d->map;
}

//##################################################################################################
QSize MapWidget::minimumSizeHint() const
{
  return QSize(50, 50);
}

//##################################################################################################
QSize MapWidget::sizeHint() const
{
  return QSize(200, 200);
}

//##################################################################################################
void MapWidget::initializeGL()
{
  d->map->initializeGL();

  //After a reparent paintGL does not get called and calling update in here does not seem to have
  //any effect, so this little kludge calls update once control returns to the event loop.
  {
    auto t = new QTimer(this);
    t->setSingleShot(true);
    connect(t, &QTimer::timeout, [&, t](){update();delete t;});
    t->start(0);
  }
}

//##################################################################################################
void MapWidget::resizeGL(int width, int height)
{
  d->map->resizeGL(width, height);
}

//##################################################################################################
void MapWidget::paintGL()
{
  d->map->paintGL();
}

//################################################################################################
void MapWidget::mousePressEvent(QMouseEvent* event)
{
  tp_maps::MouseEvent e(tp_maps::MouseEventType::Press);

  e.button = Private::convertMouseButton(event->button());
  e.pos.x = event->pos().x();
  e.pos.y = event->pos().y();

  if(d->map->mouseEvent(e))
    event->accept();
}

//################################################################################################
void MapWidget::mouseMoveEvent(QMouseEvent* event)
{
  tp_maps::MouseEvent e(tp_maps::MouseEventType::Move);

  e.button = Private::convertMouseButton(event->button());
  e.pos.x = event->pos().x();
  e.pos.y = event->pos().y();

  if(d->map->mouseEvent(e))
    event->accept();
}

//################################################################################################
void MapWidget::mouseReleaseEvent(QMouseEvent* event)
{
  tp_maps::MouseEvent e(tp_maps::MouseEventType::Release);

  e.button = Private::convertMouseButton(event->button());
  e.pos.x = event->pos().x();
  e.pos.y = event->pos().y();

  if(d->map->mouseEvent(e))
    event->accept();
}

//################################################################################################
void MapWidget::wheelEvent(QWheelEvent* event)
{
  tp_maps::MouseEvent e(tp_maps::MouseEventType::Wheel);

  e.pos.x = event->pos().x();
  e.pos.y = event->pos().y();

  e.delta = event->delta();

  if(d->map->mouseEvent(e))
    event->accept();
}

//################################################################################################
void MapWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
  tp_maps::MouseEvent e(tp_maps::MouseEventType::DoubleClick);

  e.button = Private::convertMouseButton(event->button());
  e.pos.x = event->pos().x();
  e.pos.y = event->pos().y();

  if(d->map->mouseEvent(e))
    event->accept();
}
}
