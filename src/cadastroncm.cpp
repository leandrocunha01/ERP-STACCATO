#include "cadastroncm.h"
#include "ui_cadastroncm.h"

#include "application.h"
#include "porcentagemdelegate.h"

#include <QDebug>

CadastroNCM::CadastroNCM(QWidget *parent) : QDialog(parent), ui(new Ui::CadastroNCM) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);

  setupTables();

  setConnections();
}

CadastroNCM::~CadastroNCM() { delete ui; }

void CadastroNCM::setConnections() {
  const auto connectionType = static_cast<Qt::ConnectionType>(Qt::AutoConnection | Qt::UniqueConnection);

  connect(ui->lineEditBusca, &QLineEdit::textChanged, this, &CadastroNCM::on_lineEditBusca_textChanged, connectionType);
  connect(ui->pushButtonAdicionar, &QPushButton::clicked, this, &CadastroNCM::on_pushButtonAdicionar_clicked, connectionType);
  connect(ui->pushButtonCancelar, &QPushButton::clicked, this, &CadastroNCM::on_pushButtonCancelar_clicked, connectionType);
  connect(ui->pushButtonRemover, &QPushButton::clicked, this, &CadastroNCM::on_pushButtonRemover_clicked, connectionType);
  connect(ui->pushButtonSalvar, &QPushButton::clicked, this, &CadastroNCM::on_pushButtonSalvar_clicked, connectionType);
}

void CadastroNCM::setupTables() {
  model.setTable("ncm");

  model.setSort("ncm");

  model.setHeaderData("ncm", "NCM");
  model.setHeaderData("cest", "CEST");
  model.setHeaderData("mva4", "MVA 4%");
  model.setHeaderData("mva12", "MVA 12%");
  model.setHeaderData("aliq", "Alíq. ICMS");

  model.setFilter("");

  model.select();

  ui->table->setModel(&model);

  ui->table->hideColumn("idncm");

  ui->table->setItemDelegateForColumn("mva4", new PorcentagemDelegate(false, this));
  ui->table->setItemDelegateForColumn("mva12", new PorcentagemDelegate(false, this));
  ui->table->setItemDelegateForColumn("aliq", new PorcentagemDelegate(false, this));
}

void CadastroNCM::on_pushButtonSalvar_clicked() {
  for (int row = 0; row < model.rowCount(); ++row) {
    if (model.data(row, "ncm").toString().length() != 8) { throw RuntimeError("NCM deve ter 8 dígitos!", this); }
    // TODO: se houver algum dado em CEST verificar se o tamanho é 7
  }

  model.submitAll();

  qApp->enqueueInformation("Dados atualizados!", this);
  close();
}

void CadastroNCM::on_pushButtonCancelar_clicked() { close(); }

void CadastroNCM::on_lineEditBusca_textChanged(const QString &text) { model.setFilter("ncm LIKE '%" + text + "%'"); }

void CadastroNCM::on_pushButtonAdicionar_clicked() {
  const int row = model.insertRowAtEnd();
  ui->table->selectRow(row);
}

void CadastroNCM::on_pushButtonRemover_clicked() {
  const auto selection = ui->table->selectionModel()->selectedRows();

  if (selection.isEmpty()) { throw RuntimeError("Nenhuma linha selecionada!", this); }

  model.removeSelection(selection);

  model.submitAll();
}

// TODO: avisar após digitar NCM se ele já estiver cadastrado (o banco de dados não vai permitir cadastrar duplicado mas a mensagem de
// erro dele não é amigável
// TODO: colocar created/lastUpdated no sql
// TODO: CEST deve ter 7 digitos, validar
