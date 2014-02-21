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

#include <QString>
#include <QStringList>
#include <QHash>
#include <QVector>
#include <QRegExp>

#include "guessing.h"
#include "general.h"

Guessing::Guessing()
{
    /// nothing
}

QString Guessing::fontToXML(const QString &fontName, const QString &typeName)
{
    QHash<QString, QString> name, beautifiedName, license, technology;
    name[""] = fontName;

    if (fontName.contains("Libertine")) {
        license["type"] = "open";
        license["name"] = "SIL Open Font License;GNU General Public License";
    } else if (fontName.contains("Nimbus")) {
        license["type"] = "open"; // URW++, released as GPL and AFPL
        license["name"] = "GNU General Public License;Aladdin Free Public License";
    } else if (fontName.startsWith("URWPalladio")) {
        license["type"] = "open"; // URW++, released as GPL and AFPL
    } else if (fontName.contains("Liberation")) {
        license["type"] = "open";
    } else if (fontName.contains("DejaVu")) {
        license["type"] = "open";
    } else if (fontName.contains("Ubuntu")) {
        license["type"] = "open";
        license["name"] = "Ubuntu Font Licence";
    } else if (fontName.contains("Gentium")) {
        license["type"] = "open";
    } else if (fontName.startsWith("FreeSans") || fontName.startsWith("FreeSerif") || fontName.startsWith("FreeMono")) {
        license["type"] = "open";
    } else if (fontName.contains("Vera") || fontName.contains("Bera")) {
        license["type"] = "open";
    } else if (fontName.contains("Computer Modern")) { // TeX fonts
        license["type"] = "open";
        license["name"] = "SIL Open Font License";
    } else if (fontName.startsWith("wasy") || fontName.contains(QRegExp(QLatin1String("^(CM|SF|MS)[A-Z]+[0-9]+$")))) { // TeX fonts
        license["type"] = "open";
        license["name"] = "SIL Open Font License";
    } else if (fontName.contains("Marvosym")) {
        license["type"] = "open";
        license["name"] = "SIL Open Font License";
    } else if (fontName.contains("OpenSymbol")) {
        license["type"] = "open";
        license["name"] = "LGPLv3?";
    } else if (fontName.startsWith("MnSymbol")) {
        license["type"] = "open";
        license["name"] = "PD";
    } else if (fontName.startsWith("Antenna")) {
        license["type"] = "proprietary"; // Font Bureau
    } else if (fontName.startsWith("Gotham") || fontName.startsWith("NewLibrisSerif")) {
        license["type"] = "proprietary"; // fonts used by Forsakringskassan
    } else if (fontName.startsWith("Zapf") || fontName.startsWith("Frutiger")) {
        license["type"] = "proprietary";
    } else if (fontName.startsWith("Arial") || fontName.startsWith("Verdana") || fontName.startsWith("TimesNewRoman") || fontName.startsWith("CourierNew") || fontName.startsWith("Georgia") || fontName == QLatin1String("Symbol")) {
        license["type"] = "proprietary"; // Microsoft
    } else if (fontName.startsWith("Lucinda") || fontName.startsWith("Trebuchet") || fontName.startsWith("Franklin Gothic") || fontName.startsWith("Century Schoolbook") || fontName.startsWith("CenturySchoolbook")) {
        license["type"] = "proprietary"; // Microsoft
    } else if (fontName.startsWith("Calibri") || fontName.startsWith("Cambria")  || fontName.startsWith("Constantia") || fontName.startsWith("Candara") || fontName.startsWith("Corbel") || fontName.startsWith("Consolas")) {
        license["type"] = "proprietary"; // Microsoft ClearType Font Collection
    } else if (fontName.startsWith("Futura") || fontName.startsWith("NewCenturySchlbk") || fontName.startsWith("TradeGothic") || fontName.startsWith("Univers") || fontName.contains("Palatino")) {
        license["type"] = "proprietary"; // Linotype
    } else if (fontName.contains("Monospace821") || fontName.contains("Swiss721") || fontName.contains("Dutch801")) {
        license["type"] = "proprietary"; // Bitstream
    } else if (fontName.contains("Helvetica") && fontName.contains("Neue")) {
        license["type"] = "proprietary"; // Neue Helvetica by Linotype
    } else if (fontName.startsWith("Times") || fontName.startsWith("Tahoma") || fontName.contains("Helvetica") || fontName.contains("Wingdings")) {
        license["type"] = "proprietary";
    } else if (fontName.startsWith("SymbolMT")) {
        license["type"] = "proprietary"; // MonoType's font as shipped with Windows
    } else if (fontName.startsWith("CenturyGothic") || fontName.startsWith("Bembo") || fontName.startsWith("GillSans") ||  fontName.startsWith("Rockwell") || fontName.startsWith("Lucida") || fontName.startsWith("Perpetua")) {
        license["type"] = "proprietary"; // MonoType
    } else if (fontName.startsWith("ACaslon") || fontName.contains("EuroSans") || fontName.startsWith("Minion") || fontName.startsWith("Myriad")) {
        license["type"] = "proprietary"; // Adobe
    } else if (fontName.startsWith("DIN")) {
        license["type"] = "proprietary"; // FF?
    } else if (fontName.contains("Officina") || fontName.contains("Kabel") || fontName.contains("Cheltenham")) {
        license["type"] = "proprietary"; // ITC
    } else if (fontName.startsWith("Bookman Old Style") || fontName.startsWith("Gill Sans")) {
        license["type"] = "proprietary"; // multiple alternatives
    } else
        license["type"] = "unknown";
    // "Courier" could be either. More fonts?

    QString bName = fontName;
    bool bNameChanged = true;
    while (bNameChanged) {
        const QString bNameOriginal = bName;
        static const QStringList suffixes = QStringList()
                                            << QLatin1String("MT") << QLatin1String("PS") << QLatin1String("BT") << QLatin1String("Bk")
                                            << QLatin1String("-Normal") << QLatin1String("-Book") << QLatin1String("-Md") << QLatin1String("-Medium") << QLatin1String("-Caps") << QLatin1String("-Roman") << QLatin1String("-Roma") << QLatin1String("-Regular") << QLatin1String("-Regu") << QLatin1String("-DisplayRegular")
                                            << QLatin1String("-Demi") << QLatin1String("-Blk") << QLatin1String("-Black") << QLatin1String("Bla") << QLatin1String("-Ultra") << QLatin1String("-Extra") << QLatin1String("-ExtraBold") << QLatin1String("Obl") << QLatin1String("-Hv") << QLatin1String("-HvIt") << QLatin1String("-Heavy") << QLatin1String("-BoldIt") << QLatin1String("-BoldItal") << QLatin1String("-BdIt") << QLatin1String("-Bd") << QLatin1String("-It")
                                            << QLatin1String("-Condensed") << QLatin1String("-Light") << QLatin1String("-Lt") << QLatin1String("-Slant") << QLatin1String("-LightCond") << QLatin1String("Lig") << QLatin1String("-Narrow")
                                            << QLatin1String("Ext") << QLatin1String("SWA") << QLatin1String("-Identity-H") << QLatin1String("-DTC");
        foreach(const QString &suffix, suffixes) {
            if (bName.endsWith(suffix))
                bName = bName.left(bName.length() - suffix.length());
        }
        static const QVector<QRegExp> suffixesRegExp = QVector<QRegExp>()
                << QRegExp(QLatin1String("[,-]?(Ital(ic)?|Oblique|Black|Bold)$"))
                << QRegExp(QLatin1String("[,-](BdCn|SC)[0-9]*$")) << QRegExp(QLatin1String("[,-][A-Z][0-9]$")) << QRegExp(QLatin1String("_[0-9]+$"))
                << QRegExp(QLatin1String("[+][A-Z]+$"))
                << QRegExp(QLatin1String("[*][0-9]+$"));
        foreach(const QRegExp &suffix, suffixesRegExp) {
            bName.remove(suffix);
        }
        static const QRegExp teXFonts = QRegExp(QLatin1String("^((CM|SF|MS)[A-Z]+|wasy)([0-9]+)$"));
        bName.replace(teXFonts, QLatin1String("\\1"));
        bNameChanged = bName != bNameOriginal;
    }
    beautifiedName[""] = DocScan::xmlify(bName);

    QString text = typeName.toLower();
    if (text.indexOf("truetype") >= 0)
        technology["type"] = "truetype";
    else if (text.indexOf("type1") >= 0)
        technology["type"] = "type1";
    else if (text.indexOf("type3") >= 0)
        technology["type"] = "type3";

    const QString result = DocScan::formatMap("name", name) + DocScan::formatMap("beautified", beautifiedName) + DocScan::formatMap("technology", technology) + DocScan::formatMap("license", license);

    return result;
}

QString Guessing::programToXML(const QString &program) {
    const QString text = program.toLower();
    QHash<QString, QString> xml;
    xml[""] = program;
    bool checkOOoVersion = false;

    if (text.indexOf("dvips") >= 0) {
        static const QRegExp radicaleyeVersion("\\b\\d+\\.\\d+[a-z]*\\b");
        xml["manufacturer"] = "radicaleye";
        if (radicaleyeVersion.indexIn(text) >= 0)
            xml["version"] = radicaleyeVersion.cap(0);
    } else if (text.indexOf("ghostscript") >= 0) {
        static const QRegExp ghostscriptVersion("\\b\\d+(\\.\\d+)+\\b");
        xml["manufacturer"] = "artifex ";
        xml["product"] = "ghostscript";
        if (ghostscriptVersion.indexIn(text) >= 0)
            xml["version"] = ghostscriptVersion.cap(0);
    } else if (text.startsWith("cairo ")) {
        static const QRegExp cairoVersion("\\b\\d+(\\.\\d+)+\\b");
        xml["manufacturer"] = "cairo ";
        xml["product"] = "cairo";
        if (cairoVersion.indexIn(text) >= 0)
            xml["version"] = cairoVersion.cap(0);
    } else if (text.indexOf("pdftex") >= 0) {
        static const QRegExp pdftexVersion("\\d+(\\.\\d+)+\\b");
        xml["manufacturer"] = "pdftex ";
        xml["product"] = "pdftex";
        if (pdftexVersion.indexIn(text) >= 0)
            xml["version"] = pdftexVersion.cap(0);
    } else if (text.indexOf("latex") >= 0) {
        xml["manufacturer"] = "latex ";
        xml["product"] = "latex";
    } else if (text.indexOf("dvipdfm") >= 0) {
        static const QRegExp dvipdfmVersion("\\b\\d+(\\.\\d+)+[a-z]*\\b");
        xml["manufacturer"] = "dvipdfm ";
        xml["product"] = "dvipdfm";
        if (dvipdfmVersion.indexIn(text) >= 0)
            xml["version"] = dvipdfmVersion.cap(0);
    } else if (text.indexOf("tex output") >= 0) {
        static const QRegExp texVersion("\\b\\d+([.:]\\d+)+\\b");
        xml["manufacturer"] = "tex ";
        xml["product"] = "tex";
        if (texVersion.indexIn(text) >= 0)
            xml["version"] = texVersion.cap(0);
    } else if (text.indexOf("koffice") >= 0) {
        static const QRegExp kofficeVersion("/(d+([.]\\d+)*)\\b");
        xml["manufacturer"] = "kde";
        xml["product"] = "koffice";
        if (kofficeVersion.indexIn(text) >= 0)
            xml["version"] = kofficeVersion.cap(1);
    } else if (text.indexOf("calligra") >= 0) {
        static const QRegExp calligraVersion("/(d+([.]\\d+)*)\\b");
        xml["manufacturer"] = "kde";
        xml["product"] = "calligra";
        if (calligraVersion.indexIn(text) >= 0)
            xml["version"] = calligraVersion.cap(1);
    } else if (text.indexOf("abiword") >= 0) {
        xml["manufacturer"] = "abisource";
        xml["product"] = "abiword";
    } else if (text.indexOf("office_one") >= 0) {
        checkOOoVersion = true;
        xml["product"] = "office_one";
        xml["based-on"] = "openoffice";
    } else if (text.indexOf("infraoffice") >= 0) {
        checkOOoVersion = true;
        xml["product"] = "infraoffice";
        xml["based-on"] = "openoffice";
    } else if (text.indexOf("aksharnaveen") >= 0) {
        checkOOoVersion = true;
        xml["product"] = "aksharnaveen";
        xml["based-on"] = "openoffice";
    } else if (text.indexOf("redoffice") >= 0) {
        checkOOoVersion = true;
        xml["manufacturer"] = "china";
        xml["product"] = "redoffice";
        xml["based-on"] = "openoffice";
    } else if (text.indexOf("sun_odf_plugin") >= 0) {
        checkOOoVersion = true;
        xml["manufacturer"] = "oracle";
        xml["product"] = "odfplugin";
        xml["based-on"] = "openoffice";
    } else if (text.indexOf("libreoffice") >= 0) {
        checkOOoVersion = true;
        xml["manufacturer"] = "tdf";
        xml["product"] = "libreoffice";
        xml["based-on"] = "openoffice";
    }  else if (text.indexOf("lotus symphony") >= 0) {
        static const QRegExp lotusSymphonyVersion("Symphony (\\d+(\\.\\d+)*)");
        xml["manufacturer"] = "ibm";
        xml["product"] = "lotus-symphony";
        xml["based-on"] = "openoffice";
        if (lotusSymphonyVersion.indexIn(text) >= 0)
            xml["version"] = lotusSymphonyVersion.cap(1);
    }  else if (text.indexOf("Lotus_Symphony") >= 0) {
        checkOOoVersion = true;
        xml["manufacturer"] = "ibm";
        xml["product"] = "lotus-symphony";
        xml["based-on"] = "openoffice";
    } else if (text.indexOf("openoffice") >= 0) {
        checkOOoVersion = true;
        if (text.indexOf("staroffice") >= 0) {
            xml["manufacturer"] = "oracle";
            xml["based-on"] = "openoffice";
            xml["product"] = "staroffice";
        } else if (text.indexOf("broffice") >= 0) {
            xml["product"] = "broffice";
            xml["based-on"] = "openoffice";
        } else if (text.indexOf("neooffice") >= 0) {
            xml["manufacturer"] = "planamesa";
            xml["product"] = "neooffice";
            xml["based-on"] = "openoffice";
        } else {
            xml["manufacturer"] = "oracle";
            xml["product"] = "openoffice";
        }
    } else if (text == QLatin1String("writer") || text == QLatin1String("calc") || text == QLatin1String("impress")) {
        /// for Creator/Editor string
        xml["manufacturer"] = "oracle;tdf";
        xml["product"] = "openoffice;libreoffice";
        xml["based-on"] = "openoffice";
    } else if (text.startsWith("pdfscanlib ")) {
        static const QRegExp pdfscanlibVersion("v(\\d+(\\.\\d+)+)\\b");
        xml["manufacturer"] = "kodak?";
        xml["product"] = "pdfscanlib";
        if (pdfscanlibVersion.indexIn(text) >= 0)
            xml["version"] = pdfscanlibVersion.cap(1);
    } else if (text.indexOf("framemaker") >= 0) {
        static const QRegExp framemakerVersion("\\b\\d+(\\.\\d+)+(\\b|\\.|p\\d+)");
        xml["manufacturer"] = "adobe";
        xml["product"] = "framemaker";
        if (framemakerVersion.indexIn(text) >= 0)
            xml["version"] = framemakerVersion.cap(0);
    } else if (text.contains("distiller")) {
        static const QRegExp distillerVersion("\\b\\d+(\\.\\d+)+\\b");
        xml["manufacturer"] = "adobe";
        xml["product"] = "distiller";
        if (distillerVersion.indexIn(text) >= 0)
            xml["version"] = distillerVersion.cap(0);
    } else if (text.startsWith("pdflib plop")) {
        static const QRegExp plopVersion("\\b\\d+(\\.\\d+)+\\b");
        xml["manufacturer"] = "pdflib";
        xml["product"] = "plop";
        if (plopVersion.indexIn(text) >= 0)
            xml["version"] = plopVersion.cap(0);
    } else if (text.startsWith("pdflib")) {
        static const QRegExp pdflibVersion("\\b\\d+(\\.[0-9p]+)+\\b");
        xml["manufacturer"] = "pdflib";
        xml["product"] = "pdflib";
        if (pdflibVersion.indexIn(text) >= 0)
            xml["version"] = pdflibVersion.cap(0);
    } else if (text.indexOf("pdf library") >= 0) {
        static const QRegExp pdflibraryVersion("\\b\\d+(\\.\\d+)+\\b");
        xml["manufacturer"] = "adobe";
        xml["product"] = "pdflibrary";
        if (pdflibraryVersion.indexIn(text) >= 0)
            xml["version"] = pdflibraryVersion.cap(0);
    } else if (text.indexOf("pdfwriter") >= 0) {
        static const QRegExp pdfwriterVersion("\\b\\d+(\\.\\d+)+\\b");
        xml["manufacturer"] = "adobe";
        xml["product"] = "pdfwriter";
        if (pdfwriterVersion.indexIn(text) >= 0)
            xml["version"] = pdfwriterVersion.cap(0);
    } else if (text.indexOf("easypdf") >= 0) {
        static const QRegExp easypdfVersion("\\b\\d+(\\.\\d+)+\\b");
        xml["manufacturer"] = "bcl";
        xml["product"] = "easypdf";
        if (easypdfVersion.indexIn(text) >= 0)
            xml["version"] = easypdfVersion.cap(0);
    } else if (text.contains("pdfmaker")) {
        static const QRegExp pdfmakerVersion("\\b\\d+(\\.\\d+)+\\b");
        xml["manufacturer"] = "adobe";
        xml["product"] = "pdfmaker";
        if (pdfmakerVersion.indexIn(text) >= 0)
            xml["version"] = pdfmakerVersion.cap(0);
    } else if (text.startsWith("fill-in ")) {
        static const QRegExp fillInVersion("\\b\\d+(\\.\\d+)+\\b");
        xml["manufacturer"] = "textcenter";
        xml["product"] = "fill-in";
        if (fillInVersion.indexIn(text) >= 0)
            xml["version"] = fillInVersion.cap(0);
    } else if (text.startsWith("itext ")) {
        static const QRegExp iTextVersion("\\b((\\d+)(\\.\\d+)+)\\b");
        xml["manufacturer"] = "itext";
        xml["product"] = "itext";
        if (iTextVersion.indexIn(text) >= 0) {
            xml["version"] = iTextVersion.cap(0);
            bool ok = false;
            const int majorVersion = iTextVersion.cap(2).toInt(&ok);
            if (ok && majorVersion > 0) {
                if (majorVersion <= 4)
                    xml["license"] = "MPL;LGPL";
                else if (majorVersion >= 5)
                    xml["license"] = "commercial;AGPLv3";
            }
        }
    } else if (text.startsWith("amyuni pdf converter ")) {
        static const QRegExp amyunitVersion("\\b\\d+(\\.\\d+)+\\b");
        xml["manufacturer"] = "amyuni";
        xml["product"] = "pdfconverter";
        if (amyunitVersion.indexIn(text) >= 0)
            xml["version"] = amyunitVersion.cap(0);
    } else if (text.indexOf("pdfout v") >= 0) {
        static const QRegExp pdfoutVersion("v(\\d+(\\.\\d+)+)\\b");
        xml["manufacturer"] = "verypdf";
        xml["product"] = "docconverter";
        if (pdfoutVersion.indexIn(text) >= 0)
            xml["version"] = pdfoutVersion.cap(1);
    } else if (text.indexOf("jaws pdf creator") >= 0) {
        static const QRegExp pdfcreatorVersion("v(\\d+(\\.\\d+)+)\\b");
        xml["manufacturer"] = "jaws";
        xml["product"] = "pdfcreator";
        if (pdfcreatorVersion.indexIn(text) >= 0)
            xml["version"] = pdfcreatorVersion.cap(1);
    } else if (text.startsWith("arbortext ")) {
        static const QRegExp arbortextVersion("\\d+(\\.\\d+)+)");
        xml["manufacturer"] = "ptc";
        xml["product"] = "arbortext";
        if (arbortextVersion.indexIn(text) >= 0)
            xml["version"] = arbortextVersion.cap(0);
    } else if (text.contains("3b2")) {
        static const QRegExp threeB2Version("\\d+(\\.[0-9a-z]+)+)");
        xml["manufacturer"] = "ptc";
        xml["product"] = "3b2";
        if (threeB2Version.indexIn(text) >= 0)
            xml["version"] = threeB2Version.cap(0);
    } else if (text.startsWith("3-heights")) {
        static const QRegExp threeHeightsVersion("\\b\\d+(\\.\\d+)+)");
        xml["manufacturer"] = "pdftoolsag";
        xml["product"] = "3-heights";
        if (threeHeightsVersion.indexIn(text) >= 0)
            xml["version"] = threeHeightsVersion.cap(0);
    } else if (text.contains("abcpdf")) {
        xml["manufacturer"] = "websupergoo";
        xml["product"] = "abcpdf";
    } else if (text.indexOf("primopdf") >= 0) {
        xml["manufacturer"] = "nitro";
        xml["product"] = "primopdf";
        xml["based-on"] = "nitropro";
    } else if (text.indexOf("nitro") >= 0) {
        xml["manufacturer"] = "nitro";
        xml["product"] = "nitropro";
    } else if (text.indexOf("pdffactory") >= 0) {
        static const QRegExp pdffactoryVersion("\\b\\d+(\\.\\d+)+\\b");
        xml["manufacturer"] = "softwarelabs";
        xml["product"] = "pdffactory";
        if (pdffactoryVersion.indexIn(text) >= 0)
            xml["version"] = pdffactoryVersion.cap(0);
    } else if (text.startsWith("ibex pdf")) {
        static const QRegExp ibexVersion("\\b\\d+(\\.\\[0-9/]+)+\\b");
        xml["manufacturer"] = "visualprogramming";
        xml["product"] = "ibexpdfcreator";
        if (ibexVersion.indexIn(text) >= 0)
            xml["version"] = ibexVersion.cap(0);
    } else if (text.startsWith("arc/info") || text.startsWith("arcinfo")) {
        static const QRegExp arcinfoVersion("\\b\\d+(\\.\\d+)+\\b");
        xml["manufacturer"] = "esri";
        xml["product"] = "arcinfo";
        if (arcinfoVersion.indexIn(text) >= 0)
            xml["version"] = arcinfoVersion.cap(0);
    } else if (text.startsWith("paperport ")) {
        static const QRegExp paperportVersion("\\b\\d+(\\.\\d+)+\\b");
        xml["manufacturer"] = "nuance";
        xml["product"] = "paperport";
        if (paperportVersion.indexIn(text) >= 0)
            xml["version"] = paperportVersion.cap(0);
    } else if (text.indexOf("indesign") >= 0) {
        static const QRegExp indesignVersion("\\b\\d+(\\.\\d+)+\\b");
        xml["manufacturer"] = "adobe";
        xml["product"] = "indesign";
        if (indesignVersion.indexIn(text) >= 0)
            xml["version"] = indesignVersion.cap(0);
        else {
            static const QRegExp csVersion(QLatin1String("\\bCS(\\d*)\\b"));
            if (csVersion.indexIn(text) >= 0) {
                bool ok = false;
                double versionNumber = csVersion.cap(1).toDouble(&ok);
                if (csVersion.cap(0) == QLatin1String("CS"))
                    xml["version"] = QLatin1String("3.0");
                else if (ok && versionNumber > 1) {
                    versionNumber += 2;
                    xml["version"] = QString::number(versionNumber, 'f', 1);
                }
            }
        }
    } else if (text.indexOf("illustrator") >= 0) {
        static const QRegExp illustratorVersion("\\b\\d+(\\.\\d+)+\\b");
        xml["manufacturer"] = "adobe";
        xml["product"] = "illustrator";
        if (illustratorVersion.indexIn(text) >= 0)
            xml["version"] = illustratorVersion.cap(0);
        else {
            static const QRegExp csVersion(QLatin1String("\\bCS(\\d*)\\b"));
            if (csVersion.indexIn(text) >= 0) {
                bool ok = false;
                double versionNumber = csVersion.cap(1).toDouble(&ok);
                if (csVersion.cap(0) == QLatin1String("CS"))
                    xml["version"] = QLatin1String("11.0");
                else if (ok && versionNumber > 1) {
                    versionNumber += 10;
                    xml["version"] = QString::number(versionNumber, 'f', 1);
                }
            }
        }
    } else if (text.indexOf("pagemaker") >= 0) {
        static const QRegExp pagemakerVersion("\\b\\d+(\\.\\d+)+\\b");
        xml["manufacturer"] = "adobe";
        xml["product"] = "pagemaker";
        if (pagemakerVersion.indexIn(text) >= 0)
            xml["version"] = pagemakerVersion.cap(0);
    } else if (text.indexOf("acrobat capture") >= 0) {
        static const QRegExp acrobatCaptureVersion("\\b\\d+(\\.\\d+)+\\b");
        xml["manufacturer"] = "adobe";
        xml["product"] = "acrobatcapture";
        if (acrobatCaptureVersion.indexIn(text) >= 0)
            xml["version"] = acrobatCaptureVersion.cap(0);
    } else if (text.indexOf("acrobat pro") >= 0) {
        static const QRegExp acrobatProVersion("\\b\\d+(\\.\\d+)+\\b");
        xml["manufacturer"] = "adobe";
        xml["product"] = "acrobatpro";
        if (acrobatProVersion.indexIn(text) >= 0)
            xml["version"] = acrobatProVersion.cap(0);
    } else if (text.indexOf("acrobat") >= 0) {
        static const QRegExp acrobatVersion("\\b\\d+(\\.\\d+)+\\b");
        xml["manufacturer"] = "adobe";
        xml["product"] = "acrobat";
        if (acrobatVersion.indexIn(text) >= 0)
            xml["version"] = acrobatVersion.cap(0);
    } else if (text.indexOf("livecycle") >= 0) {
        static const QRegExp livecycleVersion("\\b\\d+(\\.\\d+)+[a-z]?\\b");
        xml["manufacturer"] = "adobe";
        int regExpPos;
        if ((regExpPos = livecycleVersion.indexIn(text)) >= 0)
            xml["version"] = livecycleVersion.cap(0);
        if (regExpPos <= 0)
            regExpPos = 1024;
        QString product = text;
        xml["product"] = product.left(regExpPos - 1).replace("adobe", "").replace(livecycleVersion.cap(0), "").replace(" ", "") + QLatin1Char('?');
    } else if (text.startsWith("adobe photoshop elements")) {
        xml["manufacturer"] = "adobe";
        xml["product"] = "photoshopelements";
    } else if (text.startsWith("adobe photoshop")) {
        static const QRegExp photoshopVersion("\\bCS|(CS)?\\d+(\\.\\d+)+\\b");
        xml["manufacturer"] = "adobe";
        xml["product"] = "photoshop";
        if (photoshopVersion.indexIn(text) >= 0)
            xml["version"] = photoshopVersion.cap(0);
    } else if (text.indexOf("adobe") >= 0) {
        /// some unknown Adobe product
        static const QRegExp adobeVersion("\\b\\d+(\\.\\d+)+\\b");
        xml["manufacturer"] = "adobe";
        if (adobeVersion.indexIn(text) >= 0)
            xml["version"] = adobeVersion.cap(0);
        QString product = text;
        xml["product"] = product.replace("adobe", "").replace(adobeVersion.cap(0), "").replace(" ", "") + QLatin1Char('?');
    } else if (text.contains("pages")) {
        xml["manufacturer"] = "apple";
        xml["product"] = "pages";
    } else if (text.contains("keynote")) {
        static const QRegExp keynoteVersion("\\b\\d+(\\.\\d+)+\\b");
        xml["manufacturer"] = "apple";
        xml["product"] = "keynote";
        if (keynoteVersion.indexIn(text) >= 0)
            xml["version"] = keynoteVersion.cap(0);
    } else if (text.indexOf("quartz") >= 0) {
        static const QRegExp quartzVersion("\\b\\d+(\\.\\d+)+\\b");
        xml["manufacturer"] = "apple";
        xml["product"] = "quartz";
        if (quartzVersion.indexIn(text) >= 0)
            xml["version"] = quartzVersion.cap(0);
    } else if (text.indexOf("pscript5.dll") >= 0 || text.indexOf("pscript.dll") >= 0) {
        static const QRegExp pscriptVersion("\\b\\d+(\\.\\d+)+\\b");
        xml["manufacturer"] = "microsoft";
        xml["product"] = "pscript";
        xml["opsys"] = QLatin1String("windows");
        if (pscriptVersion.indexIn(text) >= 0)
            xml["version"] = pscriptVersion.cap(0);
    } else if (text.indexOf("quarkxpress") >= 0) {
        static const QRegExp quarkxpressVersion("\\b\\d+(\\.\\d+)+\\b");
        xml["manufacturer"] = "quark";
        xml["product"] = "xpress";
        if (quarkxpressVersion.indexIn(text) >= 0)
            xml["version"] = quarkxpressVersion.cap(0);
    } else if (text.indexOf("pdfcreator") >= 0) {
        static const QRegExp pdfcreatorVersion("\\b\\d+(\\.\\d+)+\\b");
        xml["manufacturer"] = "pdfforge";
        xml["product"] = "pdfcreator";
        xml["opsys"] = QLatin1String("windows");
        if (pdfcreatorVersion.indexIn(text) >= 0)
            xml["version"] = pdfcreatorVersion.cap(0);
    } else if (text.startsWith("stamppdf batch")) {
        static const QRegExp stamppdfbatchVersion("\\b\\d+(\\.\\d+)+\\b");
        xml["manufacturer"] = "appligent";
        xml["product"] = "stamppdfbatch";
        if (stamppdfbatchVersion.indexIn(text) >= 0)
            xml["version"] = stamppdfbatchVersion.cap(0);
    } else if (text.startsWith("xyenterprise ")) {
        static const QRegExp xyVersion("\b\\d+(\\.\\[0-9a-z])+)( patch \\S*\\d)\\b");
        xml["manufacturer"] = "dakota";
        xml["product"] = "xyenterprise";
        if (xyVersion.indexIn(text) >= 0)
            xml["version"] = xyVersion.cap(1);
    } else if (text.startsWith("edocprinter ")) {
        static const QRegExp edocprinterVersion("ver (\\d+(\\.\\d+)+)\\b");
        xml["manufacturer"] = "itek";
        xml["product"] = "edocprinter";
        if (edocprinterVersion.indexIn(text) >= 0)
            xml["version"] = edocprinterVersion.cap(1);
    } else if (text.startsWith("pdf code ")) {
        static const QRegExp pdfcodeVersion("\\b(\\d{8}}|d+(\\.\\d+)+)\\b");
        xml["manufacturer"] = "europeancommission";
        xml["product"] = "pdfcode";
        if (pdfcodeVersion.indexIn(text) >= 0)
            xml["version"] = pdfcodeVersion.cap(1);
    } else if (text.indexOf("pdf printer") >= 0) {
        xml["manufacturer"] = "bullzip";
        xml["product"] = "pdfprinter";
    } else if (text.contains("aspose") && text.contains("words")) {
        static const QRegExp asposewordsVersion("\\b\\d+(\\.\\d+)+\\b");
        xml["manufacturer"] = "aspose";
        xml["product"] = "aspose.words";
        if (asposewordsVersion.indexIn(text) >= 0)
            xml["version"] = asposewordsVersion.cap(0);
    } else if (text.contains("arcmap")) {
        static const QRegExp arcmapVersion("\\b\\d+(\\.\\d+)+\\b");
        xml["manufacturer"] = "esri";
        xml["product"] = "arcmap";
        if (arcmapVersion.indexIn(text) >= 0)
            xml["version"] = arcmapVersion.cap(0);
    } else if (text.contains("ocad")) {
        static const QRegExp ocadVersion("\\b\\d+(\\.\\d+)+\\b");
        xml["manufacturer"] = "ocad";
        xml["product"] = "ocad";
        if (ocadVersion.indexIn(text) >= 0)
            xml["version"] = ocadVersion.cap(0);
    } else if (text.contains("gnostice")) {
        static const QRegExp gnosticeVersion("\\b[v]?\\d+(\\.\\d+)+\\b");
        xml["manufacturer"] = "gnostice";
        if (gnosticeVersion.indexIn(text) >= 0)
            xml["version"] = gnosticeVersion.cap(0);
        QString product = text;
        xml["product"] = product.replace("gnostice", "").replace(gnosticeVersion.cap(0), "").replace(" ", "") + QLatin1Char('?');
    } else if (text.contains("canon")) {
        static const QRegExp canonVersion("\\b[v]?\\d+(\\.\\d+)+\\b");
        xml["manufacturer"] = "canon";
        if (canonVersion.indexIn(text) >= 0)
            xml["version"] = canonVersion.cap(0);
        QString product = text;
        xml["product"] = product.replace("canon", "").replace(canonVersion.cap(0), "").replace(" ", "") + QLatin1Char('?');
    } else if (text.startsWith("creo")) {
        xml["manufacturer"] = "creo";
        QString product = text;
        xml["product"] = product.replace("creo", "").replace(" ", "") + QLatin1Char('?');
    } else if (text.contains("apogee")) {
        xml["manufacturer"] = "agfa";
        xml["product"] = "apogee";
    } else if (text.contains("ricoh")) {
        xml["manufacturer"] = "ricoh";
        const int i = text.indexOf(QLatin1String("aficio"));
        if (i >= 0)
            xml["product"] = text.mid(i).replace(QLatin1Char(' '), QString::null);
    } else if (text.contains("toshiba") || text.contains("mfpimglib")) {
        static const QRegExp toshibaVersion("\\b[v]?\\d+(\\.\\d+)+\\b");
        xml["manufacturer"] = "toshiba";
        if (toshibaVersion.indexIn(text) >= 0)
            xml["version"] = toshibaVersion.cap(0);
        QString product = text;
        xml["product"] = product.replace("toshiba", "").replace(toshibaVersion.cap(0), "").replace(" ", "") + QLatin1Char('?');
    } else if (text.startsWith("hp ") || text.startsWith("hewlett packard ")) {
        xml["manufacturer"] = "hewlettpackard";
        QString product = text;
        xml["product"] = product.replace("hp ", "").replace("hewlett packard", "").replace(" ", "") + QLatin1Char('?');
    } else if (text.startsWith("xerox ")) {
        xml["manufacturer"] = "xerox";
        QString product = text;
        xml["product"] = product.replace("xerox ", "").replace(" ", "") + QLatin1Char('?');
    } else if (text.startsWith("kodak ")) {
        xml["manufacturer"] = "kodak";
        QString product = text;
        xml["product"] = product.replace("kodak ", "").replace("scanner: ", "").replace(" ", "") + QLatin1Char('?');
    } else if (text.contains("konica") || text.contains("minolta")) {
        static const QRegExp konicaMinoltaVersion("\\b[v]?\\d+(\\.\\d+)+\\b");
        xml["manufacturer"] = "konica;minolta";
        if (konicaMinoltaVersion.indexIn(text) >= 0)
            xml["version"] = konicaMinoltaVersion.cap(0);
        QString product = text;
        xml["product"] = product.replace("konica", "").replace("minolta", "").replace(konicaMinoltaVersion.cap(0), "").replace(" ", "") + QLatin1Char('?');
    } else if (text.contains("corel")) {
        static const QRegExp corelVersion("\\b[v]?\\d+(\\.\\d+)+\\b");
        xml["manufacturer"] = "corel";
        if (corelVersion.indexIn(text) >= 0)
            xml["version"] = corelVersion.cap(0);
        QString product = text;
        xml["product"] = product.replace("corel", "").replace(corelVersion.cap(0), "").replace(" ", "") + QLatin1Char('?');
    } else if (text.contains("scansoft pdf create")) {
        static const QRegExp scansoftVersion("\\b([a-zA-Z]+[ ])?[A-Za-z0-9]+\\b");
        xml["manufacturer"] = "scansoft";
        xml["product"] = "pdfcreate";
        if (scansoftVersion.indexIn(text) >= 0)
            xml["version"] = scansoftVersion.cap(0);
    } else if (text.contains("alivepdf")) {
        static const QRegExp alivepdfVersion("\\b\\d+(\\.\\d+)+( RC)?\\b");
        xml["manufacturer"] = "thibault.imbert";
        xml["product"] = "alivepdf";
        if (alivepdfVersion.indexIn(text) >= 0)
            xml["version"] = alivepdfVersion.cap(0);
        xml["opsys"] = QLatin1String("flash");
    } else if (text == QLatin1String("google")) {
        xml["manufacturer"] = "google";
        xml["product"] = "docs";
    } else if (!text.contains("words")) {
        static const QRegExp microsoftProducts("powerpoint|excel|word|outlook|visio|access");
        static const QRegExp microsoftVersion("\\b(starter )?(20[01][0-9]|1?[0-9]\\.[0-9]+|9[5-9])\\b");
        if (microsoftProducts.indexIn(text) >= 0) {
            xml["manufacturer"] = "microsoft";
            xml["product"] = microsoftProducts.cap(0);
            if (!xml.contains("version") && microsoftVersion.indexIn(text) >= 0)
                xml["version"] = microsoftVersion.cap(2);
            if (!xml.contains("subversion") && !microsoftVersion.cap(1).isEmpty())
                xml["subversion"] = microsoftVersion.cap(1);

            if (text.contains(QLatin1String("Macintosh")) || text.contains(QLatin1String("Mac OS X")))
                xml["opsys"] = QLatin1String("macosx");
            else
                xml["opsys"] = QLatin1String("windows?");
        }
    }

    if (checkOOoVersion) {
        /// Looks like "Win32/2.3.1"
        static const QRegExp OOoVersion1("[a-z]/(\\d(\\.\\d+)+)(_beta|pre)?[$a-z]", Qt::CaseInsensitive);
        if (OOoVersion1.indexIn(text) >= 0)
            xml["version"] = OOoVersion1.cap(1);
        else {
            /// Fallback: conventional version string like "3.0"
            static const QRegExp OOoVersion2("\\b(\\d+(\\.\\d+)+)\\b", Qt::CaseInsensitive);
            if (OOoVersion2.indexIn(text) >= 0)
                xml["version"] = OOoVersion2.cap(1);
        }

        if (text.indexOf(QLatin1String("unix")) >= 0)
            xml["opsys"] = QLatin1String("generic-unix");
        else if (text.indexOf(QLatin1String("linux")) >= 0)
            xml["opsys"] = QLatin1String("linux");
        else if (text.indexOf(QLatin1String("win32")) >= 0)
            xml["opsys"] = QLatin1String("windows");
        else if (text.indexOf(QLatin1String("solaris")) >= 0)
            xml["opsys"] = QLatin1String("solaris");
        else if (text.indexOf(QLatin1String("freebsd")) >= 0)
            xml["opsys"] = QLatin1String("bsd");
    }

    if (!xml.contains("manufacturer") && (text.contains("adobe") || text.contains("acrobat")))
        xml["manufacturer"] = "adobe";

    if (!xml.contains("opsys")) {
        /// automatically guess operating system
        if (text.contains(QLatin1String("macint")))
            xml["opsys"] = QLatin1String("macosx");
        else if (text.contains(QLatin1String("solaris")))
            xml["opsys"] = QLatin1String("solaris");
        else if (text.contains(QLatin1String("linux")))
            xml["opsys"] = QLatin1String("linux");
        else if (text.contains(QLatin1String("windows")) || text.contains(QLatin1String("win32")) || text.contains(QLatin1String("win64")))
            xml["opsys"] = QLatin1String("windows");
    }



    const QString result = DocScan::formatMap("name", xml);

    return result;
}