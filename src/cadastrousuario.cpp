#include <QCheckBox>
#include <QMessageBox>
#include <QSqlQuery>

#include "cadastrousuario.h"
#include "ui_cadastrousuario.h"
#include "usersession.h"
#include "searchdialog.h"

CadastroUsuario::CadastroUsuario(QWidget *parent)
  : RegisterDialog("usuario", "idUsuario", parent), ui(new Ui::CadastroUsuario) {
  ui->setupUi(this);

  for (const auto *line : findChildren<QLineEdit *>()) {
    connect(line, &QLineEdit::textEdited, this, &RegisterDialog::marcarDirty);
  }

  setupTablePermissoes();
  fillCombobox();

  ui->lineEditSigla->setInputMask(">AAA");

  ui->tablePermissoes->setEnabled(false);
  ui->tablePermissoes->setToolTip("Função indisponível nesta versão!");
  ui->tablePermissoes->resizeColumnsToContents();

  ui->tabWidget->setTabEnabled(1, false);

  setupMapper();
  newRegister();
}

CadastroUsuario::~CadastroUsuario() { delete ui; }

void CadastroUsuario::setupTablePermissoes() {
  ui->tablePermissoes->resizeColumnsToContents();
  ui->tablePermissoes->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  ui->tablePermissoes->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  // NOTE: checkbox em tableWidget

  for (int i = 0, rowCount = ui->tablePermissoes->rowCount(); i < rowCount; ++i) {
    for (int j = 0, columnCount = ui->tablePermissoes->columnCount(); j < columnCount; ++j) {
      QWidget *widget = new QWidget();
      QCheckBox *checkBox = new QCheckBox();
      QHBoxLayout *layout = new QHBoxLayout(widget);
      layout->addWidget(checkBox);
      layout->setAlignment(Qt::AlignCenter);
      layout->setContentsMargins(0, 0, 0, 0);
      widget->setLayout(layout);
      ui->tablePermissoes->setCellWidget(i, j, widget);
    }
  }
}

bool CadastroUsuario::verifyFields() {
  for (auto const &line : ui->gridLayout_2->findChildren<QLineEdit *>()) {
    if (not verifyRequiredField(line)) {
      return false;
    }
  }

  if (ui->lineEditPasswd->text() != ui->lineEditPasswd_2->text()) {
    ui->lineEditPasswd->setFocus();
    QMessageBox::critical(this, "Erro!", "As senhas não batem!");
    return false;
  }

  return true;
}

void CadastroUsuario::clearFields() { RegisterDialog::clearFields(); }

void CadastroUsuario::setupMapper() {
  addMapping(ui->lineEditNome, "nome");
  addMapping(ui->lineEditUser, "user");
  addMapping(ui->comboBoxTipo, "tipo");
  addMapping(ui->comboBoxLoja, "idLoja", "currentValue");
  addMapping(ui->lineEditSigla, "sigla");
}

void CadastroUsuario::registerMode() {
  ui->pushButtonCadastrar->show();
  ui->pushButtonAtualizar->hide();
  ui->pushButtonRemover->hide();
}

void CadastroUsuario::updateMode() {
  ui->pushButtonCadastrar->hide();
  ui->pushButtonAtualizar->show();
  ui->pushButtonRemover->show();
}

bool CadastroUsuario::savingProcedures() {
  setData("nome", ui->lineEditNome->text());
  setData("idLoja", ui->comboBoxLoja->getCurrentValue());
  setData("tipo", ui->comboBoxTipo->currentText());
  setData("user", ui->lineEditUser->text());
  setData("sigla", ui->lineEditSigla->text());
  setData("user", ui->lineEditUser->text());

  if (ui->lineEditPasswd->text() != "********") {
    QSqlQuery query("SELECT PASSWORD('" + ui->lineEditPasswd->text() + "')");
    query.first();
    setData("passwd", query.value(0));
  }

  return isOk;
}

bool CadastroUsuario::viewRegister(const QModelIndex index) {
  if (not RegisterDialog::viewRegister(index)) {
    return false;
  }

  ui->lineEditPasswd->setText("********");
  ui->lineEditPasswd_2->setText("********");

  return true;
}

void CadastroUsuario::fillCombobox() {
  QSqlQuery query("SELECT * FROM loja");

  while (query.next()) {
    ui->comboBoxLoja->addItem(query.value("descricao").toString(), query.value("idLoja"));
  }

  ui->comboBoxLoja->setCurrentValue(UserSession::getLoja());
}

void CadastroUsuario::on_pushButtonCadastrar_clicked() { save(); }

void CadastroUsuario::on_pushButtonAtualizar_clicked() { update(); }

void CadastroUsuario::on_pushButtonNovoCad_clicked() { newRegister(); }

void CadastroUsuario::on_pushButtonRemover_clicked() { remove(); }

void CadastroUsuario::on_pushButtonCancelar_clicked() { close(); }

void CadastroUsuario::on_pushButtonBuscar_clicked() {
  SearchDialog *sdUsuario = SearchDialog::usuario(this);
  connect(sdUsuario, &SearchDialog::itemSelected, this, &CadastroUsuario::viewRegisterById);
  sdUsuario->show();
}

void CadastroUsuario::show() {
  QWidget::show();
  adjustSize();
}
