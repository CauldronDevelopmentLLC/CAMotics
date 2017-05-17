/******************************************************************************\

    CAMotics is an Open-Source simulation and CAM software.
    Copyright (C) 2011-2017 Joseph Coffland <joseph@cauldrondevelopment.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

\******************************************************************************/

#pragma once

#include <cbang/json/JSON.h>

#include <QObject>

#if 0x50000 <= QT_VERSION
#include <QtWebSockets/QtWebSockets>
#endif

class QNetworkAccessManager;


namespace CAMotics {
  class QtWin;

  class BBCtrlAPI : public QObject {
    Q_OBJECT;

    QtWin *parent;
    QNetworkAccessManager *netManager;

    bool active;
    uint64_t lastMessage;
    QTimer updateTimer;

    QWebSocket webSocket;
    QUrl url;

    cb::JSON::Dict vars;

  public:
    BBCtrlAPI(QtWin *parent);

    void connectCNC(const QString &address);
    void disconnectCNC();
    void reconnect();
    void uploadGCode(const std::string &filename, const char *data,
                     unsigned length);

  protected slots:
    void onError(QAbstractSocket::SocketError error);
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString &message);
    void onUpdate();
  };
}
