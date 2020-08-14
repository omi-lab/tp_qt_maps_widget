#include "tp_qt_maps_widget/MapWidget.h"

#include "tp_qt_maps/Globals.h"

#include "tp_maps/MouseEvent.h"
#include "tp_maps/KeyEvent.h"

#include "tp_utils/DebugUtils.h"
#include "tp_utils/TimeUtils.h"

#include <QGLFormat>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QTimer>

namespace tp_qt_maps_widget
{
namespace
{

//##################################################################################################
int32_t toScancode(int key)
{
  switch(key)
  {
  case Qt::Key_PageUp:	   return tp_maps::PAGE_UP_KEY;
  case Qt::Key_PageDown:   return tp_maps::PAGE_DOWN_KEY;
  case Qt::Key_Up:		     return tp_maps::UP_KEY;
  case Qt::Key_Down:		   return tp_maps::DOWN_KEY;
  case Qt::Key_Left:		   return tp_maps::LEFT_KEY;
  case Qt::Key_Right:		   return tp_maps::RIGHT_KEY;
  case Qt::Key_Space:		   return tp_maps::SPACE_KEY;
  case Qt::Key_Shift:      return tp_maps::L_SHIFT_KEY;
  case Qt::Key_Control:    return tp_maps::L_CTRL_KEY;
  }

  return int32_t(key - Qt::Key_A) + tp_maps::A_KEY;
}

class Map_lt : public tp_maps::Map
{
public:
  TP_NONCOPYABLE(Map_lt);

  //################################################################################################
  Map_lt(MapWidget* mapWidget_):
    tp_maps::Map(false)
  {
    mapWidget = mapWidget_;
    setOpenGLProfile(tp_qt_maps::getOpenGLProfile());
    //tpWarning() << int(tp_qt_maps::getOpenGLProfile());
  }

  //################################################################################################
  ~Map_lt() final
  {
    preDelete();
  }

  // GL functions
  //################################################################################################
  void makeCurrent() final
  {
    if(!inPaint)
      mapWidget->makeCurrent();
  }

  //################################################################################################
  void update() final
  {
    if(!inPaint)
      mapWidget->update();
  }

  MapWidget* mapWidget;
  bool inPaint{false};
};
}

//##################################################################################################
struct MapWidget::Private
{
  TP_REF_COUNT_OBJECTS("tp_qt_maps_widget::MapWidget::Private");
  TP_NONCOPYABLE(Private);

  MapWidget* q;
  Map_lt* map;

  int animationTimerID{-1};

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
  //setFocusPolicy(Qt::ClickFocus);

  //Moved into staticInit()
  //  QSurfaceFormat format;
  //  format.setMajorVersion(3);
  //  format.setMinorVersion(2);
  //  QSurfaceFormat::setDefaultFormat(format);
  //  setFormat(format);
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
void MapWidget::setAnimationInterval(int64_t interval)
{
  if(d->animationTimerID>0)
    killTimer(d->animationTimerID);
  d->animationTimerID = startTimer(int(interval));
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
  d->map->makeCurrent();
  d->map->inPaint=true;
  d->map->paintGL();
  d->map->inPaint=false;
}

//##################################################################################################
void MapWidget::mousePressEvent(QMouseEvent* event)
{
  tp_maps::MouseEvent e(tp_maps::MouseEventType::Press);

  e.button = Private::convertMouseButton(event->button());
  e.pos.x = event->pos().x();
  e.pos.y = event->pos().y();

  if(d->map->mouseEvent(e))
    event->accept();
}

//##################################################################################################
void MapWidget::mouseMoveEvent(QMouseEvent* event)
{
  tp_maps::MouseEvent e(tp_maps::MouseEventType::Move);

  e.button = Private::convertMouseButton(event->button());
  e.pos.x = event->pos().x();
  e.pos.y = event->pos().y();

  if(d->map->mouseEvent(e))
    event->accept();
}

//##################################################################################################
void MapWidget::mouseReleaseEvent(QMouseEvent* event)
{
  tp_maps::MouseEvent e(tp_maps::MouseEventType::Release);

  e.button = Private::convertMouseButton(event->button());
  e.pos.x = event->pos().x();
  e.pos.y = event->pos().y();

  if(d->map->mouseEvent(e))
    event->accept();
}

//##################################################################################################
void MapWidget::wheelEvent(QWheelEvent* event)
{
  tp_maps::MouseEvent e(tp_maps::MouseEventType::Wheel);

  e.pos.x = event->pos().x();
  e.pos.y = event->pos().y();

  e.delta = event->delta();

  if(d->map->mouseEvent(e))
    event->accept();
}

//##################################################################################################
void MapWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
  tp_maps::MouseEvent e(tp_maps::MouseEventType::DoubleClick);

  e.button = Private::convertMouseButton(event->button());
  e.pos.x = event->pos().x();
  e.pos.y = event->pos().y();

  if(d->map->mouseEvent(e))
    event->accept();
}

//##################################################################################################
void MapWidget::keyPressEvent(QKeyEvent *event)
{
  tp_maps::KeyEvent e(tp_maps::KeyEventType::Press);
  e.scancode = toScancode(event->key());
  d->map->keyEvent(e);
}

//##################################################################################################
void MapWidget::keyReleaseEvent(QKeyEvent *event)
{
  tp_maps::KeyEvent e(tp_maps::KeyEventType::Release);
  e.scancode = toScancode(event->key());
  d->map->keyEvent(e);
}

//##################################################################################################
void MapWidget::timerEvent(QTimerEvent* event)
{
  if(event->timerId() == d->animationTimerID)
    d->map->animate(double(tp_utils::currentTimeMS()));
}
}
