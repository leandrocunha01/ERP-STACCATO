#pragma once

#include "sqlrelationaltablemodel.h"

#include <QDialog>

namespace Ui {
class InputDialogConfirmacao;
}

class InputDialogConfirmacao final : public QDialog {
  Q_OBJECT

public:
  enum class Tipo { Recebimento, Entrega, Representacao };

  explicit InputDialogConfirmacao(const Tipo tipo, QWidget *parent = nullptr);
  ~InputDialogConfirmacao() final;
  auto getDate() const -> QDate;
  auto getEntregou() const -> QString;
  auto getNextDateTime() const -> QDate;
  auto getRecebeu() const -> QString;
  auto setFilterEntrega(const QString &id, const QString &idEvento) -> bool;
  auto setFilterRecebe(const QStringList &ids) -> bool;

private:
  // attributes
  const Tipo tipo;
  SqlRelationalTableModel modelEstoque;
  SqlRelationalTableModel modelVeiculo;
  Ui::InputDialogConfirmacao *ui;
  // methods
  auto cadastrar() -> bool;
  auto criarReposicaoCliente(SqlRelationalTableModel &modelVendaProduto, const double caixasDefeito, const double unCaixa) -> bool;
  auto desfazerConsumo(const int idEstoque, const double caixasDefeito) -> bool;
  auto dividirEntrega(const int row, const int choice, const double caixasDefeito, const double unCaixa) -> bool;
  auto dividirLinhaRecebimento(const int row, const double caixas, const double caixasDefeito, const double unCaixa) -> bool;
  auto dividirRecebimento(const int row, const double caixasDefeito, const double unCaixa) -> bool;
  auto gerarCreditoCliente(const SqlRelationalTableModel &modelVendaProduto, const double caixasDefeito, const double unCaixa) -> bool;
  auto on_dateEditEvento_dateChanged(const QDate &date) -> void;
  auto on_pushButtonFaltando_clicked() -> void;
  auto on_pushButtonFoto_clicked() -> void;
  auto on_pushButtonQuebrado_clicked() -> void;
  auto on_pushButtonSalvar_clicked() -> void;
  auto processarQuebra(const int row, const int choice, const double caixasDefeito, const double unCaixa) -> bool;
  auto setupTables() -> void;
};
