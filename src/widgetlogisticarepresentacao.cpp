#include <QDate>
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>

#include "application.h"
#include "estoqueprazoproxymodel.h"
#include "inputdialogconfirmacao.h"
#include "ui_widgetlogisticarepresentacao.h"
#include "widgetlogisticarepresentacao.h"

WidgetLogisticaRepresentacao::WidgetLogisticaRepresentacao(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetLogisticaRepresentacao) { ui->setupUi(this); }

WidgetLogisticaRepresentacao::~WidgetLogisticaRepresentacao() { delete ui; }

void WidgetLogisticaRepresentacao::setConnections() {
  connect(ui->lineEditBusca, &QLineEdit::textChanged, this, &WidgetLogisticaRepresentacao::on_lineEditBusca_textChanged);
  connect(ui->pushButtonMarcarEntregue, &QPushButton::clicked, this, &WidgetLogisticaRepresentacao::on_pushButtonMarcarEntregue_clicked);
}

void WidgetLogisticaRepresentacao::updateTables() {
  if (not isSet) {
    setConnections();
    isSet = true;
  }

  if (not modelIsSet) {
    setupTables();
    modelIsSet = true;
  }

  if (not modelViewLogisticaRepresentacao.select()) { return; }
}

void WidgetLogisticaRepresentacao::tableFornLogistica_clicked(const QString &fornecedor) {
  ui->lineEditBusca->clear();

  const QString filtro = fornecedor.isEmpty() ? "" : "fornecedor = '" + fornecedor + "'";

  modelViewLogisticaRepresentacao.setFilter(filtro);
}

void WidgetLogisticaRepresentacao::resetTables() { modelIsSet = false; }

void WidgetLogisticaRepresentacao::setupTables() {
  modelViewLogisticaRepresentacao.setTable("view_logistica_representacao");

  modelViewLogisticaRepresentacao.setFilter("");

  modelViewLogisticaRepresentacao.setHeaderData("idVenda", "Venda");
  modelViewLogisticaRepresentacao.setHeaderData("cliente", "Cliente");
  modelViewLogisticaRepresentacao.setHeaderData("descricao", "Produto");
  modelViewLogisticaRepresentacao.setHeaderData("codComercial", "Cód. Com.");
  modelViewLogisticaRepresentacao.setHeaderData("quant", "Quant.");
  modelViewLogisticaRepresentacao.setHeaderData("un", "Un.");
  modelViewLogisticaRepresentacao.setHeaderData("caixas", "Cx.");
  modelViewLogisticaRepresentacao.setHeaderData("kgcx", "Kg./Cx.");
  modelViewLogisticaRepresentacao.setHeaderData("ordemCompra", "OC");
  modelViewLogisticaRepresentacao.setHeaderData("prazoEntrega", "Prazo Limite");

  modelViewLogisticaRepresentacao.setSort("prazoEntrega", Qt::AscendingOrder);

  modelViewLogisticaRepresentacao.proxyModel = new EstoquePrazoProxyModel(&modelViewLogisticaRepresentacao, this);

  ui->table->setModel(&modelViewLogisticaRepresentacao);

  ui->table->hideColumn("idPedido");
  ui->table->hideColumn("fornecedor");
  ui->table->hideColumn("idVendaProduto");
}

void WidgetLogisticaRepresentacao::on_pushButtonMarcarEntregue_clicked() {
  const auto list = ui->table->selectionModel()->selectedRows();

  if (list.isEmpty()) { return qApp->enqueueError("Nenhum item selecionado!", this); }

  InputDialogConfirmacao input(InputDialogConfirmacao::Tipo::Representacao, this);

  if (input.exec() != InputDialogConfirmacao::Accepted) { return; }

  if (not qApp->startTransaction()) { return; }

  if (not processRows(list, input.getDateTime(), input.getRecebeu())) { return qApp->rollbackTransaction(); }

  if (not qApp->endTransaction()) { return; }

  updateTables();
  qApp->enqueueInformation("Atualizado!", this);
}

bool WidgetLogisticaRepresentacao::processRows(const QModelIndexList &list, const QDateTime &dataEntrega, const QString &recebeu) {
  QSqlQuery query1;
  query1.prepare("UPDATE pedido_fornecedor_has_produto SET status = 'ENTREGUE', dataRealEnt = :dataRealEnt WHERE status = 'EM ENTREGA' AND idVendaProduto = :idVendaProduto");

  QSqlQuery query2;
  query2.prepare("UPDATE venda_has_produto SET status = 'ENTREGUE', dataRealEnt = :dataRealEnt, recebeu = :recebeu WHERE status = 'EM ENTREGA' AND idVendaProduto = :idVendaProduto");

  for (const auto &item : list) {
    query1.bindValue(":dataRealEnt", dataEntrega);
    query1.bindValue(":idVendaProduto", modelViewLogisticaRepresentacao.data(item.row(), "idVendaProduto"));

    if (not query1.exec()) { return qApp->enqueueError(false, "Erro salvando status no pedido_fornecedor: " + query1.lastError().text(), this); }

    query2.bindValue(":dataRealEnt", dataEntrega);
    query2.bindValue(":idVendaProduto", modelViewLogisticaRepresentacao.data(item.row(), "idVendaProduto"));
    query2.bindValue(":recebeu", recebeu);

    if (not query2.exec()) { return qApp->enqueueError(false, "Erro salvando status na venda_produto: " + query2.lastError().text(), this); }
  }

  return true;
}

void WidgetLogisticaRepresentacao::on_lineEditBusca_textChanged(const QString &text) { modelViewLogisticaRepresentacao.setFilter("(idVenda LIKE '%" + text + "%' OR cliente LIKE '%" + text + "%')"); }

// TODO: 2palimanan precisa de coleta/recebimento (colocar flag no cadastro dizendo que entra no fluxo de logistica)
