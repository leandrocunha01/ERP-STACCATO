/*
Copyright (c) 2013 Raivis Strogonovs

http://morf.lv

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.*/

#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QResource>

#include "application.h"
#include "smtp.h"

Smtp::Smtp(const QString &user, const QString &pass, const QString &host, const quint16 port, const int timeout) : timeout(timeout), host(host), pass(pass), user(user), port(port) {
  socket = new QSslSocket(this);

  connect(socket, &QIODevice::readyRead, this, &Smtp::readyRead);
  connect(socket, &QAbstractSocket::connected, this, &Smtp::connected);
  connect(socket, qOverload<QAbstractSocket::SocketError>(&QAbstractSocket::error), this, &Smtp::errorReceived);
  connect(socket, &QAbstractSocket::stateChanged, this, &Smtp::stateChanged);
  connect(socket, &QAbstractSocket::disconnected, this, &Smtp::disconnected);
}

void Smtp::sendMail(const QString &from, const QString &to, const QString &cc, const QString &subject, const QString &body, const QStringList &files,
                    const QString &assinatura) { // FIXME: shadows //TODO: V688 http://www.viva64.com/en/V688 The 'from' function argument possesses the same name as one of the class members, which
                                                 // can result in a confusion.void Smtp::sendMail(const QString &from, const QString &to, const QString &cc, const QString &subject, const QString
                                                 // &body, const QStringList &files, const QString &assinatura) { // FIXME: shadows
  message = "To: " + to + "\n";
  message.append("Cc: " + cc + "\n");
  message.append("From: " + from + "\n");
  message.append("Subject: " + subject + "\n");

  // Let's intitiate multipart MIME with cutting boundary "frontier"
  message.append("MIME-Version: 1.0\n");
  message.append("Content-Type: multipart/mixed; boundary=frontier\n\n");

  message.append("--frontier\n");
  message.append("Content-Type: text/html\n\n"); // Uncomment this for HTML formating, coment the line below
  //  message.append("Content-Type: text/plain\n\n");
  message.append(body);
  message.append("\n\n");

  // REFAC: 5dont hardcode this
  // REFAC:__project public code
  //
  if (not assinatura.isEmpty()) {
    //    QFile file("://assinatura conrado.png");
    QFile file(assinatura);

    if (not file.open(QIODevice::ReadOnly)) { return qApp->enqueueError("Erro abrindo arquivo: " + file.errorString()); }

    const QByteArray bytes = file.readAll();
    message.append("--frontier\n");
    message.append("Content-Type: image/png\nContent-ID: <assinatura.png@gmail.com>\nContent-Disposition: inline; "
                   "filename=assinatura.png;\nContent-Transfer-Encoding: base64\n\n");
    message.append(bytes.toBase64());
    message.append("\n");
  }
  //

  if (not files.isEmpty()) {
    //    qDebug() << "Files to be sent: " << files.size();

    for (const auto &filePath : files) {
      QFile file(filePath);

      if (file.exists()) {
        if (not file.open(QIODevice::ReadOnly)) { return qApp->enqueueError("Erro ao abrir o arquivo do anexo: " + file.errorString()); }

        const QByteArray bytes = file.readAll();
        message.append("--frontier\n");
        message.append(R"(Content-Type: application/octet-stream; name=")" + QFileInfo(file.fileName()).fileName() + "\"\nContent-Disposition: attachment; filename=\"" +
                       QFileInfo(file.fileName()).fileName() + "\";\nContent-Transfer-Encoding: base64\n\n");
        message.append(bytes.toBase64());
        message.append("\n");
      }
    }
  } else {
    //    qDebug() << "No attachments found";
  }

  message.append("--frontier--\n");

  message.replace(QString::fromLatin1("\n"), QString::fromLatin1("\r\n"));
  message.replace(QString::fromLatin1("\r\n.\r\n"), QString::fromLatin1("\r\n..\r\n"));

  this->from = from;

  rcpt.append(to.split(";"));

  if (not cc.isEmpty()) { rcpt.append(cc.split(";")); }

  state = States::Init;
  socket->connectToHostEncrypted(host, port); //"smtp.gmail.com" and 465 for gmail TLS

  if (not socket->waitForConnected(timeout)) {
    //    qDebug() << "timeout: " << socket->errorString();
    emit status("Esgotado tempo de espera do servidor!");
    return;
  }

  t = new QTextStream(socket);
}

Smtp::~Smtp() {
  delete t;
  delete socket;
}
void Smtp::stateChanged(QAbstractSocket::SocketState socketState) {
  Q_UNUSED(socketState);
  //  qDebug() << "stateChanged " << socketState;

  //  if (socketState == QAbstractSocket::UnconnectedState) { emit status("Não conseguiu conectar ao servidor SMTP!"); }
}

void Smtp::errorReceived(QAbstractSocket::SocketError socketError) {
  if (socketError == QAbstractSocket::RemoteHostClosedError) { return; }
  //  qDebug() << "error: " << socketError;
  if (socketError == QAbstractSocket::HostNotFoundError) { emit status("Não encontrou o servidor SMTP!"); }
}

void Smtp::disconnected() {
  //  qDebug() << "disconneted";
  if (socket->errorString() == "The remote host closed the connection") { return; }
  //  qDebug() << "error " << socket->errorString();
}

void Smtp::connected() {
  //    qDebug() << "Connected ";
}

void Smtp::readyRead() {
  //  qDebug() << "readyRead";
  // SMTP is line-oriented

  QString responseLine;

  do {
    responseLine = socket->readLine();
    response += responseLine;
  } while (socket->canReadLine() and responseLine[3] != ' ');

  responseLine.truncate(3);

  //  qDebug() << "Server response code:" << responseLine;
  //  qDebug() << "Server response: " << response;
  //  qDebug() << "State: " << state;

  if (state == States::Close) { return; }

  if (responseLine == "221") { return; }

  if (state == States::Init and responseLine == "220") {
    // banner was okay, let's go on
    *t << "EHLO localhost"
       << "\r\n";
    t->flush();

    state = States::HandShake;
  }

  // No need, because I'm using socket->startClienEncryption() which makes the SSL handshake for you
  /*else if (state == Tls and responseLine == "250")
  {
      // Trying AUTH
      qDebug() << "STarting Tls";
      *t << "STARTTLS" << "\r\n";
      t->flush();
      state = HandShake;
  }*/

  else if (state == States::HandShake and responseLine == "250") {
    socket->startClientEncryption();

    if (not socket->waitForEncrypted(timeout)) {
      //      qDebug() << socket->errorString();
      state = States::Close;
    }

    // Send EHLO once again but now encrypted

    *t << "EHLO localhost"
       << "\r\n";
    t->flush();
    state = States::Auth;
  } else if (state == States::Auth and responseLine == "250") {
    // Trying AUTH
    //    qDebug() << "Auth";
    // NOTE: try using AUTH XOAUTH2 as described in the developers.google below
    *t << "AUTH LOGIN"
       //      *t << "AUTH XOAUTH2 + base64("user=" {User} "^Aauth=Bearer " {Access Token} "^A^A")"
       << "\r\n";
    t->flush();
    state = States::User;
  } else if (state == States::User and responseLine == "334") {
    // Trying User
    //    qDebug() << "Username";
    // GMAIL is using XOAUTH2 protocol, which basically means that password and username has to be sent in base64 coding
    // https://developers.google.com/gmail/xoauth2_protocol
    *t << QByteArray().append(user).toBase64() << "\r\n";
    t->flush();

    state = States::Pass;
  } else if (state == States::Pass and responseLine == "334") {
    // Trying pass
    //    qDebug() << "Pass";
    *t << QByteArray().append(pass).toBase64() << "\r\n";
    t->flush();

    state = States::Mail;
  } else if (state == States::Mail and responseLine == "235") {
    // HELO response was okay (well, it has to be)

    // Apperantly for Google it is mandatory to have MAIL FROM and RCPT email formated the following way ->
    // <email@gmail.com>
    //    qDebug() << "MAIL FROM:<" << from << ">";
    *t << "MAIL FROM:<" << from << ">\r\n";
    t->flush();
    state = States::Rcpt;
  } else if (state == States::Rcpt and responseLine == "250") {
    // Apperantly for Google it is mandatory to have MAIL FROM and RCPT email formated the following way ->
    // <email@gmail.com>
    //    qDebug() << "RCPT TO:<" << rcpt.first() << ">\r\n";
    *t << "RCPT TO:<" << rcpt.first() << ">\r\n"; // r
    rcpt.removeFirst();
    t->flush();
    //    qDebug() << "size: " << rcpt.size();
    state = not rcpt.isEmpty() ? States::Rcpt : States::Data;
  } else if (state == States::Data and responseLine == "250") {

    *t << "DATA\r\n";
    t->flush();
    state = States::Body;
  } else if (state == States::Body and responseLine == "354") {

    *t << message << "\r\n.\r\n";
    t->flush();
    state = States::Quit;
  } else if (state == States::Quit and responseLine == "250") {

    *t << "QUIT\r\n";
    t->flush();
    // here, we just close.
    state = States::Close;
    emit status(tr("Message sent"));
  } else if (state == States::Close) {
    deleteLater();
    return;
  } else {
    // something broke.
    qApp->enqueueError(tr("Unexpected reply from SMTP server:\n\n") + response);
    state = States::Close;
    emit status(tr("Failed to send message"));
  }

  response = "";
}
