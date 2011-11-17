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

#include <QFile>
#include <QTextStream>
#include <QRegExp>
#include <QDebug>
#include <QTimer>

#include "general.h"
#include "fromlogfile.h"

FromLogFileFileFinder::FromLogFileFileFinder(const QString &logfilename, const QStringList &filters, QObject *parent)
    : FileFinder(parent), m_isAlive(true)
{
    QFile input(logfilename);
    if (input.open(QFile::ReadOnly)) {
        QTextStream textStream(&input);
        const QString text = textStream.readAll();
        input.close();

        QRegExp hitRegExp = QRegExp(QLatin1String("<filefinder event=\"hit\" href=\"([^\"]+)\" />"));
        QRegExp filenameRegExp = QRegExp(QString(QLatin1String("(^|/)%1$")).arg(filters.join(QChar('|'))).replace(QChar('.'), QLatin1String("[.]")).replace(QChar('*'), QLatin1String(".*")));

        int p = -1;
        while ((p = hitRegExp.indexIn(text, p + 1)) >= 0) {
            QUrl url(DocScan::dexmlify(hitRegExp.cap(1)));
            const QString name = url.toString();
            if (filenameRegExp.indexIn(name) >= 0) {
                qDebug() << "FromLogFileFileFinder  url=" << name;
                m_urlSet.insert(url);
            }
        }
    }
}

void FromLogFileFileFinder::startSearch(int numExpectedHits)
{
    emit report(QString("<filefinder type=\"fromlogfilefilefinder\" count=\"%1\" />\n").arg(m_urlSet.count()));
    int count = numExpectedHits;
    foreach(const QUrl &url, m_urlSet) {
        if (count <= 0) break;
        emit foundUrl(url);
        --count;
    }
    m_isAlive = false;
}

bool FromLogFileFileFinder::isAlive()
{
    return m_isAlive;
}

FromLogFileDownloader::FromLogFileDownloader(const QString &logfilename, const QStringList &filters, QObject *parent)
    : Downloader(parent), m_logfilename(logfilename), m_isAlive(true), m_filters(filters)
{
    QTimer::singleShot(500, this, SLOT(startParsingAndEmitting()));
}

void FromLogFileDownloader::startParsingAndEmitting()
{
    QFile input(m_logfilename);
    if (input.open(QFile::ReadOnly)) {
        QRegExp filenameRegExp = QRegExp(QString(QLatin1String("(^|/)%1$")).arg(m_filters.join(QChar('|'))).replace(QChar('.'), QLatin1String("[.]")).replace(QChar('*'), QLatin1String(".*")));
        QRegExp hitRegExp = QRegExp(QLatin1String("<download url=\"([^\"]+)\" filename=\"([^\"]+)\" status=\"success\""));
        QRegExp searchEngineNumResultsRegExp = QRegExp(QLatin1String("<searchengine\\b[^>]* numresults=\"([0-9]*)\""));
        int count = 0;

        QTextStream textStream(&input);
        QString line = textStream.readLine();
        while (!line.isNull()) {
            if (hitRegExp.indexIn(line) >= 0) {
                const QString filename(hitRegExp.cap(2));
                if (filenameRegExp.indexIn(filename) >= 0) {
                    const QUrl url(hitRegExp.cap(1));
                    emit downloaded(url, filename);
                    emit downloaded(filename);
                    ++count;
                }
            } else if (searchEngineNumResultsRegExp.indexIn(line) >= 0)
                emit report(QString(QLatin1String("<searchengine numresults=\"%1\" />")).arg(searchEngineNumResultsRegExp.cap(1)));

            line = textStream.readLine();
        }
        input.close();

        const QString s = QString("<downloader type=\"fromlogfiledownloader\" count=\"%1\" />\n").arg(count);
        emit report(s);
    }

    m_isAlive = false;
}

void FromLogFileDownloader::download(const QUrl &url)
{
    qWarning() << "This should never be called (url =" << url.toString() << ")";
}

void FromLogFileDownloader::finalReport()
{
    // nothing
}

bool FromLogFileDownloader::isAlive()
{
    return m_isAlive;
}
