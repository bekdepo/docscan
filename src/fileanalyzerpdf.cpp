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

#include "fileanalyzerpdf.h"

#include <QFileInfo>
#include <QDebug>
#include <QDateTime>
#include <QProcess>
#include <QCoreApplication>
#include <QDir>
#include <QRegularExpression>

#include "popplerwrapper.h"
#include "watchdog.h"
#include "guessing.h"
#include "general.h"

static const int oneMinuteInMillisec = 60000;
static const int twoMinutesInMillisec = oneMinuteInMillisec * 2;
static const int fourMinutesInMillisec = oneMinuteInMillisec * 4;
static const int sixMinutesInMillisec = oneMinuteInMillisec * 6;

FileAnalyzerPDF::FileAnalyzerPDF(QObject *parent)
    : FileAnalyzerAbstract(parent), m_isAlive(false)
{
    // nothing
}

bool FileAnalyzerPDF::isAlive()
{
    return m_isAlive;
}

void FileAnalyzerPDF::setupJhove(const QString &shellscript)
{
    m_jhoveShellscript = shellscript;
}

void FileAnalyzerPDF::setupVeraPDF(const QString &cliTool)
{
    m_veraPDFcliTool = cliTool;
}

void FileAnalyzerPDF::setupPdfBoXValidator(const QString &pdfboxValidatorJavaClass) {
    m_pdfboxValidatorJavaClass = pdfboxValidatorJavaClass;
}

void FileAnalyzerPDF::setupCallasPdfAPilotCLI(const QString &callasPdfAPilotCLI) {
    m_callasPdfAPilotCLI = callasPdfAPilotCLI;
}

void FileAnalyzerPDF::analyzeFile(const QString &filename)
{
    if (filename.endsWith(QStringLiteral(".xz")) || filename.endsWith(QStringLiteral(".gz")) || filename.endsWith(QStringLiteral(".bz2")) || filename.endsWith(QStringLiteral(".lzma"))) {
        /// File is compressed
        qWarning() << "Compressed files like " << filename << " should not directly send through this analyzer, but rather be uncompressed by FileAnalyzerMultiplexer first";
        m_isAlive = false;
        return;
    }

    QString logText, metaText;
    m_isAlive = true;
    const qint64 startTime = QDateTime::currentMSecsSinceEpoch();

    /// External programs should be both CPU and I/O 'nice'
    static const QStringList defaultArgumentsForNice = QStringList() << QStringLiteral("-n") << QStringLiteral("17") << QStringLiteral("ionice") << QStringLiteral("-c") << QStringLiteral("3");

    bool veraPDFStartedRun1 = false, veraPDFStartedRun2 = false;
    bool veraPDFIsPDFA1B = false, veraPDFIsPDFA1A = false;
    QString veraPDFStandardOutput;
    QString veraPDFErrorOutput;
    long veraPDFfilesize = 0;
    int veraPDFExitCode = INT_MIN;
    QProcess veraPDF(this);
    if (!m_veraPDFcliTool.isEmpty()) {
        const QStringList arguments = QStringList(defaultArgumentsForNice) << m_veraPDFcliTool << QStringLiteral("-x") << QStringLiteral("-f") /** Chooses built-in Validation Profile flavour, e.g. '1b'. */ << QStringLiteral("1b") << QStringLiteral("--maxfailures") << QStringLiteral("1") << QStringLiteral("--format") << QStringLiteral("xml") << filename;
        veraPDF.start(QStringLiteral("/usr/bin/nice"), arguments, QIODevice::ReadOnly);
        veraPDFStartedRun1 = veraPDF.waitForStarted(twoMinutesInMillisec);
        if (!veraPDFStartedRun1)
            qWarning() << "Failed to start veraPDF for file " << filename << " and " << veraPDF.program() << veraPDF.arguments().join(' ') << " in directory " << veraPDF.workingDirectory();
    }

    bool callasPdfAPilotStartedRun1 = false, callasPdfAPilotStartedRun2 = false;
    QString callasPdfAPilotStandardOutput;
    QString callasPdfAPilotErrorOutput;
    int callasPdfAPilotExitCode = INT_MIN;
    int callasPdfAPilotCountErrors = -1;
    int callasPdfAPilotCountWarnings = -1;
    char callasPdfAPilotPDFA1letter = '\0';
    QProcess callasPdfAPilot(this);
    if (!m_callasPdfAPilotCLI.isEmpty()) {
        const QStringList arguments = QStringList() << defaultArgumentsForNice << m_callasPdfAPilotCLI << QStringLiteral("--quickpdfinfo") << filename;
        callasPdfAPilot.start(QStringLiteral("/usr/bin/nice"), arguments, QIODevice::ReadOnly);
        callasPdfAPilotStartedRun1 = callasPdfAPilot.waitForStarted(oneMinuteInMillisec);
        if (!callasPdfAPilotStartedRun1)
            qWarning() << "Failed to start callas PDF/A Pilot for file " << filename << " and " << callasPdfAPilot.program() << callasPdfAPilot.arguments().join(' ') << " in directory " << callasPdfAPilot.workingDirectory();
    }

    bool jhoveStarted = false;
    bool jhoveIsPDF = false;
    bool jhovePDFWellformed = false, jhovePDFValid = false;
    QString jhovePDFversion;
    QString jhovePDFprofile;
    QString jhoveStandardOutput;
    QString jhoveErrorOutput;
    int jhoveExitCode = INT_MIN;
    QProcess jhove(this);
    if (!m_jhoveShellscript.isEmpty()) {
        const QStringList arguments = QStringList(defaultArgumentsForNice) << QStringLiteral("/bin/bash") << m_jhoveShellscript << QStringLiteral("-m") << QStringLiteral("PDF-hul") << QStringLiteral("-t") << QStringLiteral("/tmp") << QStringLiteral("-b") << QStringLiteral("131072") << filename;
        jhove.start(QStringLiteral("/usr/bin/nice"), arguments, QIODevice::ReadOnly);
        jhoveStarted = jhove.waitForStarted(oneMinuteInMillisec);
        if (!jhoveStarted)
            qWarning() << "Failed to start jhove for file " << filename << " and " << jhove.program() << jhove.arguments().join(' ') << " in directory " << jhove.workingDirectory();
    }

    bool pdfboxValidatorStarted = false;
    bool pdfboxValidatorValidPdf = false;
    QString pdfboxValidatorStandardOutput;
    QString pdfboxValidatorErrorOutput;
    int pdfboxValidatorExitCode = INT_MIN;
    QProcess pdfboxValidator(this);
    if (!m_pdfboxValidatorJavaClass.isEmpty()) {
        const QFileInfo fi(m_pdfboxValidatorJavaClass);
        const QDir dir = fi.dir();
        const QStringList jarFiles = dir.entryList(QStringList() << QStringLiteral("*.jar"), QDir::Files, QDir::Name);
        pdfboxValidator.setWorkingDirectory(dir.path());
        const QStringList arguments = QStringList(defaultArgumentsForNice) << QStringLiteral("java") << QStringLiteral("-cp") << QStringLiteral(".:") + jarFiles.join(':') << fi.fileName().remove(QStringLiteral(".class")) << filename;
        pdfboxValidator.start(QStringLiteral("/usr/bin/nice"), arguments, QIODevice::ReadOnly);
        pdfboxValidatorStarted = pdfboxValidator.waitForStarted(oneMinuteInMillisec);
        if (!pdfboxValidatorStarted)
            qWarning() << "Failed to start pdfbox Validator for file " << filename << " and " << pdfboxValidator.program() << pdfboxValidator.arguments().join(' ') << " in directory " << pdfboxValidator.workingDirectory() << ": " << pdfboxValidatorErrorOutput;
    }

    if (veraPDFStartedRun1) {
        if (!veraPDF.waitForFinished(sixMinutesInMillisec))
            qWarning() << "Waiting for veraPDF failed or exceeded time limit for file " << filename << " and " << veraPDF.program() << veraPDF.arguments().join(' ') << " in directory " << veraPDF.workingDirectory();
        veraPDFExitCode = veraPDF.exitCode();
        veraPDFStandardOutput = QString::fromUtf8(veraPDF.readAllStandardOutput().constData());
        /// Sometimes veraPDF does not return complete and valid XML code. veraPDF's bug or DocScan's bug?
        if ((!veraPDFStandardOutput.contains(QStringLiteral("<rawResults>")) || !veraPDFStandardOutput.contains(QStringLiteral("</rawResults>"))) && (!veraPDFStandardOutput.contains(QStringLiteral("<ns2:cliReport")) || !veraPDFStandardOutput.contains(QStringLiteral("</ns2:cliReport>"))))
            veraPDFStandardOutput = QStringLiteral("<error>No matching opening and closing 'rawResults' or 'ns2:cliReport' tags found in output:\n") + DocScan::xmlify(veraPDFStandardOutput) + QStringLiteral("</error>");
        veraPDFErrorOutput = QString::fromUtf8(veraPDF.readAllStandardError().constData());
        if (veraPDFExitCode == 0 && !veraPDFStandardOutput.isEmpty()) {
            const QString startOfOutput = veraPDFStandardOutput.left(8192);
            const int p1 = startOfOutput.indexOf(QStringLiteral(" flavour=\"PDF"));
            const int p2 = startOfOutput.indexOf(QStringLiteral(" flavour=\"PDFA_1_B\""), p1);
            const int p3a = startOfOutput.indexOf(QStringLiteral(" isCompliant=\"true\""), p2 - 64);
            const int p3b = startOfOutput.indexOf(QStringLiteral(" recordPasses=\"true\""), p2 - 64);
            veraPDFIsPDFA1B = p1 == p2 && ((p3a > 0 && p3a < p2 + 64) || (p3b > 0 && p3b < p2 + 64));
            const int p4 = startOfOutput.indexOf(QStringLiteral("item size=\""));
            if (p4 > 1) {
                const int p5 = startOfOutput.indexOf(QStringLiteral("\""), p4 + 11);
                if (p5 > p4) {
                    bool ok = false;
                    veraPDFfilesize = startOfOutput.mid(p4 + 11, p5 - p4 - 11).toLong(&ok);
                    if (!ok) veraPDFfilesize = 0;
                }
            }

            if (veraPDFIsPDFA1B) {
                /// So, it is PDF-A/1b, then test for PDF-A/1a
                const QStringList arguments = QStringList(defaultArgumentsForNice) << m_veraPDFcliTool << QStringLiteral("-x") /** Extracts and reports PDF features. */ << QStringLiteral("-f") /** Chooses built-in Validation Profile flavour, e.g. '1b'. */ << QStringLiteral("1a") << QStringLiteral("--maxfailures") << QStringLiteral("1") << QStringLiteral("--format") << QStringLiteral("xml") << filename;
                veraPDF.start(QStringLiteral("/usr/bin/nice"), arguments, QIODevice::ReadOnly);
                veraPDFStartedRun2 = veraPDF.waitForStarted(twoMinutesInMillisec);
                if (!veraPDFStartedRun2)
                    qWarning() << "Failed to start veraPDF for file " << filename << " and " << veraPDF.program() << veraPDF.arguments().join(' ') << " in directory " << veraPDF.workingDirectory();
            } else
                qDebug() << "Skipping second run of veraPDF as file " << filename << "is not PDF/A-1b";
        } else
            qWarning() << "Execution of veraPDF failed for file " << filename << " and " << veraPDF.program() << veraPDF.arguments().join(' ') << " in directory " << veraPDF.workingDirectory() << ": " << veraPDFErrorOutput;
    }

    if (callasPdfAPilotStartedRun1) {
        if (!callasPdfAPilot.waitForFinished(twoMinutesInMillisec))
            qWarning() << "Waiting for callas PDF/A Pilot failed or exceeded time limit for file " << filename << " and " << callasPdfAPilot.program() << callasPdfAPilot.arguments().join(' ') << " in directory " << callasPdfAPilot.workingDirectory();
        callasPdfAPilotExitCode = callasPdfAPilot.exitCode();
        callasPdfAPilotStandardOutput = QString::fromUtf8(callasPdfAPilot.readAllStandardOutput().constData());
        callasPdfAPilotErrorOutput = QString::fromUtf8(callasPdfAPilot.readAllStandardError().constData());

        if (callasPdfAPilotExitCode == 0 && !callasPdfAPilotStandardOutput.isEmpty()) {
            static const QRegularExpression rePDFA(QStringLiteral("\\bInfo\\s+PDFA\\s+PDF/A-1([ab])"));
            const QRegularExpressionMatch match = rePDFA.match(callasPdfAPilotStandardOutput.right(512));
            callasPdfAPilotPDFA1letter = match.hasMatch() ? match.captured(1).at(0).toLatin1() : '\0';
            if (callasPdfAPilotPDFA1letter == 'a' || callasPdfAPilotPDFA1letter == 'b') {
                /// Document claims to be PDF/A-1a or PDF/A-1b, so test for errors
                const QStringList arguments = QStringList(defaultArgumentsForNice) << m_callasPdfAPilotCLI << QStringLiteral("-a") << filename;
                callasPdfAPilot.start(QStringLiteral("/usr/bin/nice"), arguments, QIODevice::ReadOnly);
                callasPdfAPilotStartedRun2 = callasPdfAPilot.waitForStarted(oneMinuteInMillisec);
                if (!callasPdfAPilotStartedRun2)
                    qWarning() << "Failed to start callas PDF/A Pilot for file " << filename << " and " << callasPdfAPilot.program() << callasPdfAPilot.arguments().join(' ') << " in directory " << callasPdfAPilot.workingDirectory();
            } else
                qDebug() << "Skipping second run of callas PDF/A Pilot as file " << filename << "is not PDF/A-1";
        } else
            qWarning() << "Execution of callas PDF/A Pilot failed for file " << filename << " and " << callasPdfAPilot.program() << callasPdfAPilot.arguments().join(' ') << " in directory " << callasPdfAPilot.workingDirectory() << ": " << callasPdfAPilotErrorOutput;
    }

    if (jhoveStarted) {
        if (!jhove.waitForFinished(fourMinutesInMillisec))
            qWarning() << "Waiting for jHove failed or exceeded time limit for file " << filename << " and " << jhove.program() << jhove.arguments().join(' ') << " in directory " << jhove.workingDirectory();
        jhoveExitCode = jhove.exitCode();
        jhoveStandardOutput = QString::fromUtf8(jhove.readAllStandardOutput().constData()).replace(QLatin1Char('\n'), QStringLiteral("###"));
        jhoveErrorOutput = QString::fromUtf8(jhove.readAllStandardError().constData()).replace(QLatin1Char('\n'), QStringLiteral("###"));
        if (jhoveExitCode == 0 && !jhoveStandardOutput.isEmpty()) {
            jhoveIsPDF = jhoveStandardOutput.contains(QStringLiteral("Format: PDF")) && !jhoveStandardOutput.contains(QStringLiteral("ErrorMessage:"));
            static const QRegExp pdfStatusRegExp(QStringLiteral("\\bStatus: ([^#]+)"));
            if (pdfStatusRegExp.indexIn(jhoveStandardOutput) >= 0) {
                jhovePDFWellformed = pdfStatusRegExp.cap(1).startsWith(QStringLiteral("Well-Formed"), Qt::CaseInsensitive);
                jhovePDFValid = pdfStatusRegExp.cap(1).endsWith(QStringLiteral("and valid"));
            }
            static const QRegExp pdfVersionRegExp(QStringLiteral("\\bVersion: ([^#]+)#"));
            jhovePDFversion = pdfVersionRegExp.indexIn(jhoveStandardOutput) >= 0 ? pdfVersionRegExp.cap(1) : QString();
            static const QRegExp pdfProfileRegExp(QStringLiteral("\\bProfile: ([^#]+)(#|$)"));
            jhovePDFprofile = pdfProfileRegExp.indexIn(jhoveStandardOutput) >= 0 ? pdfProfileRegExp.cap(1) : QString();
        } else
            qWarning() << "Execution of jHove failed for file " << filename << " and " << jhove.program() << jhove.arguments().join(' ') << " in directory " << jhove.workingDirectory() << ": " << jhoveErrorOutput;
    }

    if (pdfboxValidatorStarted) {
        if (!pdfboxValidator.waitForFinished(twoMinutesInMillisec))
            qWarning() << "Waiting for pdfbox Validator failed or exceeded time limit for file " << filename << " and " << pdfboxValidator.program() << pdfboxValidator.arguments().join(' ') << " in directory " << pdfboxValidator.workingDirectory();
        pdfboxValidatorExitCode = pdfboxValidator.exitCode();
        pdfboxValidatorStandardOutput = QString::fromUtf8(pdfboxValidator.readAllStandardOutput().constData());
        pdfboxValidatorErrorOutput = QString::fromUtf8(pdfboxValidator.readAllStandardError().constData());
        if (pdfboxValidatorExitCode == 0 && !pdfboxValidatorStandardOutput.isEmpty())
            pdfboxValidatorValidPdf = pdfboxValidatorStandardOutput.contains(QStringLiteral("is a valid PDF/A-1b file"));
        else
            qWarning() << "Execution of pdfbox Validator failed for file " << filename << " and " << pdfboxValidator.program() << pdfboxValidator.arguments().join(' ') << " in directory " << pdfboxValidator.workingDirectory() << ": " << pdfboxValidatorErrorOutput;
    }

    if (veraPDFStartedRun2) {
        if (!veraPDF.waitForFinished(sixMinutesInMillisec))
            qWarning() << "Waiting for veraPDF failed or exceeded time limit for file " << filename << " and " << veraPDF.program() << veraPDF.arguments().join(' ') << " in directory " << veraPDF.workingDirectory();
        veraPDFExitCode = veraPDF.exitCode();
        /// Some string magic to skip '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>' from second output
        const QString newStdOut = QString::fromUtf8(veraPDF.readAllStandardOutput().constData());
        const int p = newStdOut.indexOf(QStringLiteral("?>"));
        /// Sometimes veraPDF does not return complete and valid XML code. veraPDF's bug or DocScan's bug?
        if ((newStdOut.contains(QStringLiteral("<rawResults>")) && newStdOut.contains(QStringLiteral("</rawResults>"))) || (newStdOut.contains(QStringLiteral("<ns2:cliReport")) && newStdOut.contains(QStringLiteral("</ns2:cliReport>"))))
            veraPDFStandardOutput.append(QStringLiteral("\n") + (p > 1 ? newStdOut.mid(veraPDFStandardOutput.indexOf(QStringLiteral("<"), p)) : newStdOut));
        else
            veraPDFStandardOutput.append(QStringLiteral("<error>No matching opening and closing 'rawResults' or 'ns2:cliReport' tags found in output:\n") + DocScan::xmlify(newStdOut) + QStringLiteral("</error>"));

        veraPDFErrorOutput = veraPDFErrorOutput + QStringLiteral("\n") + QString::fromUtf8(veraPDF.readAllStandardError().constData());
        if (veraPDFExitCode == 0) {
            const QString startOfOutput = newStdOut.left(8192);
            const int p1 = startOfOutput.indexOf(QStringLiteral(" flavour=\"PDFA_1_A\""));
            const int p2a = startOfOutput.indexOf(QStringLiteral(" isCompliant=\"true\""), p1 - 64);
            const int p2b = startOfOutput.indexOf(QStringLiteral(" recordPasses=\"true\""), p1 - 64);
            veraPDFIsPDFA1A = p1 > 0 && ((p2a > 0 && p2a < p1 + 64) || (p2b > 0 && p2b < p1 + 64));
        } else
            qWarning() << "Execution of veraPDF failed for file " << filename << " and " << veraPDF.program() << veraPDF.arguments().join(' ') << " in directory " << veraPDF.workingDirectory() << ": " << veraPDFErrorOutput;
    }

    if (callasPdfAPilotStartedRun2) {
        if (!callasPdfAPilot.waitForFinished(fourMinutesInMillisec))
            qWarning() << "Waiting for callas PDF/A Pilot failed or exceeded time limit for file " << filename << " and " << callasPdfAPilot.program() << callasPdfAPilot.arguments().join(' ') << " in directory " << callasPdfAPilot.workingDirectory();
        callasPdfAPilotExitCode = callasPdfAPilot.exitCode();
        callasPdfAPilotStandardOutput = callasPdfAPilotStandardOutput + QStringLiteral("\n") + QString::fromUtf8(callasPdfAPilot.readAllStandardOutput().constData());
        callasPdfAPilotErrorOutput = callasPdfAPilotErrorOutput + QStringLiteral("\n") + QString::fromUtf8(callasPdfAPilot.readAllStandardError().constData());
        if (callasPdfAPilotExitCode == 0) {
            static const QRegularExpression reSummary(QStringLiteral("\\bSummary\\t(Errors|Warnings)\\t(0|[1-9][0-9]*)\\b"));
            QRegularExpressionMatchIterator reIter = reSummary.globalMatch(callasPdfAPilotStandardOutput.right(512));
            while (reIter.hasNext()) {
                const QRegularExpressionMatch match = reIter.next();
                if (match.captured(1) == QStringLiteral("Errors")) {
                    bool ok = false;
                    callasPdfAPilotCountErrors = match.captured(2).toInt(&ok);
                    if (!ok) callasPdfAPilotCountErrors = -1;
                } else if (match.captured(1) == QStringLiteral("Warnings")) {
                    bool ok = false;
                    callasPdfAPilotCountWarnings = match.captured(2).toInt(&ok);
                    if (!ok) callasPdfAPilotCountWarnings = -1;
                }
            }
        } else
            qWarning() << "Execution of callas PDF/A Pilot failed for file " << filename << " and " << callasPdfAPilot.program() << callasPdfAPilot.arguments().join(' ') << " in directory " << callasPdfAPilot.workingDirectory() << ": " << callasPdfAPilotErrorOutput;
    }

    const qint64 externalProgramsEndTime = QDateTime::currentMSecsSinceEpoch();

    PopplerWrapper *wrapper = PopplerWrapper::createPopplerWrapper(filename);
    const bool popplerWrapperOk = wrapper != nullptr;
    if (popplerWrapperOk) {
        QString guess, headerText;

        /// file format including mime type and file format version
        int majorVersion = 0, minorVersion = 0;
        wrapper->getPdfVersion(majorVersion, minorVersion);
        metaText.append(QString(QStringLiteral("<fileformat>\n<mimetype>application/pdf</mimetype>\n<version major=\"%1\" minor=\"%2\">%1.%2</version>\n<security locked=\"%3\" encrypted=\"%4\" />\n</fileformat>\n")).arg(QString::number(majorVersion), QString::number(minorVersion), wrapper->isLocked() ? QStringLiteral("yes") : QStringLiteral("no"), wrapper->isEncrypted() ? QStringLiteral("yes") : QStringLiteral("no")));

        /// guess and evaluate editor (a.k.a. creator)
        QString toolXMLtext;
        QString creator = wrapper->info(QStringLiteral("Creator"));
        guess.clear();
        if (!creator.isEmpty())
            guess = guessTool(creator, wrapper->info(QStringLiteral("Title")));
        if (!guess.isEmpty())
            toolXMLtext.append(QString(QStringLiteral("<tool type=\"editor\">\n%1</tool>\n")).arg(guess));
        /// guess and evaluate producer
        QString producer = wrapper->info(QStringLiteral("Producer"));
        guess.clear();
        if (!producer.isEmpty())
            guess = guessTool(producer, wrapper->info(QStringLiteral("Title")));
        if (!guess.isEmpty())
            toolXMLtext.append(QString(QStringLiteral("<tool type=\"producer\">\n%1</tool>\n")).arg(guess));
        if (!toolXMLtext.isEmpty())
            metaText.append(QStringLiteral("<tools>\n")).append(toolXMLtext).append(QStringLiteral("</tools>\n"));

        if (!wrapper->isLocked()) {
            /// some functions are sensitive if PDF is locked

            /// retrieve font information
            const QStringList fontNames = wrapper->fontNames();
            static QRegExp fontNameNormalizer(QStringLiteral("^[A-Z]+\\+"), Qt::CaseInsensitive);
            QSet<QString> knownFonts;
            QString fontXMLtext;
            for (const QString &fi : fontNames) {
                QStringList fields = fi.split(QLatin1Char('|'), QString::KeepEmptyParts);
                if (fields.length() < 2) continue;
                const QString fontName = fields[0].remove(fontNameNormalizer);
                QString fontFilename;
                const int p1 = fi.indexOf(QStringLiteral("|FONTFILENAME:"));
                if (p1 > 0) {
                    const int p2 = fi.indexOf(QStringLiteral("|"), p1 + 4);
                    fontFilename = fi.mid(p1 + 15, p2 - p1 - 15).replace(QStringLiteral("#20"), QStringLiteral(" "));
                }
                if (fontName.isEmpty()) continue;
                if (knownFonts.contains(fontName)) continue; else knownFonts.insert(fontName);
                fontXMLtext.append(QString(QStringLiteral("<font embedded=\"%2\" subset=\"%3\"%4>\n%1</font>\n")).arg(Guessing::fontToXML(fontName, fields[1]), fi.contains(QStringLiteral("|EMBEDDED:1")) ? QStringLiteral("yes") : QStringLiteral("no"), fi.contains(QStringLiteral("|SUBSET:1")) ? QStringLiteral("yes") : QStringLiteral("no"), fontFilename.isEmpty() ? QString() : QString(QStringLiteral(" filename=\"%1\"")).arg(fontFilename)));
            }
            if (!fontXMLtext.isEmpty())
                /// Wrap multiple <font> tags into one <fonts> tag
                metaText.append(QStringLiteral("<fonts>\n")).append(fontXMLtext).append(QStringLiteral("</fonts>\n"));
        }

        /// format creation date
        QDate date = wrapper->date(QStringLiteral("CreationDate")).toUTC().date();
        if (date.isValid())
            headerText.append(formatDate(date, creationDate));
        /// format modification date
        date = wrapper->date(QStringLiteral("ModDate")).toUTC().date();
        if (date.isValid())
            headerText.append(formatDate(date, modificationDate));

        /// retrieve author
        QString author = wrapper->info(QStringLiteral("Author")).simplified();
        if (!author.isEmpty())
            headerText.append(QString(QStringLiteral("<author>%1</author>\n")).arg(DocScan::xmlify(author)));

        /// retrieve title
        QString title = wrapper->info(QStringLiteral("Title")).simplified();
        /// clean-up title
        if (microsoftToolRegExp.indexIn(title) == 0)
            title = microsoftToolRegExp.cap(3);
        if (!title.isEmpty())
            headerText.append(QString(QStringLiteral("<title>%1</title>\n")).arg(DocScan::xmlify(title)));

        /// retrieve subject
        QString subject = wrapper->info(QStringLiteral("Subject")).simplified();
        if (!subject.isEmpty())
            headerText.append(QString(QStringLiteral("<subject>%1</subject>\n")).arg(DocScan::xmlify(subject)));

        /// retrieve keywords
        QString keywords = wrapper->info(QStringLiteral("Keywords")).simplified();
        if (!keywords.isEmpty())
            headerText.append(QString(QStringLiteral("<keyword>%1</keyword>\n")).arg(DocScan::xmlify(keywords)));

        if (!wrapper->isLocked()) {
            /// some functions are sensitive if PDF is locked

            QString bodyText;
            if (textExtraction > teNone) {
                int length = 0;
                const QString text = wrapper->plainText(&length);
                QString language;
                if (textExtraction >= teAspell) {
                    language = guessLanguage(text);
                    if (!language.isEmpty())
                        headerText.append(QString(QStringLiteral("<language origin=\"aspell\">%1</language>\n")).arg(language));
                }
                bodyText = QString(QStringLiteral("<body length=\"%1\"")).arg(length);
                if (textExtraction >= teFullText)
                    bodyText.append(QStringLiteral(">\n")).append(wrapper->popplerLog()).append(QStringLiteral("</body>\n"));
                else
                    bodyText.append(QStringLiteral("/>\n"));
            }
            if (!bodyText.isEmpty())
                logText.append(bodyText);

            /// look into first page for info
            int numPages = wrapper->numPages();
            headerText.append(QString(QStringLiteral("<num-pages>%1</num-pages>\n")).arg(numPages));
            if (numPages > 0) {
                /// retrieve and evaluate paper size
                QSizeF size = wrapper->pageSize();
                int mmw = size.width() * 0.3527778;
                int mmh = size.height() * 0.3527778;
                if (mmw > 0 && mmh > 0) {
                    headerText += evaluatePaperSize(mmw, mmh);
                }
            }
        }

        if (!headerText.isEmpty())
            logText.append(QStringLiteral("<header>\n")).append(headerText).append(QStringLiteral("</header>\n"));

        delete wrapper;
    }

    if (jhoveExitCode > INT_MIN) {
        /// insert data from jHove
        metaText.append(QString(QStringLiteral("<jhove exitcode=\"%1\" wellformed=\"%2\" valid=\"%3\" pdf=\"%4\"")).arg(QString::number(jhoveExitCode), jhovePDFWellformed ? QStringLiteral("yes") : QStringLiteral("no"), jhovePDFValid ? QStringLiteral("yes") : QStringLiteral("no"), jhoveIsPDF ? QStringLiteral("yes") : QStringLiteral("no")));
        if (jhovePDFversion.isEmpty() && jhovePDFprofile.isEmpty() && jhoveStandardOutput.isEmpty() && jhoveErrorOutput.isEmpty())
            metaText.append(QStringLiteral(" />\n"));
        else {
            metaText.append(QStringLiteral(">\n"));
            if (!jhovePDFversion.isEmpty())
                metaText.append(QString(QStringLiteral("<version>%1</version>\n")).arg(DocScan::xmlify(jhovePDFversion)));
            if (!jhovePDFprofile.isEmpty()) {
                const bool isPDFA1a = jhovePDFprofile.contains(QStringLiteral("ISO PDF/A-1, Level A"));
                const bool isPDFA1b = isPDFA1a || jhovePDFprofile.contains(QStringLiteral("ISO PDF/A-1, Level B"));
                metaText.append(QString(QStringLiteral("<profile linear=\"%2\" tagged=\"%3\" pdfa1a=\"%4\" pdfa1b=\"%5\" pdfx3=\"%6\">%1</profile>\n")).arg(DocScan::xmlify(jhovePDFprofile), jhovePDFprofile.contains(QStringLiteral("Linearized PDF")) ? QStringLiteral("yes") : QStringLiteral("no"), jhovePDFprofile.contains(QStringLiteral("Tagged PDF")) ? QStringLiteral("yes") : QStringLiteral("no"), isPDFA1a ? QStringLiteral("yes") : QStringLiteral("no"), isPDFA1b ? QStringLiteral("yes") : QStringLiteral("no"), jhovePDFprofile.contains(QStringLiteral("ISO PDF/X-3")) ? QStringLiteral("yes") : QStringLiteral("no")));
            }
            /*
            if (!jhoveStandardOutput.isEmpty())
                metaText.append(QString(QStringLiteral("<output>%1</output>\n")).arg(DocScan::xmlify(jhoveStandardOutput.replace(QStringLiteral("###"), QStringLiteral("\n")))));
            */
            if (!jhoveErrorOutput.isEmpty())
                metaText.append(QString(QStringLiteral("<error>%1</error>\n")).arg(DocScan::xmlify(jhoveErrorOutput.replace(QStringLiteral("###"), QStringLiteral("\n")))));
            metaText.append(QStringLiteral("</jhove>\n"));
        }
    } else if (!m_jhoveShellscript.isEmpty())
        metaText.append(QStringLiteral("<jhove><error>jHove failed to start or was never started</error></jhove>\n"));
    else
        metaText.append(QStringLiteral("<jhove><info>jHove not configured to run</info></jhove>\n"));

    if (veraPDFExitCode > INT_MIN) {
        /// insert XML data from veraPDF
        metaText.append(QString(QStringLiteral("<verapdf exitcode=\"%1\" filesize=\"%2\" pdfa1b=\"%3\" pdfa1a=\"%4\">\n")).arg(QString::number(veraPDFExitCode), QString::number(veraPDFfilesize), veraPDFIsPDFA1B ? QStringLiteral("yes") : QStringLiteral("no"), veraPDFIsPDFA1A ? QStringLiteral("yes") : QStringLiteral("no")));
        if (!veraPDFStandardOutput.isEmpty()) {
            /// Check for and omit XML header if it exists
            const int p = veraPDFStandardOutput.indexOf(QStringLiteral("?>"));
            metaText.append(p > 1 ? veraPDFStandardOutput.mid(veraPDFStandardOutput.indexOf(QStringLiteral("<"), p)) : veraPDFStandardOutput);
        } else if (!veraPDFErrorOutput.isEmpty())
            metaText.append(QString(QStringLiteral("<error>%1</error>\n")).arg(DocScan::xmlify(veraPDFErrorOutput)));
        metaText.append(QStringLiteral("</verapdf>\n"));
    } else if (!m_veraPDFcliTool.isEmpty())
        metaText.append(QStringLiteral("<verapdf><error>veraPDF failed to start or was never started</error></verapdf>\n"));
    else
        metaText.append(QStringLiteral("<verapdf><info>veraPDF not configured to run</info></verapdf>\n"));

    if (pdfboxValidatorExitCode > INT_MIN) {
        /// insert result from Apache's PDFBox
        metaText.append(QString(QStringLiteral("<pdfboxvalidator exitcode=\"%1\" pdfa1b=\"%2\">\n")).arg(QString::number(pdfboxValidatorExitCode), pdfboxValidatorValidPdf ? QStringLiteral("yes") : QStringLiteral("no")));
        if (!pdfboxValidatorStandardOutput.isEmpty())
            metaText.append(QString(QStringLiteral("<output>%1</output>\n")).arg(DocScan::xmlify(pdfboxValidatorStandardOutput)));
        else if (!pdfboxValidatorErrorOutput.isEmpty())
            metaText.append(QString(QStringLiteral("<error>%1</error>\n")).arg(DocScan::xmlify(pdfboxValidatorErrorOutput)));
        metaText.append(QStringLiteral("</pdfboxvalidator>\n"));
    } else if (!m_pdfboxValidatorJavaClass.isEmpty())
        metaText.append(QStringLiteral("<pdfboxvalidator><error>pdfbox Validator failed to start or was never started</error></pdfboxvalidator>\n"));
    else
        metaText.append(QStringLiteral("<pdfboxvalidator><info>pdfbox Validator not configured to run</info></pdfboxvalidator>\n"));

    if (callasPdfAPilotExitCode > INT_MIN) {
        const bool isPDFA1a = callasPdfAPilotPDFA1letter == 'a' && callasPdfAPilotCountErrors == 0 && callasPdfAPilotCountWarnings == 0;
        const bool isPDFA1b = isPDFA1a || (callasPdfAPilotPDFA1letter == 'b' && callasPdfAPilotCountErrors == 0 && callasPdfAPilotCountWarnings == 0);
        metaText.append(QString(QStringLiteral("<callaspdfapilot exitcode=\"%1\" pdfa1b=\"%2\" pdfa1a=\"%3\">\n")).arg(QString::number(callasPdfAPilotExitCode), isPDFA1b ? QStringLiteral("yes") : QStringLiteral("no"), isPDFA1a ? QStringLiteral("yes") : QStringLiteral("no")));
        if (!callasPdfAPilotStandardOutput.isEmpty())
            metaText.append(DocScan::xmlify(callasPdfAPilotStandardOutput));
        else if (!callasPdfAPilotErrorOutput.isEmpty())
            metaText.append(QString(QStringLiteral("<error>%1</error>\n")).arg(DocScan::xmlify(callasPdfAPilotErrorOutput)));
        metaText.append(QStringLiteral("</callaspdfapilot>"));
    } else if (!m_callasPdfAPilotCLI.isEmpty())
        metaText.append(QStringLiteral("<callaspdfapilot><error>callas PDF/A Pilot failed to start or was never started</error></callaspdfapilot>\n"));
    else
        metaText.append(QStringLiteral("<callaspdfapilot><info>callas PDF/A Pilot not configured to run</info></callaspdfapilot>\n"));

    /// file information including size
    const QFileInfo fi = QFileInfo(filename);
    metaText.append(QString(QStringLiteral("<file size=\"%1\" />\n")).arg(fi.size()));

    if (!metaText.isEmpty())
        logText.append(QStringLiteral("<meta>\n")).append(metaText).append(QStringLiteral("</meta>\n"));
    const qint64 endTime = QDateTime::currentMSecsSinceEpoch();

    logText.prepend(QString(QStringLiteral("<fileanalysis filename=\"%1\" status=\"ok\" time=\"%2\" external_time=\"%3\">\n")).arg(DocScan::xmlify(filename), QString::number(endTime - startTime), QString::number(externalProgramsEndTime - startTime)));
    logText += QStringLiteral("</fileanalysis>\n");

    if (popplerWrapperOk || jhoveIsPDF || pdfboxValidatorValidPdf)
        /// At least one tool thought the file was ok
        emit analysisReport(logText);
    else
        /// No tool could handle this file, so give error message
        emit analysisReport(QString(QStringLiteral("<fileanalysis filename=\"%1\" message=\"invalid-fileformat\" status=\"error\" external_time=\"%2\"><meta><file size=\"%3\" /></meta></fileanalysis>\n")).arg(filename, QString::number(externalProgramsEndTime - startTime)).arg(fi.size()));

    m_isAlive = false;
}
