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

#ifndef WEBCRAWLER_H
#define WEBCRAWLER_H

#include <QMap>
#include <QStringList>

#include "filefinder.h"

class QNetworkAccessManager;
class QTimer;
class QNetworkReply;

class WebCrawler : public FileFinder
{
    Q_OBJECT
public:
    explicit WebCrawler(QNetworkAccessManager *networkAccessManager, const QStringList &filters, const QUrl &baseUrl, QObject *parent = 0);

    virtual void startSearch(int numExpectedHits);
    virtual bool isAlive();

private:
    QNetworkAccessManager *m_networkAccessManager;
    const QUrl m_baseUrl;
    QRegExp m_filePattern;
    int m_runningDownloads;
    int m_numExpectedHits, m_numFoundHits, m_visitedPages;
    static const int maxParallelDownloads, maxVisitedPages;
    QMap<QTimer *, QNetworkReply *> m_mapTimerToReply;
    QStringList m_knownUrls;
    QStringList m_queuedUrls;

    void startDownload(const QUrl &url);
    QString completeUrl(const QString &partialUrl, const QUrl &baseUrl);

private slots:
    void finishedDownload();
    void timeout();
};

#endif // WEBCRAWLER_H
