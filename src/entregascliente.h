#ifndef ENTREGASCLIENTE_H
#define ENTREGASCLIENTE_H

#include <QDialog>

#include "sqltablemodel.h"

namespace Ui {
class EntregasCliente;
}

class EntregasCliente : public QDialog {
  Q_OBJECT

public:
  explicit EntregasCliente(QWidget *parent = 0);
  ~EntregasCliente();
  void viewEntrega(const QString &idVenda);

private slots:
  void on_pushButtonNFe_clicked();
  void on_tableEntregas_entered(const QModelIndex &);
  void on_tableProdutos_entered(const QModelIndex &);

private:
  // attributes
  Ui::EntregasCliente *ui;
  SqlTableModel modelProdutos, modelEntregas;
  QString idVenda;
  // methods
  void setupTables();
};

#endif // ENTREGASCLIENTE_H
