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

#include <typeinfo>

#include <QTextStream>
#include <QDateTime>

#include "logcollector.h"

LogCollector::LogCollector(QIODevice &output, QObject *parent)
    : QObject(parent), m_output(output), m_tagStart("<(\\w+)\\b")
{
}

bool LogCollector::isAlive()
{
    return false;
}

void LogCollector::receiveLog(QString message)
{
    QString key = QString(typeid(*(sender())).name()).toLower().replace(QRegExp("[0-9]+"), "");

    QString time = QDateTime::currentDateTime().toUTC().toString(Qt::ISODate);
    m_logData << "<logitem source=\"" + key + "\" time=\"" + time + "\">\n" + message + "</logitem>\n";
}

void LogCollector::writeOut()
{
    QTextStream ts(&m_output);

    ts << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << endl << "<log>" << endl;
    foreach(QString logText, m_logData) {
        ts << logText;
    }
    ts << "</log>" << endl;


    ts.flush();
}