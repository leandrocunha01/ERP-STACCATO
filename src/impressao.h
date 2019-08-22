#pragma once

#include <QSqlQuery>

#include "lrreportengine.h"
#include "sqlrelationaltablemodel.h"

class Impressao final {

public:
  enum class Tipo { Orcamento, Venda };
  explicit Impressao(const QString &id, const Tipo tipo, QObject *parent);
  ~Impressao() = default;
  Impressao(const Impressao &) = delete;
  auto print() -> void;

private:
  // attributes
  const Tipo tipo;
  const QString id;
  QObject *parent;
  QSqlQuery queryCliente;
  QSqlQuery queryEndEnt;
  QSqlQuery queryEndFat;
  QSqlQuery queryLoja;
  QSqlQuery queryLojaEnd;
  QSqlQuery query;
  QSqlQuery queryProfissional;
  QSqlQuery queryVendedor;
  SqlRelationalTableModel modelItem;
  // methods
  auto setQuerys() -> bool;
};
