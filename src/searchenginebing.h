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

#ifndef SEARCHENGINEBING_H
#define SEARCHENGINEBING_H

#include <QObject>

#include "searchengineabstract.h"

class QNetworkReply;

class NetworkAccessManager;

/**
 * Search engine using Microsoft's Bing to search for files.
 *
 * @author Thomas Fischer <thomas.fischer@his.se>
 */
class SearchEngineBing : public SearchEngineAbstract
{
    Q_OBJECT
public:
    explicit SearchEngineBing(NetworkAccessManager *networkAccessManager, const QString &searchTerm, QObject *parent = nullptr);

    virtual void startSearch(int numExpectedHits);
    virtual bool isAlive();

private:
    NetworkAccessManager *m_networkAccessManager;
    const QString m_searchTerm;
    int m_numExpectedHits, m_currentPage, m_numFoundHits;
    int m_runningSearches;

private slots:
    void finished();
};

#endif // SEARCHENGINEBING_H
