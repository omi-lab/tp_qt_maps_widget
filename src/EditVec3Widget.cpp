#include "tp_qt_maps_widget/EditVec3Widget.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QBoxLayout>
#include <QLabel>
#include <QPointer>

namespace tp_qt_maps_widget
{
//##################################################################################################
struct EditVec3Widget::Private
{
  QDoubleSpinBox* x{nullptr};
  QDoubleSpinBox* y{nullptr};
  QDoubleSpinBox* z{nullptr};
};

//##################################################################################################
EditVec3Widget::EditVec3Widget(const QString& label, QWidget* parent):
  QWidget(parent),
  d(new Private())
{
  auto l = new QVBoxLayout(this);
  l->setContentsMargins(0,0,0,0);

  l->addWidget(new QLabel(label));

  auto hl = new QHBoxLayout();
  hl->setContentsMargins(0,0,0,0);
  l->addLayout(hl);

  auto addSpin = [&]()
  {
    auto spin = new QDoubleSpinBox();
    spin->setRange(-1000.0, 1000.0);
    spin->setDecimals(2);
    hl->addWidget(spin);
    return spin;
  };

  d->x = addSpin();
  d->y = addSpin();
  d->z = addSpin();
}

//##################################################################################################
EditVec3Widget::~EditVec3Widget()
{
  delete d;
}

//##################################################################################################
void EditVec3Widget::setVec3(const glm::vec3& vec)
{
  d->x->setValue(double(vec.x));
  d->y->setValue(double(vec.y));
  d->z->setValue(double(vec.z));
}

//##################################################################################################
glm::vec3 EditVec3Widget::vec() const
{
  return glm::vec3(float(d->x->value()), float(d->y->value()), float(d->z->value()));
}

//##################################################################################################
bool EditVec3Widget::editVec3Dialog(QWidget* parent, const QString& title, const QString& label, glm::vec3& vec)
{
  QPointer<QDialog> dialog = new QDialog(parent);
  TP_CLEANUP([&]{delete dialog;});
  dialog->setWindowTitle(title);
  auto l = new QVBoxLayout(dialog);

  auto widget = new EditVec3Widget(label);
  widget->setVec3(vec);
  l->addWidget(widget);

  l->addStretch();

  auto buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  l->addWidget(buttons);
  connect(buttons, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, dialog, &QDialog::reject);

  if(dialog->exec() == QDialog::Accepted)
  {
    vec = widget->vec();
    return true;
  }

  return false;
}

}
