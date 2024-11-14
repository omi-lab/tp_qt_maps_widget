#include "tp_qt_maps_widget/ProgressEventsGraph.h"
#include "tp_qt_maps_widget/MapWidget.h"

#include "tp_maps/controllers/GraphController.h"
#include "tp_maps/layers/LinesLayer.h"
#include "tp_maps/PickingResult.h"
#include "tp_maps/picking_results/LinesPickingResult.h"

#include "tp_utils/TimeUtils.h"

#include <QBoxLayout>
#include <QHelpEvent>
#include <QToolTip>

namespace tp_qt_maps_widget
{

#ifdef TP_LINUX
#warning rename this class ProgressEventsGraphWidget
#endif

namespace
{

//##################################################################################################
class MapWidget_lt : public tp_qt_maps_widget::MapWidget
{
public:
  //################################################################################################
  MapWidget_lt()
  {

  }

  //################################################################################################
  ~MapWidget_lt() override
  {

  }

  //################################################################################################
  tp_utils::CallbackCollection<void(QHelpEvent*, tp_maps::LinesPickingResult*)> linesLayerToolTipEvent;

protected:
  //################################################################################################
  bool event(QEvent* event) override
  {
    if(event->type() == QEvent::ToolTip)
    {
      QHelpEvent* helpEvent = static_cast<QHelpEvent*>(event);

      std::unique_ptr<tp_maps::PickingResult> pickingResult{map()->performPicking("Tool tip", {helpEvent->x(), helpEvent->y()})};
      if(!pickingResult)
      {
        event->ignore();
        return true;
      }

      {
        auto linesLayerPickingResult = dynamic_cast<tp_maps::LinesPickingResult*>(pickingResult.get());
        if(linesLayerPickingResult)
        {
          linesLayerToolTipEvent(helpEvent, linesLayerPickingResult);
          return true;
        }
      }

      event->ignore();
      return true;
    }
    return QWidget::event(event);
  }
};
}

//##################################################################################################
struct ProgressEventsGraph::Private
{
  MapWidget_lt* mapWidget{nullptr};
  tp_maps::GraphController* graphController{nullptr};

  std::vector<tp_maps::Layer*> layers;
  std::vector<tp_utils::ProgressEvent> progressEvents;

  //################################################################################################
  Private()
  {

  }

  //################################################################################################
  void updateDisplay()
  {
    tpDeleteAll(layers);
    layers.clear();

    if(progressEvents.empty())
      return;

    struct TreeItem_lt
    {
      tp_utils::ProgressEvent item;
      std::vector<TreeItem_lt> children;

      bool tryInsert(const tp_utils::ProgressEvent& i)
      {
        if(i.parentId == item.id)
        {
          TreeItem_lt& child = children.emplace_back();
          child.item = i;
          return true;
        }

        for(auto& child : children)
          if(child.tryInsert(i))
            return true;

        return false;
      }

      void flatten(std::vector<tp_utils::ProgressEvent>& progressEvents) const
      {
        progressEvents.push_back(item);
        for(const auto& child : children)
          child.flatten(progressEvents);
      }
    };

    TreeItem_lt root;
    root.item = progressEvents.front();

    for(size_t i=1; i<progressEvents.size(); i++)
      root.tryInsert(progressEvents.at(i));

    progressEvents.clear();
    root.flatten(progressEvents);

    int64_t min = progressEvents.front().start;
    int64_t max = progressEvents.front().end;

    int64_t now = tp_utils::currentTimeMS();

    for(auto& progressEvent : progressEvents)
    {
      if(progressEvent.active)
        progressEvent.end = now;

      if(progressEvent.start<min)
        min = progressEvent.start;

      if(progressEvent.end>max)
        max = progressEvent.end;
    }

    if(max<=min)
      return;

    float range = float(max-min);
    auto toFt = [&](size_t t)
    {
      return float(t-min) / range;
    };

    for(size_t i=0; i<progressEvents.size(); i++)
    {
      const auto& progressEvent = progressEvents.at(i);

      tp_maps::Lines line;
      line.mode = GL_LINE_LOOP;
      line.color = progressEvent.color.toFloat4<glm::vec4>();
      line.lines.reserve(4);

      float x1 = toFt(progressEvent.start);
      float x2 = toFt(progressEvent.end  );

      float y1 = 1.0f - ((float(i)+0.1f) / float(progressEvents.size()));
      float y2 = 1.0f - ((float(i)-0.1f) / float(progressEvents.size()));

      line.lines.emplace_back(x1, y1, 0.0f);
      line.lines.emplace_back(x2, y1, 0.0f);
      line.lines.emplace_back(x2, y2, 0.0f);
      line.lines.emplace_back(x1, y2, 0.0f);

      {
        auto layer = new tp_maps::LinesLayer();
        layer->setDefaultRenderPass(tp_maps::RenderPass::GUI);
        layer->setExcludeFromPicking(false);
        layer->setLines({line});
        mapWidget->map()->addLayer(layer);
        layers.push_back(layer);
      }
    }
  }

  //################################################################################################
  tp_utils::Callback<void(QHelpEvent*, tp_maps::LinesPickingResult*)> linesLayerToolTipEvent = [&](QHelpEvent* helpEvent, tp_maps::LinesPickingResult* result)
  {
    for(size_t i=0; i<layers.size(); i++)
    {
      auto l = layers.at(i);
      if(l == result->layer)
      {
        if(i>=progressEvents.size())
          break;

        const auto& item = progressEvents.at(i);

        uint64_t duration = item.end-item.start;

        auto t=QString("%1 (%2)").arg(QString::fromStdString(item.name)).arg(duration);

        QToolTip::showText(helpEvent->globalPos(), t);

        break;
      }
    }
  };

};

//##################################################################################################
ProgressEventsGraph::ProgressEventsGraph(QWidget* parent):
  QWidget(parent),
  d(new Private())

{
  auto l = new QVBoxLayout(this);
  l->setContentsMargins(0,0,0,0);

  d->mapWidget = new MapWidget_lt();
  l->addWidget(d->mapWidget);
  d->linesLayerToolTipEvent.connect(d->mapWidget->linesLayerToolTipEvent);

  d->graphController = new tp_maps::GraphController(d->mapWidget->map());
}

//##################################################################################################
ProgressEventsGraph::~ProgressEventsGraph()
{
  delete d;
}

//##################################################################################################
void ProgressEventsGraph::setProgressEvents(const std::vector<tp_utils::ProgressEvent>& progressEvents)
{
  d->progressEvents = progressEvents;
  d->updateDisplay();
}

}
