#include <QApplication>

#include "guiutil.h"

#include "eraaddressvalidator.h"
#include "walletmodel.h"
#include "eraunits.h"

#include "util.h"
#include "init.h"

#include <QDateTime>
#include <QDoubleValidator>
#include <QFont>
#include <QLineEdit>
#include <QUrl>
#include <QTextDocument> // For Qt::escape
#include <QAbstractItemView>
#include <QClipboard>
#include <QFileDialog>
#include <QDesktopServices>
#include <QThread>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#ifdef WIN32
#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT 0x0501
#ifdef _WIN32_IE
#undef _WIN32_IE
#endif
#define _WIN32_IE 0x0501
#define WIN32_LEAN_AND_MEAN 1
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "shlwapi.h"
#include "shlobj.h"
#include "shellapi.h"
#endif

namespace GUIUtil {

QString dateTimeStr(const QDateTime &date)
{
    return date.date().toString(Qt::SystemLocaleShortDate) + QString(" ") + date.toString("hh:mm");
}

QString dateTimeStr(qint64 nTime)
{
    return dateTimeStr(QDateTime::fromTime_t((qint32)nTime));
}

QFont eraAddressFont()
{
    QFont font("Monospace");
#if QT_VERSION >= 0x040800
    font.setStyleHint(QFont::Monospace);
#else
    font.setStyleHint(QFont::TypeWriter);
#endif
    return font;
}

void setupAddressWidget(QLineEdit *widget, QWidget *parent)
{
    widget->setMaxLength(EraAddressValidator::MaxAddressLength);
    widget->setValidator(new EraAddressValidator(parent));
    widget->setFont(eraAddressFont());
}

void setupAmountWidget(QLineEdit *widget, QWidget *parent)
{
    QDoubleValidator *amountValidator = new QDoubleValidator(parent);
    amountValidator->setDecimals(8);
    amountValidator->setBottom(0.0);
    widget->setValidator(amountValidator);
    widget->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
}

bool parseEraURI(const QUrl &uri, SendCoinsRecipient *out)
{
    // NovaCoin: check prefix
    if(uri.scheme() != QString("era"))
        return false;

    SendCoinsRecipient rv;
    rv.address = uri.path();
    rv.amount = 0;
    QList<QPair<QString, QString> > items = uri.queryItems();
    for (QList<QPair<QString, QString> >::iterator i = items.begin(); i != items.end(); i++)
    {
        bool fShouldReturnFalse = false;
        if (i->first.startsWith("req-"))
        {
            i->first.remove(0, 4);
            fShouldReturnFalse = true;
        }

        if (i->first == "label")
        {
            rv.label = i->second;
            fShouldReturnFalse = false;
        }
        else if (i->first == "amount")
        {
            if(!i->second.isEmpty())
            {
                if(!EraUnits::parse(EraUnits::ERA, i->second, &rv.amount))
                {
                    return false;
                }
            }
            fShouldReturnFalse = false;
        }

        if (fShouldReturnFalse)
            return false;
    }
    if(out)
    {
        *out = rv;
    }
    return true;
}

bool parseEraURI(QString uri, SendCoinsRecipient *out)
{
    // Convert era:// to era:
    //
    //    Cannot handle this later, because era:// will cause Qt to see the part after // as host,
    //    which will lower-case it (and thus invalidate the address).
    if(uri.startsWith("era://"))
    {
        uri.replace(0, 12, "era:");
    }
    QUrl uriInstance(uri);
    return parseEraURI(uriInstance, out);
}

QString HtmlEscape(const QString& str, bool fMultiLine)
{
    QString escaped = Qt::escape(str);
    if(fMultiLine)
    {
        escaped = escaped.replace("\n", "<br>\n");
    }
    return escaped;
}

QString HtmlEscape(const std::string& str, bool fMultiLine)
{
    return HtmlEscape(QString::fromStdString(str), fMultiLine);
}

void copyEntryData(QAbstractItemView *view, int column, int role)
{
    if(!view || !view->selectionModel())
        return;
    QModelIndexList selection = view->selectionModel()->selectedRows(column);

    if(!selection.isEmpty())
    {
        // Copy first item
        QApplication::clipboard()->setText(selection.at(0).data(role).toString());
    }
}

QString getSaveFileName(QWidget *parent, const QString &caption,
                                 const QString &dir,
                                 const QString &filter,
                                 QString *selectedSuffixOut)
{
    QString selectedFilter;
    QString myDir;
    if(dir.isEmpty()) // Default to user documents location
    {
        myDir = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
    }
    else
    {
        myDir = dir;
    }
    QString result = QFileDialog::getSaveFileName(parent, caption, myDir, filter, &selectedFilter);

    /* Extract first suffix from filter pattern "Description (*.foo)" or "Description (*.foo *.bar ...) */
    QRegExp filter_re(".* \\(\\*\\.(.*)[ \\)]");
    QString selectedSuffix;
    if(filter_re.exactMatch(selectedFilter))
    {
        selectedSuffix = filter_re.cap(1);
    }

    /* Add suffix if needed */
    QFileInfo info(result);
    if(!result.isEmpty())
    {
        if(info.suffix().isEmpty() && !selectedSuffix.isEmpty())
        {
            /* No suffix specified, add selected suffix */
            if(!result.endsWith("."))
                result.append(".");
            result.append(selectedSuffix);
        }
    }

    /* Return selected suffix if asked to */
    if(selectedSuffixOut)
    {
        *selectedSuffixOut = selectedSuffix;
    }
    return result;
}

Qt::ConnectionType blockingGUIThreadConnection()
{
    if(QThread::currentThread() != QCoreApplication::instance()->thread())
    {
        return Qt::BlockingQueuedConnection;
    }
    else
    {
        return Qt::DirectConnection;
    }
}

bool checkPoint(const QPoint &p, const QWidget *w)
{
    QWidget *atW = qApp->widgetAt(w->mapToGlobal(p));
    if (!atW) return false;
    return atW->topLevelWidget() == w;
}

bool isObscured(QWidget *w)
{
    return !(checkPoint(QPoint(0, 0), w)
        && checkPoint(QPoint(w->width() - 1, 0), w)
        && checkPoint(QPoint(0, w->height() - 1), w)
        && checkPoint(QPoint(w->width() - 1, w->height() - 1), w)
        && checkPoint(QPoint(w->width() / 2, w->height() / 2), w));
}

void openDebugLogfile()
{
    boost::filesystem::path pathDebug = GetDataDir() / "debug.log";

    /* Open debug.log with the associated application */
    if (boost::filesystem::exists(pathDebug))
        QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdString(pathDebug.string())));
}

void openConfigfile()
{
    boost::filesystem::path pathConfig = GetConfigFile();

    /* Open era.conf with the associated application */
    if (boost::filesystem::exists(pathConfig))
        QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdString(pathConfig.string())));
}

ToolTipToRichTextFilter::ToolTipToRichTextFilter(int size_threshold, QObject *parent) :
    QObject(parent), size_threshold(size_threshold)
{

}

bool ToolTipToRichTextFilter::eventFilter(QObject *obj, QEvent *evt)
{
    if(evt->type() == QEvent::ToolTipChange)
    {
        QWidget *widget = static_cast<QWidget*>(obj);
        QString tooltip = widget->toolTip();
        if(tooltip.size() > size_threshold && !tooltip.startsWith("<qt>") && !Qt::mightBeRichText(tooltip))
        {
            // Prefix <qt/> to make sure Qt detects this as rich text
            // Escape the current message as HTML and replace \n by <br>
            tooltip = "<qt>" + HtmlEscape(tooltip, true) + "<qt/>";
            widget->setToolTip(tooltip);
            return true;
        }
    }
    return QObject::eventFilter(obj, evt);
}

#ifdef WIN32
boost::filesystem::path static StartupShortcutPath()
{
    return GetSpecialFolderPath(CSIDL_STARTUP) / "Era.lnk";
}

bool GetStartOnSystemStartup()
{
    // check for Era.lnk
    return boost::filesystem::exists(StartupShortcutPath());
}

bool SetStartOnSystemStartup(bool fAutoStart)
{
    // If the shortcut exists already, remove it for updating
    boost::filesystem::remove(StartupShortcutPath());

    if (fAutoStart)
    {
        CoInitialize(NULL);

        // Get a pointer to the IShellLink interface.
        IShellLink* psl = NULL;
        HRESULT hres = CoCreateInstance(CLSID_ShellLink, NULL,
                                CLSCTX_INPROC_SERVER, IID_IShellLink,
                                reinterpret_cast<void**>(&psl));

        if (SUCCEEDED(hres))
        {
            // Get the current executable path
            TCHAR pszExePath[MAX_PATH];
            GetModuleFileName(NULL, pszExePath, sizeof(pszExePath));

            TCHAR pszArgs[5] = TEXT("-min");

            // Set the path to the shortcut target
            psl->SetPath(pszExePath);
            PathRemoveFileSpec(pszExePath);
            psl->SetWorkingDirectory(pszExePath);
            psl->SetShowCmd(SW_SHOWMINNOACTIVE);
            psl->SetArguments(pszArgs);

            // Query IShellLink for the IPersistFile interface for
            // saving the shortcut in persistent storage.
            IPersistFile* ppf = NULL;
            hres = psl->QueryInterface(IID_IPersistFile,
                                       reinterpret_cast<void**>(&ppf));
            if (SUCCEEDED(hres))
            {
                WCHAR pwsz[MAX_PATH];
                // Ensure that the string is ANSI.
                MultiByteToWideChar(CP_ACP, 0, StartupShortcutPath().string().c_str(), -1, pwsz, MAX_PATH);
                // Save the link by calling IPersistFile::Save.
                hres = ppf->Save(pwsz, TRUE);
                ppf->Release();
                psl->Release();
                CoUninitialize();
                return true;
            }
            psl->Release();
        }
        CoUninitialize();
        return false;
    }
    return true;
}

#elif defined(Q_OS_LINUX)

// Follow the Desktop Application Autostart Spec:
//  http://standards.freedesktop.org/autostart-spec/autostart-spec-latest.html

boost::filesystem::path static GetAutostartDir()
{
    namespace fs = boost::filesystem;

    char* pszConfigHome = getenv("XDG_CONFIG_HOME");
    if (pszConfigHome) return fs::path(pszConfigHome) / "autostart";
    char* pszHome = getenv("HOME");
    if (pszHome) return fs::path(pszHome) / ".config" / "autostart";
    return fs::path();
}

boost::filesystem::path static GetAutostartFilePath()
{
    return GetAutostartDir() / "era.desktop";
}

bool GetStartOnSystemStartup()
{
    boost::filesystem::ifstream optionFile(GetAutostartFilePath());
    if (!optionFile.good())
        return false;
    // Scan through file for "Hidden=true":
    std::string line;
    while (!optionFile.eof())
    {
        getline(optionFile, line);
        if (line.find("Hidden") != std::string::npos &&
            line.find("true") != std::string::npos)
            return false;
    }
    optionFile.close();

    return true;
}

bool SetStartOnSystemStartup(bool fAutoStart)
{
    if (!fAutoStart)
        boost::filesystem::remove(GetAutostartFilePath());
    else
    {
        char pszExePath[MAX_PATH+1];
        memset(pszExePath, 0, sizeof(pszExePath));
        if (readlink("/proc/self/exe", pszExePath, sizeof(pszExePath)-1) == -1)
            return false;

        boost::filesystem::create_directories(GetAutostartDir());

        boost::filesystem::ofstream optionFile(GetAutostartFilePath(), std::ios_base::out|std::ios_base::trunc);
        if (!optionFile.good())
            return false;
        // Write a era.desktop file to the autostart directory:
        optionFile << "[Desktop Entry]\n";
        optionFile << "Type=Application\n";
        optionFile << "Name=Era\n";
        optionFile << "Exec=" << pszExePath << " -min\n";
        optionFile << "Terminal=false\n";
        optionFile << "Hidden=false\n";
        optionFile.close();
    }
    return true;
}
#else

// TODO: OSX startup stuff; see:
// https://developer.apple.com/library/mac/#documentation/MacOSX/Conceptual/BPSystemStartup/Articles/CustomLogin.html

bool GetStartOnSystemStartup() { return false; }
bool SetStartOnSystemStartup(bool fAutoStart) { return false; }

#endif

HelpMessageBox::HelpMessageBox(QWidget *parent) :
    QMessageBox(parent)
{
    header = tr("Era-Qt") + " " + tr("version") + " " +
        QString::fromStdString(FormatFullVersion()) + "\n\n" +
        tr("Usage:") + "\n" +
        "  era-qt [" + tr("command-line options") + "]                     " + "\n";

    coreOptions = QString::fromStdString(HelpMessage());

    uiOptions = tr("UI options") + ":\n" +
        "  -lang=<lang>           " + tr("Set language, for example \"de_DE\" (default: system locale)") + "\n" +
        "  -min                   " + tr("Start minimized") + "\n" +
        "  -splash                " + tr("Show splash screen on startup (default: 1)") + "\n";

    setWindowTitle(tr("Era-Qt"));
    setTextFormat(Qt::PlainText);
    // setMinimumWidth is ignored for QMessageBox so put in non-breaking spaces to make it wider.
    setText(header + QString(QChar(0x2003)).repeated(50));
    setDetailedText(coreOptions + "\n" + uiOptions);
}

void HelpMessageBox::printToConsole()
{
    // On other operating systems, the expected action is to print the message to the console.
    QString strUsage = header + "\n" + coreOptions + "\n" + uiOptions;
    fprintf(stdout, "%s", strUsage.toStdString().c_str());
}

void HelpMessageBox::showOrPrint()
{
#if defined(WIN32)
        // On Windows, show a message box, as there is no stderr/stdout in windowed applications
        exec();
#else
        // On other operating systems, print help text to console
        printToConsole();
#endif
}

void SetBlackThemeQSS(QApplication& app)
{
    app.setStyleSheet(
                // areas
                "QWidget        { background: #0e0b20; border: none; }"
                "QFrame         { border: none; }"

                // top menu
                "QMenuBar       { background: #0e0b20; color: #827ca1; }"
                "QMenu          { background: #0e0b20; color: #827ca1; border: 1px solid #2924eb; }"
                "QMenu::item    { padding: 5px; padding-left: 10px; padding-right: 10px; min-width: 120px; }"
                "QMenu::item:selected { background-color: #222034; color: #fff; }"

                // main menu
                "QToolBar       { border: none; }"
                "QToolButton    { text-align: left; font-size: 16px; height: 28px; color: #827ca1; border: none; border-left-color: #0e0b20; border-left-style: solid; border-left-width: 4px; margin-bottom: 8px; }"
                "QToolButton:checked { color: #fff; border: none; border-left-color: #2924eb; border-left-style: solid; border-left-width: 4px; }"

                // table
                "QTableView     { outline: 0; color: #827ca1; background: #0e0b20; gridline-color: #38354a; border: 1px solid #38354a; border-left: none; border-top: none; border-right: none; }"
                "QTableView::item { padding-bottom: 10px; padding-top: 10px; background: #0e0b20; text-align: left; border: none; border-bottom: 1px solid #38354a; }"
                "QTableView::item:selected { background: #38354a; }"
                "QHeaderView { qproperty-defaultAlignment: AlignLeft; }"
                "QHeaderView::section { font-weight: bold; padding-left: 3px; padding-bottom: 7px; padding-top: 7px; background: #0e0b20; color: #827ca1; text-align: left; border: none; border-bottom: 1px solid #38354a; }"

                // tree view
                "QTreeView::item { background: #0e0b20; color: #827ca1; padding: 3px }"
                "QTreeView::item:selected { background-color: #38354a; }"

                // scrollbar
                "QScrollBar     { color: #2b2838; background: #2b2838; background-color: #2b2838; border: 1px solid #2b2838; }"
                "QScrollBar::handle { border: 1px solid #38354a; background: #38354a; }"
                "QScrollBar::sub-page { border: 1px solid #2b2838; }"
                "QScrollBar::add-page { border: 1px solid #2b2838; }"
                "QScrollBar::add-line { border: 1px solid #2b2838; }"
                "QScrollBar::sub-line { border: 1px solid #2b2838; }"

                // custom elements
                "QToolButton[accessibleName=payToButton] { margin: 0; margin-left: 5px; height:20px; font-size: 12px; border: none; }"

                // elements
                "QLabel         { color: #827ca1; }"
                "QPushButton    { font-weight: bold; background-color: rgb(41,36,235); color: #ffffff; border: 2px solid rgb(41,36,235); border-radius: 3px; height: 20px; padding-right: 10px; padding-left: 10px; padding-top: 3px; padding-bottom: 3px; }"
                "QPushButton::hover { background-color: rgb(41,36,200); border: 2px solid rgb(41,36,180); color: rgb(255,255,255); }"
                "QPushButton::disabled { background: #0e0b20; border: 2px solid #827ca1; color: #827ca1; }"
                "QLineEdit      { background: #38354a; color: #827ca1; border: 1px solid #38354a; border-radius: 3px; height: 20px; padding-right: 10px; padding-left: 10px; padding-top: 3px; padding-bottom: 3px; }"
                "QLineEdit::hover { border: 1px solid rgb(41,36,235); }"
                "QLineEdit::focus { border: 1px solid rgb(41,36,235); }"
                "QLineEdit::disabled { background: #0e0b20; border: 1px solid #38354a; color: #827ca1; }"
                "QCheckBox      { color: #827ca1; }"
                "QRadioButton   { color: #827ca1; }"
                "QDoubleSpinBox { background-color: #38354a; color: #827ca1; border: 1px solid #38354a; border-radius: 3px; height: 20px; padding-right: 10px; padding-left: 10px; padding-top: 3px; padding-bottom: 3px; }"
                "QSpinBox       { background-color: #38354a; color: #827ca1; border: 1px solid #38354a; border-radius: 3px; height: 20px; padding-right: 10px; padding-left: 10px; padding-top: 3px; padding-bottom: 3px; }"
                "QComboBox      { background-color: #38354a; color: #827ca1; border: 1px solid #38354a; border-radius: 3px; height: 20px; padding-right: 10px; padding-left: 10px; padding-top: 3px; padding-bottom: 3px; }"
                "QComboBox QAbstractItemView::item { color: #827ca1; }"
                "QTextEdit      { background: #38354a; color: #827ca1; padding: 8px; border-radius: 3px; }"
                "QPlainTextEdit { background: #38354a; color: #827ca1; padding: 8px; border-radius: 3px; }"
                "QTabWidget::pane { border: none; }"
                "QTabBar::tab   { color: #827ca1; border-top: 4px solid transparent; padding: 8px; }"
                "QTabBar::tab:selected { color: #ffffff; border-top: 4px solid rgb(41,36,235); }"
                "QProgressBar   { color: #827ca1; background: rgb(255,255,255); border: none ; }"
                "QProgressBar::chunk { background: #0e0b20; border: none; }"
            );
}

void SetWhiteThemeQSS(QApplication& app)
{
    app.setStyleSheet(
                // areas
                "QWidget        { background: white; border: none; }"
                "QFrame         { border: none; }"

                // top menu
                "QMenuBar       { background: rgb(255,255,255); color: rgb(5,6,45); }"
                "QMenu          { background: rgb(255,255,255); color: rgb(5,6,45); border: 1px solid rgb(41,36,235); }"
                "QMenu::item    { padding: 5px; padding-left: 10px; padding-right: 10px; min-width: 120px; }"
                "QMenu::item:selected { background-color: rgb(255,255,255); color: rgb(41,36,235); }"

                // main menu
                "QToolBar       { border: none; }"
                "QToolButton    { text-align: left; font-size: 16px; height: 28px; color: rgb(164,164,171); border: none; border-left-color: rgb(255,255,255); border-left-style: solid; border-left-width: 4px; margin-bottom: 8px; }"
                "QToolButton:checked { color: rgb(41,36,235); border: none; border-left-color: rgb(41,36,235); border-left-style: solid; border-left-width: 4px; }"

                // table
                "QTableView     { outline: 0; background: rgb(255,255,255); color: rgb(5,6,45); gridline-color: rgb(255,255,255); border: 1px solid rgb(215,215,225); border-left: none; border-top: none; border-right: none; }"
                "QTableView::item { padding-bottom: 10px; padding-top: 10px; background: rgb(255,255,255); text-align: left; border: none; border-bottom: 1px solid rgb(215,215,225); }"
                "QTableView::item:selected { background: rgb(234,234,244); color: rgb(5,6,45); }"
                "QHeaderView { qproperty-defaultAlignment: AlignLeft; }"
                "QHeaderView::section { font-weight: bold; padding-left: 3px; padding-bottom: 7px; padding-top: 7px; background: rgb(255,255,255); color: rgb(41,36,235); text-align: left; border: none; border-bottom: 1px solid rgb(215,215,225); }"

                // tree view
                "QTreeView::item { background: rgb(255,255,255); color: rgb(5,6,45); padding: 3px }"
                "QTreeView::item:selected { background-color: rgb(234,234,244); }"

                // scrollbar
                "QScrollBar     { color: rgb(0,0,0); background: rgb(234,234,244); background-color: rgb(234,234,244); border: 1px solid rgb(234,234,244); }"
                "QScrollBar::handle { border: 1px solid rgb(215,215,225); background: rgb(215,215,225); }"
                "QScrollBar::sub-page { border: 1px solid rgb(234,234,244); }"
                "QScrollBar::add-page { border: 1px solid rgb(234,234,244); }"
                "QScrollBar::add-line { border: 1px solid rgb(234,234,244); }"
                "QScrollBar::sub-line { border: 1px solid rgb(234,234,244); }"

                // custom elements
                "QToolButton[accessibleName=payToButton] { margin: 0; margin-left: 5px; height:20px; font-size: 12px; border: none; }"

                // elements
                "QLabel         { color: rgb(5,6,45); }"
                "QPushButton    { font-weight: bold; background-color: rgb(255,255,255); color: rgb(41,36,235); border: 2px solid rgb(41,36,235); border-radius: 3px; height: 20px; padding-right: 10px; padding-left: 10px; padding-top: 3px; padding-bottom: 3px; }"
                "QPushButton::hover { background-color: rgb(41,36,235); color: rgb(255,255,255); }"
                "QPushButton::disabled { border: 2px solid rgb(215,215,225); color: rgb(215,215,225); }"
                "QLineEdit      { background: rgb(255,255,255); color: rgb(5,6,45); border: 1px solid rgb(215,215,225); border-radius: 3px; height: 20px; padding-right: 10px; padding-left: 10px; padding-top: 3px; padding-bottom: 3px; }"
                "QLineEdit::hover { border: 1px solid rgb(41,36,235); }"
                "QLineEdit::focus { border: 1px solid rgb(41,36,235); }"
                "QLineEdit::disabled { background: rgb(215,215,225); border: 1px solid rgb(215,215,225); color: rgb(5,6,45); }"
                "QCheckBox      { color: rgb(5,6,45); }"
                "QRadioButton   { color: rgb(5,6,45); }"
                "QDoubleSpinBox { background-color: rgb(255,255,255); color: rgb(5,6,45); border: 1px solid rgb(215,215,225); border-radius: 3px; height: 20px; padding-right: 10px; padding-left: 10px; padding-top: 3px; padding-bottom: 3px; }"
                "QSpinBox       { background-color: rgb(255,255,255); color: rgb(5,6,45); border: 1px solid rgb(215,215,225); border-radius: 3px; height: 20px; padding-right: 10px; padding-left: 10px; padding-top: 3px; padding-bottom: 3px; }"
                "QComboBox      { background-color: rgb(255,255,255); color: rgb(5,6,45); border: 1px solid rgb(215,215,225); border-radius: 3px; height: 20px; padding-right: 10px; padding-left: 10px; padding-top: 3px; padding-bottom: 3px; }"
                "QComboBox QAbstractItemView::item { color: rgb(5,6,45); }"
                "QTextEdit      { background: rgb(234,234,244); color: rgb(5,6,45); padding: 8px; border-radius: 3px; }"
                "QPlainTextEdit { background: rgb(234,234,244); color: rgb(5,6,45); padding: 8px; border-radius: 3px; }"
                "QTabWidget::pane { border: none; }"
                "QTabBar::tab   { color: rgb(164,164,171); border-top: 4px solid transparent; padding: 8px; }"
                "QTabBar::tab:selected { color: rgb(41,36,235); border-top: 4px solid rgb(41,36,235); }"
                "QProgressBar   { color: rgb(164,164,171); background: rgb(255,255,255); border: none ; }"
                "QProgressBar::chunk { background: rgb(41,36,235); border: none; }"
             );
}

} // namespace GUIUtil

