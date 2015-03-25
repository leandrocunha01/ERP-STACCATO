#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlRelationalDelegate>
#include <QTime>
#include <QXmlStreamWriter>

#include "mainwindow.h"
#include "nfe.h"
#include "orcamento.h"
#include "ui_venda.h"
#include "venda.h"
#include "usersession.h"
#include "cadastrocliente.h"
#include "cadastrarnfe.h"

Venda::Venda(QWidget *parent) : RegisterDialog("Venda", "idVenda", parent), ui(new Ui::Venda) {
  ui->setupUi(this);
  modelItem.setTable("Venda_has_Produto");
  modelItem.setHeaderData(modelItem.fieldIndex("fornecedor"), Qt::Horizontal, "Fornecedor");
  modelItem.setHeaderData(modelItem.fieldIndex("produto"), Qt::Horizontal, "Produto");
  modelItem.setHeaderData(modelItem.fieldIndex("obs"), Qt::Horizontal, "Obs.");
  modelItem.setHeaderData(modelItem.fieldIndex("prcUnitario"), Qt::Horizontal, "Preço/Un");
  modelItem.setHeaderData(modelItem.fieldIndex("caixas"), Qt::Horizontal, "Caixas");
  modelItem.setHeaderData(modelItem.fieldIndex("qte"), Qt::Horizontal, "Qte.");
  modelItem.setHeaderData(modelItem.fieldIndex("un"), Qt::Horizontal, "Un.");
  modelItem.setHeaderData(modelItem.fieldIndex("unCaixa"), Qt::Horizontal, "Un/Caixa");
  modelItem.setHeaderData(modelItem.fieldIndex("parcial"), Qt::Horizontal, "Parcial");
  modelItem.setHeaderData(modelItem.fieldIndex("desconto"), Qt::Horizontal, "Desconto");
  modelItem.setHeaderData(modelItem.fieldIndex("parcialDesc"), Qt::Horizontal, "Desc. Parc.");
  modelItem.setHeaderData(modelItem.fieldIndex("descGlobal"), Qt::Horizontal, "Desc. Glob.");
  modelItem.setHeaderData(modelItem.fieldIndex("total"), Qt::Horizontal, "Total");
  modelItem.setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelItem.select();

  modelFluxoCaixa.setTable("Venda_has_Pagamento");
  modelFluxoCaixa.setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelFluxoCaixa.setHeaderData(modelFluxoCaixa.fieldIndex("tipo"), Qt::Horizontal, "Tipo");
  modelFluxoCaixa.setHeaderData(modelFluxoCaixa.fieldIndex("parcela"), Qt::Horizontal, "Parcela");
  modelFluxoCaixa.setHeaderData(modelFluxoCaixa.fieldIndex("valor"), Qt::Horizontal, "R$");
  modelFluxoCaixa.setHeaderData(modelFluxoCaixa.fieldIndex("data"), Qt::Horizontal, "Data");
  modelFluxoCaixa.select();

  ui->tableFluxoCaixa->setModel(&modelFluxoCaixa);
  ui->tableFluxoCaixa->setColumnHidden(modelFluxoCaixa.fieldIndex("idVenda"), true);
  ui->tableFluxoCaixa->setColumnHidden(modelFluxoCaixa.fieldIndex("idLoja"), true);
  ui->tableFluxoCaixa->setColumnHidden(modelFluxoCaixa.fieldIndex("idPagamento"), true);

  modelVenda.setTable("Venda");
  modelVenda.setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelVenda.select();

  ui->tableVenda->setModel(&modelItem);
  ui->tableVenda->setColumnHidden(modelItem.fieldIndex("idVenda"), true);
  ui->tableVenda->setColumnHidden(modelItem.fieldIndex("idLoja"), true);
  ui->tableVenda->setColumnHidden(modelItem.fieldIndex("idProduto"), true);
  ui->tableVenda->setColumnHidden(modelItem.fieldIndex("item"), true);
  ui->tableVenda->setColumnHidden(modelItem.fieldIndex("status"), true);

  QStringList list {"Escolha uma opção!", "Cartão de débito", "Cartão de crédito", "Cheque", "Dinheiro",
                    "Boleto"
                   };

  ui->comboBoxPgt1->insertItems(0, list);
  ui->comboBoxPgt2->insertItems(0, list);
  ui->comboBoxPgt3->insertItems(0, list);

  ui->tableVenda->resizeColumnsToContents();
  ui->tableVenda->setItemDelegate(new QSqlRelationalDelegate(ui->tableVenda));

  ui->pushButtonNFe->hide();

  ui->dateEditPgt1->setDate(QDate::currentDate());
  ui->dateEditPgt2->setDate(QDate::currentDate());
  ui->dateEditPgt3->setDate(QDate::currentDate());

  SearchDialog *sdEndereco = SearchDialog::endereco(ui->itemBoxEndereco);

  ui->itemBoxEndereco->setSearchDialog(sdEndereco);
  SearchDialog::endereco(ui->itemBoxEndereco);

  ui->itemBoxEnderecoFat->setSearchDialog(sdEndereco);
  //  show();
  showMaximized();
}

Venda::~Venda() {
  delete ui;
}

void Venda::resetarPagamentos() {
  ui->doubleSpinBoxTotalPag->setValue(ui->doubleSpinBoxFinal->value());
  ui->doubleSpinBoxPgt1->setMaximum(ui->doubleSpinBoxFinal->value());
  ui->doubleSpinBoxPgt1->setValue(ui->doubleSpinBoxFinal->value());
  ui->doubleSpinBoxPgt2->setValue(0);
  ui->doubleSpinBoxPgt3->setValue(0);
  ui->doubleSpinBoxPgt2->setMaximum(0);
  ui->doubleSpinBoxPgt3->setMaximum(0);
  ui->comboBoxPgt1->setCurrentIndex(0);
  ui->comboBoxPgt2->setCurrentIndex(0);
  ui->comboBoxPgt3->setCurrentIndex(0);

  ui->comboBoxPgt1Parc->setCurrentIndex(0);
  ui->comboBoxPgt2Parc->setCurrentIndex(0);
  ui->comboBoxPgt3Parc->setCurrentIndex(0);
  montarFluxoCaixa();
}

void Venda::fecharOrcamento(const QString &idOrcamento) {
  clearFields();
  this->idOrcamento = idOrcamento;
  QSqlQuery produtos("SELECT * FROM Orcamento_has_Produto WHERE idOrcamento = '" + idOrcamento + "'");
  if (!produtos.exec()) {
    qDebug() << "Erro buscando produtos: " << produtos.lastError();
  }

  modelItem.setFilter("idVenda = '" + idOrcamento + "'");

  while (produtos.next()) {
    //    qDebug() << "Adding to venda";
    int row = modelItem.rowCount();
    modelItem.insertRow(row);
    for (int field = 0; field < produtos.record().count(); field++) {
      modelItem.setData(modelItem.index(row, field), produtos.value(field));
    }
    modelItem.setData(modelItem.index(row, modelItem.fieldIndex("status")), "PENDENTE");
  }
  ui->tableVenda->resizeColumnsToContents();

  QSqlQuery qry("SELECT * FROM Orcamento WHERE idOrcamento = '" + idOrcamento + "'");
  if (!qry.exec()) {
    qDebug() << "Erro buscando orcamento: " << qry.lastError();
  }

  if (!qry.first()) {
    qDebug() << "Erro selecionando primeiro resultado: " << qry.lastError();
  }

  ui->itemBoxEndereco->searchDialog()->setFilter("idCliente = " + qry.value("idCliente").toString() + " AND ativo = 1");
  ui->itemBoxEnderecoFat->searchDialog()->setFilter("idCliente = " + qry.value("idCliente").toString() + " AND ativo = 1");
  //  qDebug() << "idCliente: " << qry.value("idCliente").toString();

  ui->itemBoxEndereco->setValue(qry.value("idEnderecoEntrega"));

  int row = modelVenda.rowCount();
  modelVenda.insertRow(row);
  for (int field = 0; field < modelVenda.columnCount(); field++) {
    modelVenda.setData(modelVenda.index(row, field), qry.value(field));
  }
  fillTotals();
  resetarPagamentos();

  modelFluxoCaixa.setFilter("idVenda = '" + idOrcamento + "'");

  qDebug() << "idOrcamento: " << idOrcamento;
}

bool Venda::cadastrar() {
  return true;
}

bool Venda::verifyFields(int row) {
  Q_UNUSED(row);
  return true;
}

bool Venda::verifyRequiredField(QLineEdit *line) {
  line->objectName();
  return true;
}

QString Venda::requiredStyle() {
  return QString();
}

void Venda::calcPrecoGlobalTotal(bool ajusteTotal) {
  subTotal = 0.0;
  double subTotalItens = 0.0;
  double subTotalBruto = 0.0;
  double minimoFrete = 0, porcFrete = 0;

  QSqlQuery qryFrete;
  if (!qryFrete.exec("SELECT * FROM Loja WHERE idLoja = '" + QString::number(UserSession::getLoja()) + "'")) {
    qDebug() << "Erro buscando parâmetros do frete: " << qryFrete.lastError();
  }
  if (qryFrete.next()) {
    minimoFrete = qryFrete.value("valorMinimoFrete").toDouble();
    porcFrete = qryFrete.value("porcentagemFrete").toDouble();
  }

  for (int row = 0; row < modelItem.rowCount(); ++row) {
    double prcUnItem = modelItem.data(modelItem.index(row, modelItem.fieldIndex("prcUnitario"))).toDouble();
    double qteItem = modelItem.data(modelItem.index(row, modelItem.fieldIndex("qte"))).toDouble();
    double descItem =
      modelItem.data(modelItem.index(row, modelItem.fieldIndex("desconto"))).toDouble() / 100.0;
    double itemBruto = qteItem * prcUnItem;
    subTotalBruto += itemBruto;
    double stItem = itemBruto * (1.0 - descItem);
    subTotalItens += stItem;
    modelItem.setData(modelItem.index(row, modelItem.fieldIndex("parcial")), itemBruto);  // Pr. Parcial
    modelItem.setData(modelItem.index(row, modelItem.fieldIndex("parcialDesc")), stItem); // Pr. Parcial Desc.
  }
  double frete = qMax(subTotalBruto * porcFrete / 100.0, minimoFrete);
  if(ui->checkBoxFreteManual->isChecked()) {
    frete = ui->doubleSpinBoxFrete->value();
    qDebug() << "novo frete: " << frete;
  }
  double descGlobal = ui->doubleSpinBoxDescontoGlobal->value() / 100.0;
  subTotal = subTotalItens * (1.0 - descGlobal);
  if (ajusteTotal) {
    const double Final = ui->doubleSpinBoxFinal->value();
    subTotal = Final - frete;
    if(subTotalItens == 0.0) {
      descGlobal = 0;
    } else {
      descGlobal = 1 - (subTotal / subTotalItens);
    }
  }

  for (int row = 0; row < modelItem.rowCount(); ++row) {
    double stItem = modelItem.data(modelItem.index(row, modelItem.fieldIndex("parcialDesc"))).toDouble();
    modelItem.setData(modelItem.index(row, modelItem.fieldIndex("descGlobal")),
                      descGlobal * 100.0); // Desconto
    // Distr.
    double totalItem = stItem * (1 - descGlobal);
    modelItem.setData(modelItem.index(row, modelItem.fieldIndex("total")), totalItem); // Pr. Final
  }
  ui->doubleSpinBoxSubTotalBruto->setValue(subTotalBruto);
  ui->doubleSpinBoxDescontoGlobal->setValue(descGlobal * 100);
  ui->doubleSpinBoxDescontoRS->setValue(subTotalItens - subTotal);
  ui->doubleSpinBoxFrete->setValue(frete);
  ui->doubleSpinBoxTotal->setValue(subTotalItens);
  ui->doubleSpinBoxFinal->setValue(subTotal + frete);
  resetarPagamentos();
  montarFluxoCaixa();
}

void Venda::fillTotals() {
  QSqlQuery query;
  if (!query.exec("SELECT * FROM Orcamento WHERE idOrcamento = '" + idOrcamento + "'")) {
    qDebug() << "Erro buscando orçamento: " << query.lastError();
    qDebug() << "query: " << query.lastQuery();
  }

  if (!query.first()) {
    qDebug() << "Não achou orçamento: " << query.size();
    qDebug() << "query: " << query.lastQuery();

    if(!query.exec("SELECT * FROM Venda WHERE idVenda = '"+idOrcamento+"'")) {
      qDebug() << "Erro buscando venda: " << query.lastError();
    }
    if(!query.first()) {
      qDebug() << "Não achou venda: " << query.size();
      qDebug() << "query: " << query.lastQuery();
    }
  }

  ui->doubleSpinBoxSubTotalBruto->setValue(query.value("subTotalBru").toDouble());
  ui->doubleSpinBoxTotal->setValue(query.value("subTotalLiq").toDouble());
  ui->doubleSpinBoxFrete->setValue(query.value("frete").toDouble());
  ui->doubleSpinBoxDescontoGlobal->setValue(query.value("descontoPorc").toDouble());
  ui->doubleSpinBoxDescontoRS->setValue(query.value("descontoReais").toDouble());
  ui->doubleSpinBoxFinal->setValue(query.value("total").toDouble());
}

void Venda::clearFields() {
  idOrcamento = QString();
}

void Venda::setupMapper() {
  addMapping(ui->itemBoxEndereco, "idEnderecoEntrega", "value");
  addMapping(ui->doubleSpinBoxSubTotalBruto, "subTotalBru");
  addMapping(ui->doubleSpinBoxTotal, "subTotalLiq");
  addMapping(ui->doubleSpinBoxFrete, "frete");
  addMapping(ui->doubleSpinBoxDescontoGlobal, "descontoPorc");
  addMapping(ui->doubleSpinBoxDescontoRS, "descontoReais");
  addMapping(ui->doubleSpinBoxFinal, "total");
}

void Venda::updateId() {}

void Venda::on_pushButtonCancelar_clicked() {
  close();
}

void Venda::on_pushButtonFecharPedido_clicked() {
  if(!ui->frame_2->isHidden()) {
    if (ui->doubleSpinBoxPgt1->value() + ui->doubleSpinBoxPgt2->value() + ui->doubleSpinBoxPgt3->value() <
        ui->doubleSpinBoxTotalPag->value()) {
      QMessageBox::warning(this, "Aviso!", "Soma dos pagamentos não é igual ao total! Favor verificar.");
      return;
    }

    if (ui->doubleSpinBoxPgt1->value() > 0 and ui->comboBoxPgt1->currentText() == "Escolha uma opção!") {
      QMessageBox::warning(this, "Aviso!", "Por favor escolha a forma de pagamento 1.");
      return;
    }
    if (ui->doubleSpinBoxPgt2->value() > 0 and ui->comboBoxPgt2->currentText() == "Escolha uma opção!") {
      QMessageBox::warning(this, "Aviso!", "Por favor escolha a forma de pagamento 2.");
      return;
    }
    if (ui->doubleSpinBoxPgt3->value() > 0 and ui->comboBoxPgt3->currentText() == "Escolha uma opção!") {
      QMessageBox::warning(this, "Aviso!", "Por favor escolha a forma de pagamento 3.");
      return;
    }
  }

  QSqlQuery qry;

  //  qDebug() << "id: " << idOrcamento;

  qry.exec("START TRANSACTION");
  qry.exec("SET AUTOCOMMIT = 0");

  if (!qry.exec("INSERT INTO Venda SELECT idOrcamento, idLoja, idUsuario, idCliente, idEnderecoEntrega, "
                "idProfissional, data, subTotalBru, subTotalLiq, frete, descontoPorc, descontoReais, total, validade, status FROM Orcamento WHERE "
                "idOrcamento = '" + idOrcamento + "'")) {
    qDebug() << "Erro inserindo em Venda: " << qry.lastError();
    qDebug() << "qry: " << qry.lastQuery();
    qry.exec("ROLLBACK");
    return;
  }

  if (!qry.exec("UPDATE Venda SET status = 'ABERTO' WHERE idVenda = '" + idOrcamento + "'")) {
    qDebug() << "Erro atualizando status de Venda: " << qry.lastError();
    qry.exec("ROLLBACK");
    return;
  }

  if (!qry.exec("UPDATE Venda SET data = '" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") +
                "'")) {
    qDebug() << "Erro setando data em Venda: " << qry.lastError();
    qry.exec("ROLLBACK");
    return;
  }

  if (!modelFluxoCaixa.submitAll()) {
    qDebug() << "Erro submetendo fluxoCaixa: " << modelFluxoCaixa.lastError();
  }

  if (!modelItem.submitAll()) {
    qDebug() << "Erro submetendo modelItem: " << modelItem.lastError();
    //    qDebug() << "query: " << modelItem.query().lastQuery();
    qry.exec("ROLLBACK");
    return;
  }

  QSqlQuery qryEstoque("SELECT Produto.descricao, Produto.estoque, Venda_has_Produto.idVenda FROM "
                       "Venda_has_Produto INNER JOIN Produto ON Produto.idProduto = "
                       "Venda_has_Produto.idProduto WHERE estoque = 0 AND Venda_has_Produto.idVenda = '" +
                       idOrcamento + "';");
  if (!qryEstoque.exec()) {
    qDebug() << qryEstoque.lastError();
    qry.exec("ROLLBACK");
    return;
  }
  if (qryEstoque.size() > 0) {
    if (!qry.exec(
          "INSERT INTO PedidoFornecedor (idPedido, idLoja, idUsuario, idCliente, "
          "idEnderecoEntrega, idProfissional, data, subTotalBru, subTotalLiq, frete, descontoPorc, descontoReais, total, validade, status) SELECT * "
          "FROM Venda WHERE idVenda = '" +
          idOrcamento + "'")) {
      qDebug() << "Erro na criação do pedido fornecedor: " << qry.lastError();
      qry.exec("ROLLBACK");
      return;
    }
    if (!qry.exec("INSERT INTO PedidoFornecedor_has_Produto SELECT Venda_has_Produto.* FROM "
                  "Venda_has_Produto INNER JOIN Produto ON Produto.idProduto = "
                  "Venda_has_Produto.idProduto WHERE estoque = 0 AND Venda_has_Produto.idVenda = '" +
                  idOrcamento + "';")) {
      qDebug() << "Erro na inserção dos produtos em pedidofornecedor_has_produto: " << qry.lastError();
      qry.exec("ROLLBACK");
      return;
    }
  }

  if (!qry.exec("DELETE FROM Orcamento_has_Produto WHERE idOrcamento = '" + idOrcamento + "'")) {
    qDebug() << "Error deleting items from Orcamento_has_Produto: " << qry.lastError();
    qry.exec("ROLLBACK");
    return;
  }

  if (!qry.exec("DELETE FROM Orcamento WHERE idOrcamento = '" + idOrcamento + "'")) {
    qDebug() << "Error deleting row from Orcamento: " << qry.lastError();
    qry.exec("ROLLBACK");
    return;
  }

  if (!qry.exec("INSERT INTO ContaAReceber (dataEmissao, idVenda) VALUES ('" +
                QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "', '" + idOrcamento + "')")) {
    qDebug() << "Error inserting contaareceber: " << qry.lastError();
    qry.exec("ROLLBACK");
    return;
  }

  if (qry.exec("COMMIT")) {
    QMessageBox::information(this, "Atenção!", "Venda cadastrada com sucesso!", QMessageBox::Ok,
                             QMessageBox::NoButton);
  }

  if (MainWindow *window = qobject_cast<MainWindow *>(parentWidget())) {
    window->updateTables();
  } else {
    qDebug() << "something went wrong";
  }

  close();
  //  cancel();
}

void Venda::on_pushButtonNFe_clicked() {
  CadastrarNFE * cadNfe = new CadastrarNFE(this);
  QList<int> lista;
  for(int item = 0; item < modelItem.rowCount(); ++item) {
    lista.append(modelItem.data(modelItem.index(item,modelItem.fieldIndex(primaryKey))).toInt());
  }
  cadNfe->gerarNFe(idOrcamento,lista);
  cadNfe->showMaximized();
//  NFe nota(idOrcamento, this);
//  //  qDebug() << "xml: " << nota.XML();
//  if (nota.TXT()) {
//    QMessageBox::information(this, "Aviso!", "NFe gerada com sucesso!");
//  } else {
//    QMessageBox::warning(this, "Aviso!", "Ocorreu algum erro, NFe não foi gerada.");
//  }
  //  qDebug() << "txt: " << nota.TXT();
}

void Venda::viewVenda(QString idVenda) {
  clearFields();
  idOrcamento = idVenda;

  setWindowTitle("Venda: " + idVenda);
  ui->pushButtonCancelar->hide();
  ui->pushButtonFecharPedido->hide();

  modelItem.setFilter("idVenda = '" + idVenda + "'");
  ui->tableVenda->resizeColumnsToContents();
}

void Venda::on_doubleSpinBoxPgt1_valueChanged(double) {
//  montarFluxoCaixa();
}

void Venda::on_doubleSpinBoxPgt2_valueChanged(double) {
//  montarFluxoCaixa();
}

void Venda::on_doubleSpinBoxPgt3_valueChanged(double) {
//  montarFluxoCaixa();
}

// void Venda::on_doubleSpinBoxRestante_valueChanged(double value) {
//  Q_UNUSED(value);
//}

void Venda::calculoSpinBox1() {
  double pgt1 = ui->doubleSpinBoxPgt1->value();
  double pgt2 = ui->doubleSpinBoxPgt2->value();
  double pgt3 = ui->doubleSpinBoxPgt3->value();
  double total = ui->doubleSpinBoxTotalPag->value();
  double restante = total - (pgt1 + pgt2 + pgt3);
  ui->doubleSpinBoxPgt2->setValue(pgt2 + restante);
  if (pgt2 == 0.0 || pgt3 >= 0.0) {
    ui->doubleSpinBoxPgt2->setMaximum(restante + pgt2);
    ui->doubleSpinBoxPgt2->setValue(restante + pgt2);
    ui->doubleSpinBoxPgt3->setMaximum(pgt3);
    restante = 0;
  } else if (pgt3 == 0.0) {
    ui->doubleSpinBoxPgt3->setMaximum(restante + pgt3);
    ui->doubleSpinBoxPgt3->setValue(restante + pgt3);
    ui->doubleSpinBoxPgt2->setMaximum(pgt2);
    restante = 0;
  }
  //  ui->doubleSpinBoxRestante->setValue(restante);
}

void Venda::on_doubleSpinBoxPgt1_editingFinished() {
  calculoSpinBox1();
  montarFluxoCaixa();
}

void Venda::calculoSpinBox2() {
  double pgt1 = ui->doubleSpinBoxPgt1->value();
  double pgt2 = ui->doubleSpinBoxPgt2->value();
  double pgt3 = ui->doubleSpinBoxPgt3->value();
  double total = ui->doubleSpinBoxTotalPag->value();
  double restante = total - (pgt1 + pgt2 + pgt3);
  ui->doubleSpinBoxPgt3->setMaximum(restante + pgt3);
  ui->doubleSpinBoxPgt3->setValue(restante + pgt3);
  //  ui->doubleSpinBoxRestante->setValue(restante);
}

void Venda::on_doubleSpinBoxPgt2_editingFinished() {
  calculoSpinBox2();
  montarFluxoCaixa();
}

void Venda::calculoSpinBox3() {
//  double pgt1 = ui->doubleSpinBoxPgt1->value();
//  double pgt2 = ui->doubleSpinBoxPgt2->value();
//  double pgt3 = ui->doubleSpinBoxPgt3->value();
//  double total = ui->doubleSpinBoxTotalPag->value();
//  double restante = total - (pgt1 + pgt2 + pgt3);
//  //  ui->doubleSpinBoxRestante->setValue(restante);
//  ui->doubleSpinBoxPgt1->setMaximum(pgt1 + restante);
//  ui->doubleSpinBoxPgt2->setMaximum(pgt2 + restante);
}

void Venda::on_doubleSpinBoxPgt3_editingFinished() {
  calculoSpinBox3();
  montarFluxoCaixa();
}

void Venda::on_comboBoxPgt1_currentTextChanged(const QString &text) {
  if (text == "Escolha uma opção!") {
    return;
  }

  if (text == "Cartão de crédito" or text == "Cheque" or text == "Boleto") {
    ui->comboBoxPgt1Parc->setEnabled(true);
  } else {
    ui->comboBoxPgt1Parc->setDisabled(true);
  }

  if (text == "Cheque" or text == "Boleto") {
    ui->dateEditPgt1->setEnabled(true);
  } else {
    ui->dateEditPgt1->setDisabled(true);
  }

  montarFluxoCaixa();
}

void Venda::on_comboBoxPgt2_currentTextChanged(const QString &text) {
  if (text == "Escolha uma opção!") {
    return;
  }

  if (text == "Cartão de crédito" or text == "Cheque" or text == "Boleto") {
    ui->comboBoxPgt2Parc->setEnabled(true);
  } else {
    ui->comboBoxPgt2Parc->setDisabled(true);
  }

  if (text == "Cheque" or text == "Boleto") {
    ui->dateEditPgt2->setEnabled(true);
  } else {
    ui->dateEditPgt2->setDisabled(true);
  }

  montarFluxoCaixa();
}

void Venda::on_comboBoxPgt3_currentTextChanged(const QString &text) {
  if (text == "Escolha uma opção!") {
    return;
  }

  if (text == "Cartão de crédito" or text == "Cheque" or text == "Boleto") {
    ui->comboBoxPgt3Parc->setEnabled(true);
  } else {
    ui->comboBoxPgt3Parc->setDisabled(true);
  }

  if (text == "Cheque" or text == "Boleto") {
    ui->dateEditPgt3->setEnabled(true);
  } else {
    ui->dateEditPgt3->setDisabled(true);
  }

  montarFluxoCaixa();
}

bool Venda::savingProcedures(int row) {
  Q_UNUSED(row);
  montarFluxoCaixa();
  return true;
}

void Venda::registerMode() {
  ui->frame_2->show();
  ui->pushButtonNFe->hide();
  ui->pushButtonFecharPedido->show();
  ui->pushButtonVoltar->show();
  ui->doubleSpinBoxDescontoGlobal->setReadOnly(false);
  ui->doubleSpinBoxDescontoGlobal->setFrame(true);
  ui->doubleSpinBoxDescontoGlobal->setButtonSymbols(QDoubleSpinBox::UpDownArrows);
  ui->doubleSpinBoxFinal->setReadOnly(false);
  ui->doubleSpinBoxFinal->setFrame(true);
  ui->doubleSpinBoxFinal->setButtonSymbols(QDoubleSpinBox::UpDownArrows);
  ui->checkBoxFreteManual->show();
}

void Venda::updateMode() {
  ui->frame_2->hide();
  ui->pushButtonNFe->show();
  ui->pushButtonFecharPedido->hide();
  ui->pushButtonVoltar->hide();
  ui->doubleSpinBoxDescontoGlobal->setReadOnly(true);
  ui->doubleSpinBoxDescontoGlobal->setFrame(false);
  ui->doubleSpinBoxDescontoGlobal->setButtonSymbols(QDoubleSpinBox::NoButtons);
  ui->doubleSpinBoxFinal->setReadOnly(true);
  ui->doubleSpinBoxFinal->setFrame(false);
  ui->doubleSpinBoxFinal->setButtonSymbols(QDoubleSpinBox::NoButtons);
  ui->checkBoxFreteManual->hide();
}

bool Venda::viewRegister(QModelIndex index) {
  if (!RegisterDialog::viewRegister(index)) {
    return false;
  }

  QString idVenda = data(primaryKey).toString();
  idOrcamento = idVenda;
  //  qDebug() << "idVenda: " << idVenda;
  modelItem.setFilter("idVenda = '" + idVenda + "'");
  modelItem.select();
  modelFluxoCaixa.setFilter("idVenda = '" + idVenda + "'");
  modelFluxoCaixa.select();
  //  novoItem();

  ui->tableFluxoCaixa->resizeColumnsToContents();
  fillTotals();
  on_pushButtonNFe_clicked();
  return true;
}

void Venda::on_pushButtonVoltar_clicked() {
  Orcamento *orc = new Orcamento(parentWidget());
  orc->viewRegisterById(idOrcamento);
  //  qDebug() << "idOrcamento: " << idOrcamento;
  close();
}

void Venda::montarFluxoCaixa() {
  if(ui->frame_2->isHidden()) {
    return;
  }
  modelFluxoCaixa.removeRows(0, modelFluxoCaixa.rowCount());

  int parcelas1 = ui->comboBoxPgt1Parc->currentIndex() + 1;
  int row = 0;
  if (ui->comboBoxPgt1->currentText() != "Escolha uma opção!") {
    for (int i = 0, z = parcelas1 - 1; i < parcelas1; ++i, --z) {
      modelFluxoCaixa.insertRow(modelFluxoCaixa.rowCount());
      modelFluxoCaixa.setData(modelFluxoCaixa.index(row, modelFluxoCaixa.fieldIndex("idVenda")), idOrcamento);
      modelFluxoCaixa.setData(modelFluxoCaixa.index(row, modelFluxoCaixa.fieldIndex("idLoja")),
                              UserSession::getLoja());
      //      modelFluxoCaixa.setData(modelFluxoCaixa.index(row, modelFluxoCaixa.fieldIndex("idPagamento")),
      //      );
      modelFluxoCaixa.setData(modelFluxoCaixa.index(row, modelFluxoCaixa.fieldIndex("tipo")),
                              "1. " + ui->comboBoxPgt1->currentText());
      modelFluxoCaixa.setData(modelFluxoCaixa.index(row, modelFluxoCaixa.fieldIndex("parcela")),
                              parcelas1 - z);
      modelFluxoCaixa.setData(modelFluxoCaixa.index(row, modelFluxoCaixa.fieldIndex("valor")),
                              ui->doubleSpinBoxPgt1->value() / parcelas1);
      modelFluxoCaixa.setData(modelFluxoCaixa.index(row, modelFluxoCaixa.fieldIndex("data")),
                              ui->dateEditPgt1->date().addMonths(i));
      //                              QDate::currentDate().addMonths(i));
      ++row;
    }
  } else {
  }

  if (ui->comboBoxPgt2->currentText() != "Escolha uma opção!") {
    int parcelas2 = ui->comboBoxPgt2Parc->currentIndex() + 1;
    for (int i = 0, z = parcelas2 - 1; i < parcelas2; ++i, --z) {
      modelFluxoCaixa.insertRow(modelFluxoCaixa.rowCount());
      modelFluxoCaixa.setData(modelFluxoCaixa.index(row, modelFluxoCaixa.fieldIndex("idVenda")), idOrcamento);
      modelFluxoCaixa.setData(modelFluxoCaixa.index(row, modelFluxoCaixa.fieldIndex("idLoja")),
                              UserSession::getLoja());
      modelFluxoCaixa.setData(modelFluxoCaixa.index(row, modelFluxoCaixa.fieldIndex("tipo")),
                              "2. " + ui->comboBoxPgt2->currentText());
      modelFluxoCaixa.setData(modelFluxoCaixa.index(row, modelFluxoCaixa.fieldIndex("parcela")),
                              parcelas2 - z);
      modelFluxoCaixa.setData(modelFluxoCaixa.index(row, modelFluxoCaixa.fieldIndex("valor")),
                              ui->doubleSpinBoxPgt2->value() / parcelas2);
      modelFluxoCaixa.setData(modelFluxoCaixa.index(row, modelFluxoCaixa.fieldIndex("data")),
                              ui->dateEditPgt2->date().addMonths(i));
      //                              QDate::currentDate().addMonths(i));
      ++row;
    }
  }

  if (ui->comboBoxPgt3->currentText() != "Escolha uma opção!") {
    int parcelas3 = ui->comboBoxPgt3Parc->currentIndex() + 1;
    for (int i = 0, z = parcelas3 - 1; i < parcelas3; ++i, --z) {
      modelFluxoCaixa.insertRow(modelFluxoCaixa.rowCount());
      modelFluxoCaixa.setData(modelFluxoCaixa.index(row, modelFluxoCaixa.fieldIndex("idVenda")), idOrcamento);
      modelFluxoCaixa.setData(modelFluxoCaixa.index(row, modelFluxoCaixa.fieldIndex("idLoja")),
                              UserSession::getLoja());
      modelFluxoCaixa.setData(modelFluxoCaixa.index(row, modelFluxoCaixa.fieldIndex("tipo")),
                              "3. " + ui->comboBoxPgt3->currentText());
      modelFluxoCaixa.setData(modelFluxoCaixa.index(row, modelFluxoCaixa.fieldIndex("parcela")),
                              parcelas3 - z);
      modelFluxoCaixa.setData(modelFluxoCaixa.index(row, modelFluxoCaixa.fieldIndex("valor")),
                              ui->doubleSpinBoxPgt3->value() / parcelas3);
      modelFluxoCaixa.setData(modelFluxoCaixa.index(row, modelFluxoCaixa.fieldIndex("data")),
                              ui->dateEditPgt3->date().addMonths(i));
      //                              QDate::currentDate().addMonths(i));
      ++row;
    }
  }

  ui->tableFluxoCaixa->resizeColumnsToContents();
}

void Venda::on_comboBoxPgt1Parc_currentTextChanged(const QString &text) {
  Q_UNUSED(text);

  montarFluxoCaixa();
}

void Venda::on_comboBoxPgt2Parc_currentTextChanged(const QString &text) {
  Q_UNUSED(text);

  montarFluxoCaixa();
}

void Venda::on_comboBoxPgt3Parc_currentTextChanged(const QString &text) {
  Q_UNUSED(text);

  montarFluxoCaixa();
}

// TODO: colocar campos de data nos pagamentos para indicar a data do pagamento? (cheque, cartão etc)

void Venda::on_dateEditPgt1_dateChanged(const QDate &) {
  montarFluxoCaixa();
}

void Venda::on_dateEditPgt2_dateChanged(const QDate &) {
  montarFluxoCaixa();
}

void Venda::on_dateEditPgt3_dateChanged(const QDate &) {
  montarFluxoCaixa();
}

void Venda::on_pushButton_clicked() {
  resetarPagamentos();
}

void Venda::on_doubleSpinBoxFinal_editingFinished() {
  if (modelItem.rowCount() == 0 || ui->doubleSpinBoxTotal->value() == 0) {
    calcPrecoGlobalTotal();
    return;
  }

  double new_total = ui->doubleSpinBoxFinal->value();
  double frete = ui->doubleSpinBoxFrete->value();
  double new_subtotal = new_total - frete;

  if (new_subtotal >= ui->doubleSpinBoxTotal->value()) {
    ui->doubleSpinBoxDescontoGlobal->setValue(0.0);
    calcPrecoGlobalTotal();
  } else {
    calcPrecoGlobalTotal(true);
  }
}

void Venda::on_checkBoxFreteManual_clicked(bool checked) {
  if(checked == true && UserSession::getTipo() != "ADMINISTRADOR") {
    ui->checkBoxFreteManual->setChecked(false);
    return;
  }
  ui->doubleSpinBoxFrete->setFrame(checked);
  ui->doubleSpinBoxFrete->setReadOnly(!checked);
  if(checked) {
    ui->doubleSpinBoxFrete->setButtonSymbols(QDoubleSpinBox::UpDownArrows);
  } else {
    ui->doubleSpinBoxFrete->setButtonSymbols(QDoubleSpinBox::NoButtons);
  }
  calcPrecoGlobalTotal();
}

void Venda::on_doubleSpinBoxFrete_editingFinished() {
  calcPrecoGlobalTotal();
}

void Venda::on_doubleSpinBoxDescontoGlobal_valueChanged(double) {
  calcPrecoGlobalTotal();
}
