#include "tp_qt_maps_widget/MapWidget.h"
#include "tp_qt_maps_widget/ConnectContext.h"

#include "tp_qt_maps/Globals.h"

#include "tp_qt_utils/CrossThreadCallback.h"

#include "tp_maps/MouseEvent.h"
#include "tp_maps/KeyEvent.h"

#include "tp_utils/DebugUtils.h"
#include "tp_utils/TimeUtils.h"
#include "tp_utils/StackTrace.h"

#include <QMouseEvent>
#include <QWheelEvent>
#include <QTimer>

namespace tp_qt_maps_widget
{
namespace
{

//##################################################################################################
tp_maps::KeyboardModifier convertKeyboardModifiers(Qt::KeyboardModifiers modifiers)
{
  tp_maps::KeyboardModifier m{tp_maps::KeyboardModifier::None};
  if(modifiers & Qt::ShiftModifier  ) m = m | tp_maps::KeyboardModifier::Shift;
  if(modifiers & Qt::ControlModifier) m = m | tp_maps::KeyboardModifier::Control;
  if(modifiers & Qt::AltModifier    ) m = m | tp_maps::KeyboardModifier::Alt;
  return m;
}

//##################################################################################################
int32_t toScancode(int key)
{
  switch(key)
  {
  case Qt::Key_PageUp:	   return TP_PAGE_UP_KEY;
  case Qt::Key_PageDown:   return TP_PAGE_DOWN_KEY;
  case Qt::Key_Up:		     return TP_UP_KEY;
  case Qt::Key_Down:		   return TP_DOWN_KEY;
  case Qt::Key_Left:		   return TP_LEFT_KEY;
  case Qt::Key_Right:		   return TP_RIGHT_KEY;
  case Qt::Key_Space:		   return TP_SPACE_KEY;
  case Qt::Key_Shift:      return TP_L_SHIFT_KEY;
  case Qt::Key_Control:    return TP_L_CTRL_KEY;
  }

  return int32_t(key - Qt::Key_A) + TP_A_KEY;
}

class Map_lt final : public tp_maps::Map
{
public:
  TP_NONCOPYABLE(Map_lt);

  //################################################################################################
  Map_lt(MapWidget* mapWidget_):
    tp_maps::Map(false)
  {
    mapWidget = mapWidget_;
    setOpenGLProfile(tp_qt_maps::getOpenGLProfile());
  }

  //################################################################################################
  ~Map_lt() override
  {
    preDelete();
  }

  //################################################################################################
  using tp_maps::Map::setVisible;

  //################################################################################################
  using tp_maps::Map::invalidateBuffers;

  // GL functions
  //################################################################################################
  void makeCurrent() override
  {
    if(inPaint() == nullptr)
    {
      assert(mapWidget->isValid());
      mapWidget->makeCurrent();
    }
    else if(inPaint() != this)
    {
      tpWarning() << "Nested makeCurrent() call for a different context.";
      tp_utils::printStackTrace();
    }
  }

  //################################################################################################
  void update(tp_maps::RenderFromStage renderFromStage=tp_maps::RenderFromStage::Full) override
  {
    tp_maps::Map::update(renderFromStage);

    if(!inPaint())
      mapWidget->update();
  }

  //################################################################################################
  void callAsync(const std::function<void()>& callback) override
  {
    callAsyncRequests.push_back(callback);
    callAsyncProcess.call();
  }

  //################################################################################################
  std::vector<std::function<void()>> callAsyncRequests;
  tp_qt_utils::CrossThreadCallback callAsyncProcess = tp_qt_utils::CrossThreadCallback([&]
  {
    while(!callAsyncRequests.empty())
      tpTakeFirst(callAsyncRequests)();
  });

  MapWidget* mapWidget;
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

  QMetaObject::Connection aboutToBeDestroyedConnection;

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
    case Qt::RightButton:  return tp_maps::Button::RightButton;
    case Qt::LeftButton:   return tp_maps::Button::LeftButton;
    case Qt::MiddleButton: return tp_maps::Button::MiddleButton;
    default:               return tp_maps::Button::NoButton;
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

  d->map->setVisible(false);
}

//##################################################################################################
MapWidget::~MapWidget()
{
  disconnect(d->aboutToBeDestroyedConnection);
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
    connect(t, &QTimer::timeout, this, [&, t](){update();delete t;});
    t->start(0);
  }

  Q_EMIT initialized();

  if(d->aboutToBeDestroyedConnection)
    disconnect(d->aboutToBeDestroyedConnection);

  d->aboutToBeDestroyedConnection = connectContext(context(), this, [&]
  {
    d->map->invalidateBuffers();
  });
}

//##################################################################################################
void MapWidget::resizeGL(int width, int height)
{
  d->map->resizeGL(width*devicePixelRatio(), height*devicePixelRatio());
}

//##################################################################################################
void MapWidget::paintGL()
{
  d->map->paintGL();
}

//##################################################################################################
void MapWidget::mousePressEvent(QMouseEvent* event)
{
  tp_maps::MouseEvent e(tp_maps::MouseEventType::Press);

  e.button = Private::convertMouseButton(event->button());
  e.pos.x = event->pos().x()*devicePixelRatio();
  e.pos.y = event->pos().y()*devicePixelRatio();

  e.modifiers = convertKeyboardModifiers(event->modifiers());

  if(d->map->mouseEvent(e))
    event->accept();
}

//##################################################################################################
void MapWidget::mouseMoveEvent(QMouseEvent* event)
{
  tp_maps::MouseEvent e(tp_maps::MouseEventType::Move);

  e.button = Private::convertMouseButton(event->button());
  e.pos.x = event->pos().x()*devicePixelRatio();
  e.pos.y = event->pos().y()*devicePixelRatio();

  e.modifiers = convertKeyboardModifiers(event->modifiers());

  if(d->map->mouseEvent(e))
    event->accept();
}

//##################################################################################################
void MapWidget::mouseReleaseEvent(QMouseEvent* event)
{
  tp_maps::MouseEvent e(tp_maps::MouseEventType::Release);

  e.button = Private::convertMouseButton(event->button());
  e.pos.x = event->pos().x()*devicePixelRatio();
  e.pos.y = event->pos().y()*devicePixelRatio();

  e.modifiers = convertKeyboardModifiers(event->modifiers());

  if(d->map->mouseEvent(e))
    event->accept();
}

//##################################################################################################
void MapWidget::wheelEvent(QWheelEvent* event)
{
  tp_maps::MouseEvent e(tp_maps::MouseEventType::Wheel);

#if QT_VERSION < 0x060000
  e.pos.x = event->pos().x()*devicePixelRatio();
  e.pos.y = event->pos().y()*devicePixelRatio();
  e.delta = event->delta();
#else
  e.pos.x = event->position().x()*devicePixelRatio();
  e.pos.y = event->position().y()*devicePixelRatio();
  e.delta = event->angleDelta().y();
#endif

  e.modifiers = convertKeyboardModifiers(event->modifiers());

  if(d->map->mouseEvent(e))
    event->accept();
}

//##################################################################################################
void MapWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
  tp_maps::MouseEvent e(tp_maps::MouseEventType::DoubleClick);

  e.button = Private::convertMouseButton(event->button());
  e.pos.x = event->pos().x()*devicePixelRatio();
  e.pos.y = event->pos().y()*devicePixelRatio();

  e.modifiers = convertKeyboardModifiers(event->modifiers());

  if(d->map->mouseEvent(e))
    event->accept();
}

//##################################################################################################
void MapWidget::keyPressEvent(QKeyEvent *event)
{
  tp_maps::KeyEvent e(tp_maps::KeyEventType::Press);
  e.scancode = toScancode(event->key());
  e.modifiers = convertKeyboardModifiers(event->modifiers());
  if(d->map->keyEvent(e))
    event->accept();
}

//##################################################################################################
void MapWidget::keyReleaseEvent(QKeyEvent *event)
{
  tp_maps::KeyEvent e(tp_maps::KeyEventType::Release);
  e.scancode = toScancode(event->key());
  e.modifiers = convertKeyboardModifiers(event->modifiers());
  if(d->map->keyEvent(e))
    event->accept();
}

//##################################################################################################
void MapWidget::timerEvent(QTimerEvent* event)
{
  if(event->timerId() == d->animationTimerID)
    d->map->animate(double(tp_utils::currentTimeMS()));
}

//##################################################################################################
void MapWidget::hideEvent(QHideEvent* event)
{
  d->map->setVisible(false);
  event->accept();
}

//##################################################################################################
void MapWidget::showEvent(QShowEvent* event)
{
  d->map->setVisible(true);
  event->accept();
}

}
