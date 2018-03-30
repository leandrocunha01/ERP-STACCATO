#ifndef WIDGETCOMPRARESUMO_H
#define WIDGETCOMPRARESUMO_H

#include "sqlrelationaltablemodel.h"
#include "widget.h"

namespace Ui {
class WidgetCompraResumo;
}

class WidgetCompraResumo : public Widget {
  Q_OBJECT

public:
  explicit WidgetCompraResumo(QWidget *parent = nullptr);
  ~WidgetCompraResumo();
  auto updateTables() -> bool;

private:
  // attributes
  Ui::WidgetCompraResumo *ui;
  SqlRelationalTableModel modelResumo;
  // methods
  auto setupTables() -> void;
};

#endif // WIDGETCOMPRARESUMO_H