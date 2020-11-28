#include "nfedistribuicao.h"
#include "ui_nfedistribuicao.h"

#include "application.h"
#include "nfeproxymodel.h"
#include "reaisdelegate.h"
#include "usersession.h"
#include "xml_viewer.h"

#include <QFile>
#include <QInputDialog>
#include <QSqlError>
#include <QTimer>

NFeDistribuicao::NFeDistribuicao(QWidget *parent) : QWidget(parent), ui(new Ui::NFeDistribuicao) {
  ui->setupUi(this);

  if (UserSession::getSetting("User/monitorarNFe").toBool()) {
    updateTables();

    connect(&timer, &QTimer::timeout, this, &NFeDistribuicao::downloadAutomatico);
    timer.start(QDateTime::currentDateTime().msecsTo(QDateTime::currentDateTime().addSecs(tempoTimer)));
  }
}

NFeDistribuicao::~NFeDistribuicao() { delete ui; }

void NFeDistribuicao::downloadAutomatico() {
  qApp->setSilent(true);

  try {
    on_pushButtonPesquisar_clicked();
  } catch (std::exception &e) {
    qDebug() << "what: " << e.what();
    qApp->setSilent(false);
    qApp->rollbackTransaction();
    return;
  }

  qApp->setSilent(false);

  //-----------------------------------------------------------------

  if (UserSession::getSetting("User/monitorarNFe").toBool()) {
    qDebug() << "timer restart";
    timer.start(QDateTime::currentDateTime().msecsTo(QDateTime::currentDateTime().addSecs(tempoTimer)));
  }
}

void NFeDistribuicao::resetTables() { modelIsSet = false; }

void NFeDistribuicao::updateTables() {
  if (not isSet) {
    setConnections();
    ui->itemBoxLoja->setSearchDialog(SearchDialog::loja(this));
    ui->lineEditCNPJ->setVisible(false);

    if (UserSession::getSetting("User/monitorarNFe").toBool()) { ui->itemBoxLoja->setId(UserSession::getSetting("User/lojaACBr")); }

    isSet = true;
  }

  if (not modelIsSet) {
    setupTables();
    modelIsSet = true;
  }

  buscarNSU();

  model.select();
}

void NFeDistribuicao::buscarNSU() {
  const QString idLoja = ui->itemBoxLoja->getId().toString();

  if (idLoja.isEmpty()) { return; }

  SqlQuery queryLoja;

  if (not queryLoja.exec("SELECT cnpj, ultimoNSU, maximoNSU FROM loja WHERE idLoja = " + idLoja) or not queryLoja.first()) {
    throw RuntimeException("Erro buscando CNPJ da loja: " + queryLoja.lastError().text(), this);
  }

  ui->spinBoxMaxNSU->setValue(queryLoja.value("maximoNSU").toInt());
  ui->spinBoxUltNSU->setValue(queryLoja.value("ultimoNSU").toInt());
  ui->lineEditCNPJ->setText(queryLoja.value("cnpj").toString().remove(".").remove("/").remove("-"));
}

void NFeDistribuicao::setConnections() {
  const auto connectionType = static_cast<Qt::ConnectionType>(Qt::AutoConnection | Qt::UniqueConnection);

  connect(ui->checkBoxCiencia, &QCheckBox::toggled, this, &NFeDistribuicao::montaFiltro, connectionType);
  connect(ui->checkBoxConfirmacao, &QCheckBox::toggled, this, &NFeDistribuicao::montaFiltro, connectionType);
  connect(ui->checkBoxDesconhecido, &QCheckBox::toggled, this, &NFeDistribuicao::montaFiltro, connectionType);
  connect(ui->checkBoxDesconhecimento, &QCheckBox::toggled, this, &NFeDistribuicao::montaFiltro, connectionType);
  connect(ui->checkBoxNaoRealizada, &QCheckBox::toggled, this, &NFeDistribuicao::montaFiltro, connectionType);
  connect(ui->groupBoxFiltros, &QGroupBox::toggled, this, &NFeDistribuicao::on_groupBoxFiltros_toggled, connectionType);
  connect(ui->itemBoxLoja, &ItemBox::textChanged, this, &NFeDistribuicao::montaFiltro, connectionType);
  connect(ui->itemBoxLoja, &ItemBox::textChanged, this, &NFeDistribuicao::buscarNSU, connectionType);
  connect(ui->pushButtonCiencia, &QPushButton::clicked, this, &NFeDistribuicao::on_pushButtonCiencia_clicked, connectionType);
  connect(ui->pushButtonConfirmacao, &QPushButton::clicked, this, &NFeDistribuicao::on_pushButtonConfirmacao_clicked, connectionType);
  connect(ui->pushButtonDesconhecimento, &QPushButton::clicked, this, &NFeDistribuicao::on_pushButtonDesconhecimento_clicked, connectionType);
  connect(ui->pushButtonNaoRealizada, &QPushButton::clicked, this, &NFeDistribuicao::on_pushButtonNaoRealizada_clicked, connectionType);
  connect(ui->pushButtonPesquisar, &QPushButton::clicked, this, &NFeDistribuicao::on_pushButtonPesquisar_clicked, connectionType);
  connect(ui->table, &TableView::activated, this, &NFeDistribuicao::on_table_activated, connectionType);
}

void NFeDistribuicao::unsetConnections() {
  disconnect(ui->checkBoxCiencia, &QCheckBox::toggled, this, &NFeDistribuicao::montaFiltro);
  disconnect(ui->checkBoxConfirmacao, &QCheckBox::toggled, this, &NFeDistribuicao::montaFiltro);
  disconnect(ui->checkBoxDesconhecido, &QCheckBox::toggled, this, &NFeDistribuicao::montaFiltro);
  disconnect(ui->checkBoxDesconhecimento, &QCheckBox::toggled, this, &NFeDistribuicao::montaFiltro);
  disconnect(ui->checkBoxNaoRealizada, &QCheckBox::toggled, this, &NFeDistribuicao::montaFiltro);
  disconnect(ui->groupBoxFiltros, &QGroupBox::toggled, this, &NFeDistribuicao::on_groupBoxFiltros_toggled);
  disconnect(ui->itemBoxLoja, &ItemBox::textChanged, this, &NFeDistribuicao::buscarNSU);
  disconnect(ui->itemBoxLoja, &ItemBox::textChanged, this, &NFeDistribuicao::montaFiltro);
  disconnect(ui->pushButtonCiencia, &QPushButton::clicked, this, &NFeDistribuicao::on_pushButtonCiencia_clicked);
  disconnect(ui->pushButtonConfirmacao, &QPushButton::clicked, this, &NFeDistribuicao::on_pushButtonConfirmacao_clicked);
  disconnect(ui->pushButtonDesconhecimento, &QPushButton::clicked, this, &NFeDistribuicao::on_pushButtonDesconhecimento_clicked);
  disconnect(ui->pushButtonNaoRealizada, &QPushButton::clicked, this, &NFeDistribuicao::on_pushButtonNaoRealizada_clicked);
  disconnect(ui->pushButtonPesquisar, &QPushButton::clicked, this, &NFeDistribuicao::on_pushButtonPesquisar_clicked);
  disconnect(ui->table, &TableView::activated, this, &NFeDistribuicao::on_table_activated);
}

void NFeDistribuicao::setupTables() {
  model.setTable("nfe");

  model.setFilter("nsu IS NOT NULL");

  model.setSort("nsu", Qt::DescendingOrder);

  model.setHeaderData("numeroNFe", "NFe");
  model.setHeaderData("tipo", "Tipo");
  model.setHeaderData("status", "Status");
  model.setHeaderData("emitente", "Emitente");
  model.setHeaderData("cnpjDest", "CNPJ Dest.");
  model.setHeaderData("cnpjOrig", "CNPJ Orig.");
  model.setHeaderData("chaveAcesso", "Chave Acesso");
  model.setHeaderData("valor", "R$");
  model.setHeaderData("nsu", "NSU");
  model.setHeaderData("statusDistribuicao", "Distribuição");
  model.setHeaderData("dataDistribuicao", "Data Evento");

  model.proxyModel = new NFeProxyModel(&model, this);

  ui->table->setModel(&model);

  ui->table->hideColumn("idNFe");
  ui->table->hideColumn("idVenda");
  ui->table->hideColumn("xml");
  ui->table->hideColumn("obs");
  ui->table->hideColumn("transportadora");
  ui->table->hideColumn("gare");
  ui->table->hideColumn("gareData");
  ui->table->hideColumn("infCpl");
  ui->table->hideColumn("utilizada");
  ui->table->hideColumn("ciencia");
  ui->table->hideColumn("confirmar");
  ui->table->hideColumn("desconhecer");
  ui->table->hideColumn("naoRealizar");

  ui->table->setItemDelegateForColumn("valor", new ReaisDelegate(2, true, this));
}

void NFeDistribuicao::on_pushButtonPesquisar_clicked() {
  // 1. chamar funcao DistribuicaoDFePorUltNSU no ACBr
  // 2. guardar resumo e eventos das NFes retornadas
  // 3. enquanto maxNSU != ultNSU repetir os passos
  // 4. fazer evento de ciencia para cada nota
  // 5. repetir passo 1 para pegar os xmls das notas

  const QString idLoja = UserSession::getSetting("User/lojaACBr").toString();

  if (idLoja == "0") { throw RuntimeError("Não está configurado qual loja usar no ACBr! Escolher em Configurações!", this); }

  //----------------------------------------------------------

  //  QFile file("LOG_DFe.txt");

  //  if (not file.open(QFile::ReadOnly)) { return; }

  //  const QString resposta = file.readAll();

  //  file.close();

  //  qDebug() << "size: " << resposta.length();

  //  if (resposta.contains("ERRO: ")) { throw RuntimeException(resposta, this); }

  //----------------------------------------------------------

  qDebug() << "pesquisar nsu: " << ui->spinBoxUltNSU->value();
  const QString resposta = acbrRemoto.enviarComando("NFe.DistribuicaoDFePorUltNSU(\"35\", \"" + ui->lineEditCNPJ->text() + "\", " + QString::number(ui->spinBoxUltNSU->value()) + ")");

  if (resposta.contains("ERRO: ")) { throw RuntimeException(resposta, this); }

  //----------------------------------------------------------

  qApp->startTransaction("NFeDistribuicao::on_pushButtonPesquisar");

  pesquisarNFes(resposta, idLoja);

  qApp->endTransaction();

  //----------------------------------------------------------

  model.select();

  if (ui->spinBoxUltNSU->value() < ui->spinBoxMaxNSU->value()) {
    qDebug() << "pesquisar novamente";
    return on_pushButtonPesquisar_clicked();
  }

  qDebug() << "darCiencia";
  darCiencia();
  qDebug() << "confirmar";
  confirmar();
  qDebug() << "desconhecer";
  desconhecer();
  qDebug() << "naoRealizar";
  naoRealizar();

  qDebug() << "maintenance";
  SqlQuery queryMaintenance;

  if (not queryMaintenance.exec("UPDATE maintenance SET lastDistribuicao = NOW()")) { throw RuntimeException("Erro guardando lastDistribuicao:" + queryMaintenance.lastError().text(), this); }

  qApp->enqueueInformation("Operação realizada com sucesso!", this);
  qDebug() << "end pesquisa";
}

void NFeDistribuicao::confirmar() {
  auto match = model.multiMatch({{"confirmar", true}});

  while (match.size() > 0) { // select up to 20 rows
    qDebug() << "confirmar_size: " << match.size();
    const auto selection = match.mid(0, 20);

    if (not enviarEvento("CONFIRMAÇÃO", selection)) { return; }

    match = model.multiMatch({{"confirmar", true}});
  }
}

void NFeDistribuicao::desconhecer() {
  auto match = model.multiMatch({{"desconhecer", true}});

  while (match.size() > 0) { // select up to 20 rows
    qDebug() << "desconhecer_size: " << match.size();
    const auto selection = match.mid(0, 20);

    if (not enviarEvento("DESCONHECIMENTO", selection)) { return; }

    match = model.multiMatch({{"desconhecer", true}});
  }
}

void NFeDistribuicao::naoRealizar() {
  auto match = model.multiMatch({{"naoRealizar", true}});

  while (match.size() > 0) { // select up to 20 rows
    qDebug() << "naoRealizar_size: " << match.size();
    const auto selection = match.mid(0, 20);

    if (not enviarEvento("NÃO REALIZADA", selection)) { return; }

    match = model.multiMatch({{"naoRealizar", true}});
  }
}

void NFeDistribuicao::darCiencia() {
  auto match = model.multiMatch({{"ciencia", true}});

  while (match.size() > 0) { // select up to 20 rows
    qDebug() << "ciencia_size: " << match.size();
    const auto selection = match.mid(0, 20);

    if (not enviarEvento("CIÊNCIA", selection)) { return; }

    match = model.multiMatch({{"ciencia", true}});
  }
}

void NFeDistribuicao::pesquisarNFes(const QString &resposta, const QString &idLoja) {
  const QStringList eventos = resposta.split("\r\n\r\n", Qt::SkipEmptyParts);
  const QString cnpjDest = ui->lineEditCNPJ->text();

  for (const auto &evento : eventos) {
    if (evento.contains("[DistribuicaoDFe]")) {
      const int indexMaxNSU = evento.indexOf("\r\nmaxNSU=");
      const int indexUltNSU = evento.indexOf("\r\nultNSU=");

      if (indexMaxNSU == -1 or indexUltNSU == -1) { throw RuntimeException("Não encontrou o campo 'maxNSU/ultNSU': " + evento); }

      const QString maxNSU = evento.mid(indexMaxNSU + 9).split("\r\n").first();
      const QString ultNSU = evento.mid(indexUltNSU + 9).split("\r\n").first();

      ui->spinBoxMaxNSU->setValue(maxNSU.toInt());
      ui->spinBoxUltNSU->setValue(ultNSU.toInt());

      SqlQuery queryLoja2;

      if (not queryLoja2.exec("UPDATE loja SET ultimoNSU = " + ultNSU + ", maximoNSU = " + maxNSU + " WHERE idLoja = " + idLoja)) {
        throw RuntimeException("Erro guardando NSU: " + queryLoja2.lastError().text());
      }
    }

    if (evento.contains("[ResNFe")) {
      const int indexChave = evento.indexOf("\r\nchNFe=");

      if (indexChave == -1) { throw RuntimeException("Não encontrou o campo 'chNFe': " + evento); }

      const QString chaveAcesso = evento.mid(indexChave + 8).split("\r\n").first();
      const QString numeroNFe = chaveAcesso.mid(25, 9);

      //----------------------------------------------------------

      const int indexCnpj = evento.indexOf("\r\nCNPJCPF=");

      if (indexCnpj == -1) { throw RuntimeException("Não encontrou o campo 'CNPJCPF': " + evento); }

      const QString cnpjOrig = evento.mid(indexCnpj + 10).split("\r\n").first();

      //----------------------------------------------------------

      const int indexNome = evento.indexOf("\r\nxNome=");

      if (indexNome == -1) { throw RuntimeException("Não encontrou o campo 'xNome': " + evento); }

      const QString nomeEmitente = evento.mid(indexNome + 8).split("\r\n").first();

      //----------------------------------------------------------

      const int indexValor = evento.indexOf("\r\nvNF=");

      if (indexValor == -1) { throw RuntimeException("Não encontrou o campo 'vNF': " + evento); }

      const QString valor = evento.mid(indexValor + 6).split("\r\n").first().replace(",", ".");

      //----------------------------------------------------------

      const int indexNsu = evento.indexOf("\r\nNSU=");

      if (indexNsu == -1) { throw RuntimeException("Não encontrou o campo 'NSU': " + evento); }

      const QString nsu = evento.mid(indexNsu + 6).split("\r\n").first();

      //----------------------------------------------------------

      const int indexXML = evento.indexOf("\r\nXML=");
      const int indexArquivo = evento.indexOf("\r\narquivo=");

      if (indexXML == -1 or indexArquivo == -1) { throw RuntimeException("Não encontrou o campo 'XML': " + evento); }

      const QString xml = evento.mid(indexXML + 6, indexArquivo - indexXML - 6);

      //----------------------------------------------------------

      const int indexSchema = evento.indexOf("\r\nschema=");

      if (indexSchema == -1) { throw RuntimeException("Não encontrou o campo 'schema': " + evento); }

      const QString schemaEvento = evento.mid(indexSchema + 9).split("\r\n").first();

      //----------------------------------------------------------

      SqlQuery queryExiste;

      if (not queryExiste.exec("SELECT status FROM nfe WHERE chaveAcesso = '" + chaveAcesso + "'")) {
        throw RuntimeException("Erro verificando se NFe já cadastrada: " + queryExiste.lastError().text());
      }

      const bool existe = queryExiste.first();

      //----------------------------------------------------------

      if (not existe) {
        const QString status = (schemaEvento == "procNFe") ? "AUTORIZADO" : "RESUMO";
        const QString ciencia = (schemaEvento == "procNFe") ? "0" : "1";

        SqlQuery queryCadastrar;
        queryCadastrar.prepare("INSERT INTO nfe (numeroNFe, tipo, xml, status, emitente, cnpjDest, cnpjOrig, chaveAcesso, transportadora, valor, infCpl, nsu, statusDistribuicao, ciencia) VALUES "
                               "(:numeroNFe, 'ENTRADA', :xml, :status, :emitente, :cnpjDest, :cnpjOrig, :chaveAcesso, :transportadora, :valor, :infCpl, :nsu, 'DESCONHECIDO', :ciencia)");
        queryCadastrar.bindValue(":numeroNFe", numeroNFe);
        queryCadastrar.bindValue(":xml", xml);
        queryCadastrar.bindValue(":status", status);
        queryCadastrar.bindValue(":emitente", nomeEmitente);
        queryCadastrar.bindValue(":cnpjDest", cnpjDest);
        queryCadastrar.bindValue(":cnpjOrig", cnpjOrig);
        queryCadastrar.bindValue(":chaveAcesso", chaveAcesso);
        queryCadastrar.bindValue(":transportadora", encontraTransportadora(xml));
        queryCadastrar.bindValue(":valor", valor);
        queryCadastrar.bindValue(":infCpl", encontraInfCpl(xml));
        queryCadastrar.bindValue(":nsu", nsu);
        queryCadastrar.bindValue(":ciencia", ciencia);

        if (not queryCadastrar.exec()) { throw RuntimeException("Erro cadastrando resumo da NFe: " + queryCadastrar.lastError().text()); }
      } else if (existe and schemaEvento == "procNFe") {
        if (queryExiste.value("status").toString() == "AUTORIZADO") { continue; }

        SqlQuery queryAtualizar;
        queryAtualizar.prepare("UPDATE nfe SET status = 'AUTORIZADO', xml = :xml, transportadora = :transportadora, infCpl = :infCpl WHERE chaveAcesso = :chaveAcesso AND status = 'RESUMO'");
        queryAtualizar.bindValue(":xml", xml);
        queryAtualizar.bindValue(":transportadora", encontraTransportadora(xml));
        queryAtualizar.bindValue(":infCpl", encontraInfCpl(xml));
        queryAtualizar.bindValue(":chaveAcesso", chaveAcesso);

        if (not queryAtualizar.exec()) { throw RuntimeException("Erro atualizando xml: " + queryAtualizar.lastError().text()); }
      } else {
        throw RuntimeException("Evento não tratado: " + evento);
      }
    }

    if (evento.contains("[InfEve")) {
      const int indexTipo = evento.indexOf("\r\nxEvento=");

      if (indexTipo == -1) { throw RuntimeException("Não encontrou o campo 'xEvento': " + evento); }

      const QString eventoTipo = evento.mid(indexTipo + 10).split("\r\n").first();

      //----------------------------------------------------------

      const int indexMotivo = evento.indexOf("\r\nxMotivo=");

      if (indexMotivo == -1) { throw RuntimeException("Não encontrou o campo 'xMotivo': " + evento); }

      const QString motivo = evento.mid(indexMotivo + 10).split("\r\n").first();

      //----------------------------------------------------------

      const int indexChave = evento.indexOf("\r\nchNFe=");

      if (indexChave == -1) { throw RuntimeException("Não encontrou o campo 'chNFe': " + evento); }

      const QString chaveAcesso = evento.mid(indexChave + 8).split("\r\n").first();

      //----------------------------------------------------------

      SqlQuery queryExiste;

      if (not queryExiste.exec("SELECT statusDistribuicao FROM nfe WHERE chaveAcesso = '" + chaveAcesso + "'")) {
        throw RuntimeException("Erro verificando se NFe já cadastrada: " + queryExiste.lastError().text());
      } else if (queryExiste.first()) {
        const QString statusDistribuicao = queryExiste.value("statusDistribuicao").toString();

        if (motivo.contains("Evento registrado e vinculado a NF-e")) {
          if (eventoTipo == "Ciencia da Operacao" and statusDistribuicao == "DESCONHECIDO") {
            SqlQuery queryAtualiza;

            if (not queryAtualiza.exec("UPDATE nfe SET statusDistribuicao = 'CIÊNCIA', ciencia = FALSE, confirmar = FALSE, desconhecer = FALSE, naoRealizar = FALSE WHERE chaveAcesso = '" +
                                       chaveAcesso + "'")) {
              throw RuntimeException("Erro atualizando statusDistribuicao: " + queryAtualiza.lastError().text());
            }
          } else if (eventoTipo == "Confirmacao da Operacao" and statusDistribuicao != "CONFIRMAÇÃO") {
            SqlQuery queryAtualiza;

            if (not queryAtualiza.exec("UPDATE nfe SET statusDistribuicao = 'CONFIRMAÇÃO', ciencia = FALSE, confirmar = FALSE, desconhecer = FALSE, naoRealizar = FALSE WHERE chaveAcesso = '" +
                                       chaveAcesso + "'")) {
              throw RuntimeException("Erro atualizando statusDistribuicao: " + queryAtualiza.lastError().text());
            }
          } else if (eventoTipo == "CANCELAMENTO" and statusDistribuicao != "CANCELADA") {
            SqlQuery queryAtualiza;

            if (not queryAtualiza.exec(
                    "UPDATE nfe SET status = 'CANCELADA', statusDistribuicao = 'CANCELADA', ciencia = FALSE, confirmar = FALSE, desconhecer = FALSE, naoRealizar = FALSE WHERE chaveAcesso = '" +
                    chaveAcesso + "'")) {
              throw RuntimeException("Erro atualizando statusDistribuicao: " + queryAtualiza.lastError().text());
            }
          } else {
            throw RuntimeException("Evento não tratado: " + motivo);
          }
        }

        // TODO: implement
        //        if (eventoTipo == "DESCONHECIMENTO" and statusDistribuicao == "DESCONHECIDO") {}
        //        if (eventoTipo == "NÃO REALIZADA" and statusDistribuicao == "DESCONHECIDO") {}
      } else {
        throw RuntimeError("Evento não tratado: " + evento);
      }
    }
  }
}

void NFeDistribuicao::on_pushButtonCiencia_clicked() {
  const auto selection = ui->table->selectionModel()->selectedRows();

  if (selection.isEmpty()) { throw RuntimeError("Nenhuma linha selecionada!", this); }

  for (auto &index : selection) {
    const QString statusDistribuicao = model.data(index.row(), "statusDistribuicao").toString();

    if (statusDistribuicao == "CANCELADA") { continue; }

    model.setData(index.row(), "ciencia", true);
  }

  const auto monitorar = UserSession::getSetting("User/monitorarNFe").toBool();

  monitorar ? darCiencia() : agendarOperacao();
}

void NFeDistribuicao::on_pushButtonConfirmacao_clicked() {
  const auto selection = ui->table->selectionModel()->selectedRows();

  if (selection.isEmpty()) { throw RuntimeError("Nenhuma linha selecionada!", this); }

  for (auto &index : selection) {
    const QString statusDistribuicao = model.data(index.row(), "statusDistribuicao").toString();

    if (statusDistribuicao == "CANCELADA") { continue; }

    model.setData(index.row(), "confirmar", true);
  }

  const auto monitorar = UserSession::getSetting("User/monitorarNFe").toBool();

  monitorar ? confirmar() : agendarOperacao();
}

void NFeDistribuicao::on_pushButtonDesconhecimento_clicked() {
  const auto selection = ui->table->selectionModel()->selectedRows();

  if (selection.isEmpty()) { throw RuntimeError("Nenhuma linha selecionada!", this); }

  for (auto &index : selection) {
    const QString statusDistribuicao = model.data(index.row(), "statusDistribuicao").toString();

    if (statusDistribuicao == "CANCELADA") { continue; }

    model.setData(index.row(), "desconhecer", true);
  }

  const auto monitorar = UserSession::getSetting("User/monitorarNFe").toBool();

  monitorar ? desconhecer() : agendarOperacao();
}

void NFeDistribuicao::on_pushButtonNaoRealizada_clicked() {
  const auto selection = ui->table->selectionModel()->selectedRows();

  if (selection.isEmpty()) { throw RuntimeError("Nenhuma linha selecionada!", this); }

  for (auto &index : selection) {
    const QString statusDistribuicao = model.data(index.row(), "statusDistribuicao").toString();

    if (statusDistribuicao == "CANCELADA") { continue; }

    model.setData(index.row(), "naoRealizar", true);
  }

  const auto monitorar = UserSession::getSetting("User/monitorarNFe").toBool();

  monitorar ? naoRealizar() : agendarOperacao();
}

void NFeDistribuicao::agendarOperacao() {
  qApp->startTransaction("NFeDistribuicao::agendarOperacao");

  model.submitAll();

  qApp->endTransaction();

  qApp->enqueueInformation("Operação agendada com sucesso!", this);
}

bool NFeDistribuicao::enviarEvento(const QString &operacao, const QVector<int> &selection) {
  QString column;

  if (operacao == "CIÊNCIA") { column = "ciencia"; }
  if (operacao == "CONFIRMAÇÃO") { column = "confirmar"; }
  if (operacao == "DESCONHECIMENTO") { column = "desconhecer"; }
  if (operacao == "NÃO REALIZADA") { column = "naoRealizar"; }

  if (column.isEmpty()) { return false; }

  QString codigoEvento;

  if (operacao == "CIÊNCIA") { codigoEvento = "210210"; }
  if (operacao == "CONFIRMAÇÃO") { codigoEvento = "210200"; }
  if (operacao == "DESCONHECIMENTO") { codigoEvento = "210220"; }
  if (operacao == "NÃO REALIZADA") { codigoEvento = "210240"; }

  if (codigoEvento.isEmpty()) { return false; }

  //----------------------------------------------------------

  const QString cnpj = ui->lineEditCNPJ->text();
  const QString dataHora = qApp->serverDateTime().toString("dd/MM/yyyy HH:mm");

  QString justificativa;

  if (operacao == "NÃO REALIZADA") {
    justificativa = QInputDialog::getText(this, "Justificativa", "Entre 15 e 255 caracteres: ");
    if (justificativa.size() < 15 or justificativa.size() > 255) { throw RuntimeError("Justificativa fora do tamanho!", this); }
  }

  QString comando;
  comando += "NFE.EnviarEvento(\"[EVENTO]\r\n";
  comando += "idLote = 1\r\n";

  int count = 0;

  for (const auto row : selection) {
    const QString statusDistribuicao = model.data(row, "statusDistribuicao").toString();

    if (statusDistribuicao == "CANCELADA") { continue; }

    if (operacao == "CIÊNCIA" and statusDistribuicao != "DESCONHECIDO") { continue; }

    const QString chaveAcesso = model.data(row, "chaveAcesso").toString();
    const QString numEvento = QString("%1").arg(++count, 3, 10, QChar('0')); // padding with zeros

    comando += "[EVENTO" + numEvento + "]\r\n";
    comando += "cOrgao = 91\r\n";
    comando += "CNPJ = " + cnpj + "\r\n";
    comando += "chNFe = " + chaveAcesso + "\r\n";
    comando += "dhEvento = " + dataHora + "\r\n";
    comando += "tpEvento = " + codigoEvento + "\r\n";
    comando += "nSeqEvento = 1\r\n";
    comando += "versaoEvento = 1.00\r\n";
    if (operacao == "NÃO REALIZADA") { comando += "xJust = " + justificativa + "\r\n"; }
  }

  comando += "\")";

  //----------------------------------------------------------

  const QString resposta = acbrRemoto.enviarComando(comando);

  if (resposta.contains("ERRO: ")) { throw RuntimeException(resposta, this); }

  //----------------------------------------------------------

  const QStringList eventos = resposta.split("\r\n\r\n", Qt::SkipEmptyParts);

  qApp->startTransaction("NFeDistribuicao::enviarEvento_" + operacao);

  for (const auto &evento : eventos) {
    const int indexMotivo = evento.indexOf("\r\nXMotivo=");

    if (indexMotivo == -1) { throw RuntimeException("Não encontrou o campo 'xMotivo': " + evento); }

    const QString motivo = evento.mid(indexMotivo + 10).split("\r\n").first();

    if (motivo == "Lote de evento processado") { continue; }

    //----------------------------------------------------------

    const int indexChave = evento.indexOf("\r\nchNFe=");

    if (indexChave == -1) { throw RuntimeException("Não encontrou o campo 'chNFe': " + evento); }

    const QString chaveAcesso = evento.mid(indexChave + 8).split("\r\n").first();

    //----------------------------------------------------------

    if (motivo.contains("Evento de Ciencia da Operacao informado apos a manifestacao final do destinatario")) {
      SqlQuery query;

      if (not query.exec(
              "UPDATE nfe SET statusDistribuicao = 'FINALIZADA', dataDistribuicao = NOW(), ciencia = FALSE, confirmar = FALSE, desconhecer = FALSE, naoRealizar = FALSE WHERE chaveAcesso = '" +
              chaveAcesso + "'")) {
        throw RuntimeException("Erro atualizando status da NFe: " + query.lastError().text());
      }
    } else if (motivo.contains("Rejeicao:") and motivo.contains("para NFe cancelada ou denegada")) {
      SqlQuery query;

      if (not query.exec("UPDATE nfe SET status = 'CANCELADA', statusDistribuicao = 'CANCELADA', ciencia = FALSE, confirmar = FALSE, desconhecer = FALSE, naoRealizar = FALSE WHERE chaveAcesso = '" +
                         chaveAcesso + "'")) {
        throw RuntimeException("Erro atualizando status da NFe: " + query.lastError().text());
      }
    } else if (motivo.contains("Evento registrado e vinculado a NF-e") or motivo.contains("Rejeicao: Duplicidade de evento")) {
      SqlQuery query;

      if (not query.exec("UPDATE nfe SET statusDistribuicao = '" + operacao +
                         "', dataDistribuicao = NOW(), ciencia = FALSE, confirmar = FALSE, desconhecer = FALSE, naoRealizar = FALSE WHERE chaveAcesso = '" + chaveAcesso + "'")) {
        throw RuntimeException("Erro atualizando status da NFe: " + query.lastError().text());
      }
    } else {
      throw RuntimeException("Evento não tratado: " + motivo);
    }
  }

  model.submitAll();

  qApp->endTransaction();

  qApp->enqueueInformation("Operação realizada com sucesso!", this);

  return true;
}

void NFeDistribuicao::on_table_activated(const QModelIndex &index) {
  const QByteArray xml = model.data(index.row(), "xml").toByteArray();

  if (xml.isEmpty()) { throw RuntimeException("XML vazio!", this); }

  auto *viewer = new XML_Viewer(xml, this);
  viewer->setAttribute(Qt::WA_DeleteOnClose);
}

QString NFeDistribuicao::encontraInfCpl(const QString &xml) {
  const int indexInfCpl1 = xml.indexOf("<infCpl>");
  const int indexInfCpl2 = xml.indexOf("</infCpl>");

  if (indexInfCpl1 != -1 and indexInfCpl2 != -1) { return xml.mid(indexInfCpl1, indexInfCpl2 - indexInfCpl1).remove("<infCpl>"); }

  return "";
}

QString NFeDistribuicao::encontraTransportadora(const QString &xml) {
  const int indexTransportadora1 = xml.indexOf("<transporta>");
  const int indexTransportadora2 = xml.indexOf("</transporta>");

  if (indexTransportadora1 != -1 and indexTransportadora2 != -1) {
    const QString transportaTemp = xml.mid(indexTransportadora1, indexTransportadora2 - indexTransportadora1).remove("<transporta>");

    const int indexTransportaNome1 = transportaTemp.indexOf("<xNome>");
    const int indexTransportaNome2 = transportaTemp.indexOf("</xNome>");

    if (indexTransportaNome1 != -1 and indexTransportaNome2 != -1) { return transportaTemp.mid(indexTransportaNome1, indexTransportaNome2 - indexTransportaNome1).remove("<xNome>"); }
  }

  return "";
}

void NFeDistribuicao::montaFiltro() {
  QStringList filtros;

  filtros << "nsu IS NOT NULL";

  //-------------------------------------

  QStringList filtroCheck;

  for (const auto &child : ui->groupBoxFiltros->findChildren<QCheckBox *>()) {
    if (child->isChecked()) { filtroCheck << "'" + child->text().toUpper() + "'"; }
  }

  if (not filtroCheck.isEmpty()) { filtros << "statusDistribuicao IN (" + filtroCheck.join(", ") + ")"; }

  //-------------------------------------

  const QString lojaNome = ui->itemBoxLoja->text();

  if (not lojaNome.isEmpty()) {
    QSqlQuery queryLoja;

    if (not queryLoja.exec("SELECT cnpj FROM loja WHERE idLoja = " + ui->itemBoxLoja->getId().toString()) or not queryLoja.first()) {
      throw RuntimeException("Erro buscando CNPJ loja: " + queryLoja.lastError().text());
    }

    filtros << "cnpjDest = '" + queryLoja.value("cnpj").toString().remove(".").remove("/").remove("-") + "'";
  }

  //-------------------------------------

  model.setFilter(filtros.join(" AND "));
}

void NFeDistribuicao::on_groupBoxFiltros_toggled(const bool enabled) {
  unsetConnections();

  try {
    [&] {
      const auto children = ui->groupBoxFiltros->findChildren<QCheckBox *>();

      for (const auto &child : children) {
        child->setEnabled(true);
        child->setChecked(enabled);
      }
    }();
  } catch (std::exception &e) {}

  setConnections();

  montaFiltro();
}

// TODO: nos casos em que o usuario importar um xml já cadastrado como RESUMO utilizar o xml do usuario
