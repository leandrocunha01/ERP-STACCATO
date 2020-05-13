#pragma once

#include "sqltablemodel.h"

#include <QWidget>

namespace Ui {
class WidgetGare;
}

class WidgetGare : public QWidget
{
  Q_OBJECT

public:
  explicit WidgetGare(QWidget *parent = nullptr);
  ~WidgetGare();
  auto resetTables() -> void;
  auto updateTables() -> void;

private:
  // attributes
  bool isSet = false;
  bool modelIsSet = false;
  SqlTableModel model;
  Ui::WidgetGare *ui;
  // methods
  auto on_pushButtonBaixaCNAB_clicked() -> void;
  auto on_pushButtonGerarCNAB_clicked() -> void;
  auto setupTables() -> void;
};
