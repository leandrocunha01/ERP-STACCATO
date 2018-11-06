#include <QDebug>
#include <QDesktopServices>
#include <QMessageBox>
#include <QSqlError>
#include <QtMath>

#include "application.h"
#include "baixaorcamento.h"
#include "cadastrocliente.h"
#include "calculofrete.h"
#include "doubledelegate.h"
#include "excel.h"
#include "impressao.h"
#include "logindialog.h"
#include "orcamento.h"
#include "porcentagemdelegate.h"
#include "reaisdelegate.h"
#include "searchdialogproxymodel.h"
#include "ui_orcamento.h"
#include "usersession.h"
#include "venda.h"

Orcamento::Orcamento(QWidget *parent) : RegisterDialog("orcamento", "idOrcamento", parent), ui(new Ui::Orcamento) {
  ui->setupUi(this);

  setupTables();

  connect(ui->pushButtonCalculadora, &QPushButton::clicked, this, &Orcamento::on_pushButtonCalculadora_clicked);

  Q_FOREACH (const auto &line, findChildren<QLineEdit *>()) { connect(line, &QLineEdit::textEdited, this, &RegisterDialog::marcarDirty); }

  ui->itemBoxCliente->setRegisterDialog(new CadastroCliente(this));
  ui->itemBoxCliente->setSearchDialog(SearchDialog::cliente(this));
  ui->itemBoxConsultor->setSearchDialog(SearchDialog::vendedor(this));
  ui->itemBoxEndereco->setSearchDialog(SearchDialog::enderecoCliente(this));
  ui->itemBoxProduto->setSearchDialog(SearchDialog::produto(false, this));
  ui->itemBoxProfissional->setSearchDialog(SearchDialog::profissional(this));
  ui->itemBoxVendedor->setSearchDialog(SearchDialog::vendedor(this));

  setupMapper();
  newRegister();

  ui->labelConsultor->hide();
  ui->itemBoxConsultor->hide();
  ui->itemBoxConsultor->setReadOnlyItemBox(true);

  if (UserSession::tipoUsuario() == "ADMINISTRADOR") {
    ui->dateTimeEdit->setReadOnly(false);
    ui->dateTimeEdit->setCalendarPopup(true);
    ui->checkBoxFreteManual->show();
  }

  if (UserSession::tipoUsuario() == "VENDEDOR") { buscarParametrosFrete(); }

  on_checkBoxRepresentacao_toggled(false);

  ui->labelBaixa->hide();
  ui->plainTextEditBaixa->hide();
  ui->labelReplicaDe->hide();
  ui->labelReplicadoEm->hide();
  ui->lineEditReplicaDe->hide();
  ui->lineEditReplicadoEm->hide();

  setConnections();

  ui->pushButtonCalcularFrete->hide();
}

Orcamento::~Orcamento() { delete ui; }

void Orcamento::show() {
  RegisterDialog::show();
  ui->tableProdutos->resizeColumnsToContents();
}

void Orcamento::on_tableProdutos_clicked(const QModelIndex &index) {
  if (isReadOnly) { return; }

  ui->pushButtonAtualizarItem->show();
  ui->pushButtonAdicionarItem->hide();
  ui->pushButtonRemoverItem->show();
  mapperItem.setCurrentModelIndex(index);
  ui->tableProdutos->selectRow(index.row());
}

void Orcamento::setConnections() {
  connect(ui->checkBoxFreteManual, &QCheckBox::clicked, this, &Orcamento::on_checkBoxFreteManual_clicked);
  connect(ui->checkBoxRepresentacao, &QCheckBox::toggled, this, &Orcamento::on_checkBoxRepresentacao_toggled);
  connect(ui->doubleSpinBoxCaixas, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Orcamento::on_doubleSpinBoxCaixas_valueChanged);
  connect(ui->doubleSpinBoxDesconto, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Orcamento::on_doubleSpinBoxDesconto_valueChanged);
  connect(ui->doubleSpinBoxDescontoGlobal, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Orcamento::on_doubleSpinBoxDescontoGlobal_valueChanged);
  connect(ui->doubleSpinBoxDescontoGlobalReais, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Orcamento::on_doubleSpinBoxDescontoGlobalReais_valueChanged);
  connect(ui->doubleSpinBoxFrete, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Orcamento::on_doubleSpinBoxFrete_valueChanged);
  connect(ui->doubleSpinBoxQuant, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Orcamento::on_doubleSpinBoxQuant_valueChanged);
  connect(ui->doubleSpinBoxTotal, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Orcamento::on_doubleSpinBoxTotal_valueChanged);
  connect(ui->doubleSpinBoxTotalItem, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Orcamento::on_doubleSpinBoxTotalItem_valueChanged);
  connect(ui->itemBoxCliente, &ItemBox::textChanged, this, &Orcamento::on_itemBoxCliente_textChanged);
  connect(ui->itemBoxProduto, &ItemBox::valueChanged, this, &Orcamento::on_itemBoxProduto_valueChanged);
  connect(ui->itemBoxVendedor, &ItemBox::textChanged, this, &Orcamento::on_itemBoxVendedor_textChanged);
  connect(ui->pushButtonAdicionarItem, &QPushButton::clicked, this, &Orcamento::on_pushButtonAdicionarItem_clicked);
  connect(ui->pushButtonApagarOrc, &QPushButton::clicked, this, &Orcamento::on_pushButtonApagarOrc_clicked);
  connect(ui->pushButtonAtualizarItem, &QPushButton::clicked, this, &Orcamento::on_pushButtonAtualizarItem_clicked);
  connect(ui->pushButtonAtualizarOrcamento, &QPushButton::clicked, this, &Orcamento::on_pushButtonAtualizarOrcamento_clicked);
  connect(ui->pushButtonCadastrarOrcamento, &QPushButton::clicked, this, &Orcamento::on_pushButtonCadastrarOrcamento_clicked);
  connect(ui->pushButtonCalcularFrete, &QPushButton::clicked, this, &Orcamento::on_pushButtonCalcularFrete_clicked);
  connect(ui->pushButtonGerarExcel, &QPushButton::clicked, this, &Orcamento::on_pushButtonGerarExcel_clicked);
  connect(ui->pushButtonGerarVenda, &QPushButton::clicked, this, &Orcamento::on_pushButtonGerarVenda_clicked);
  connect(ui->pushButtonImprimir, &QPushButton::clicked, this, &Orcamento::on_pushButtonImprimir_clicked);
  connect(ui->pushButtonLimparSelecao, &QPushButton::clicked, this, &Orcamento::on_pushButtonLimparSelecao_clicked);
  connect(ui->pushButtonRemoverItem, &QPushButton::clicked, this, &Orcamento::on_pushButtonRemoverItem_clicked);
  connect(ui->pushButtonReplicar, &QPushButton::clicked, this, &Orcamento::on_pushButtonReplicar_clicked);
  connect(ui->tableProdutos, &TableView::clicked, this, &Orcamento::on_tableProdutos_clicked);
}

void Orcamento::unsetConnections() {
  disconnect(ui->checkBoxFreteManual, &QCheckBox::clicked, this, &Orcamento::on_checkBoxFreteManual_clicked);
  disconnect(ui->checkBoxRepresentacao, &QCheckBox::toggled, this, &Orcamento::on_checkBoxRepresentacao_toggled);
  disconnect(ui->doubleSpinBoxCaixas, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Orcamento::on_doubleSpinBoxCaixas_valueChanged);
  disconnect(ui->doubleSpinBoxDesconto, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Orcamento::on_doubleSpinBoxDesconto_valueChanged);
  disconnect(ui->doubleSpinBoxDescontoGlobal, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Orcamento::on_doubleSpinBoxDescontoGlobal_valueChanged);
  disconnect(ui->doubleSpinBoxDescontoGlobalReais, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Orcamento::on_doubleSpinBoxDescontoGlobalReais_valueChanged);
  disconnect(ui->doubleSpinBoxFrete, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Orcamento::on_doubleSpinBoxFrete_valueChanged);
  disconnect(ui->doubleSpinBoxQuant, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Orcamento::on_doubleSpinBoxQuant_valueChanged);
  disconnect(ui->doubleSpinBoxTotal, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Orcamento::on_doubleSpinBoxTotal_valueChanged);
  disconnect(ui->doubleSpinBoxTotalItem, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Orcamento::on_doubleSpinBoxTotalItem_valueChanged);
  disconnect(ui->itemBoxCliente, &ItemBox::textChanged, this, &Orcamento::on_itemBoxCliente_textChanged);
  disconnect(ui->itemBoxProduto, &ItemBox::valueChanged, this, &Orcamento::on_itemBoxProduto_valueChanged);
  disconnect(ui->itemBoxVendedor, &ItemBox::textChanged, this, &Orcamento::on_itemBoxVendedor_textChanged);
  disconnect(ui->pushButtonAdicionarItem, &QPushButton::clicked, this, &Orcamento::on_pushButtonAdicionarItem_clicked);
  disconnect(ui->pushButtonApagarOrc, &QPushButton::clicked, this, &Orcamento::on_pushButtonApagarOrc_clicked);
  disconnect(ui->pushButtonAtualizarItem, &QPushButton::clicked, this, &Orcamento::on_pushButtonAtualizarItem_clicked);
  disconnect(ui->pushButtonAtualizarOrcamento, &QPushButton::clicked, this, &Orcamento::on_pushButtonAtualizarOrcamento_clicked);
  disconnect(ui->pushButtonCadastrarOrcamento, &QPushButton::clicked, this, &Orcamento::on_pushButtonCadastrarOrcamento_clicked);
  disconnect(ui->pushButtonCalcularFrete, &QPushButton::clicked, this, &Orcamento::on_pushButtonCalcularFrete_clicked);
  disconnect(ui->pushButtonGerarExcel, &QPushButton::clicked, this, &Orcamento::on_pushButtonGerarExcel_clicked);
  disconnect(ui->pushButtonGerarVenda, &QPushButton::clicked, this, &Orcamento::on_pushButtonGerarVenda_clicked);
  disconnect(ui->pushButtonImprimir, &QPushButton::clicked, this, &Orcamento::on_pushButtonImprimir_clicked);
  disconnect(ui->pushButtonLimparSelecao, &QPushButton::clicked, this, &Orcamento::on_pushButtonLimparSelecao_clicked);
  disconnect(ui->pushButtonRemoverItem, &QPushButton::clicked, this, &Orcamento::on_pushButtonRemoverItem_clicked);
  disconnect(ui->pushButtonReplicar, &QPushButton::clicked, this, &Orcamento::on_pushButtonReplicar_clicked);
  disconnect(ui->tableProdutos, &TableView::clicked, this, &Orcamento::on_tableProdutos_clicked);
}

bool Orcamento::viewRegister() {
  unsetConnections();

  auto load = [&] {
    modelItem.setFilter("idOrcamento = '" + data(0, "idOrcamento").toString() + "'");

    if (not modelItem.select()) { return false; }

    if (not RegisterDialog::viewRegister()) { return false; }

    if (not buscarParametrosFrete()) { return false; }

    novoItem();

    const QString status = data("status").toString();

    if (status == "PERDIDO" or status == "CANCELADO") {
      ui->labelBaixa->show();
      ui->plainTextEditBaixa->show();
    }

    if (status == "ATIVO") { ui->pushButtonReplicar->hide(); }

    if (ui->dateTimeEdit->dateTime().addDays(data("validade").toInt()).date() < QDateTime::currentDateTime().date() or status != "ATIVO") {
      isReadOnly = true;

      ui->pushButtonGerarVenda->hide();
      ui->pushButtonAtualizarOrcamento->hide();
      ui->pushButtonReplicar->show();

      ui->pushButtonAdicionarItem->hide();
      ui->pushButtonAtualizarItem->hide();
      ui->pushButtonRemoverItem->hide();
      ui->pushButtonLimparSelecao->hide();
      ui->pushButtonCalculadora->hide();

      ui->itemBoxCliente->setReadOnlyItemBox(true);
      ui->itemBoxEndereco->setReadOnlyItemBox(true);
      ui->itemBoxProduto->setReadOnlyItemBox(true);
      ui->itemBoxProfissional->setReadOnlyItemBox(true);
      ui->itemBoxVendedor->setReadOnlyItemBox(true);

      ui->spinBoxPrazoEntrega->setReadOnly(true);

      ui->doubleSpinBoxDesconto->setReadOnly(true);
      ui->doubleSpinBoxDescontoGlobal->setReadOnly(true);
      ui->doubleSpinBoxDescontoGlobalReais->setReadOnly(true);
      ui->doubleSpinBoxFrete->setReadOnly(true);
      ui->doubleSpinBoxTotalItem->setReadOnly(true);
      ui->doubleSpinBoxQuant->setReadOnly(true);
      ui->doubleSpinBoxSubTotalBruto->setReadOnly(true);
      ui->doubleSpinBoxSubTotalLiq->setReadOnly(true);
      ui->doubleSpinBoxTotal->setReadOnly(true);

      ui->lineEditCodComercial->hide();
      ui->lineEditEstoque->hide();
      ui->lineEditFormComercial->hide();
      ui->lineEditFornecedor->hide();
      ui->spinBoxMinimo->hide();
      ui->lineEditObs->hide();
      ui->lineEditPrecoUn->hide();
      ui->lineEditUn->hide();
      ui->itemBoxProduto->hide();
      ui->doubleSpinBoxQuant->hide();
      ui->doubleSpinBoxCaixas->hide();
      ui->labelCaixa->hide();
      ui->spinBoxUnCx->hide();
      ui->doubleSpinBoxDesconto->hide();
      ui->doubleSpinBoxTotalItem->hide();
      ui->labelCaixas->hide();
      ui->labelCodComercial->hide();
      ui->labelProduto->hide();
      ui->labelDesconto->hide();
      ui->labelTotalItem->hide();
      ui->labelUn->hide();
      ui->labelPrecoUn->hide();
      ui->labelFornecedor->hide();
      ui->labelQuant->hide();
      ui->labelEstoque->hide();
      ui->labelFormComercial->hide();
      ui->labelMinimo->hide();
      ui->labelObs->hide();

      ui->plainTextEditObs->setReadOnly(true);

      ui->checkBoxFreteManual->setDisabled(true);
    } else {
      ui->pushButtonGerarVenda->show();
    }

    ui->lineEditReplicaDe->setReadOnly(true);
    ui->lineEditReplicadoEm->setReadOnly(true);

    ui->tableProdutos->resizeColumnsToContents();

    ui->checkBoxRepresentacao->setDisabled(true);

    ui->plainTextEditObs->setPlainText(data("observacao").toString());

    ui->doubleSpinBoxFrete->setValue(data("frete").toDouble());

    ui->doubleSpinBoxDescontoGlobalReais->setMaximum(ui->doubleSpinBoxSubTotalLiq->value());

    if (ui->checkBoxRepresentacao->isChecked()) { ui->itemBoxProduto->setRepresentacao(true); }

    if (not data("replicadoDe").toString().isEmpty()) {
      ui->labelReplicaDe->show();
      ui->lineEditReplicaDe->show();
    }

    if (not data("replicadoEm").toString().isEmpty()) {
      ui->labelReplicadoEm->show();
      ui->lineEditReplicadoEm->show();
    }

    if (data("idUsuarioConsultor").toInt() != 0) {
      ui->labelConsultor->show();
      ui->itemBoxConsultor->show();
    }

    return true;
  }();

  setConnections();

  return load;
}

void Orcamento::novoItem() {
  ui->pushButtonAdicionarItem->show();
  ui->pushButtonAtualizarItem->hide();
  ui->pushButtonRemoverItem->hide();
  ui->itemBoxProduto->clear();
  ui->tableProdutos->clearSelection();
  ui->tableProdutos->resizeColumnsToContents();

  // -----------------------

  ui->doubleSpinBoxCaixas->setDisabled(true);
  ui->doubleSpinBoxCaixas->setSingleStep(1.);
  ui->doubleSpinBoxCaixas->clear();
  ui->doubleSpinBoxDesconto->setDisabled(true);
  ui->doubleSpinBoxDesconto->clear();
  ui->doubleSpinBoxQuant->setDisabled(true);
  ui->doubleSpinBoxQuant->setSingleStep(1.);
  ui->doubleSpinBoxQuant->clear();
  ui->doubleSpinBoxTotalItem->clear();
  ui->doubleSpinBoxTotalItem->setDisabled(true);
  ui->lineEditCodComercial->clear();
  ui->lineEditEstoque->clear();
  ui->lineEditFormComercial->clear();
  ui->lineEditFornecedor->clear();
  ui->lineEditObs->clear();
  ui->lineEditPrecoUn->clear();
  ui->lineEditPrecoUn->setDisabled(true);
  ui->lineEditUn->clear();
  ui->lineEditUn->setDisabled(true);
  ui->spinBoxMinimo->clear();
  ui->spinBoxMinimo->setDisabled(true);
  ui->spinBoxUnCx->clear();
  ui->spinBoxUnCx->setDisabled(true);
}

void Orcamento::setupMapper() {
  addMapping(ui->checkBoxFreteManual, "freteManual");
  addMapping(ui->checkBoxRepresentacao, "representacao");
  addMapping(ui->dateTimeEdit, "data");
  addMapping(ui->doubleSpinBoxDescontoGlobal, "descontoPorc");
  addMapping(ui->doubleSpinBoxDescontoGlobalReais, "descontoReais");
  addMapping(ui->doubleSpinBoxFrete, "frete");
  addMapping(ui->doubleSpinBoxSubTotalBruto, "subTotalBru");
  addMapping(ui->doubleSpinBoxSubTotalLiq, "subTotalLiq");
  addMapping(ui->doubleSpinBoxTotal, "total");
  addMapping(ui->itemBoxCliente, "idCliente", "value");
  addMapping(ui->itemBoxEndereco, "idEnderecoEntrega", "value");
  addMapping(ui->itemBoxProfissional, "idProfissional", "value");
  addMapping(ui->itemBoxVendedor, "idUsuario", "value");
  addMapping(ui->itemBoxConsultor, "idUsuarioConsultor", "value");
  addMapping(ui->lineEditOrcamento, "idOrcamento");
  addMapping(ui->lineEditReplicaDe, "replicadoDe");
  addMapping(ui->lineEditReplicadoEm, "replicadoEm");
  addMapping(ui->plainTextEditBaixa, "observacaoCancelamento");
  addMapping(ui->plainTextEditObs, "observacao");
  addMapping(ui->spinBoxPrazoEntrega, "prazoEntrega");
  addMapping(ui->spinBoxValidade, "validade");

  mapperItem.setModel(ui->tableProdutos->model());
  mapperItem.setSubmitPolicy(QDataWidgetMapper::ManualSubmit);

  mapperItem.addMapping(ui->itemBoxProduto, modelItem.fieldIndex("idProduto"), "value");
  mapperItem.addMapping(ui->lineEditCodComercial, modelItem.fieldIndex("codComercial"));
  mapperItem.addMapping(ui->lineEditFormComercial, modelItem.fieldIndex("formComercial"));
  mapperItem.addMapping(ui->lineEditObs, modelItem.fieldIndex("obs"), "text");
  mapperItem.addMapping(ui->lineEditPrecoUn, modelItem.fieldIndex("prcUnitario"), "value");
  mapperItem.addMapping(ui->lineEditUn, modelItem.fieldIndex("un"), "text");
  mapperItem.addMapping(ui->doubleSpinBoxQuant, modelItem.fieldIndex("quant"), "value");
  mapperItem.addMapping(ui->doubleSpinBoxDesconto, modelItem.fieldIndex("desconto"), "value");
}

void Orcamento::registerMode() {
  ui->pushButtonCadastrarOrcamento->show();
  ui->pushButtonAtualizarOrcamento->hide();
  ui->pushButtonReplicar->hide();

  ui->pushButtonApagarOrc->setDisabled(true);
  ui->pushButtonGerarExcel->setDisabled(true);
  ui->pushButtonImprimir->setDisabled(true);
  ui->pushButtonGerarVenda->setEnabled(true);
  ui->itemBoxEndereco->setDisabled(true);
}

void Orcamento::updateMode() {
  ui->pushButtonCadastrarOrcamento->hide();
  ui->pushButtonAtualizarOrcamento->show();
  ui->pushButtonReplicar->show();

  ui->pushButtonApagarOrc->setEnabled(true);
  ui->pushButtonGerarExcel->setEnabled(true);
  ui->pushButtonImprimir->setEnabled(true);
  ui->pushButtonGerarVenda->setEnabled(true);
  ui->itemBoxEndereco->setVisible(true);
  ui->spinBoxValidade->setDisabled(true);
}

bool Orcamento::newRegister() {
  if (not RegisterDialog::newRegister()) { return false; }

  ui->lineEditOrcamento->setText("Auto gerado");
  ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());
  ui->spinBoxValidade->setValue(7);
  novoItem();

  return true;
}

void Orcamento::removeItem() {
  if (not modelItem.removeRow(ui->tableProdutos->currentIndex().row())) { return qApp->enqueueError("Erro removendo linha: " + modelItem.lastError().text()); }

  calcPrecoGlobalTotal();

  if (ui->lineEditOrcamento->text() != "Auto gerado") {
    if (not modelItem.submitAll()) { return; }

    save();
  }

  if (modelItem.rowCount() == 0) {
    if (ui->lineEditOrcamento->text() == "Auto gerado") { ui->checkBoxRepresentacao->setEnabled(true); }
    ui->itemBoxProduto->setFornecedorRep("");
  }

  novoItem();
}

bool Orcamento::generateId() {
  const auto siglaLoja = UserSession::fromLoja("sigla", ui->itemBoxVendedor->text());

  if (not siglaLoja) { return qApp->enqueueError(false, "Erro buscando sigla da loja!"); }

  QString id = siglaLoja.value().toString() + "-" + QDate::currentDate().toString("yy");

  const QString replica = ui->lineEditReplicaDe->text();

  if (replica.isEmpty()) {
    QSqlQuery query;
    query.prepare("SELECT MAX(idOrcamento) AS idOrcamento FROM orcamento WHERE idOrcamento LIKE :id");
    query.bindValue(":id", id + "%");

    if (not query.exec()) { return qApp->enqueueError(false, "Erro buscando próximo id disponível: " + query.lastError().text()); }

    const int last = query.first() ? query.value("idOrcamento").toString().remove(id).leftRef(4).toInt() : 0;

    id += QString("%1").arg(last + 1, 4, 10, QChar('0'));
    id += ui->checkBoxRepresentacao->isChecked() ? "R" : "";
    id += "O";

    if (id.size() != 12 and id.size() != 13) { return qApp->enqueueError(false, "Tamanho do Id errado: " + id); }
  } else {
    QSqlQuery query;
    query.prepare("SELECT COALESCE(MAX(CAST(REPLACE(idOrcamento, LEFT(idOrcamento, LOCATE('Rev', idOrcamento) + 2), '') AS UNSIGNED)) + 1, 1) AS revisao FROM orcamento WHERE LENGTH(idOrcamento) > 16 "
                  "AND idOrcamento LIKE :idOrcamento");
    query.bindValue(":idOrcamento", replica.left(16) + "%");

    if (not query.exec() or not query.first()) { return qApp->enqueueError(false, "Erro buscando próxima revisão disponível: " + query.lastError().text()); }

    id = replica.left(replica.indexOf("-Rev")) + "-Rev" + query.value("revisao").toString();
  }

  ui->lineEditOrcamento->setText(id);

  return true;
}

bool Orcamento::recalcularTotais() {
  double subTotalBruto = 0.;
  double subTotalLiq = 0.;
  double total = 0.;

  for (int row = 0; row < modelItem.rowCount(); ++row) {
    subTotalBruto += modelItem.data(row, "parcial").toDouble();
    subTotalLiq += modelItem.data(row, "parcialDesc").toDouble();
    total += modelItem.data(row, "total").toDouble();
  }

  if (abs(subTotalBruto - ui->doubleSpinBoxSubTotalBruto->value()) > 1) {
    calcPrecoGlobalTotal();
    return qApp->enqueueError(false, "Subtotal dos itens não confere com SubTotalBruto! Recalculando valores!");
  }

  if (abs(subTotalLiq - ui->doubleSpinBoxSubTotalLiq->value()) > 1) {
    calcPrecoGlobalTotal();
    return qApp->enqueueError(false, "Total dos itens não confere com SubTotalLíquido! Recalculando valores!");
  }

  if (abs(total - (ui->doubleSpinBoxTotal->value() - ui->doubleSpinBoxFrete->value())) > 1) {
    calcPrecoGlobalTotal();
    return qApp->enqueueError(false, "Total dos itens não confere com Total! Recalculando valores!");
  }

  return true;
}

bool Orcamento::verifyFields() {
  if (not recalcularTotais()) { return false; }

  if (ui->itemBoxCliente->text().isEmpty()) {
    ui->itemBoxCliente->setFocus();
    return qApp->enqueueError(false, "Cliente inválido!");
  }

  if (ui->itemBoxVendedor->text().isEmpty()) {
    ui->itemBoxVendedor->setFocus();
    return qApp->enqueueError(false, "Vendedor inválido!");
  }

  if (ui->itemBoxProfissional->text().isEmpty()) {
    ui->itemBoxProfissional->setFocus();
    return qApp->enqueueError(false, "Profissional inválido!");
  }

  if (ui->itemBoxEndereco->text().isEmpty()) {
    ui->itemBoxEndereco->setFocus();
    return qApp->enqueueError(false, R"(Endereço inválido! Se não possui endereço, escolha "Não há".)");
  }

  if (modelItem.rowCount() == 0) {
    ui->itemBoxProduto->setFocus();
    return qApp->enqueueError(false, "Não pode cadastrar um orçamento sem itens!");
  }

  return true;
}

bool Orcamento::savingProcedures() {
  if (not setData("data", ui->dateTimeEdit->dateTime())) { return false; }
  if (not setData("descontoPorc", ui->doubleSpinBoxDescontoGlobal->value())) { return false; }
  if (not setData("descontoReais", ui->doubleSpinBoxSubTotalLiq->value() * ui->doubleSpinBoxDescontoGlobal->value() / 100.)) { return false; }
  if (not setData("frete", ui->doubleSpinBoxFrete->value())) { return false; }
  if (not setData("idCliente", ui->itemBoxCliente->getValue())) { return false; }
  if (not setData("idEnderecoEntrega", ui->itemBoxEndereco->getValue())) { return false; }

  const auto idLoja = UserSession::fromLoja("usuario.idLoja", ui->itemBoxVendedor->text());

  if (not idLoja) { return qApp->enqueueError(false, "Erro buscando idLoja!"); }

  if (not setData("idLoja", idLoja.value().toInt())) { return false; }
  if (not setData("idOrcamento", ui->lineEditOrcamento->text())) { return false; }
  if (not setData("idProfissional", ui->itemBoxProfissional->getValue())) { return false; }
  if (not setData("idUsuario", ui->itemBoxVendedor->getValue())) { return false; }
  if (not setData("idUsuarioConsultor", ui->itemBoxConsultor->getValue())) { return false; }
  if (not setData("observacao", ui->plainTextEditObs->toPlainText())) { return false; }
  if (not setData("prazoEntrega", ui->spinBoxPrazoEntrega->value())) { return false; }
  if (not setData("replicadoDe", ui->lineEditReplicaDe->text())) { return false; }
  if (not setData("representacao", ui->checkBoxRepresentacao->isChecked())) { return false; }
  if (not setData("subTotalBru", ui->doubleSpinBoxSubTotalBruto->value())) { return false; }
  if (not setData("subTotalLiq", ui->doubleSpinBoxSubTotalLiq->value())) { return false; }
  if (not setData("total", ui->doubleSpinBoxTotal->value())) { return false; }
  if (not setData("validade", ui->spinBoxValidade->value())) { return false; }
  if (not setData("freteManual", ui->checkBoxFreteManual->isChecked())) { return false; }

  for (int row = 0, rowCount = modelItem.rowCount(); row < rowCount; ++row) {
    if (not modelItem.setData(row, "idOrcamento", ui->lineEditOrcamento->text())) { return false; }
    if (not modelItem.setData(row, "idLoja", model.data(currentRow, "idLoja"))) { return false; }
    const double prcUnitario = modelItem.data(row, "prcUnitario").toDouble();
    const double desconto = modelItem.data(row, "desconto").toDouble() / 100.;
    if (not modelItem.setData(row, "descUnitario", prcUnitario - (prcUnitario * desconto))) { return false; }
  }

  if (not buscarCadastrarConsultor()) { return false; }

  return atualizaReplica();
}

bool Orcamento::buscarCadastrarConsultor() {
  QStringList fornecedores;

  for (int row = 0, rowCount = modelItem.rowCount(); row < rowCount; ++row) { fornecedores << modelItem.data(row, "fornecedor").toString(); }

  fornecedores.removeDuplicates();

  for (auto &fornecedor : fornecedores) {
    fornecedor.prepend("'");
    fornecedor.append("'");
  }

  QSqlQuery query;

  if (not query.exec("SELECT idUsuario FROM usuario WHERE especialidade > 0 AND especialidade IN (SELECT especialidade FROM fornecedor WHERE razaoSocial IN (" + fornecedores.join(", ") + "))")) {
    return qApp->enqueueError(false, "Erro buscando consultor: " + query.lastError().text());
  }

  if (query.size() > 1) { return qApp->enqueueError(false, "Mais de um consultor disponível!"); }

  if (query.size() == 1 and query.first() and not setData("idUsuarioConsultor", query.value("idUsuario"))) { return false; }

  return true;
}

bool Orcamento::atualizaReplica() {
  if (not ui->lineEditReplicaDe->text().isEmpty()) {
    QSqlQuery query;
    query.prepare("UPDATE orcamento SET status = 'REPLICADO', replicadoEm = :idReplica WHERE idOrcamento = :idOrcamento");
    query.bindValue(":idReplica", ui->lineEditOrcamento->text());
    query.bindValue(":idOrcamento", ui->lineEditReplicaDe->text());

    if (not query.exec()) { return qApp->enqueueError(false, "Erro salvando replicadoEm: " + query.lastError().text()); }
  }

  return true;
}

void Orcamento::clearFields() {
  RegisterDialog::clearFields();

  if (UserSession::tipoUsuario() == "VENDEDOR" or UserSession::tipoUsuario() == "VENDEDOR ESPECIAL") { ui->itemBoxVendedor->setValue(UserSession::idUsuario()); }

  ui->itemBoxEndereco->setEnabled(false);
}

void Orcamento::on_pushButtonRemoverItem_clicked() { removeItem(); }

void Orcamento::on_doubleSpinBoxQuant_valueChanged(const double quant) {
  const double step = ui->doubleSpinBoxQuant->singleStep();
  const double quant2 = not qFuzzyIsNull(fmod(quant, step)) ? ceil(quant / step) * step : quant;

  if (not qFuzzyCompare(quant, quant2)) { ui->doubleSpinBoxQuant->setValue(quant2); }

  const double caixas = quant2 / ui->spinBoxUnCx->value();

  if (not qFuzzyCompare(ui->doubleSpinBoxCaixas->value(), caixas)) { ui->doubleSpinBoxCaixas->setValue(caixas); }
}

void Orcamento::on_pushButtonCadastrarOrcamento_clicked() {
  // TODO: ao fechar pedido calcular o frete com o endereco selecinado em 'end. entrega'
  // se o valor calculado for maior que o do campo frete pedir autorizacao do gerente para manter o valor atual
  // senao usa o valor calculado

  // pedir login caso o frete (manual ou automatico) seja menor que ou o valorPeso ou a porcentagem parametrizada

  save();
}

void Orcamento::on_pushButtonAtualizarOrcamento_clicked() { save(); }

void Orcamento::calcPrecoGlobalTotal() {
  double subTotalBruto = 0.;
  double subTotalItens = 0.;

  for (int row = 0, rowCount = modelItem.rowCount(); row < rowCount; ++row) {
    const double itemBruto = modelItem.data(row, "quant").toDouble() * modelItem.data(row, "prcUnitario").toDouble();
    const double descItem = modelItem.data(row, "desconto").toDouble() / 100.;
    const double stItem = itemBruto * (1. - descItem);
    subTotalBruto += itemBruto;
    subTotalItens += stItem;
  }

  ui->doubleSpinBoxSubTotalBruto->setValue(subTotalBruto);
  ui->doubleSpinBoxSubTotalLiq->setValue(subTotalItens);

  // calcula totais considerando desconto global atual

  ui->doubleSpinBoxFrete->setValue(ui->checkBoxFreteManual->isChecked() ? ui->doubleSpinBoxFrete->value() : qMax(ui->doubleSpinBoxSubTotalBruto->value() * porcFrete / 100., minimoFrete));

  const double frete = ui->doubleSpinBoxFrete->value();
  const double descGlobal = ui->doubleSpinBoxDescontoGlobal->value();

  ui->doubleSpinBoxDescontoGlobalReais->setMaximum(subTotalItens);
  ui->doubleSpinBoxDescontoGlobalReais->setValue(subTotalItens * descGlobal / 100);

  ui->doubleSpinBoxTotal->setMaximum(subTotalItens + frete);
  ui->doubleSpinBoxTotal->setValue(subTotalItens * (1 - (descGlobal / 100)) + frete);
}

void Orcamento::on_pushButtonImprimir_clicked() {
  Impressao impressao(data("idOrcamento").toString());
  impressao.print();
}

void Orcamento::setupTables() {
  modelItem.setTable("orcamento_has_produto");
  modelItem.setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelItem.setHeaderData("produto", "Produto");
  modelItem.setHeaderData("fornecedor", "Fornecedor");
  modelItem.setHeaderData("obs", "Obs.");
  modelItem.setHeaderData("prcUnitario", "Preço/Un.");
  modelItem.setHeaderData("caixas", "Caixas");
  modelItem.setHeaderData("quant", "Quant.");
  modelItem.setHeaderData("un", "Un.");
  modelItem.setHeaderData("codComercial", "Código");
  modelItem.setHeaderData("formComercial", "Formato");
  modelItem.setHeaderData("unCaixa", "Un./Caixa");
  modelItem.setHeaderData("parcial", "Subtotal");
  modelItem.setHeaderData("desconto", "Desc. %");
  modelItem.setHeaderData("parcialDesc", "Total");

  modelItem.setFilter("0");

  if (not modelItem.select()) { return; }

  ui->tableProdutos->setModel(new SearchDialogProxyModel(&modelItem, this));
  ui->tableProdutos->hideColumn("idOrcamentoProduto");
  ui->tableProdutos->hideColumn("idProduto");
  ui->tableProdutos->hideColumn("idOrcamento");
  ui->tableProdutos->hideColumn("idLoja");
  ui->tableProdutos->hideColumn("unCaixa");
  ui->tableProdutos->hideColumn("descUnitario");
  ui->tableProdutos->hideColumn("descGlobal");
  ui->tableProdutos->hideColumn("total");
  ui->tableProdutos->hideColumn("estoque");
  ui->tableProdutos->hideColumn("promocao");
  ui->tableProdutos->hideColumn("mostrarDesconto");

  ui->tableProdutos->setItemDelegate(new DoubleDelegate(this));
  ui->tableProdutos->setItemDelegateForColumn(modelItem.fieldIndex("quant"), new DoubleDelegate(this, 4));
  ui->tableProdutos->setItemDelegateForColumn("prcUnitario", new ReaisDelegate(this));
  ui->tableProdutos->setItemDelegateForColumn("parcial", new ReaisDelegate(this));
  ui->tableProdutos->setItemDelegateForColumn("parcialDesc", new ReaisDelegate(this));
  ui->tableProdutos->setItemDelegateForColumn("desconto", new PorcentagemDelegate(this));
}

void Orcamento::atualizarItem() { adicionarItem(true); }

void Orcamento::adicionarItem(const bool isUpdate) {
  unsetConnections();

  [=] {
    ui->checkBoxRepresentacao->setDisabled(true);

    if (ui->itemBoxProduto->text().isEmpty()) { return qApp->enqueueError("Item inválido!"); }

    if (qFuzzyIsNull(ui->doubleSpinBoxQuant->value())) { return qApp->enqueueError("Quantidade inválida!"); }

    const int row = isUpdate ? mapperItem.currentIndex() : modelItem.rowCount();

    if (row == -1) { return qApp->enqueueError("Erro linha - 1 adicionarItem"); }

    if (not isUpdate) { modelItem.insertRow(row); }

    if (not modelItem.setData(row, "idProduto", ui->itemBoxProduto->getValue().toInt())) { return; }
    if (not modelItem.setData(row, "fornecedor", ui->lineEditFornecedor->text())) { return; }
    if (not modelItem.setData(row, "produto", ui->itemBoxProduto->text())) { return; }
    if (not modelItem.setData(row, "obs", ui->lineEditObs->text())) { return; }
    if (not modelItem.setData(row, "prcUnitario", ui->lineEditPrecoUn->getValue())) { return; }
    if (not modelItem.setData(row, "caixas", ui->doubleSpinBoxCaixas->value())) { return; }
    if (not modelItem.setData(row, "quant", ui->doubleSpinBoxQuant->value())) { return; }
    if (not modelItem.setData(row, "unCaixa", ui->doubleSpinBoxQuant->singleStep())) { return; }
    if (not modelItem.setData(row, "un", ui->lineEditUn->text())) { return; }
    if (not modelItem.setData(row, "codComercial", ui->lineEditCodComercial->text())) { return; }
    if (not modelItem.setData(row, "formComercial", ui->lineEditFormComercial->text())) { return; }
    if (not modelItem.setData(row, "desconto", ui->doubleSpinBoxDesconto->value())) { return; }
    if (not modelItem.setData(row, "estoque", currentItemIsEstoque)) { return; }
    if (not modelItem.setData(row, "promocao", currentItemIsPromocao)) { return; }
    if (not modelItem.setData(row, "parcial", modelItem.data(row, "quant").toDouble() * modelItem.data(row, "prcUnitario").toDouble())) { return; }
    if (not modelItem.setData(row, "parcialDesc", ui->doubleSpinBoxTotalItem->value())) { return; }
    if (not modelItem.setData(row, "descGlobal", ui->doubleSpinBoxDescontoGlobal->value())) { return; }
    if (not modelItem.setData(row, "total", ui->doubleSpinBoxTotalItem->value() * (1 - (ui->doubleSpinBoxDescontoGlobal->value() / 100)))) { return; }
    const bool mostrarDesconto = (modelItem.data(row, "total").toDouble() - modelItem.data(row, "parcial").toDouble()) < -0.1;
    if (not modelItem.setData(row, "mostrarDesconto", mostrarDesconto)) { return; }

    if (ui->lineEditOrcamento->text() != "Auto gerado") { save(true); }

    if (modelItem.rowCount() == 1 and ui->checkBoxRepresentacao->isChecked()) { ui->itemBoxProduto->setFornecedorRep(modelItem.data(row, "fornecedor").toString()); }

    isDirty = true;

    ui->tableProdutos->resizeColumnsToContents();
  }();

  calcPrecoGlobalTotal();

  setConnections();
}

void Orcamento::on_pushButtonAdicionarItem_clicked() { adicionarItem(); }

void Orcamento::on_pushButtonAtualizarItem_clicked() { atualizarItem(); }

void Orcamento::on_pushButtonGerarVenda_clicked() {
  if (not save(true)) { return; }

  const QDateTime time = ui->dateTimeEdit->dateTime();

  if (not time.isValid()) { return; }

  if (time.addDays(data("validade").toInt()).date() < QDateTime::currentDateTime().date()) { return qApp->enqueueError("Orçamento vencido!"); }

  if (ui->itemBoxEndereco->text().isEmpty()) {
    ui->itemBoxEndereco->setFocus();
    return qApp->enqueueError("Deve selecionar endereço!");
  }

  if (not verificaCadastroCliente()) { return; }

  auto *venda = new Venda(parentWidget());
  venda->prepararVenda(ui->lineEditOrcamento->text());

  close();
}

void Orcamento::on_doubleSpinBoxCaixas_valueChanged(const double caixas) {
  const double caixas2 = not qFuzzyIsNull(fmod(caixas, ui->doubleSpinBoxCaixas->singleStep())) ? ceil(caixas) : caixas;
  const double quant = caixas2 * ui->spinBoxUnCx->value();
  const double prcUn = ui->lineEditPrecoUn->getValue();
  const double desc = ui->doubleSpinBoxDesconto->value() / 100.;
  const double itemBruto = quant * prcUn;

  if (not qFuzzyCompare(caixas, caixas2)) { ui->doubleSpinBoxCaixas->setValue(caixas2); }

  unsetConnections();

  ui->doubleSpinBoxQuant->setValue(quant);
  ui->doubleSpinBoxTotalItem->setValue(itemBruto * (1. - desc));

  setConnections();
}

void Orcamento::on_pushButtonApagarOrc_clicked() {
  auto *baixa = new BaixaOrcamento(data("idOrcamento").toString(), this);
  baixa->show();
}

void Orcamento::on_itemBoxProduto_valueChanged(const QVariant &) {
  if (ui->itemBoxProduto->text().isEmpty()) { return; }

  // -------------------------------------------------------------------------

  ui->doubleSpinBoxCaixas->clear();
  ui->doubleSpinBoxDesconto->clear();
  ui->doubleSpinBoxQuant->clear();
  ui->doubleSpinBoxTotalItem->clear();
  ui->lineEditCodComercial->clear();
  ui->lineEditEstoque->clear();
  ui->lineEditFormComercial->clear();
  ui->lineEditFornecedor->clear();
  if (ui->pushButtonAdicionarItem->isVisible()) { ui->lineEditObs->clear(); }
  ui->lineEditPrecoUn->clear();
  ui->lineEditUn->clear();
  ui->spinBoxMinimo->clear();
  ui->spinBoxUnCx->clear();

  // -------------------------------------------------------------------------

  QSqlQuery query;
  query.prepare("SELECT un, precoVenda, estoqueRestante, fornecedor, codComercial, formComercial, m2cx, pccx, minimo, multiplo, estoque, promocao FROM produto WHERE idProduto = :idProduto");
  query.bindValue(":idProduto", ui->itemBoxProduto->getValue());

  if (not query.exec() or not query.first()) { return qApp->enqueueError("Erro na busca do produto: " + query.lastError().text()); }

  const QString un = query.value("un").toString().toUpper();

  ui->lineEditUn->setText(un);
  ui->lineEditPrecoUn->setValue(query.value("precoVenda").toDouble());
  ui->lineEditEstoque->setValue(query.value("estoqueRestante").toDouble());
  ui->lineEditFornecedor->setText(query.value("fornecedor").toString());
  ui->lineEditCodComercial->setText(query.value("codComercial").toString());
  ui->lineEditFormComercial->setText(query.value("formComercial").toString());

  const QString uncxString = un.contains("M2") or un.contains("M²") or un.contains("ML") ? "m2cx" : "pccx";

  ui->spinBoxUnCx->setValue(query.value(uncxString).toDouble());

  const double minimo = query.value("minimo").toDouble();
  const double multiplo = query.value("multiplo").toDouble();
  const double uncx = query.value(uncxString).toDouble();

  ui->spinBoxMinimo->setValue(minimo);
  ui->doubleSpinBoxQuant->setMinimum(minimo);
  ui->doubleSpinBoxCaixas->setMinimum(minimo / uncx);

  currentItemIsEstoque = query.value("estoque").toInt();
  currentItemIsPromocao = query.value("promocao").toBool();

  if (currentItemIsEstoque != 0) {
    ui->doubleSpinBoxQuant->setMaximum(query.value("estoqueRestante").toDouble());
    ui->doubleSpinBoxCaixas->setMaximum(query.value("estoqueRestante").toDouble() / uncx);
  } else {
    ui->doubleSpinBoxQuant->setMaximum(9999999.000000);
    ui->doubleSpinBoxCaixas->setMaximum(9999999.000000);
  }

  ui->doubleSpinBoxCaixas->setEnabled(true);
  ui->doubleSpinBoxCaixas->setEnabled(true);
  ui->doubleSpinBoxDesconto->setEnabled(true);
  ui->doubleSpinBoxQuant->setEnabled(true);
  ui->doubleSpinBoxTotalItem->setEnabled(true);
  ui->lineEditPrecoUn->setEnabled(true);
  ui->lineEditUn->setEnabled(true);
  ui->spinBoxMinimo->setEnabled(true);
  ui->spinBoxUnCx->setEnabled(true);

  ui->doubleSpinBoxCaixas->setSingleStep(1.);
  ui->doubleSpinBoxQuant->setSingleStep(uncx);

  // TODO: 0verificar se preciso tratar os casos sem multiplo
  // if (minimo != 0) ...
  if (not qFuzzyIsNull(minimo) and not qFuzzyIsNull(multiplo)) {
    ui->doubleSpinBoxCaixas->setSingleStep(multiplo / uncx);
    ui->doubleSpinBoxQuant->setSingleStep(multiplo);
  }

  ui->doubleSpinBoxQuant->setValue(0.);
  ui->doubleSpinBoxCaixas->setValue(0.);
  ui->doubleSpinBoxDesconto->setValue(0.);

  ui->tableProdutos->clearSelection();
}

void Orcamento::on_itemBoxCliente_textChanged(const QString &) {
  ui->itemBoxEndereco->setFilter("idCliente = " + QString::number(ui->itemBoxCliente->getValue().toInt()) + " AND desativado = FALSE OR idEndereco = 1");

  QSqlQuery queryCliente;
  queryCliente.prepare("SELECT idProfissionalRel FROM cliente WHERE idCliente = :idCliente");
  queryCliente.bindValue(":idCliente", ui->itemBoxCliente->getValue());

  if (not queryCliente.exec() or not queryCliente.first()) { return qApp->enqueueError("Erro ao buscar cliente: " + queryCliente.lastError().text()); }

  ui->itemBoxProfissional->setValue(queryCliente.value("idProfissionalRel"));
  ui->itemBoxEndereco->setEnabled(true);
  ui->itemBoxEndereco->clear();
}

void Orcamento::on_pushButtonLimparSelecao_clicked() { novoItem(); }

void Orcamento::on_checkBoxFreteManual_clicked(const bool checked) {
  ui->doubleSpinBoxFrete->setFrame(checked);
  ui->doubleSpinBoxFrete->setReadOnly(not checked);
  ui->doubleSpinBoxFrete->setButtonSymbols(checked ? QDoubleSpinBox::UpDownArrows : QDoubleSpinBox::NoButtons);

  ui->doubleSpinBoxFrete->setValue(ui->checkBoxFreteManual->isChecked() ? ui->doubleSpinBoxFrete->value() : qMax(ui->doubleSpinBoxSubTotalBruto->value() * porcFrete / 100., minimoFrete));
}

void Orcamento::on_pushButtonReplicar_clicked() {
  auto *replica = new Orcamento(parentWidget());

  replica->ui->pushButtonReplicar->hide();

  replica->ui->itemBoxCliente->setValue(data("idCliente"));
  replica->ui->itemBoxProfissional->setValue(data("idProfissional"));
  replica->ui->itemBoxVendedor->setValue(data("idUsuario"));
  replica->ui->itemBoxEndereco->setValue(data("idEnderecoEntrega"));
  replica->ui->spinBoxValidade->setValue(data("validade").toInt());
  replica->ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());
  replica->ui->checkBoxRepresentacao->setChecked(ui->checkBoxRepresentacao->isChecked());
  replica->ui->lineEditReplicaDe->setText(data("idOrcamento").toString());
  replica->ui->plainTextEditObs->setPlainText(data("observacao").toString());

  for (int row = 0; row < modelItem.rowCount(); ++row) {
    replica->ui->itemBoxProduto->setValue(modelItem.data(row, "idProduto"));
    replica->ui->doubleSpinBoxQuant->setValue(modelItem.data(row, "quant").toDouble());
    replica->ui->doubleSpinBoxDesconto->setValue(modelItem.data(row, "desconto").toDouble());
    replica->ui->lineEditObs->setText(modelItem.data(row, "obs").toString());
    replica->adicionarItem();
  }

  replica->show();
}

bool Orcamento::cadastrar() {
  currentRow = tipo == Tipo::Atualizar ? mapper.currentIndex() : model.rowCount();

  if (currentRow == -1) { return qApp->enqueueError(false, "Erro linha -1"); }

  if (tipo == Tipo::Cadastrar) {
    if (not generateId()) { return false; }

    if (not model.insertRow(currentRow)) { return qApp->enqueueError(false, "Erro inserindo linha na tabela"); }
  }

  if (not savingProcedures()) { return false; }

  if (not model.submitAll()) { return false; }

  primaryId = ui->lineEditOrcamento->text();

  if (primaryId.isEmpty()) { return qApp->enqueueError(false, "Id vazio!"); }

  return modelItem.submitAll();
}

bool Orcamento::verificaCadastroCliente() {
  const int idCliente = ui->itemBoxCliente->getValue().toInt();

  // REFAC: simplify this function

  QSqlQuery queryCliente;
  queryCliente.prepare("SELECT cpf, cnpj FROM cliente WHERE idCliente = :idCliente");
  queryCliente.bindValue(":idCliente", idCliente);

  if (not queryCliente.exec() or not queryCliente.first()) { return qApp->enqueueError(false, "Erro verificando se cliente possui CPF/CNPJ: " + queryCliente.lastError().text()); }

  if (queryCliente.value("cpf").toString().isEmpty() and queryCliente.value("cnpj").toString().isEmpty()) {
    auto *cadCliente = new CadastroCliente(this);
    cadCliente->viewRegisterById(idCliente);
    cadCliente->show();

    return qApp->enqueueError(false, "Cliente não possui CPF/CNPJ cadastrado!");
  }

  QSqlQuery queryCadastro;
  queryCadastro.prepare("SELECT idCliente FROM cliente_has_endereco WHERE idCliente = :idCliente");
  queryCadastro.bindValue(":idCliente", idCliente);

  if (not queryCadastro.exec()) { return qApp->enqueueError(false, "Erro verificando se cliente possui endereço: " + queryCadastro.lastError().text()); }

  if (not queryCadastro.first()) {
    auto *cadCliente = new CadastroCliente(this);
    cadCliente->viewRegisterById(idCliente);
    cadCliente->show();

    return qApp->enqueueError(false, "Cliente não possui endereço cadastrado!");
  }

  queryCadastro.prepare("SELECT c.incompleto FROM orcamento o LEFT JOIN cliente c ON o.idCliente = c.idCliente WHERE c.idCliente = :idCliente AND c.incompleto = TRUE");
  queryCadastro.bindValue(":idCliente", idCliente);

  if (not queryCadastro.exec()) { return qApp->enqueueError(false, "Erro verificando se cadastro do cliente está completo: " + queryCadastro.lastError().text()); }

  if (queryCadastro.first()) {
    auto *cadCliente = new CadastroCliente(this);
    cadCliente->viewRegisterById(idCliente);
    cadCliente->show();

    return qApp->enqueueError(false, "Cadastro incompleto, deve terminar!");
  }

  return true;
}

void Orcamento::on_pushButtonGerarExcel_clicked() {
  Excel excel(ui->lineEditOrcamento->text());
  excel.gerarExcel();
}

void Orcamento::on_checkBoxRepresentacao_toggled(const bool checked) { ui->itemBoxProduto->setRepresentacao(checked); }

void Orcamento::on_doubleSpinBoxDesconto_valueChanged(const double desconto) {
  const double caixas = ui->doubleSpinBoxCaixas->value();
  const double caixas2 = not qFuzzyIsNull(fmod(caixas, ui->doubleSpinBoxCaixas->singleStep())) ? ceil(caixas) : caixas;
  const double quant = caixas2 * ui->spinBoxUnCx->value();

  unsetConnections();

  const double prcUn = ui->lineEditPrecoUn->getValue();
  const double itemBruto = quant * prcUn;

  ui->doubleSpinBoxTotalItem->setValue(itemBruto * (1. - (desconto / 100)));

  setConnections();
}

void Orcamento::on_doubleSpinBoxDescontoGlobalReais_valueChanged(const double desconto) {
  const double liq = ui->doubleSpinBoxSubTotalLiq->value();
  const double frete = ui->doubleSpinBoxFrete->value();

  unsetConnections();

  [=] {
    for (int row = 0; row < modelItem.rowCount(); ++row) {
      if (not modelItem.setData(row, "descGlobal", desconto / liq * 100)) { return; }

      const double parcialDesc = modelItem.data(row, "parcialDesc").toDouble();
      if (not modelItem.setData(row, "total", parcialDesc * (1 - (desconto / liq)))) { return; }
    }

    ui->doubleSpinBoxDescontoGlobal->setValue(desconto / liq * 100);
    ui->doubleSpinBoxTotal->setValue(liq - desconto + frete);
  }();

  setConnections();
}

void Orcamento::on_doubleSpinBoxFrete_valueChanged(const double frete) {
  const double subTotalLiq = ui->doubleSpinBoxSubTotalLiq->value();
  const double desconto = ui->doubleSpinBoxDescontoGlobalReais->value();

  unsetConnections();

  ui->doubleSpinBoxTotal->setMinimum(frete);
  ui->doubleSpinBoxTotal->setMaximum(ui->doubleSpinBoxSubTotalLiq->value() + frete);
  ui->doubleSpinBoxTotal->setValue(subTotalLiq - desconto + frete);

  setConnections();
}

void Orcamento::on_itemBoxVendedor_textChanged(const QString &) {
  if (ui->itemBoxVendedor->text().isEmpty()) { return; }
  if (data("freteManual").toBool()) { return; }

  if (not buscarParametrosFrete()) { return; }

  ui->doubleSpinBoxFrete->setValue(ui->checkBoxFreteManual->isChecked() ? ui->doubleSpinBoxFrete->value() : qMax(ui->doubleSpinBoxSubTotalBruto->value() * porcFrete / 100., minimoFrete));
}

bool Orcamento::buscarParametrosFrete() {
  const auto idLoja = UserSession::fromLoja("usuario.idLoja", ui->itemBoxVendedor->text());

  if (not idLoja) { return qApp->enqueueError(false, "Erro buscando idLoja!"); }

  QSqlQuery queryFrete;
  queryFrete.prepare("SELECT valorMinimoFrete, porcentagemFrete FROM loja WHERE idLoja = :idLoja");
  queryFrete.bindValue(":idLoja", idLoja.value().toInt());

  if (not queryFrete.exec() or not queryFrete.next()) { return qApp->enqueueError(false, "Erro buscando parâmetros do frete: " + queryFrete.lastError().text()); }

  minimoFrete = queryFrete.value("valorMinimoFrete").toDouble();
  porcFrete = queryFrete.value("porcentagemFrete").toDouble();

  return true;
}

void Orcamento::on_doubleSpinBoxDescontoGlobal_valueChanged(const double desconto) {
  const double liq = ui->doubleSpinBoxSubTotalLiq->value();
  const double frete = ui->doubleSpinBoxFrete->value();

  unsetConnections();

  [=] {
    for (int row = 0; row < modelItem.rowCount(); ++row) {
      if (not modelItem.setData(row, "descGlobal", desconto)) { return; }

      const double parcialDesc = modelItem.data(row, "parcialDesc").toDouble();
      if (not modelItem.setData(row, "total", parcialDesc * (1 - (desconto / 100)))) { return; }
    }

    ui->doubleSpinBoxDescontoGlobalReais->setValue(liq * desconto / 100);
    ui->doubleSpinBoxTotal->setValue(liq * (1 - (desconto / 100)) + frete);
  }();

  setConnections();
}

void Orcamento::on_doubleSpinBoxTotal_valueChanged(const double total) {
  const double liq = ui->doubleSpinBoxSubTotalLiq->value();
  const double frete = ui->doubleSpinBoxFrete->value();

  unsetConnections();

  [=] {
    for (int row = 0; row < modelItem.rowCount(); ++row) {
      if (not modelItem.setData(row, "descGlobal", (liq - (total - frete)) / liq * 100)) { return; }

      const double parcialDesc = modelItem.data(row, "parcialDesc").toDouble();
      if (not modelItem.setData(row, "total", parcialDesc * (1 - ((liq - (total - frete)) / liq)))) { return; }
    }

    ui->doubleSpinBoxDescontoGlobal->setValue((liq - (total - frete)) / liq * 100);
    ui->doubleSpinBoxDescontoGlobalReais->setValue(liq - (total - frete));
  }();

  setConnections();
}

void Orcamento::on_doubleSpinBoxTotalItem_valueChanged(const double) {
  if (ui->itemBoxProduto->text().isEmpty()) { return; }

  const double quant = ui->doubleSpinBoxQuant->value();
  const double prcUn = ui->lineEditPrecoUn->getValue();
  const double itemBruto = quant * prcUn;
  const double subTotalItem = ui->doubleSpinBoxTotalItem->value();
  const double desconto = (itemBruto - subTotalItem) / itemBruto * 100.;

  if (qFuzzyIsNull(itemBruto)) { return; }

  unsetConnections();

  ui->doubleSpinBoxDesconto->setValue(desconto);

  setConnections();
}

void Orcamento::successMessage() { qApp->enqueueInformation(tipo == Tipo::Atualizar ? "Cadastro atualizado!" : "Orçamento cadastrado com sucesso!"); }

void Orcamento::on_pushButtonCalculadora_clicked() { QDesktopServices::openUrl(QUrl::fromLocalFile(R"(C:\Windows\System32\calc.exe)")); }

void Orcamento::on_pushButtonCalcularFrete_clicked() {
  LoginDialog dialog(LoginDialog::Tipo::Autorizacao, this);

  if (dialog.exec() == QDialog::Rejected) { return; }

  auto *frete = new CalculoFrete(this);
  frete->setCliente(ui->itemBoxCliente->getValue());
  frete->exec();

  const double dist = frete->getDistancia();

  if (qFuzzyIsNull(dist)) { return qApp->enqueueError("Não foi possível determinar a distância!"); }

  int peso = 0;

  QSqlQuery query;
  query.prepare("SELECT kgcx FROM produto WHERE idProduto = :idProduto");

  for (int row = 0; row < modelItem.rowCount(); ++row) {
    query.bindValue(":idProduto", modelItem.data(row, "idProduto"));

    if (not query.exec() or not query.first()) { return qApp->enqueueError("Erro buscando peso do produto: " + query.lastError().text()); }

    peso += modelItem.data(row, "caixas").toInt() * query.value("kgcx").toInt();
  }

  if (not query.exec("SELECT custoTransporteTon, custoTransporte1, custoTransporte2, custoFuncionario FROM loja WHERE nomeFantasia = 'Geral'") or not query.first()) {
    return qApp->enqueueError("Erro buscando parâmetros: " + query.lastError().text());
  }

  const double custoTon = query.value("custoTransporteTon").toDouble();
  const double custo1 = query.value("custoTransporte1").toDouble();
  const double custo2 = query.value("custoTransporte2").toDouble();
  const double custoFuncionario = query.value("custoFuncionario").toDouble();

  qDebug() << "peso: " << peso;

  int cargas = peso / 4500;
  int restante = peso % 4500;

  qDebug() << "inteiro: " << cargas;
  qDebug() << "resto: " << restante;

  // TODO: se endereco for 'nao há/retira' calcular apenas o valorPeso
  const double valorPeso = (peso / 1000.0 * custoTon);

  const double valorDistCargaCheia = (cargas * custo2 * dist) + (cargas * 3 * custoFuncionario);
  const double valorDistMeiaCarga =
      cargas > 0 and restante < 200 ? 0 : (restante < 2000 ? dist * custo1 + (2 * custoFuncionario / 2000.0 * restante) : dist * custo2 + (3 * custoFuncionario / 4500.0 * restante));
  const double valorFrete = valorPeso + valorDistCargaCheia + valorDistMeiaCarga;

  qDebug() << "valorPeso: " << valorPeso;
  qDebug() << "valorDistCargaCheia: " << valorDistCargaCheia;
  qDebug() << "valorDistMeiaCarga: " << valorDistMeiaCarga;
  qDebug() << "frete: " << valorFrete;

  // frete = (pesoProduto(ton.) * 180) + (pesoProduto < 2ton. ? dist. * 1.5 : pesoProduto < 4.5 ? dist. * 2 : fracionar cargas)
}

// NOTE: model.submitAll faz mapper voltar para -1, select tambem (talvez porque
// submitAll chama select)
// TODO: 0se produto for estoque permitir vender por peça (setar minimo/multiplo)
// TODO: 2orcamento de reposicao nao pode ter profissional associado (bloquear)
// TODO: 4quando cadastrar cliente no itemBox mudar para o id dele
// TODO: ?permitir que o usuario digite um valor e o sistema faça o calculo na linha?
// TODO: limitar o total ao frete? se o desconto é 100% e o frete não é zero, o minimo é o frete
// TODO: implementar mover linha para baixo/cima (talvez com drag-n-drop?) http://apocalyptech.com/linux/qt/qtableview/
