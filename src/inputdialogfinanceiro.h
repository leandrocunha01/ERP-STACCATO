#ifndef INPUTDIALOGFINANCEIRO_H
#define INPUTDIALOGFINANCEIRO_H

#include <QComboBox>
#include <QDateEdit>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QLineEdit>

#include "sqltablemodel.h"

namespace Ui {
class InputDialogFinanceiro;
}

class InputDialogFinanceiro : public QDialog {
  Q_OBJECT

public:
  enum class Tipo { ConfirmarCompra, Financeiro };

  explicit InputDialogFinanceiro(const Tipo &tipo, QWidget *parent = 0);
  ~InputDialogFinanceiro();
  QDateTime getDate() const;
  QDateTime getNextDate() const;
  bool setFilter(const QString &idCompra);

private slots:
  void on_checkBoxMarcarTodos_toggled(bool checked);
  void on_dateEditEvento_dateChanged(const QDate &date);
  void on_dateEditPgtSt_userDateChanged(const QDate &);
  void on_doubleSpinBoxAdicionais_valueChanged(const double value);
  void on_doubleSpinBoxFrete_valueChanged(double);
  void on_doubleSpinBoxTotalPag_valueChanged(double);
  void on_pushButtonAdicionarPagamento_clicked();
  void on_pushButtonCorrigirFluxo_clicked();
  void on_pushButtonLimparPag_clicked();
  void on_pushButtonSalvar_clicked();

private:
  // attributes
  bool representacao;
  // TODO: 0refactor those out
  bool isBlockedFluxoCaixa = false;
  bool isBlockedStDate = false;
  const Tipo tipo;
  QString error;
  SqlTableModel model;
  SqlTableModel modelFluxoCaixa;
  Ui::InputDialogFinanceiro *ui;
  // methods
  bool cadastrar();
  bool verifyFields();
  void calcularTotal();
  void montarFluxoCaixa();
  void on_comboBoxPgt_currentTextChanged(const int index, const QString &text);
  void on_doubleSpinBoxPgt_valueChanged();
  void resetarPagamentos();
  void setupTables();
  void updateTableData(const QModelIndex &topLeft);
  void wrapMontarFluxoCaixa();
};

#endif // INPUTDIALOGFINANCEIRO_H
