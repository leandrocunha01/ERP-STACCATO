#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "sqltablemodel.h"

namespace Ui {
  class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

  public slots:
    bool updateTables();

  private slots:
    void on_actionCadastrarCliente_triggered();
    void on_actionCadastrarFornecedor_triggered();
    void on_actionCadastrarProdutos_triggered();
    void on_actionCadastrarProfissional_triggered();
    void on_actionCadastrarUsuario_triggered();
    void on_actionClaro_triggered();
    void on_actionConfigura_es_triggered();
    void on_actionCriarOrcamento_triggered();
    void on_actionEscuro_triggered();
    void on_actionGerenciar_Lojas_triggered();
    void on_actionGerenciar_Transportadoras_triggered();
    void on_actionImportaProdutos_triggered();
    void on_actionSobre_triggered();
    void on_tabWidget_currentChanged(int);

  private:
    // attributes
    Ui::MainWindow *ui;
    QString defaultStyle;
    QPalette defautPalette;
    // methods
    bool event(QEvent *e);
    void darkTheme();
    QVariant settings(QString key) const;
    void setSettings(QString key, QVariant value) const;
};

#endif // MAINWINDOW_H
