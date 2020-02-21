#pragma once

#include "sqltablemodel.h"

#include <QWidget>

namespace Ui {
class WidgetOrcamento;
}

class WidgetOrcamento final : public QWidget {
  Q_OBJECT

public:
  explicit WidgetOrcamento(QWidget *parent = nullptr);
  ~WidgetOrcamento();
  auto resetTables() -> void;
  auto updateTables() -> void;

private:
  // attributes
  bool isSet = false;
  bool modelIsSet = false;
  SqlTableModel modelViewOrcamento;
  Ui::WidgetOrcamento *ui;
  // methods
  auto listarLojas() -> bool;
  auto montaFiltro() -> void;
  auto on_comboBoxLojas_currentIndexChanged() -> void;
  auto on_groupBoxStatus_toggled(const bool enabled) -> void;
  auto on_pushButtonCriarOrc_clicked() -> void;
  auto on_pushButtonFollowup_clicked() -> void;
  auto on_table_activated(const QModelIndex &index) -> void;
  auto setConnections() -> void;
  auto setPermissions() -> void;
  auto setupTables() -> void;
  auto unsetConnections() -> void;
};
