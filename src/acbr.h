#ifndef ACBR_H
#define ACBR_H

#include <QObject>
#include <QTcpSocket>

class ACBr final : public QObject {
  Q_OBJECT

public:
  ACBr() = delete;
  static bool gerarDanfe(const int idNFe);
  static std::optional<QString> enviarComando(const QString &comando);
  static std::optional<QString> gerarDanfe(const QByteArray &fileContent, const bool openFile = true);
  static std::optional<std::tuple<QString, QString>> consultarNFe(const int idNFe);

private:
  // attributes
  inline static QTcpSocket *socket = new QTcpSocket(nullptr);
  // methods
  static bool abrirPdf(const QString &resposta);
};

#endif // ACBR_H
