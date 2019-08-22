/***************************************************************************
 *   This file is part of the Lime Report project                          *
 *   Copyright (C) 2015 by Alexander Arin                                  *
 *   arin_a@bk.ru                                                          *
 *                                                                         *
 **                   GNU General Public License Usage                    **
 *                                                                         *
 *   This library is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *                                                                         *
 **                  GNU Lesser General Public License                    **
 *                                                                         *
 *   This library is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation, either version 3 of the    *
 *   License, or (at your option) any later version.                       *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library.                                      *
 *   If not, see <http://www.gnu.org/licenses/>.                           *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 ****************************************************************************/
#include "ui_lrdatabrowser.h"
#include <QDebug>
#include <QDockWidget>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QMainWindow>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QTableView>
#include <QVBoxLayout>

#include "lrconnectiondialog.h"
#include "lrdatabrowser.h"
#include "lrdatabrowsertree.h"
#include "lrreportengine_p.h"
#include "lrsqleditdialog.h"
#include "lrvariabledialog.h"

namespace LimeReport {

DataBrowser::DataBrowser(QWidget *parent) : QWidget(parent), ui(new Ui::DataBrowser), m_report(nullptr), m_closingWindows(false), m_settings(nullptr), m_ownedSettings(false) {
  ui->setupUi(this);
  connect(ui->addConnection, &QAbstractButton::clicked, this, &DataBrowser::slotAddConnection);
  connect(ui->deleteConection, &QAbstractButton::clicked, this, &DataBrowser::slotDeleteConnection);
  connect(ui->addDataSource, &QAbstractButton::clicked, this, &DataBrowser::slotAddDataSource);
  connect(ui->viewDataSource, &QAbstractButton::clicked, this, &DataBrowser::slotViewDatasource);
  connect(ui->editDataSource, &QAbstractButton::clicked, this, &DataBrowser::slotEditDatasource);
  connect(ui->deleteDataSource, &QAbstractButton::clicked, this, &DataBrowser::slotDeleteDatasource);
  connect(ui->changeConnection, &QAbstractButton::clicked, this, &DataBrowser::slotChangeConnection);
  connect(ui->pbConnect, &QAbstractButton::clicked, this, &DataBrowser::slotChangeConnectionState);

  ui->dataTree->setHeaderLabel(tr("Datasources"));
  ui->pbConnect->setEnabled(false);
}

DataBrowser::~DataBrowser() {
  delete ui;
  if (m_settings && m_ownedSettings) delete m_settings;
}
QSize DataBrowser::sizeHint() const { return QSize(100, 200); }

void DataBrowser::slotAddConnection() {
  ConnectionDialog *connectionEdit = new ConnectionDialog(this, nullptr, this);
  connectionEdit->setAttribute(Qt::WA_DeleteOnClose, true);
#ifdef Q_OS_MAC
  connectionEdit->setWindowModality(Qt::WindowModal);
#else
  connectionEdit->setWindowModality(Qt::ApplicationModal);
#endif
  //   connect(connectionEdit,SIGNAL(finished(int)),this,SLOT(slotConnectionEditFinished(int)));
  //   connect(connectionEdit,SIGNAL(conectionRegistred(QString)),this,SLOT(slotConnectionRegistred(QString)));
  connectionEdit->exec();
}

void DataBrowser::slotSQLEditingFinished(SQLEditResult result) {
  if (result.dialogMode == SQLEditDialog::AddMode) {
    switch (result.resultMode) {
    case SQLEditResult::Query: addQuery(result); break;
    case SQLEditResult::SubQuery: addSubQuery(result); break;
    case SQLEditResult::SubProxy: addProxy(result); [[fallthrough]];
    default: break;
    }
  } else {
    switch (result.resultMode) {
    case SQLEditResult::Query: changeQuery(result); break;
    case SQLEditResult::SubQuery: changeSubQuery(result); break;
    case SQLEditResult::SubProxy: changeProxy(result);
    }
  }

  updateDataTree();
}

void DataBrowser::slotDeleteConnection() {
  if (!getConnectionName(NameForUser).isEmpty()) {
    if (QMessageBox::critical(this, tr("Attention"), tr("Do you really want to delete \"%1\" connection?").arg(getConnectionName(NameForUser)), QMessageBox::Ok | QMessageBox::No, QMessageBox::No) ==
        QMessageBox::Ok) {
      m_report->dataManager()->removeConnection(getConnectionName(NameForReport));
      updateDataTree();
    }
  }
}

void DataBrowser::slotAddDataSource() {
  SQLEditDialog *sqlEdit = new SQLEditDialog(this, m_report->dataManager(), SQLEditDialog::AddMode);
  sqlEdit->setAttribute(Qt::WA_DeleteOnClose, true);
#ifdef Q_OS_MAC
  sqlEdit->setWindowModality(Qt::WindowModal);
#else
  sqlEdit->setWindowModality(Qt::ApplicationModal);
#endif
  sqlEdit->setSettings(settings());
  sqlEdit->setDataSources(m_report->dataManager());
  sqlEdit->setDefaultConnection(getConnectionName(NameForReport));
  connect(sqlEdit, &SQLEditDialog::signalSqlEditingFinished, this, &DataBrowser::slotSQLEditingFinished);
  sqlEdit->exec();
}

void DataBrowser::updateDataTree() {
  QString selectedText = "";
  int selectedType = 0;

  if (ui->dataTree->selectedItems().count() == 1) {
    selectedType = ui->dataTree->selectedItems().at(0)->type();
    selectedText = ui->dataTree->selectedItems().at(0)->text(0);
  }

  initConnections();

  for (QString dataSourceName : m_report->datasourcesNames()) {

    QTreeWidgetItem *item = new QTreeWidgetItem(QStringList(dataSourceName), DataBrowserTree::Table);
    QTreeWidgetItem *parentItem = findByNameAndType(ConnectionDesc::connectionNameForUser(m_report->dataManager()->connectionName(dataSourceName)), DataBrowserTree::Connection);
    if (parentItem) {
      parentItem->addChild(item);
      if (!parentItem->isExpanded()) ui->dataTree->expandItem(parentItem);
    } else {
      ui->dataTree->addTopLevelItem(item);
    }

    try {
      IDataSource *datasource = m_report->dataManager()->dataSource(dataSourceName);
      if (datasource) {
        fillFields(item, datasource);
        if (!datasource->isInvalid())
          item->setIcon(0, QIcon(":/databrowser/images/table_good"));
        else
          item->setIcon(0, QIcon(":/databrowser/images/table_error"));

      } else
        item->setIcon(0, QIcon(":/databrowser/images/table_error"));

    } catch (ReportError & /*exception*/) {
      item->setIcon(0, QIcon(":/databrowser/images/table_error"));
      // qDebug()<<exception.what();
    }
  }

  QTreeWidgetItem *selectedItem = findByNameAndType(selectedText, selectedType);
  if (selectedItem) {
    selectedItem->setSelected(true);
    ui->dataTree->setCurrentItem(selectedItem);
  }
}

void DataBrowser::updateVariablesTree() {
  ui->variablesTree->clear();
  QTreeWidgetItem *reportVariables = new QTreeWidgetItem(QStringList(tr("Report variables")), DataBrowserTree::Category);
  reportVariables->setIcon(0, QIcon(":/report/images/folder"));
  QTreeWidgetItem *systemVariables = new QTreeWidgetItem(QStringList(tr("System variables")), DataBrowserTree::Category);
  systemVariables->setIcon(0, QIcon(":/report/images/folder"));
  QTreeWidgetItem *externalVariables = new QTreeWidgetItem(QStringList(tr("External variables")), DataBrowserTree::Category);
  externalVariables->setIcon(0, QIcon(":/report/images/folder"));
  ui->variablesTree->addTopLevelItem(reportVariables);
  ui->variablesTree->addTopLevelItem(systemVariables);
  ui->variablesTree->addTopLevelItem(externalVariables);

  for (QString variableName : m_report->dataManager()->variableNames()) {
    QStringList values;
    values << variableName + ((m_report->dataManager()->variableIsSystem(variableName)) ? "" : "  [" + m_report->dataManager()->variable(variableName).toString() + "]") << variableName;
    QTreeWidgetItem *item = new QTreeWidgetItem(values, DataBrowserTree::Variable);
    item->setIcon(0, QIcon(":/databrowser/images/value"));
    if (m_report->dataManager()->variableIsSystem(variableName)) {
      systemVariables->addChild(item);
    } else {
      reportVariables->addChild(item);
    }
  }

  for (QString variableName : m_report->dataManager()->namesOfUserVariables()) {
    if (!m_report->dataManager()->variableNames().contains(variableName)) {
      QStringList values;
      values << variableName + "  [" + m_report->dataManager()->variable(variableName).toString() + "]" << variableName;
      QTreeWidgetItem *item = new QTreeWidgetItem(values, DataBrowserTree::ExternalVariable);
      item->setIcon(0, QIcon(":/databrowser/images/value"));
      externalVariables->addChild(item);
    }
  }

  ui->variablesTree->expandItem(reportVariables);
  ui->variablesTree->expandItem(systemVariables);
  ui->variablesTree->expandItem(externalVariables);
}

void DataBrowser::closeAllDataWindows() {
  m_closingWindows = true;
  QMap<QString, QDockWidget *>::iterator it = m_dataWindows.begin();
  for (; it != m_dataWindows.end();) {
    (*it)->close();
    delete (*it);
    it = m_dataWindows.erase(it);
  }
  m_closingWindows = false;
}

void DataBrowser::setSettings(QSettings *value, bool owned) {
  if (m_settings && m_ownedSettings) delete m_settings;
  m_settings = value;
  m_ownedSettings = owned;
}

QSettings *DataBrowser::settings() {
  if (m_settings) {
    return m_settings;
  } else {
    m_settings = new QSettings("LimeReport", QApplication::applicationName());
    m_ownedSettings = true;
    return m_settings;
  }
}

void DataBrowser::slotDatasourcesChanged() { updateDataTree(); }

void DataBrowser::fillFields(QTreeWidgetItem *parentItem, LimeReport::IDataSource *dataSource) {
  if (!dataSource) return;
  for (int i = 0; i < dataSource->columnCount(); i++) {
    QTreeWidgetItem *item = new QTreeWidgetItem(QStringList(dataSource->columnNameByIndex(i)), DataBrowserTree::Row);
    item->setIcon(0, QIcon(":/databrowser/images/field"));
    parentItem->addChild(item);
  }
  parentItem->sortChildren(0, Qt::AscendingOrder);
}

QTreeWidgetItem *DataBrowser::findByNameAndType(QString name, int itemType) {
  if (name.isEmpty()) return nullptr;
  QList<QTreeWidgetItem *> items = ui->dataTree->findItems(name, Qt::MatchContains | Qt::MatchRecursive);
  if (!items.isEmpty()) {
    for (int i = 0; i < items.count(); i++) {
      if ((items.at(i)->type() == itemType) /* && (items.at(0)->text(0)==name)*/) { return items.at(i); }
    }
  }
  return nullptr;
}

void DataBrowser::slotViewDatasource() {
  QString datasourceName = getDatasourceName();
  if (!datasourceName.isEmpty()) { showDataWindow(datasourceName); }
}

QString DataBrowser::getDatasourceName() {
  if (ui->dataTree->currentItem()) {
    if (ui->dataTree->currentItem()->type() == DataBrowserTree::Table) return ui->dataTree->currentItem()->text(0);
    if (ui->dataTree->currentItem()->type() == DataBrowserTree::Row) return ui->dataTree->currentItem()->parent()->text(0);
  };
  return QString();
}

QTreeWidgetItem *findConnectionItem(QTreeWidgetItem *item) {
  if (item->type() == DataBrowserTree::Connection) {
    return item;
  } else {
    if (item->parent())
      return findConnectionItem(item->parent());
    else
      return nullptr;
  }
}

QString DataBrowser::getConnectionName(NameType nameType) {
  if (ui->dataTree->currentItem()) {
    QTreeWidgetItem *ci = findConnectionItem(ui->dataTree->currentItem());
    if (ci) {
      switch (nameType) {
      case NameForUser: return ConnectionDesc::connectionNameForUser(ci->text(0));
      case NameForReport: return ConnectionDesc::connectionNameForReport(ci->text(0));
      }
    } else
      return QString();
  };
  return QString();
}

QString DataBrowser::getVariable() {
  if (ui->variablesTree->currentItem() && (ui->variablesTree->currentItem()->type() == DataBrowserTree::Variable || ui->variablesTree->currentItem()->type() == DataBrowserTree::ExternalVariable)) {
    return ui->variablesTree->currentItem()->text(1);
  }
  return QString();
}

void DataBrowser::slotEditDatasource() {
  if (!getDatasourceName().isEmpty()) {
    closeDataWindow(getDatasourceName());
    SQLEditDialog *sqlEdit = new SQLEditDialog(this, m_report->dataManager(), SQLEditDialog::EditMode);
    sqlEdit->setAttribute(Qt::WA_DeleteOnClose);
#ifdef Q_OS_MAC
    sqlEdit->setWindowModality(Qt::WindowModal);
#else
    sqlEdit->setWindowModality(Qt::ApplicationModal);
#endif
    sqlEdit->setSettings(settings());
    sqlEdit->setDataSources(m_report->dataManager(), getDatasourceName());
    connect(sqlEdit, &SQLEditDialog::signalSqlEditingFinished, this, &DataBrowser::slotSQLEditingFinished);
    sqlEdit->exec();
  }
}

void DataBrowser::slotDeleteDatasource() {
  QString datasourceName = getDatasourceName();
  QTreeWidgetItem *item = findByNameAndType(datasourceName, DataBrowserTree::Table);
  if (item) {
    if (QMessageBox::critical(this, tr("Attention"), tr("Do you really want to delete \"%1\" datasource?").arg(datasourceName), QMessageBox::Ok | QMessageBox::No, QMessageBox::No) ==
        QMessageBox::Ok) {
      removeDatasource(datasourceName);
    }
  }
}

void DataBrowser::setReportEditor(LimeReport::ReportDesignWidget *report) {
  m_report = report;
  connect(m_report, &ReportDesignWidget::cleared, this, &DataBrowser::slotClear);
  connect(m_report->dataManager(), &DataSourceManager::datasourcesChanged, this, &DataBrowser::slotDatasourcesChanged);
  updateDataTree();
  updateVariablesTree();
}

void DataBrowser::slotClear() {
  ui->dataTree->clear();
  for (QDockWidget *window : m_dataWindows.values()) window->close();
  updateDataTree();
  updateVariablesTree();
}

void DataBrowser::initConnections() {
  ui->dataTree->clear();
  QList<QTreeWidgetItem *> items;

  QStringList connections = QSqlDatabase::connectionNames();
  for (QString connectionName : m_report->dataManager()->connectionNames()) {
    if (!connections.contains(connectionName, Qt::CaseInsensitive)) { connections.append(connectionName); }
  }
  std::sort(connections.begin(), connections.end());
  for (QString connectionName : connections) {
    QTreeWidgetItem *item = new QTreeWidgetItem(ui->dataTree, QStringList(ConnectionDesc::connectionNameForUser(connectionName)), DataBrowserTree::Connection);
    if (!m_report->dataManager()->connectionNames().contains(ConnectionDesc::connectionNameForReport(connectionName), Qt::CaseInsensitive)) {
      item->setIcon(0, QIcon(":/databrowser/images/database_connected"));
    } else {
      if (m_report->dataManager()->isConnectionConnected(connectionName))
        item->setIcon(0, QIcon(":/databrowser/images/database_connected"));
      else
        item->setIcon(0, QIcon(":/databrowser/images/database_disconnected"));
    }
    items.append(item);
  }

  //    for (QString connectionName : connections) {
  //        QTreeWidgetItem *item=new QTreeWidgetItem(
  //            ui->dataTree,
  //            QStringList(ConnectionDesc::connectionNameForUser(connectionName)),
  //            DataBrowserTree::Connection
  //        );
  //        item->setIcon(0,QIcon(":/databrowser/images/database_connected"));
  //    }

  //    connections = m_report->dataManager()->connectionNames();
  //    qSort(connections);
  //    for(QString connectionName : connectionName){
  //        if (!QSqlDatabase::contains(connectionName)){
  //            QTreeWidgetItem *item=new QTreeWidgetItem(
  //                ui->dataTree,
  //                QStringList(ConnectionDesc::connectionNameForUser(connectionName)),
  //                DataBrowserTree::Connection
  //            );
  //            if (m_report->dataManager()->isConnectionConnected(connectionName))
  //                item->setIcon(0,QIcon(":/databrowser/images/database_connected"));
  //            else
  //                item->setIcon(0,QIcon(":/databrowser/images/database_disconnected"));
  //            items.append(item);
  //        }
  //    }
  ui->dataTree->insertTopLevelItems(0, items);
}

QDockWidget *DataBrowser::createDataWindow(QString datasourceName) {
  QDockWidget *window = new QDockWidget("Table: " + datasourceName);
  QTableView *tableView = new QTableView(window);

  try {
    IDataSourceHolder *holder = m_report->dataManager()->dataSourceHolder(datasourceName);
    if (holder) holder->update();
    IDataSource *datasource = m_report->dataManager()->dataSource(datasourceName);
    if (datasource) { tableView->setModel(datasource->model()); }
  } catch (ReportError &exception) { qDebug() << exception.what(); }

  window->setWidget(tableView);
  window->setAttribute(Qt::WA_DeleteOnClose);
  connect(window, &QObject::destroyed, this, &DataBrowser::slotDataWindowClosed);

  if (!m_dataWindows.isEmpty())
    m_mainWindow->tabifyDockWidget(m_dataWindows.values().at(0), window);
  else
    m_mainWindow->addDockWidget(Qt::BottomDockWidgetArea, window);

  m_dataWindows.insert(datasourceName, window);

  return window;
}

QDockWidget *DataBrowser::dataWindow(QString datasourceName) {
  if (m_dataWindows.contains(datasourceName)) {
    return m_dataWindows.value(datasourceName);
  } else
    return createDataWindow(datasourceName);
}

void DataBrowser::setMainWindow(QMainWindow *mainWindow) { m_mainWindow = mainWindow; }

void DataBrowser::slotDataWindowClosed() {
  if (isClosingWindows()) return;
  for (int i = 0; i < m_dataWindows.count(); i++) {
    if (m_dataWindows.values().at(i) == sender()) { m_dataWindows.remove(m_dataWindows.keys().at(i)); }
  }
}

void DataBrowser::slotChangeConnection() {
  if (!getConnectionName(NameForUser).isEmpty()) {
    ConnectionDialog *connectionEdit = new ConnectionDialog(this, m_report->dataManager()->connectionByName(getConnectionName(NameForReport)), this);
    connectionEdit->setAttribute(Qt::WA_DeleteOnClose, true);
#ifdef Q_OS_MAC
    connectionEdit->setWindowModality(Qt::WindowModal);
#else
    connectionEdit->setWindowModality(Qt::ApplicationModal);
#endif
    // connect(connectionEdit,SIGNAL(finished(int)),this,SLOT(slotConnectionEditFinished(int)));
    connectionEdit->exec();
  }
}

void DataBrowser::slotChangeConnectionState() {
  QString connectionName = getConnectionName(NameForReport);
  if (!connectionName.isEmpty()) {
    if (!m_report->dataManager()->isConnectionConnected(connectionName)) {
      setCursor(Qt::WaitCursor);
      // try {
      if (!m_report->dataManager()->connectConnection(connectionName)) {
        //} catch(std::runtime_error &exception) {
        QMessageBox::critical(this, tr("Attention"), m_report->dataManager()->lastError());
        //}
      }
      updateDataTree();
      setCursor(Qt::ArrowCursor);
    } else {
      m_report->dataManager()->disconnectConnection(connectionName);
      updateDataTree();
    }
  }
}

void DataBrowser::slotVariableEditorAccept(const QString &variable) {
  updateVariablesTree();
  QList<QTreeWidgetItem *> items = ui->variablesTree->findItems(variable, Qt::MatchContains | Qt::MatchRecursive);
  if (!items.isEmpty()) { ui->variablesTree->setCurrentItem(items.at(0)); }
}

void DataBrowser::showDataWindow(QString datasourceName) {
  QDockWidget *window = dataWindow(datasourceName);
  if (window) {
    window->show();
    window->raise();
  }
}

void DataBrowser::closeDataWindow(QString datasourceName) {
  if (m_dataWindows.contains(datasourceName)) m_dataWindows.value(datasourceName)->close();
}

void DataBrowser::removeDatasource(QString datasourceName) {
  closeDataWindow(datasourceName);
  m_report->dataManager()->removeDatasource(datasourceName);
}

void DataBrowser::addQuery(SQLEditResult result) {
  try {
    m_report->dataManager()->addQuery(result.datasourceName, result.sql, result.connectionName);
  } catch (ReportError &exception) { qDebug() << exception.what(); }
}

void DataBrowser::changeQuery(SQLEditResult result) {
  try {
    m_report->dataManager()->removeDatasource(result.oldDatasourceName);
    m_report->dataManager()->addQuery(result.datasourceName, result.sql, result.connectionName);
  } catch (ReportError &exception) { qDebug() << exception.what(); }
}

void DataBrowser::addSubQuery(SQLEditResult result) {
  try {
    m_report->dataManager()->addSubQuery(result.datasourceName, result.sql, result.connectionName, result.masterDatasource);
  } catch (ReportError &exception) { qDebug() << exception.what(); }
}

void DataBrowser::changeSubQuery(SQLEditResult result) {
  try {
    m_report->dataManager()->removeDatasource(result.oldDatasourceName);
    m_report->dataManager()->addSubQuery(result.datasourceName, result.sql, result.connectionName, result.masterDatasource);
  } catch (ReportError &exception) { qDebug() << exception.what(); }
}

void DataBrowser::addProxy(SQLEditResult result) {
  try {
    m_report->dataManager()->addProxy(result.datasourceName, result.masterDatasource, result.childDataSource, result.fieldMap);
  } catch (ReportError &exception) { qDebug() << exception.what(); }
}

void DataBrowser::changeProxy(SQLEditResult result) {
  try {
    m_report->dataManager()->removeDatasource(result.oldDatasourceName);
    m_report->dataManager()->addProxy(result.datasourceName, result.masterDatasource, result.childDataSource, result.fieldMap);
  } catch (ReportError &exception) { qDebug() << exception.what(); }
}

void DataBrowser::addConnectionDesc(ConnectionDesc *connection) {
  m_report->dataManager()->addConnectionDesc(connection);
  updateDataTree();
}

void DataBrowser::changeConnectionDesc(ConnectionDesc *connection) {
  if (connection->autoconnect()) m_report->dataManager()->connectConnection(connection->name());
  updateDataTree();
}

bool DataBrowser::checkConnectionDesc(ConnectionDesc *connection) {
  bool result = m_report->dataManager()->checkConnectionDesc(connection);
  if (!result) setLastError(m_report->dataManager()->lastError());
  return result;
}

bool DataBrowser::containsDefaultConnection() {
  bool result = m_report->dataManager()->connectionByName(QSqlDatabase::defaultConnection) || QSqlDatabase::contains(QSqlDatabase::defaultConnection);
  return result;
}

QString DataBrowser::lastError() const { return m_lastError; }

void DataBrowser::setLastError(const QString &lastError) { m_lastError = lastError; }

void DataBrowser::on_dataTree_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous) {
  Q_UNUSED(previous)
  if (current && (current->type() == DataBrowserTree::Connection)) {
    bool internalConnection = m_report->dataManager()->connectionByName(ConnectionDesc::connectionNameForReport(current->text(0)));
    if (m_report->dataManager()->isConnectionConnected(ConnectionDesc::connectionNameForReport(current->text(0)))) {
      ui->pbConnect->setIcon(QIcon(":/databrowser/images/plug-connect.png"));
    } else {
      ui->pbConnect->setIcon(QIcon(":/databrowser/images/plug-disconnect.png"));
    }
    ui->editDataSource->setEnabled(false);
    ui->deleteDataSource->setEnabled(false);
    ui->viewDataSource->setEnabled(false);
    ui->pbConnect->setEnabled(internalConnection);
    ui->changeConnection->setEnabled(internalConnection);
    ui->deleteConection->setEnabled(internalConnection);
    ui->errorMessage->setDisabled(true);
  } else {
    ui->changeConnection->setEnabled(false);
    ui->deleteConection->setEnabled(false);
    ui->pbConnect->setEnabled(false);
    IDataSourceHolder *holder = m_report->dataManager()->dataSourceHolder(getDatasourceName());
    if (holder) {
      ui->viewDataSource->setEnabled(!holder->isInvalid());
      ui->editDataSource->setEnabled(holder->isEditable());
      ui->deleteDataSource->setEnabled(holder->isRemovable());
      if (!holder->lastError().isEmpty()) {
        ui->errorMessage->setEnabled(true);
      } else {
        ui->errorMessage->setEnabled(false);
      }
    }
  }
}

void LimeReport::DataBrowser::on_addVariable_clicked() {
  LRVariableDialog dialog(this);
#ifdef Q_OS_MAC
  dialog.setWindowModality(Qt::WindowModal);
#else
  dialog.setWindowModality(Qt::ApplicationModal);
#endif
  dialog.setVariableContainer(m_report->dataManager());
  connect(&dialog, &LRVariableDialog::signalVariableAccepted, this, &DataBrowser::slotVariableEditorAccept);
  dialog.exec();
}

void DataBrowser::on_editVariable_clicked() {
  if (!getVariable().isEmpty()) {
    LRVariableDialog dialog(this);
#ifdef Q_OS_MAC
    dialog.setWindowModality(Qt::WindowModal);
#else
    dialog.setWindowModality(Qt::ApplicationModal);
#endif
    dialog.setVariableContainer(m_report->dataManager());
    QString varName = getVariable();
    dialog.setVariableName(varName);
    connect(&dialog, &LRVariableDialog::signalVariableAccepted, this, &DataBrowser::slotVariableEditorAccept);
    dialog.exec();
  }
}

void DataBrowser::on_deleteVariable_clicked() {
  QString varName = getVariable();
  if (!varName.isEmpty()) {
    if (QMessageBox::critical(this, tr("Attention"), QString(tr("Do you really want to delete variable \"%1\"?")).arg(varName), QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel) ==
        QMessageBox::Ok) {
      m_report->dataManager()->deleteVariable(varName);
      updateVariablesTree();
    }
  }
}

void DataBrowser::on_variablesTree_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous) {
  Q_UNUSED(previous)
  if (current) {
    if (m_report->dataManager()->containsVariable(current->text(1))) {
      if (m_report->dataManager()->variableType(current->text(1)) == VarDesc::Report) {
        ui->editVariable->setEnabled(true);
        ui->deleteVariable->setEnabled(true);
      } else {
        ui->editVariable->setEnabled(false);
        ui->deleteVariable->setEnabled(false);
      }
      if (m_report->dataManager()->variableType(current->text(1)) == VarDesc::User) {
        ui->varToReport->setEnabled(true);
      } else {
        ui->varToReport->setEnabled(false);
      }
    } else {
      ui->editVariable->setEnabled(false);
      ui->deleteVariable->setEnabled(false);
      ui->varToReport->setEnabled(false);
    }
  }
}

void DataBrowser::on_errorMessage_clicked() {
  if (!getDatasourceName().isEmpty() && m_report->dataManager()->dataSourceHolder(getDatasourceName())) {
    QMessageBox::critical(this, tr("Error"), m_report->dataManager()->dataSourceHolder(getDatasourceName())->lastError());
  }
}

void DataBrowser::on_varToReport_clicked() {
  QString varName = getVariable();
  if (!varName.isEmpty()) {
    m_report->dataManager()->addVariable(varName, m_report->dataManager()->variable(varName), VarDesc::Report);
    ui->varToReport->setEnabled(false);
    updateVariablesTree();
  }
}

void DataBrowser::on_variablesTree_itemDoubleClicked(QTreeWidgetItem *item, int) {
  if (item) {
    QString varName = item->text(1);
    if (!varName.isEmpty() && (m_report->dataManager()->variableType(varName) == VarDesc::Report)) {
      LRVariableDialog dialog(this);
#ifdef Q_OS_MAC
      dialog.setWindowModality(Qt::WindowModal);
#else
      dialog.setWindowModality(Qt::ApplicationModal);
#endif
      dialog.setVariableContainer(m_report->dataManager());
      dialog.setVariableName(varName);
      connect(&dialog, &LRVariableDialog::signalVariableAccepted, this, &DataBrowser::slotVariableEditorAccept);
      dialog.exec();
    }
  }
}

} // namespace LimeReport
