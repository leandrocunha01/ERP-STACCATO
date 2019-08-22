#pragma once

#include <QWidget>

#include "sqlquerymodel.h"
#include "sqlrelationaltablemodel.h"

namespace Ui {
class WidgetLogisticaEntregues;
}

class WidgetLogisticaEntregues final : public QWidget {
  Q_OBJECT

public:
  explicit WidgetLogisticaEntregues(QWidget *parent = nullptr);
  ~WidgetLogisticaEntregues();
  auto resetTables() -> void;
  auto updateTables() -> void;

private:
  // attributes
  bool isSet = false;
  bool modelIsSet = false;
  SqlQueryModel modelProdutos;
  SqlRelationalTableModel modelVendas;
  Ui::WidgetLogisticaEntregues *ui;
  // methods
  auto cancelar(const QModelIndexList &list) -> bool;
  auto montaFiltro() -> void;
  auto on_pushButtonCancelar_clicked() -> void;
  auto on_tableVendas_clicked(const QModelIndex &index) -> void;
  auto setConnections() -> void;
  auto setupTables() -> void;
};
