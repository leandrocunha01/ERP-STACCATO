#ifndef WIDGETLOGISTICA_H
#define WIDGETLOGISTICA_H

#include <QWidget>

#include "sqlrelationaltablemodel.h"

namespace Ui {
class WidgetLogistica;
}

class WidgetLogistica : public QWidget {
  Q_OBJECT

public:
  explicit WidgetLogistica(QWidget *parent = 0);
  ~WidgetLogistica();
  bool updateTables();

signals:
  void errorSignal(const QString &error);
  void transactionEnded();
  void transactionStarted();

private slots:
  void on_tableForn_activated(const QModelIndex &index);
  void on_tabWidgetLogistica_currentChanged(const int);

private:
  // attributes
  SqlRelationalTableModel model;
  Ui::WidgetLogistica *ui;
  // methods
  void setConnections();
};

#endif // WIDGETLOGISTICA_H
