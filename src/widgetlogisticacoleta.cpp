#include <QDate>
#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "estoqueprazoproxymodel.h"
#include "inputdialog.h"
#include "ui_widgetlogisticacoleta.h"
#include "venda.h"
#include "widgetlogisticacoleta.h"

WidgetLogisticaColeta::WidgetLogisticaColeta(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetLogisticaColeta) {
  ui->setupUi(this);

  connect(ui->checkBoxMarcarTodos, &QCheckBox::clicked, this, &WidgetLogisticaColeta::on_checkBoxMarcarTodos_clicked);
  connect(ui->lineEditBusca, &QLineEdit::textChanged, this, &WidgetLogisticaColeta::on_lineEditBusca_textChanged);
  connect(ui->pushButtonCancelar, &QPushButton::clicked, this, &WidgetLogisticaColeta::on_pushButtonCancelar_clicked);
  connect(ui->pushButtonMarcarColetado, &QPushButton::clicked, this, &WidgetLogisticaColeta::on_pushButtonMarcarColetado_clicked);
  connect(ui->pushButtonReagendar, &QPushButton::clicked, this, &WidgetLogisticaColeta::on_pushButtonReagendar_clicked);
  connect(ui->pushButtonVenda, &QPushButton::clicked, this, &WidgetLogisticaColeta::on_pushButtonVenda_clicked);
  connect(ui->table, &TableView::entered, this, &WidgetLogisticaColeta::on_table_entered);
}

WidgetLogisticaColeta::~WidgetLogisticaColeta() { delete ui; }

bool WidgetLogisticaColeta::updateTables() {
  if (model.tableName().isEmpty()) setupTables();

  if (not model.select()) {
    emit errorSignal("Erro lendo tabela pedido_fornecedor_has_produto: " + model.lastError().text());
    return false;
  }

  ui->table->resizeColumnsToContents();

  return true;
}

void WidgetLogisticaColeta::tableFornLogistica_activated(const QString &fornecedor) {
  this->fornecedor = fornecedor;

  ui->lineEditBusca->clear();

  model.setFilter("fornecedor = '" + fornecedor + "'");

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela pedido_fornecedor_has_produto: " + model.lastError().text());
    return;
  }

  ui->checkBoxMarcarTodos->setChecked(false);

  ui->table->resizeColumnsToContents();
}

void WidgetLogisticaColeta::setupTables() {
  model.setTable("view_coleta");
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);

  model.setHeaderData("idEstoque", "Estoque");
  model.setHeaderData("numeroNFe", "NFe");
  model.setHeaderData("produto", "Produto");
  model.setHeaderData("codComercial", "Cód. Com.");
  model.setHeaderData("quant", "Quant.");
  model.setHeaderData("un", "Un.");
  model.setHeaderData("caixas", "Caixas");
  model.setHeaderData("kgcx", "Kg./Cx.");
  model.setHeaderData("idVenda", "Venda");
  model.setHeaderData("ordemCompra", "OC");
  model.setHeaderData("local", "Local");
  model.setHeaderData("dataPrevColeta", "Data Prev. Col.");
  model.setHeaderData("prazoEntrega", "Prazo Limite");

  model.setFilter("0");

  ui->table->setModel(new EstoquePrazoProxyModel(&model, this));
  ui->table->hideColumn("fornecedor");
  ui->table->hideColumn("prazoEntrega");
  ui->table->hideColumn("ordemCompra");
  ui->table->hideColumn("local");
}

void WidgetLogisticaColeta::on_pushButtonMarcarColetado_clicked() {
  const auto list = ui->table->selectionModel()->selectedRows();

  if (list.isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return;
  }

  InputDialog input(InputDialog::Tipo::Coleta);

  if (input.exec() != InputDialog::Accepted) return;

  emit transactionStarted();

  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not cadastrar(list, input.getDate(), input.getNextDate())) {
    QSqlQuery("ROLLBACK").exec();
    emit transactionEnded();
    return;
  }

  QSqlQuery("COMMIT").exec();

  emit transactionEnded();

  QSqlQuery query;

  // REFAC: replace this with a simpler query
  if (not query.exec("CALL update_venda_status()")) {
    QMessageBox::critical(this, "Erro!", "Erro atualizando status das vendas: " + query.lastError().text());
    return;
  }

  updateTables();
  QMessageBox::information(this, "Aviso!", "Confirmado coleta!");
}

bool WidgetLogisticaColeta::cadastrar(const QModelIndexList &list, const QDate &dataColeta, const QDate &dataPrevReceb) {
  // NOTE: put inputDialog and verifications before the transaction (to reduce the time the table stays locked)

  QSqlQuery query;

  for (const auto &item : list) {
    query.prepare("UPDATE estoque SET status = 'EM RECEBIMENTO' WHERE idEstoque = :idEstoque");
    query.bindValue(":idEstoque", model.data(item.row(), "idEstoque"));

    if (not query.exec()) {
      emit errorSignal("Erro salvando status do estoque: " + query.lastError().text());
      return false;
    }

    query.prepare("UPDATE pedido_fornecedor_has_produto SET status = 'EM RECEBIMENTO', dataRealColeta = :dataRealColeta, dataPrevReceb = :dataPrevReceb WHERE idCompra IN (SELECT idCompra FROM "
                  "estoque_has_compra WHERE idEstoque = :idEstoque) AND codComercial = :codComercial");
    query.bindValue(":dataRealColeta", dataColeta);
    query.bindValue(":dataPrevReceb", dataPrevReceb);
    query.bindValue(":idEstoque", model.data(item.row(), "idEstoque"));
    query.bindValue(":codComercial", model.data(item.row(), "codComercial"));

    if (not query.exec()) {
      emit errorSignal("Erro salvando status no pedido_fornecedor: " + query.lastError().text());
      return false;
    }

    // salvar status na venda
    query.prepare("UPDATE venda_has_produto SET status = 'EM RECEBIMENTO', dataRealColeta = :dataRealColeta, dataPrevReceb = :dataPrevReceb WHERE idVendaProduto IN (SELECT idVendaProduto FROM "
                  "estoque_has_consumo WHERE idEstoque = :idEstoque)");
    query.bindValue(":dataRealColeta", dataColeta);
    query.bindValue(":dataPrevReceb", dataPrevReceb);
    query.bindValue(":idEstoque", model.data(item.row(), "idEstoque"));

    if (not query.exec()) {
      emit errorSignal("Erro atualizando status da compra: " + query.lastError().text());
      return false;
    }

    //

    query.prepare("UPDATE veiculo_has_produto SET status = 'COLETADO' WHERE idEstoque = :idEstoque AND status = 'EM COLETA'");
    query.bindValue(":idEstoque", model.data(item.row(), "idEstoque"));

    if (not query.exec()) {
      emit errorSignal("Erro atualizando veiculo_has_produto: " + query.lastError().text());
      return false;
    }
  }

  return true;
}

void WidgetLogisticaColeta::on_checkBoxMarcarTodos_clicked(const bool) { ui->table->selectAll(); }

void WidgetLogisticaColeta::on_table_entered(const QModelIndex &) { ui->table->resizeColumnsToContents(); }

void WidgetLogisticaColeta::on_lineEditBusca_textChanged(const QString &) {
  const QString textoBusca = ui->lineEditBusca->text();

  model.setFilter("(numeroNFe LIKE '%" + textoBusca + "%' OR produto LIKE '%" + textoBusca + "%' OR idVenda LIKE '%" + textoBusca + "%' OR ordemCompra LIKE '%" + textoBusca + "%')");

  if (not model.select()) QMessageBox::critical(this, "Erro!", "Erro lendo tabela: " + model.lastError().text());
}

void WidgetLogisticaColeta::on_pushButtonReagendar_clicked() {
  const auto list = ui->table->selectionModel()->selectedRows();

  if (list.isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return;
  }

  InputDialog input(InputDialog::Tipo::AgendarColeta);

  if (input.exec() != InputDialog::Accepted) return;

  emit transactionStarted();

  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not reagendar(list, input.getNextDate())) {
    QSqlQuery("ROLLBACK").exec();
    emit transactionEnded();
    return;
  }

  QSqlQuery("COMMIT").exec();

  emit transactionEnded();

  updateTables();
  QMessageBox::information(this, "Aviso!", "Reagendado com sucesso!");
}

bool WidgetLogisticaColeta::reagendar(const QModelIndexList &list, const QDate &dataPrevColeta) {
  for (const auto &item : list) {
    const int idEstoque = model.data(item.row(), "idEstoque").toInt();
    const QString codComercial = model.data(item.row(), "codComercial").toString();

    QSqlQuery query;
    query.prepare("UPDATE pedido_fornecedor_has_produto SET dataPrevColeta = :dataPrevColeta WHERE idCompra IN (SELECT idCompra FROM estoque_has_compra WHERE idEstoque = :idEstoque) AND codComercial "
                  "= :codComercial");
    query.bindValue(":dataPrevColeta", dataPrevColeta);
    query.bindValue(":idEstoque", idEstoque);
    query.bindValue(":codComercial", codComercial);

    if (not query.exec()) {
      emit errorSignal("Erro salvando status no pedido_fornecedor: " + query.lastError().text());
      return false;
    }

    query.prepare(
        "UPDATE venda_has_produto SET dataPrevColeta = :dataPrevColeta WHERE idCompra IN (SELECT idCompra FROM estoque_has_compra WHERE idEstoque = :idEstoque) AND codComercial = :codComercial");
    query.bindValue(":dataPrevColeta", dataPrevColeta);
    query.bindValue(":idEstoque", idEstoque);
    query.bindValue(":codComercial", codComercial);

    if (not query.exec()) {
      emit errorSignal("Erro salvando status na venda_produto: " + query.lastError().text());
      return false;
    }

    query.prepare("UPDATE veiculo_has_produto SET data = :data WHERE idEstoque = :idEstoque AND status = 'EM COLETA'");
    query.bindValue(":data", dataPrevColeta);
    query.bindValue(":idEstoque", model.data(item.row(), "idEstoque"));

    if (not query.exec()) {
      emit errorSignal("Erro atualizando data no veiculo: " + query.lastError().text());
      return false;
    }
  }

  return true;
}

void WidgetLogisticaColeta::on_pushButtonVenda_clicked() {
  const auto list = ui->table->selectionModel()->selectedRows();

  if (list.isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return;
  }

  for (const auto &item : list) {
    const QString idVenda = model.data(item.row(), "idVenda").toString();
    const QStringList ids = idVenda.split(", ");

    if (ids.isEmpty()) return;

    for (const auto &id : ids) {
      auto *venda = new Venda(this);
      venda->setAttribute(Qt::WA_DeleteOnClose);
      venda->viewRegisterById(id);

      connect(venda, &Venda::errorSignal, this, &WidgetLogisticaColeta::errorSignal);
      connect(venda, &Venda::transactionStarted, this, &WidgetLogisticaColeta::transactionStarted);
      connect(venda, &Venda::transactionEnded, this, &WidgetLogisticaColeta::transactionEnded);
    }
  }
}

bool WidgetLogisticaColeta::cancelar(const QModelIndexList &list) {
  for (const auto &item : list) {
    const int idEstoque = model.data(item.row(), "idEstoque").toInt();
    const QString codComercial = model.data(item.row(), "codComercial").toString();

    QSqlQuery query;
    query.prepare(
        "UPDATE pedido_fornecedor_has_produto SET dataPrevColeta = NULL WHERE idCompra IN (SELECT idCompra FROM estoque_has_compra WHERE idEstoque = :idEstoque) AND codComercial = :codComercial");
    query.bindValue(":idEstoque", idEstoque);
    query.bindValue(":codComercial", codComercial);

    if (not query.exec()) {
      emit errorSignal("Erro salvando status no pedido_fornecedor: " + query.lastError().text());
      return false;
    }

    query.prepare("UPDATE venda_has_produto SET dataPrevColeta = NULL WHERE idCompra IN (SELECT idCompra FROM estoque_has_compra WHERE idEstoque = :idEstoque) AND codComercial = :codComercial");
    query.bindValue(":idEstoque", idEstoque);
    query.bindValue(":codComercial", codComercial);

    if (not query.exec()) {
      emit errorSignal("Erro salvando status na venda_produto: " + query.lastError().text());
      return false;
    }

    query.prepare("UPDATE veiculo_has_produto SET data = NULL WHERE idEstoque = :idEstoque AND status = 'EM COLETA'");
    query.bindValue(":idEstoque", model.data(item.row(), "idEstoque"));

    if (not query.exec()) {
      emit errorSignal("Erro atualizando data no veiculo: " + query.lastError().text());
      return false;
    }
  }

  return true;
}

void WidgetLogisticaColeta::on_pushButtonCancelar_clicked() {
  const auto list = ui->table->selectionModel()->selectedRows();

  if (list.isEmpty()) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return;
  }

  QMessageBox msgBox(QMessageBox::Question, "Cancelar?", "Tem certeza que deseja cancelar?", QMessageBox::Yes | QMessageBox::No, this);
  msgBox.setButtonText(QMessageBox::Yes, "Cancelar");
  msgBox.setButtonText(QMessageBox::No, "Voltar");

  if (msgBox.exec() == QMessageBox::No) return;

  emit transactionStarted();

  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not cancelar(list)) {
    QSqlQuery("ROLLBACK").exec();
    emit transactionEnded();
    return;
  }

  QSqlQuery("COMMIT").exec();

  emit transactionEnded();

  updateTables();
  QMessageBox::information(this, "Aviso!", "Cancelado com sucesso!");
}
