#pragma once

#include <QDataWidgetMapper>
#include <QDate>
#include <QFileDialog>

#include "sqlrelationaltablemodel.h"
#include "xml.h"

namespace Ui {
class ImportarXML;
}

class ImportarXML final : public QDialog {
  Q_OBJECT

public:
  explicit ImportarXML(const QStringList &idsCompra, const QDateTime &dataReal, QWidget *parent = nullptr);
  ~ImportarXML();

private:
  // attributes
  const QDateTime dataReal;
  const QStringList idsCompra;
  SqlRelationalTableModel modelCompra;
  SqlRelationalTableModel modelConsumo;
  SqlRelationalTableModel modelEstoque;
  SqlRelationalTableModel modelEstoque_compra;
  SqlRelationalTableModel modelNFe;
  Ui::ImportarXML *ui;

  enum class FieldColors {
    None = 0,      // Não processado
    Green = 1,     // Ok
    Yellow = 2,    // Quant difere
    Red = 3,       // Não encontrado
    DarkGreen = 4, // Consumo
  };

  // methods
  auto associarItens(const int rowCompra, const int rowEstoque, double &estoqueConsumido) -> bool;
  auto atualizaDados() -> bool;
  auto buscarCaixas(const int rowEstoque) -> std::optional<double>;
  auto buscarProximoIdEstoque() -> std::optional<int>;
  auto cadastrarNFe(XML &xml) -> bool;
  auto cadastrarProdutoEstoque(const QVector<std::tuple<int, int, double>> &tuples) -> bool;
  auto criarConsumo(const int rowCompra, const int rowEstoque) -> bool;
  auto importar() -> bool;
  auto inserirItemModel(const XML &xml) -> bool;
  auto lerXML() -> bool;
  auto limparAssociacoes() -> bool;
  auto mapTuples() -> QVector<std::tuple<int, int, double>>;
  auto on_checkBoxSemLote_toggled(bool checked) -> void;
  auto on_pushButtonCancelar_clicked() -> void;
  auto on_pushButtonImportar_clicked() -> void;
  auto on_pushButtonProcurar_clicked() -> void;
  auto parear() -> bool;
  auto percorrerXml(XML &xml, const QStandardItem *item) -> bool;
  auto perguntarLocal(XML &xml) -> bool;
  auto produtoCompativel(const int rowCompra, const QString &codComercialEstoque) -> bool;
  auto reparear(const QModelIndex &index) -> bool;
  auto salvarLoteNaVenda() -> bool;
  auto setConnections() -> void;
  auto setupTables() -> void;
  auto unsetConnections() -> void;
  auto updateTableData(const QModelIndex &topLeft) -> void;
  auto verificaCNPJ(const XML &xml) -> bool;
  auto verificaExiste(const XML &xml) -> bool;
  auto verificaValido(const XML &xml) -> bool;
  auto verifyFields() -> bool;
};
