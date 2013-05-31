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

#ifndef FAKEDOWNLOADER_H
#define FAKEDOWNLOADER_H

#include "downloader.h"

/**
 * Passes through local URLs pretending that they got downloaded,
 * without touching, copying, or moving the files.
 *
 * @author Thomas Fischer <thomas.fischer@his.se>
 */
class FakeDownloader : public Downloader
{
    Q_OBJECT
public:
    FakeDownloader(QObject *parent = 0);

    virtual bool isAlive();

public slots:
    void download(const QUrl &url);
    void finalReport();

signals:
    void downloaded(QUrl, QString);
    void downloaded(QString);
    void report(QString);

private:
    int m_counterLocalFiles, m_counterErrors;
};

#endif // FAKEDOWNLOADER_H