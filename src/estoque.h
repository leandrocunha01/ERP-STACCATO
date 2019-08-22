#pragma once

#include <QDialog>

#include "sqlrelationaltablemodel.h"

namespace Ui {
class Estoque;
}

class Estoque final : public QDialog {
  Q_OBJECT

public:
  // REFAC: turn showWindow into a enum
  explicit Estoque(const QString &idEstoque, const bool showWindow = true, QWidget *parent = nullptr);
  ~Estoque();
  auto criarConsumo(const int idVendaProduto, const double quant = 0) -> bool;

private:
  // attributes
  const QString idEstoque; // TODO: change this to int?
  SqlRelationalTableModel modelEstoque;
  SqlRelationalTableModel modelConsumo;
  SqlRelationalTableModel modelViewConsumo;
  Ui::Estoque *ui;

  enum class FieldColors {
    White = 0,     // Não processado
    Green = 1,     // Ok
    Yellow = 2,    // Quant difere
    Red = 3,       // Não encontrado
    DarkGreen = 4, // Consumo
    Cyan = 5       // Devolução
  };

  // methods
  auto calcularRestante() -> void;
  auto desfazerConsumo() -> bool;
  auto dividirCompra(const int idVendaProduto, const double quant) -> std::optional<int>;
  auto exibirNota() -> void;
  auto on_pushButtonExibirNfe_clicked() -> void;
  auto setupTables() -> void;
  auto viewRegisterById(const bool showWindow) -> bool;
};
