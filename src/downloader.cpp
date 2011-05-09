/*
    This file is part of DocScan.

    DocScan is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    DocScan is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with DocScan.  If not, see <https://www.gnu.org/licenses/>.


    Source code written by Thomas Fischer <thomas.fischer@his.se>

 */

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QRegExp>
#include <QCryptographicHash>
#include <QFile>
#include <QDebug>
#include <QFileInfo>
#include <QDir>

#include "downloader.h"

Downloader::Downloader(QNetworkAccessManager *networkAccessManager, const QString &filePattern, QObject *parent)
    : QObject(parent), m_networkAccessManager(networkAccessManager), m_filePattern(filePattern)
{
    // nothing
}

void Downloader::download(QUrl url)
{
    QNetworkReply *reply = m_networkAccessManager->get(QNetworkRequest(url));
    connect(reply, SIGNAL(finished()), this, SLOT(finished()));
}

void Downloader::finished()
{
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    QByteArray data(reply->readAll());
    QString filename = m_filePattern;

    QString md5sum = QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex();
    QRegExp md5sumRegExp("%\\{h(:(\\d+))?\\}");
    int p = -1;
    while ((p = md5sumRegExp.indexIn(filename)) >= 0) {
        if (md5sumRegExp.cap(1).isEmpty())
            filename = filename.replace(md5sumRegExp.cap(0), md5sum);
        else {
            bool ok = false;
            int left = md5sumRegExp.cap(2).toInt(&ok);
            if (ok && left > 0 && left <= md5sum.length())
                filename = filename.replace(md5sumRegExp.cap(0), md5sum.left(left));
        }
    }

    QString urlString = reply->url().toString().replace(QRegExp("[^a-z0-9]", Qt::CaseInsensitive), "_").replace(QRegExp("_([a-z0-9]{1,4})$", Qt::CaseInsensitive), ".\\1");
    filename = filename.replace("%{s}", urlString);

    QFileInfo fi(filename);
    fi.absoluteDir().mkpath(fi.absolutePath());

    QFile output(filename);
    if (output.open(QIODevice::WriteOnly)) {
        output.write(data);
        output.close();

        emit downloaded(reply->url(), filename);
    }
}
