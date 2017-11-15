#ifndef USERSESSION_H
#define USERSESSION_H

#include <QSettings>
#include <QSqlQuery>

class UserSession final {

public:
  enum class Tipo { Padrao, Autorizacao };

  UserSession() = delete;
  static bool login(const QString &user, const QString &password, Tipo tipo = Tipo::Padrao);
  static int idUsuario();
  static int idLoja();
  static QString nome();
  static QString tipoUsuario();
  static std::optional<QVariant> fromLoja(const QString &parameter, const QString &user = nome());
  static std::optional<QVariant> getSetting(const QString &key);
  static void setSetting(const QString &key, const QVariant &value);

private:
  // attributes
  inline static QSqlQuery *query = nullptr;
  inline static QSettings *settings = new QSettings("Staccato", "ERP");
  // methods
  static void initializeQuery();
};

#endif // USERSESSION_H
