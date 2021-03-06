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


    Copyright (2017) Thomas Fischer <thomas.fischer@his.se>, senior
    lecturer at University of Skövde, as part of the LIM-IT project.

 */

#ifndef URLDOWNLOADER_H
#define URLDOWNLOADER_H

#include <QUrl>
#include <QTimer>
#include <QSet>
#include <QMap>
#include <QQueue>

#include "downloader.h"

class QSignalMapper;
class QMutex;
class QNetworkReply;

class NetworkAccessManager;
class GeoIP;

/**
 * Download files from a remote location (specified by an URL) to a local storage.
 *
 * @author Thomas Fischer <thomas.fischer@his.se>
 */
class UrlDownloader : public Downloader
{
    Q_OBJECT
public:
    /**
     * Create an instance of this class.
     * @param networkAccessManager network access manager to use to establish network connections
     * @param filePattern pattern used to rename downloaded files to make it easier to store and categorize them
     * @param parent parent object
     */
    explicit UrlDownloader(NetworkAccessManager *networkAccessManager, const QString &filePattern, int maxDownloads = 0x0fffffff, QObject *parent = nullptr);
    ~UrlDownloader();

    virtual bool isAlive();

    friend class DownloadJob;


public slots:
    void download(const QUrl &url);
    void finalReport();

signals:
    void downloaded(QUrl, QString);
    void downloaded(QString);
    void report(QString);

private:
    QQueue<QUrl> m_urlQueue;
    QSet<QNetworkReply *> *m_setRunningJobs;
    QMutex *m_internalMutex;
    QSignalMapper *m_signalMapperTimeout;
    NetworkAccessManager *m_networkAccessManager;
    const QString m_filePattern;
    int m_maxDownloads;
    int m_runningDownloads;
    QHash<QString, int> m_runningdownloadsPerHostname;
    static const int maxParallelDownloads, maxParallelDownloadsPerHost;
    QSet<QString> m_knownUrls;
    static const QRegExp domainRegExp;
    int m_countSuccessfulDownloads, m_countFailedDownloads;
    GeoIP *m_geoip;
    QMap<QString, int> m_domainCount;

    void startNextDownload();

    QString domainFromHostname(const QString &hostname);

private slots:
    void finished();
    void timeout(QObject *);
};

#endif // URLDOWNLOADER_H
