#include <QDebug>
#include <QMessageBox>
#include <QSqlError>

#include "cadastrocliente.h"
#include "ui_cadastrocliente.h"
#include "searchdialog.h"
#include "cepcompleter.h"

CadastroCliente::CadastroCliente(QWidget *parent)
  : RegisterDialog("Cadastro", "idCadastro", parent), ui(new Ui::CadastroCliente) {
  ui->setupUi(this);
//  ui->widgetEnd_1->setupUi(ui->lineEditRG, ui->lineEditTel_Res);
//  ui->widgetEnd_2->setupUi(ui->groupBoxCobranca, ui->groupBoxEntrega);
//  ui->widgetEnd_3->setupUi(ui->groupBoxEntrega, ui->comboBoxCliente);
  ui->lineEditCEP->setInputMask("99999-999;_");
  ui->lineEditUF->setInputMask(">AA;_");
  ui->pushButtonMostrarInativos->hide();
  modelEnd.setTable("Endereco");
  ui->tableView->setModel(&modelEnd);
  modelEnd.setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelEnd.setFilter("idEndereco in (SELECT idEndereco FROM Cadastro_has_Endereco WHERE idCadastro = '" + data(primaryKey).toString() + "')");
  modelEnd.select();
  mapperEnd.setModel(&modelEnd);
  setupUi();
  setTipoClienteFornecedor("CLIENTE");

  setupMapper();
  newRegister();
}

void CadastroCliente::setupUi() {
  ui->lineEditCPF->setInputMask("999.999.999-99;_");
  ui->lineEditRG->setInputMask("99.999.999-9;_");
  ui->lineEditIdNextel->setInputMask("99*9999999*99999;_");
  ui->lineEditCNPJ->setInputMask("99.999.999/9999-99;_");
  // Placeholders
  ui->lineEditCPF->setPlaceholderText("999.999.999-99");
  ui->lineEditEmail->setPlaceholderText("usuario@email.com");
  ui->lineEditNextel->setPlaceholderText("(99)99999-9999");
  ui->comboBoxCliente->addItem("Escolha uma opção!");
  QSqlQuery query("SELECT idCadastro, nome, razaoSocial FROM Cadastro WHERE clienteFornecedor = 'CLIENTE' AND idCadastro != '"  + data(primaryKey).toString() + "'" );
  while (query.next()) {
    QString str = query.value(1).toString() + " - " + query.value(2).toString();
    ui->comboBoxCliente->addItem(str, query.value(0));
  }
  ui->comboBoxProfissional->addItem("Escolha uma opção!");
  QSqlQuery queryProf("SELECT idProfissional, nome, tipo FROM Profissional;");
  while (queryProf.next()) {
    QString str = queryProf.value(1).toString() + " [" + queryProf.value(2).toString() + "] ";
    ui->comboBoxProfissional->addItem(str, queryProf.value(0));
  }
  ui->comboBoxVendedor->addItem("Escolha uma opção!");
  QSqlQuery queryVend("SELECT idUsuario, nome FROM Usuario;");
  while (queryVend.next()) {
    QString str = queryVend.value(0).toString() + " - " + queryVend.value(1).toString();
    ui->comboBoxVendedor->addItem(str, queryVend.value(0));
  }
//  ui->radioButtonCliente->setChecked(true);
}

CadastroCliente::~CadastroCliente() {
  delete ui;
}

bool CadastroCliente::verifyRequiredField(QLineEdit *line, bool silent) {
  if (line->styleSheet() != requiredStyle()) {
    return true;
  }
  //  if(line->parent()->isWindowType() && line->parent()->objectName() != objectName() ) {
  //    return true;
  //  }
  if ((line->text().isEmpty()) || line->text() == "0,00" || line->text() == "../-" ||
      (line->text().size() < (line->inputMask().remove(";").remove(">").remove("_").size()) ||
       (line->text().size() < line->placeholderText().size() - 1))) {
    qDebug() << "ObjectName: " << line->parent()->objectName() << ", line: " << line->objectName() << " | "
             << line->text();
    if(!silent) {
      QMessageBox::warning(dynamic_cast<QWidget *>(this), "Atenção!",
                           "Você não preencheu um campo obrigatório!", QMessageBox::Ok, QMessageBox::NoButton);
      line->setFocus();
    }
    return false;
  }
  return true;
}

bool CadastroCliente::verifyFields() {
  if (!RegisterDialog::verifyFields({ui->lineEditNome, ui->lineEditCPF}))
    return false;

//  QString tipo;
//  if (ui->radioButtonCliente->isChecked()) {
//    tipo = "CLIENTE";
//  } else if (ui->radioButtonForn->isChecked()) {
//    tipo = "FORNECEDOR";
//  } else if (ui->radioButtonAmbos->isChecked()) {
//    tipo = "AMBOS";
//  } else {
//    QMessageBox::warning(this, "Atenção!", "Você não preencheu um campo obrigatório!", QMessageBox::Ok,
//                         QMessageBox::NoButton);
//    ui->radioButtonCliente->setFocus();
  //    return false;
//    setData("incompleto", true);
//  }

  if(modelEnd.rowCount() == 0) {
    setData("incompleto", true);
  }
//  if (!ui->widgetEnd_1->verifyFields(true)) {
//    //    return false;
//  }

  foreach (QLineEdit *line, ui->groupBoxContatos->findChildren<QLineEdit *>()) {
    if (!verifyRequiredField(line, true)) {
      //      return false;
      setData("incompleto", true);
    }
  }

  foreach (QLineEdit *line, ui->groupBoxPJuridica->findChildren<QLineEdit *>()) {
    if (!verifyRequiredField(line, true)) {
      //      return false;
      setData("incompleto", true);
    }
  }

//  if (!ui->widgetEnd_2->verifyFields(true)) {
//    //    return false;
//    setData("incompleto", true);
//  }

//  if (!ui->widgetEnd_3->verifyFields(true)) {
//    //    return false;
//    setData("incompleto", true);
//  }

  foreach (QComboBox *box, this->findChildren<QComboBox *>()) {
    if (box->styleSheet() == requiredStyle()) {
      if (box->currentText().isEmpty()) {
        box->setFocus();
        QMessageBox::warning(this, "Atenção!", "Você não preencheu um campo obrigatório!", QMessageBox::Ok,
                             QMessageBox::NoButton);
        //        return false;
        setData("incompleto", true);
      }
    }
  }

  setData("clienteFornecedor", tipoClienteFornecedor);
  return true;
}

bool CadastroCliente::savingProcedures(int row) {

  if(!ui->lineEditNome->text().isEmpty()) {
    setData(row, "nome", ui->lineEditNome->text());
  }
  if(!ui->lineEditCPF->text().isEmpty()) {
    setData(row, "cpf", ui->lineEditCPF->text());
  }
  if(!ui->lineEditApelido->text().isEmpty()) {
    setData(row, "apelido", ui->lineEditApelido->text());
  }
  if(!ui->lineEditRG->text().remove(".").remove("-").isEmpty()) {
    setData(row, "rg", ui->lineEditRG->text());
  }
  if(!ui->lineEditCNPJ->text().remove(".").remove("/").remove("-").isEmpty()) {
    setData(row, "cnpj", ui->lineEditCNPJ->text());
  }
  if(!ui->lineEditRazaoSocial->text().isEmpty()) {
    setData(row, "razaoSocial", ui->lineEditRazaoSocial->text());
  }
  if(!ui->lineEditNomeFantasia->text().isEmpty()) {
    setData(row, "nomeFantasia", ui->lineEditNomeFantasia->text());
  }
  if(!ui->lineEditInscEstadual->text().isEmpty()) {
    setData(row, "inscEstadual", ui->lineEditInscEstadual->text());
  }
  if(!ui->lineEditTel_Res->text().isEmpty()) {
    setData(row, "tel", ui->lineEditTel_Res->text());
  }
  if(!ui->lineEditTel_Cel->text().isEmpty()) {
    setData(row, "telCel", ui->lineEditTel_Cel->text());
  }
  if(!ui->lineEditTel_Com->text().isEmpty()) {
    setData(row, "telCom", ui->lineEditTel_Com->text());
  }
  if(!ui->lineEditNextel->text().isEmpty()) {
    setData(row, "nextel", ui->lineEditNextel->text());
  }
  if(!ui->lineEditEmail->text().isEmpty()) {
    setData(row, "email", ui->lineEditEmail->text());
  }
  setData(row, "idCadastroRel", ui->comboBoxCliente->getCurrentValue());
  setData(row, "idProfissionalRel", ui->comboBoxProfissional->getCurrentValue());
  setData(row, "idUsuarioRel", ui->comboBoxVendedor->getCurrentValue());

  setData(row, "clienteFornecedor", tipoClienteFornecedor);
  setData(row, "pfpj", tipoPFPJ);

  if(!model.submitAll()) {
    qDebug() << objectName() << " : " << __LINE__ << " : Error on model.submitAll() : " << modelEnd.lastError();
    return false;
  }
  qDebug() << "PK = " <<  data(row, primaryKey);
  int idCad = data(row, primaryKey).toInt();

  if(modelEnd.isDirty()) {
    if(!modelEnd.submitAll()) {
      qDebug() << objectName() << " : " << __LINE__ << " : Error on modelEnd.submitAll() : " << modelEnd.lastError();
      qDebug() << "QUERY : " << modelEnd.query().lastQuery();
      return false;
    }
  }

  for( int end = 0; end < modelEnd.rowCount( ); ++ end) {
    int idEnd = modelEnd.data(modelEnd.index(end,modelEnd.fieldIndex("idEndereco"))).toInt();
    QSqlQuery qry;
    qry.prepare("INSERT IGNORE INTO Cadastro_has_Endereco (idCadastro, idEndereco)  VALUES( :idCadastro, :idEndereco );");
    qry.bindValue(":idCadastro", idCad);
    qry.bindValue(":idEndereco", idEnd);
    if(!qry.exec()) {
      qDebug() <<  objectName() << " : " << __LINE__ << " : Error insert into Cadastro_has_Endereco : " << qry.lastError();
      qDebug() << "QUERY : " << qry.lastQuery();
      qDebug() << "Bound values : " << idCad << ", " << idEnd;
      return false;
    }
  }
  return true;
}

void CadastroCliente::clearFields() {
  RegisterDialog::clearFields();
  ui->radioButtonPF->setChecked(true);
//  ui->groupBoxCobranca->setChecked(false);
//  ui->groupBoxEntrega->setChecked(false);
//  foreach (WidgetEndereco *w, this->findChildren<WidgetEndereco *>()) {
//    w->clearFields();
//    w->setEnabled(false);
//  }
//  ui->widgetEnd_1->setEnabled(true);
  novoEnd();
  foreach (QComboBox *box, this->findChildren<QComboBox *>()) {
    box->clear();
  }
  setupUi();
}

void CadastroCliente::setupMapper() {
  addMapping(ui->lineEditNome, "nome");
  addMapping(ui->lineEditApelido, "apelido");
  addMapping(ui->lineEditRG, "rg");
  addMapping(ui->lineEditCPF, "cpf");
  addMapping(ui->lineEditCNPJ, "cnpj");
  addMapping(ui->lineEditRazaoSocial, "razaoSocial");
  addMapping(ui->lineEditNomeFantasia, "nomeFantasia");
  addMapping(ui->lineEditInscEstadual, "inscEstadual");
  addMapping(ui->lineEditTel_Res, "tel");
  addMapping(ui->lineEditTel_Cel, "telCel");
  addMapping(ui->lineEditTel_Com, "telCom");
  addMapping(ui->lineEditIdNextel, "idNextel");
  addMapping(ui->lineEditNextel, "nextel");
  addMapping(ui->lineEditEmail, "email");
  addMapping(ui->comboBoxCliente, "idCadastroRel", "currentValue");
  addMapping(ui->comboBoxProfissional, "idProfissionalRel", "currentValue");
  addMapping(ui->comboBoxVendedor, "idUsuarioRel", "currentValue");

  mapperEnd.addMapping(ui->lineEditDescricao, modelEnd.fieldIndex("descricao"));
  mapperEnd.addMapping(ui->lineEditCEP, modelEnd.fieldIndex("CEP"));
  mapperEnd.addMapping(ui->lineEditEndereco, modelEnd.fieldIndex("logradouro"));
  mapperEnd.addMapping(ui->lineEditNro, modelEnd.fieldIndex("numero"));
  mapperEnd.addMapping(ui->lineEditComp, modelEnd.fieldIndex("complemento"));
  mapperEnd.addMapping(ui->lineEditBairro, modelEnd.fieldIndex("bairro"));
  mapperEnd.addMapping(ui->lineEditCidade, modelEnd.fieldIndex("cidade"));
  mapperEnd.addMapping(ui->lineEditUF, modelEnd.fieldIndex("uf"));
}

void CadastroCliente::registerMode() {
  ui->pushButtonCadastrar->show();
  ui->pushButtonAtualizar->hide();
  //  ui->pushButtonNovoCad->show();
  ui->pushButtonRemover->hide();
}

void CadastroCliente::updateMode() {
  ui->pushButtonCadastrar->hide();
  ui->pushButtonAtualizar->show();
  //  ui->pushButtonNovoCad->show();
  ui->pushButtonRemover->show();
}
QString CadastroCliente::getTipoClienteFornecedor() const {
  return tipoClienteFornecedor;
}

void CadastroCliente::setTipoClienteFornecedor(const QString & value) {
  bool isForn = (value == "FORNECEDOR");
  ui->groupBoxMaisInfo->setHidden(isForn);
  ui->groupBoxPFPJ->setHidden(isForn);
  if(isForn) {
    ui->radioButtonPJ->setChecked(true);
  }
  tipoClienteFornecedor = value;
  adjustSize();
}


QString CadastroCliente::getTipo() const {
  return tipoPFPJ;
}

void CadastroCliente::setTipo(const QString &value) {
  tipoPFPJ = value;
}

bool CadastroCliente::viewRegister(QModelIndex idx) {
  if (!confirmationMessage()) {
    return false;
  }
  clearFields();
  updateMode();
  model.select();
  mapper.setCurrentModelIndex(idx);


  modelEnd.setFilter("idEndereco in (SELECT idEndereco FROM Cadastro_has_Endereco WHERE idCadastro = '" + data(primaryKey).toString() + "')");
  if( !modelEnd.select() ) {
    qDebug() << modelEnd.lastError();
  }


  tipoPFPJ = data("pfpj").toString();
  setTipoClienteFornecedor(data("clienteFornecedor").toString());
//  int idEndFaturamento = data("idEnderecoFaturamento").toInt();
//  int idEndCobranca = data("idEnderecoCobranca").toInt();
//  int idEndEntrega = data("idEnderecoEntrega").toInt();
//  ui->widgetEnd_1->viewCadastro(idEndFaturamento);
//  qDebug() << idEndFaturamento << ", " << idEndCobranca << ", " << idEndEntrega;
//  if (idEndFaturamento != idEndCobranca) {
//    ui->groupBoxCobranca->setChecked(true);
//    ui->widgetEnd_2->setEnabled(true);
//    ui->widgetEnd_2->viewCadastro(idEndCobranca);
//  } else {
//    ui->groupBoxCobranca->setChecked(false);
//  }
//  if (idEndFaturamento != idEndEntrega) {
//    ui->groupBoxEntrega->setChecked(true);
//    ui->widgetEnd_3->setEnabled(true);
//    ui->widgetEnd_3->viewCadastro(idEndEntrega);
//  } else {
//    ui->groupBoxEntrega->setChecked(false);
//  }
  ui->groupBoxPJuridica->setChecked(!data("razaoSocial").toString().isEmpty());
  int idCadastro = data("idCadastro").toInt();
  QString str = "SELECT idCadastro, nome, razaoSocial FROM Cadastro WHERE idCadastroRel = '" +
                QString::number(idCadastro) + "';";
  QSqlQuery query(str);
  while (query.next()) {
    QString line =
      query.value(0).toString() + " - " + query.value(1).toString() + " - " + query.value(2).toString();
    ui->textEditClientes->insertPlainText(line);
  }
//  QString tipo = data("tipo").toString();
//  if (tipo == "CLIENTE") {
//    ui->radioButtonCliente->setChecked(true);
//  } else if (tipo == "FORNECEDOR") {
//    ui->radioButtonForn->setChecked(true);
//  } else {
//    ui->radioButtonAmbos->setChecked(true);
//  }
  show();
  adjustSize();
  return true;
}

void CadastroCliente::on_pushButtonCadastrar_clicked() {
  if(save()) {
    accept();
  } else {
    qDebug() << "Erro :(";
  }
}

void CadastroCliente::on_pushButtonAtualizar_clicked() {
  if(save()) {
    accept();
  }
}

void CadastroCliente::enableEditor() {
  ui->frame->setEnabled(true);
  ui->frame_2->setEnabled(true);
}

void CadastroCliente::disableEditor() {
  ui->frame->setEnabled(false);
  ui->frame_2->setEnabled(false);
}

void CadastroCliente::show() {
//  if (tipoPFPJ == "PF") {
////    ui->groupBoxPJuridica->hide();
//    QWidget::show();
//    adjustSize();
//    return;
//  }
  if (tipoPFPJ == "PJ") {
    ui->radioButtonPJ->setChecked(true);
  } else {
    tipoPFPJ == "PF";
  }
//  escolherTipo = new CadastrarCliente(this);
//  escolherTipo->show();
  adjustSize();
  QWidget::show();
}

void CadastroCliente::close() {
  qDebug() << "close!";
  tipoPFPJ = QString();
  QDialog::close();
}

void CadastroCliente::accept() {
  qDebug() << "accept";
  tipoPFPJ = QString();
  QDialog::accept();
}

void CadastroCliente::reject() {
  qDebug() << "reject";
  tipoPFPJ = QString();
  QDialog::reject();
}

void CadastroCliente::on_pushButtonCancelar_clicked() {
  close();
}

void CadastroCliente::on_pushButtonRemover_clicked() {
  remove();
}

void CadastroCliente::on_pushButtonNovoCad_clicked() {
  newRegister();
}

void CadastroCliente::on_groupBoxPJuridica_toggled(bool arg1) {
  ui->widgetPJ->setEnabled(arg1);
}

void CadastroCliente::on_pushButtonBuscar_clicked() {
  SearchDialog *sdCliente = SearchDialog::cliente(this);
  sdCliente->setFilter("clienteFornecedor = '" + tipoClienteFornecedor + "'");
  sdCliente->show();
  connect(sdCliente, &SearchDialog::itemSelected, this, &CadastroCliente::changeItem);
}

void CadastroCliente::changeItem(QVariant value, QString text) {
  Q_UNUSED(text)
  viewRegisterById(value);
}

void CadastroCliente::on_lineEditCPF_textEdited(const QString &) {
  QString text = ui->lineEditCPF->text().remove(".").remove("-");
  validaCPF(text);
}

void CadastroCliente::on_lineEditCNPJ_textEdited(const QString &) {
  QString text = ui->lineEditCNPJ->text().remove(".").remove("/").remove("-");
  validaCNPJ(text);
}

void CadastroCliente::validaCNPJ(QString text) {
  if(text.size() == 14) {

    int digito1;
    int digito2;

    QString sub = text.left(12);

    QVector<int> sub2;
    for(int i = 0; i < sub.size(); ++i) {
      sub2.push_back(sub.at(i).digitValue());
    }

    QVector<int> multiplicadores = {5, 4, 3, 2, 9, 8, 7, 6, 5, 4, 3, 2};

    int soma = 0;
    for(int i = 0; i < 12; ++i) {
      soma += sub2.at(i) * multiplicadores.at(i);
    }

    int resto = soma % 11;

    if(resto < 2) {
      digito1 = 0;
    } else {
      digito1 = 11 - resto;
    }

    sub2.push_back(digito1);

    QVector<int> multiplicadores2 = {6, 5, 4, 3, 2, 9, 8, 7, 6, 5, 4, 3, 2};
    soma = 0;

    for(int i = 0; i < 13; ++i) {
      soma += sub2.at(i) * multiplicadores2.at(i);
    }

    resto = soma % 11;

    if(resto < 2) {
      digito2 = 0;
    } else {
      digito2 = 11 - resto;
    }

    if(digito1 == text.at(12).digitValue() and digito2 == text.at(13).digitValue()) {
      qDebug() << "Válido!";
    } else {
      QMessageBox::warning(this, "Aviso!", "CNPJ inválido!");
      return;
    }
  }
}

void CadastroCliente::validaCPF(QString text) {
  if(text.size() == 11) {
    if(text == "00000000000" or
        text == "11111111111" or
        text == "22222222222" or
        text == "33333333333" or
        text == "44444444444" or
        text == "55555555555" or
        text == "66666666666" or
        text == "77777777777" or
        text == "88888888888" or
        text == "99999999999") {
      QMessageBox::warning(this, "Aviso!", "CPF inválido!");
      return;
    }

    int digito1;
    int digito2;

    QString sub = text.left(9);
    //    qDebug() << "sub: " << sub;

    QVector<int> sub2;
    for(int i = 0; i < sub.size(); ++i) {
      sub2.push_back(sub.at(i).digitValue());
    }
    //    qDebug() << "sub2: " << sub2;

    QVector<int> multiplicadores = {10, 9, 8, 7, 6, 5, 4, 3, 2};

    int soma = 0;
    for(int i = 0; i < 9; ++i) {
      soma += sub2.at(i) * multiplicadores.at(i);
      //      qDebug() << "sub[i]: " << sub2.at(i);
    }
    //    qDebug() << "soma: " << soma;

    int resto = soma % 11;

    if(resto < 2) {
      digito1 = 0;
    } else {
      digito1 = 11 - resto;
    }
    //    qDebug() << "digito1: " << digito1;
    sub2.push_back(digito1);

    QVector<int> multiplicadores2 = {11, 10, 9, 8, 7, 6, 5, 4, 3, 2};
    soma = 0;

    for(int i = 0; i < 10; ++i) {
      soma += sub2.at(i) * multiplicadores2.at(i);
      //      qDebug() << "sub2[i]: " << sub2.at(i);
    }

    resto = soma % 11;

    if(resto < 2) {
      digito2 = 0;
    } else {
      digito2 = 11 - resto;
    }
    //    qDebug() << "digito2: " << digito2;
    //    qDebug() << "at(9): " << teste.at(9);
    //    qDebug() << "at(10): " << teste.at(10);

    if(digito1 == text.at(9).digitValue() and digito2 == text.at(10).digitValue()) {
      //      qDebug() << "Válido!";
    } else {
      QMessageBox::warning(this, "Aviso!", "CPF inválido!");
      return;
    }
  }
}

bool CadastroCliente::atualizarEnd() {
  if(!RegisterDialog::verifyFields({ui->lineEditDescricao, ui->lineEditCEP, ui->lineEditEndereco, ui->lineEditNro, ui->lineEditBairro, ui->lineEditCidade, ui->lineEditUF}))
    return false;
  if (!ui->lineEditCEP->isValid()) {
    ui->lineEditCEP->setFocus();
    QMessageBox::warning(this, "Atenção!", "CEP inválido!", QMessageBox::Ok, QMessageBox::NoButton);
    return false;
  }
  int row = mapperEnd.currentIndex();
  if( row == -1) {
    row = modelEnd.rowCount();
    if(!modelEnd.insertRow(row)) {
      QMessageBox::warning(this, "Atenção!", "Não foi possível cadastrar este endereço.", QMessageBox::Ok,
                           QMessageBox::NoButton);
      return false;
    }
  }
  if(!ui->lineEditDescricao->text().isEmpty())
    modelEnd.setData(modelEnd.index(row,modelEnd.fieldIndex("descricao")), ui->lineEditDescricao->text());
  if(!ui->lineEditCEP->text().isEmpty())
    modelEnd.setData(modelEnd.index(row,modelEnd.fieldIndex("CEP")), ui->lineEditCEP->text());
  if(!ui->lineEditEndereco->text().isEmpty())
    modelEnd.setData(modelEnd.index(row,modelEnd.fieldIndex("logradouro")), ui->lineEditEndereco->text());
  if(!ui->lineEditNro->text().isEmpty())
    modelEnd.setData(modelEnd.index(row,modelEnd.fieldIndex("numero")), ui->lineEditNro->text());
  if(!ui->lineEditComp->text().isEmpty())
    modelEnd.setData(modelEnd.index(row,modelEnd.fieldIndex("complemento")), ui->lineEditComp->text());
  if(!ui->lineEditBairro->text().isEmpty())
    modelEnd.setData(modelEnd.index(row,modelEnd.fieldIndex("bairro")), ui->lineEditBairro->text());
  if(!ui->lineEditCidade->text().isEmpty())
    modelEnd.setData(modelEnd.index(row,modelEnd.fieldIndex("cidade")), ui->lineEditCidade->text());
  if(!ui->lineEditUF->text().isEmpty())
    modelEnd.setData(modelEnd.index(row,modelEnd.fieldIndex("uf")), ui->lineEditUF->text());
  if(!modelEnd.data(modelEnd.index(row,modelEnd.fieldIndex("valid"))).isValid()) {
    modelEnd.setData(modelEnd.index(row,modelEnd.fieldIndex("valid")), true);
  }
  return true;
}

void CadastroCliente::on_pushButtonAdicionarEnd_clicked() {
  atualizarEnd();
}

void CadastroCliente::on_pushButtonAtualizarEnd_clicked() {
  atualizarEnd();
}

void CadastroCliente::on_pushButtonMostrarInativos_clicked(bool checked) {
  Q_UNUSED(checked)
  //TODO Fazer filtro para endereços desativados
}

void CadastroCliente::on_lineEditCEP_textChanged(const QString &cep) {
  if (!ui->lineEditCEP->isValid()) {
    return;
  }
  CepCompleter cc;
  if(cc.buscaCEP(cep)) {
    ui->lineEditUF->setText(cc.getUf());
    ui->lineEditCidade->setText(cc.getCidade());
    ui->lineEditEndereco->setText(cc.getEndereco());
    ui->lineEditBairro->setText(cc.getBairro());
  } else {
    QMessageBox::warning(this, "Aviso!", "CEP não encontrado!", QMessageBox::Ok);
  }
}

void CadastroCliente::clearEnd() {
  ui->lineEditBairro->clear();
  ui->lineEditCEP->clear();
  ui->lineEditCidade->clear();
  ui->lineEditComp->clear();
  ui->lineEditDescricao->clear();
  ui->lineEditEndereco->clear();
  ui->lineEditNro->clear();
  ui->lineEditUF->clear();
}

void CadastroCliente::novoEnd() {
  ui->pushButtonAtualizarEnd->hide();
  ui->pushButtonAdicionarEnd->show();
  ui->tableView->clearSelection();
  mapper.setCurrentIndex(-1);
  clearEnd();
}

void CadastroCliente::on_pushButtonNovoEnd_clicked() {
  novoEnd();
}

void CadastroCliente::on_tableView_clicked(const QModelIndex &index) {
  if(modelEnd.isDirty()) {
    QMessageBox msgBox(QMessageBox::Warning, "Atenção!", "Deseja aplicar as alterações?",
                       QMessageBox::Yes | QMessageBox::No);
    msgBox.setButtonText(QMessageBox::Yes, "Sim");
    msgBox.setButtonText(QMessageBox::No, "Não");
    if (msgBox.exec() == QMessageBox::Yes) {
      if(!atualizarEnd()) {
        return;
      }
    }
  }
  ui->pushButtonAtualizarEnd->show();
  ui->pushButtonAdicionarEnd->hide();
  mapperEnd.setCurrentModelIndex(index);
}

void CadastroCliente::on_radioButtonPF_toggled(bool checked) {
  ui->groupBoxPJuridica->setDisabled(checked);
  if(checked) {
    tipoPFPJ = "PF";
  } else {
    tipoPFPJ = "PJ";
  }
  adjustSize();
}
