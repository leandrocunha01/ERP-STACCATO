#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QVBoxLayout>

#include "ui_widgetpagamentos.h"
#include "usersession.h"
#include "widgetpagamentos.h"

WidgetPagamentos::WidgetPagamentos(QWidget *parent) : QScrollArea(parent), ui(new Ui::WidgetPagamentos) {
  ui->setupUi(this);
  setWidgetResizable(true);

  QFrame *frame = new QFrame(this);
  QVBoxLayout *scrollLayout = new QVBoxLayout(frame);
  scrollLayout->setSizeConstraint(QLayout::SetMinimumSize);
  setWidget(frame);
}

WidgetPagamentos::~WidgetPagamentos() { delete ui; }

void WidgetPagamentos::adicionarPagamentoCompra(const double restante) {
  QFrame *frame = new QFrame(this);
  frame->setLayout(new QHBoxLayout(frame));
  // label
  QLabel *labelPagamento = new QLabel(this);
  labelPagamento->setText("Pgt." + QString::number(widget()->children().size()));
  frame->layout()->addWidget(labelPagamento);
  // combobox pgt
  QComboBox *comboBoxPgt = new QComboBox(this);
  comboBoxPgt->setMinimumWidth(140);

  QSqlQuery queryPag;
  queryPag.prepare("SELECT pagamento FROM view_pagamento_loja WHERE idLoja = :idLoja");
  queryPag.bindValue(":idLoja", UserSession::idLoja());

  if (not queryPag.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo formas de pagamentos: " + queryPag.lastError().text());
    return;
  }

  const QStringList list([&queryPag]() {
    QStringList temp("Escolha uma opção!");
    while (queryPag.next()) temp << queryPag.value("pagamento").toString();
    return temp;
  }());

  comboBoxPgt->insertItems(0, list);
  frame->layout()->addWidget(comboBoxPgt);
  connect(comboBoxPgt, &QComboBox::currentTextChanged, this, [=] { on_comboBoxPgt_currentTextChanged(listComboPgt.indexOf(comboBoxPgt), comboBoxPgt->currentText()); });
  listComboPgt << comboBoxPgt;
  // combobox data
  QComboBox *comboBoxData = new QComboBox(this);
  comboBoxData->insertItem(0, "Data Mês");
  comboBoxData->insertItem(1, "Data + 1 Mês");
  comboBoxData->insertItem(2, "14");
  comboBoxData->insertItem(3, "20");
  comboBoxData->insertItem(4, "28");
  comboBoxData->insertItem(5, "30");
  frame->layout()->addWidget(comboBoxData);
  connect(comboBoxData, &QComboBox::currentTextChanged, this, &WidgetPagamentos::montarFluxoCaixa);
  listComboData << comboBoxData;
  // combobox parc
  QComboBox *comboboxPgtParc = new QComboBox(this);
  comboboxPgtParc->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  comboboxPgtParc->setMaximumWidth(45);
  connect(comboboxPgtParc, &QComboBox::currentTextChanged, this, &WidgetPagamentos::montarFluxoCaixa);
  frame->layout()->addWidget(comboboxPgtParc);
  listComboParc << comboboxPgtParc;
  // doublespinbox
  QDoubleSpinBox *doubleSpinBoxPgt = new QDoubleSpinBox(this);
  doubleSpinBoxPgt->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  doubleSpinBoxPgt->setMinimumWidth(80);
  doubleSpinBoxPgt->setPrefix("R$ ");
  doubleSpinBoxPgt->setMaximum(restante);
  doubleSpinBoxPgt->setValue(restante);
  doubleSpinBoxPgt->setGroupSeparatorShown(true);
  frame->layout()->addWidget(doubleSpinBoxPgt);
  connect(doubleSpinBoxPgt, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &WidgetPagamentos::valueChanged);
  listDoubleSpinPgt << doubleSpinBoxPgt;
  // dateedit
  QDateEdit *dateEditPgt = new QDateEdit(this);
  dateEditPgt->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  dateEditPgt->setDisplayFormat("dd/MM/yy");
  dateEditPgt->setCalendarPopup(true);
  dateEditPgt->setDate(QDate::currentDate());
  connect(dateEditPgt, &QDateEdit::dateChanged, this, &WidgetPagamentos::montarFluxoCaixa);
  frame->layout()->addWidget(dateEditPgt);
  listDatePgt << dateEditPgt;
  // lineedit
  QLineEdit *lineEditPgt = new QLineEdit(this);
  lineEditPgt->setPlaceholderText("Observação");
  connect(lineEditPgt, &QLineEdit::textChanged, this, &WidgetPagamentos::montarFluxoCaixa);
  frame->layout()->addWidget(lineEditPgt);
  listLinePgt << lineEditPgt;
  //
  widget()->layout()->addWidget(frame);
}

void WidgetPagamentos::adicionarPagamentoVenda(const bool representacao, const QString &idOrcamento, const double creditoTotal, const double restante) {
  QFrame *frame = new QFrame(this);
  frame->setLayout(new QHBoxLayout(frame));
  // label
  QLabel *labelPagamento = new QLabel(frame);
  labelPagamento->setText("Pgt." + QString::number(widget()->children().size()));
  frame->layout()->addWidget(labelPagamento);
  // checkbox
  QCheckBox *checkboxRep = new QCheckBox(frame);
  checkboxRep->setText("Fornecedor");
  checkboxRep->setVisible(representacao);
  checkboxRep->setChecked(representacao);
  checkboxRep->setEnabled(false);
  connect(checkboxRep, &QCheckBox::stateChanged, this, &WidgetPagamentos::montarFluxoCaixa);
  frame->layout()->addWidget(checkboxRep);
  listCheckBoxRep << checkboxRep;
  // combo
  QComboBox *comboBoxPgt = new QComboBox(frame);
  comboBoxPgt->setMinimumWidth(140);

  QSqlQuery queryOrc;
  queryOrc.prepare(
      "SELECT idUsuario, idOrcamento, idLoja, idUsuarioIndicou, idCliente, idEnderecoEntrega, idEnderecoFaturamento, idProfissional, data, subTotalBru, subTotalLiq, frete, freteManual, descontoPorc, "
      "descontoReais, total, status, observacao, prazoEntrega, representacao FROM orcamento WHERE idOrcamento = :idOrcamento");
  queryOrc.bindValue(":idOrcamento", idOrcamento);

  if (not queryOrc.exec() or not queryOrc.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando orçamento: " + queryOrc.lastError().text());
    return;
  }

  QSqlQuery queryPag;
  queryPag.prepare("SELECT pagamento FROM view_pagamento_loja WHERE idLoja = :idLoja");
  queryPag.bindValue(":idLoja", queryOrc.value("idLoja"));

  if (not queryPag.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo formas de pagamentos: " + queryPag.lastError().text());
    return;
  }

  const QStringList list([&queryPag]() {
    QStringList temp("Escolha uma opção!");
    while (queryPag.next()) temp << queryPag.value("pagamento").toString();
    return temp;
  }());

  comboBoxPgt->insertItems(0, list);
  if (creditoTotal > 0) comboBoxPgt->addItem("Conta Cliente");
  frame->layout()->addWidget(comboBoxPgt);
  connect(comboBoxPgt, &QComboBox::currentTextChanged, this, [=] { on_comboBoxPgt_currentTextChanged(listComboPgt.indexOf(comboBoxPgt), comboBoxPgt->currentText(), creditoTotal); });
  listComboPgt << comboBoxPgt;
  // combo
  QComboBox *comboboxPgtParc = new QComboBox(frame);
  comboboxPgtParc->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  comboboxPgtParc->setMaximumWidth(45);
  connect(comboboxPgtParc, &QComboBox::currentTextChanged, this, &WidgetPagamentos::montarFluxoCaixa);
  frame->layout()->addWidget(comboboxPgtParc);
  listComboParc << comboboxPgtParc;
  // doublespinbox
  QDoubleSpinBox *doubleSpinBoxPgt = new QDoubleSpinBox(frame);
  doubleSpinBoxPgt->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  doubleSpinBoxPgt->setMinimumWidth(80);
  doubleSpinBoxPgt->setPrefix("R$ ");
  doubleSpinBoxPgt->setMaximum(restante);
  doubleSpinBoxPgt->setValue(restante);
  doubleSpinBoxPgt->setGroupSeparatorShown(true);
  frame->layout()->addWidget(doubleSpinBoxPgt);
  connect(doubleSpinBoxPgt, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &WidgetPagamentos::valueChanged);
  listDoubleSpinPgt << doubleSpinBoxPgt;
  // dateedit
  QDateEdit *dateEditPgt = new QDateEdit(frame);
  dateEditPgt->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  dateEditPgt->setDisplayFormat("dd/MM/yy");
  dateEditPgt->setCalendarPopup(true);
  dateEditPgt->setDate(QDate::currentDate());
  connect(dateEditPgt, &QDateEdit::dateChanged, this, &WidgetPagamentos::montarFluxoCaixa);
  frame->layout()->addWidget(dateEditPgt);
  listDatePgt << dateEditPgt;
  // lineedit
  QLineEdit *lineEditPgt = new QLineEdit(frame);
  lineEditPgt->setPlaceholderText("Observação");
  connect(lineEditPgt, &QLineEdit::textChanged, this, &WidgetPagamentos::montarFluxoCaixa);
  frame->layout()->addWidget(lineEditPgt);
  listLinePgt << lineEditPgt;
  //

  widget()->layout()->addWidget(frame);
}

void WidgetPagamentos::on_comboBoxPgt_currentTextChanged(const int index, const QString &text, const double creditoTotal) {
  if (text == "Escolha uma opção!") return;

  if (text == "Conta Cliente") {
    listDoubleSpinPgt.at(index)->setMaximum(creditoTotal);
    listComboParc.at(index)->clear();
    listComboParc.at(index)->addItem("1x");
    emit montarFluxoCaixa();
    return;
  }

  QSqlQuery query;
  query.prepare("SELECT parcelas FROM forma_pagamento WHERE pagamento = :pagamento");
  query.bindValue(":pagamento", listComboPgt.at(index)->currentText());

  if (not query.exec() or not query.first()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo formas de pagamentos: " + query.lastError().text());
    return;
  }

  const int parcelas = query.value("parcelas").toInt();

  listComboParc.at(index)->clear();

  for (int i = 0; i < parcelas; ++i) listComboParc.at(index)->addItem(QString::number(i + 1) + "x");

  listComboParc.at(index)->setEnabled(true);

  listDatePgt.at(index)->setEnabled(true);

  emit montarFluxoCaixa();
}

void WidgetPagamentos::on_comboBoxPgt_currentTextChanged(const int index, const QString &text) {
  if (text == "Escolha uma opção!") return;

  QSqlQuery query;
  query.prepare("SELECT parcelas FROM forma_pagamento WHERE pagamento = :pagamento");
  query.bindValue(":pagamento", listComboPgt.at(index)->currentText());

  if (not query.exec() or not query.first()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo formas de pagamentos: " + query.lastError().text());
    return;
  }

  const int parcelas = query.value("parcelas").toInt();

  listComboParc.at(index)->clear();

  for (int i = 0; i < parcelas; ++i) listComboParc.at(index)->addItem(QString::number(i + 1) + "x");

  listComboParc.at(index)->setEnabled(true);

  listDatePgt.at(index)->setEnabled(true);

  emit montarFluxoCaixa();
}

void WidgetPagamentos::resetarPagamentos() {
  for (auto item : widget()->children()) {
    if (qobject_cast<QFrame *>(item)) delete item;
  }

  listCheckBoxRep.clear();
  listComboData.clear();
  listComboParc.clear();
  listComboPgt.clear();
  listDatePgt.clear();
  listDoubleSpinPgt.clear();
  listLinePgt.clear();
}