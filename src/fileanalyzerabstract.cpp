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

#include "fileanalyzerabstract.h"

#include <limits>

#include <QProcess>
#include <QCoreApplication>
#include <QDate>
#include <QTextStream>

#include "guessing.h"
#include "general.h"

FileAnalyzerAbstract::FileAnalyzerAbstract(QObject *parent)
    : QObject(parent), textExtraction(teNone)
{
}

void FileAnalyzerAbstract::setTextExtraction(TextExtraction textExtraction) {
    this->textExtraction = textExtraction;
}

QStringList FileAnalyzerAbstract::runAspell(const QString &text, const QString &dictionary) const
{
    QStringList wordList;
    QProcess aspell(QCoreApplication::instance());
    QStringList args = QStringList() << QStringLiteral("-d") << dictionary << QStringLiteral("list");
    aspell.start(QStringLiteral("/usr/bin/aspell"), args);
    if (aspell.waitForStarted(10000)) {
        qint64 bytesWritten = aspell.write(text.toUtf8());
        if (bytesWritten < 0) return QStringList(); /// something went wrong
        aspell.closeWriteChannel();

        QTextStream ts(&aspell);
        ts.setCodec("UTF-8");
        while (aspell.waitForReadyRead(10000)) {
            QString line;
            while (!(line = ts.readLine()).isNull()) {
                wordList << line;
            }
        }
        if (!aspell.waitForFinished(10000))
            aspell.kill();
    }
    return wordList;
}

QString FileAnalyzerAbstract::guessLanguage(const QString &text) const
{
    int count = std::numeric_limits<int>::max();
    QString best;

    const QStringList langs = getAspellLanguages();
    for (const QString &lang : langs) {
        int c = runAspell(text, lang).count();
        if (c > 0 && c < count) { /// if c==0, no misspelled words where found, likely due to an error
            count = c;
            best = lang;
        }
    }

    return best;
}

QStringList FileAnalyzerAbstract::getAspellLanguages() const
{
    if (aspellLanguages.isEmpty()) {
        QRegExp language(QStringLiteral("^[a-z]{2}(_[A-Z]{2})?$"));
        QProcess aspell(qApp);
        QStringList args = QStringList() << QStringLiteral("dicts");
        aspell.start(QStringLiteral("/usr/bin/aspell"), args);
        if (aspell.waitForStarted(10000)) {
            aspell.closeWriteChannel();
            while (aspell.waitForReadyRead(10000)) {
                while (aspell.canReadLine()) {
                    const QString line = aspell.readLine().simplified();
                    if (language.indexIn(line) >= 0) {
                        aspellLanguages << language.cap(0);
                    }
                }
            }
            if (!aspell.waitForFinished(10000))
                aspell.kill();
        }

        //emit analysisReport(QString(QStringLiteral("<initialization source=\"aspell\" type=\"languages\">%1</initialization>\n").arg(aspellLanguages.join(","))));
    }

    return aspellLanguages;
}

QString FileAnalyzerAbstract::guessTool(const QString &toolString, const QString &altToolString) const
{
    QString result;
    QString text;

    if (microsoftToolRegExp.indexIn(altToolString) == 0)
        text = microsoftToolRegExp.cap(1);
    else if (!toolString.isEmpty())
        text = toolString;
    else if (!altToolString.isEmpty())
        text = altToolString;

    if (!text.isEmpty())
        result += Guessing::programToXML(text);

    return result;
}

QString FileAnalyzerAbstract::formatDate(const QDate date, const QString &base) const
{
    return QString(QStringLiteral("<date epoch=\"%6\" %5 year=\"%1\" month=\"%2\" day=\"%3\">%4</date>\n")).arg(QString::number(date.year()), QString::number(date.month()), QString::number(date.day()), date.toString(Qt::ISODate), base.isEmpty() ? QString() : QString(QStringLiteral("base=\"%1\"")).arg(base), QString::number(QDateTime(date).toTime_t()));
}

QString FileAnalyzerAbstract::evaluatePaperSize(int mmw, int mmh) const
{
    QString formatName;

    if (mmw >= 208 && mmw <= 212 && mmh >= 295 && mmh <= 299)
        formatName = QStringLiteral("A4");
    else if (mmh >= 208 && mmh <= 212 && mmw >= 295 && mmw <= 299)
        formatName = QStringLiteral("A4");
    else if (mmw >= 214 && mmw <= 218 && mmh >= 277 && mmh <= 281)
        formatName = QStringLiteral("Letter");
    else if (mmh >= 214 && mmh <= 218 && mmw >= 277 && mmw <= 281)
        formatName = QStringLiteral("Letter");
    else if (mmw >= 214 && mmw <= 218 && mmh >= 254 && mmh <= 258)
        formatName = QStringLiteral("Legal");
    else if (mmh >= 214 && mmh <= 218 && mmw >= 254 && mmw <= 258)
        formatName = QStringLiteral("Legal");

    return formatName.isEmpty()
           ? QString(QStringLiteral("<papersize height=\"%1\" width=\"%2\" orientation=\"%3\" />\n")).arg(QString::number(mmh), QString::number(mmw), mmw > mmh ? QStringLiteral("landscape") : QStringLiteral("portrait"))
           : QString(QStringLiteral("<papersize height=\"%1\" width=\"%2\" orientation=\"%4\">%3</papersize>\n")).arg(QString::number(mmh), QString::number(mmw), formatName, mmw > mmh ? QStringLiteral("landscape") : QStringLiteral("portrait"));
}

QStringList FileAnalyzerAbstract::aspellLanguages;

const QRegExp FileAnalyzerAbstract::microsoftToolRegExp(QStringLiteral("^(Microsoft\\s(.+\\S) [ -][ ]?(\\S.*)$"));
const QString FileAnalyzerAbstract::creationDate = QStringLiteral("creation");
const QString FileAnalyzerAbstract::modificationDate = QStringLiteral("modification");
