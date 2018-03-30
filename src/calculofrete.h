#ifndef CALCULOFRETE_H
#define CALCULOFRETE_H

#include <QNetworkAccessManager>

#include "dialog.h"

namespace Ui {
class CalculoFrete;
}

class CalculoFrete : public Dialog {
  Q_OBJECT

public:
  explicit CalculoFrete(QWidget *parent = nullptr);
  ~CalculoFrete();
  auto setCliente(const QVariant idCliente) -> void;
  auto getDistancia() -> double;

private:
  // attributes
  Ui::CalculoFrete *ui;
  QNetworkAccessManager networkManager;
  const QString searchUrl = "https://maps.googleapis.com/maps/api/distancematrix/xml?origins=%1&destinations=%2&key=AIzaSyCeaSwBk9LfNUQULva4BM8uOswdix6xH8Q";
  // methods
  void handleNetworkData(QNetworkReply *networkReply);
  void on_itemBoxCliente_textChanged(const QString &);
  void on_pushButton_clicked();
};

#endif // CALCULOFRETE_H