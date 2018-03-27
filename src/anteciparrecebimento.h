#ifndef ANTECIPARRECEBIMENTO_H
#define ANTECIPARRECEBIMENTO_H

#include "dialog.h"
#include "sqlrelationaltablemodel.h"

namespace Ui {
class AnteciparRecebimento;
}

class AnteciparRecebimento final : public Dialog {
  Q_OBJECT

public:
  explicit AnteciparRecebimento(QWidget *parent = nullptr);
  ~AnteciparRecebimento();

private:
  // attributes
  // REFAC: 0refactor those out
  bool isBlockedPresente = false;
  bool isBlockedMes = false;
  SqlRelationalTableModel modelViewContaReceber;
  Ui::AnteciparRecebimento *ui;
  // methods
  auto calcularTotais() -> void;
  auto setupTables() -> void;
  auto on_comboBox_currentTextChanged(const QString &text) -> void;
  auto on_doubleSpinBoxValorPresente_valueChanged(double) -> void;
  auto on_pushButtonGerar_clicked() -> void;
  auto on_table_entered(const QModelIndex) -> void;
};

#endif // ANTECIPARRECEBIMENTO_H
