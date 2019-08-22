#pragma once

#include <QDataWidgetMapper>
#include <QDialog>

#include "sqlrelationaltablemodel.h"

namespace Ui {
class Devolucao;
}

class Devolucao final : public QDialog {
  Q_OBJECT

public:
  explicit Devolucao(const QString &idVenda, QWidget *parent = nullptr);
  ~Devolucao();

private:
  // attributes
  bool createNewId = false; // REFAC: make this a local variable
  const QString idVenda;
  QDataWidgetMapper mapperItem;
  QString idDevolucao;
  SqlRelationalTableModel modelCliente;
  SqlRelationalTableModel modelDevolvidos;
  SqlRelationalTableModel modelPagamentos;
  SqlRelationalTableModel modelProdutos;
  SqlRelationalTableModel modelVenda;
  Ui::Devolucao *ui;
  // methods
  auto atualizarDevolucao() -> bool;
  auto calcPrecoItemTotal() -> void;
  auto criarContas() -> bool;
  auto criarDevolucao() -> bool;
  auto determinarIdDevolucao() -> void;
  auto devolverItem(const int currentRow) -> bool;
  auto inserirItens(const int currentRow) -> bool;
  auto limparCampos() -> void;
  auto on_doubleSpinBoxCaixas_valueChanged(const double caixas) -> void;
  auto on_doubleSpinBoxQuant_editingFinished() -> void;
  auto on_doubleSpinBoxQuant_valueChanged(double) -> void;
  auto on_doubleSpinBoxTotalItem_valueChanged(double value) -> void;
  auto on_pushButtonDevolverItem_clicked() -> void;
  auto on_tableProdutos_clicked(const QModelIndex &index) -> void;
  auto salvarCredito() -> bool;
  auto setupTables() -> void;
};
