#include "MarkdownCore.h"
#include "PluginInterface.h"
#include <QApplication>
#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDockWidget>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QLineEdit>
#include <QMainWindow>
#include <QMessageBox>
#include <QPointer>
#include <QSettings>
#include <QSpinBox>
#include <QScrollBar>
#include <QTextBrowser>
#include <QTimer>
#include <QToolBar>
#include <QVBoxLayout>
#include <cstddef>
#include <cstring>
namespace {
const NppPluginHostInfo* host=nullptr;QString iniPath,cssFile,htmlFile;int zoom=100;bool showToolbar=true,syncCaret=true;QPointer<QDockWidget> dock;QPointer<QTextBrowser> preview;QPointer<QToolBar> toolbar;QPointer<QTimer> renderTimer;
QMainWindow* window(){return qobject_cast<QMainWindow*>(QApplication::activeWindow());}
QByteArray text(){if(!host||!host->get_current_document)return{};size_t n=host->get_current_document(host->host_context,nullptr,0);QByteArray d(int(n),Qt::Uninitialized);if(n)host->get_current_document(host->host_context,reinterpret_cast<uint8_t*>(d.data()),n);return d;}
QString currentPath(){if(!host||!host->get_current_file_path)return{};size_t n=host->get_current_file_path(host->host_context,nullptr,0);QByteArray d(int(n)+1,'\0');host->get_current_file_path(host->host_context,d.data(),d.size());return QString::fromUtf8(d.constData());}
QString css(){QFile f(cssFile);return f.open(QIODevice::ReadOnly)?QString::fromUtf8(f.readAll()):QString();}
void save(){QSettings s(iniPath,QSettings::IniFormat);s.setValue(QStringLiteral("Options/SyncViewWithCaretPosition"),syncCaret);s.setValue(QStringLiteral("Options/CssFileName"),cssFile);s.setValue(QStringLiteral("Options/ZoomLevel"),zoom);s.setValue(QStringLiteral("Options/HtmlFileName"),htmlFile);s.setValue(QStringLiteral("Options/ShowToolbar"),showToolbar);}
void render(){if(!preview)return;preview->setHtml(MarkdownCore::toHtml(QString::fromUtf8(text()),css()));preview->setSearchPaths({QFileInfo(currentPath()).absolutePath()});QFont font=QApplication::font();font.setPointSizeF(font.pointSizeF()*zoom/100.0);preview->setFont(font);if(toolbar)toolbar->setVisible(showToolbar);if(!htmlFile.isEmpty()){QFile f(htmlFile);if(f.open(QIODevice::WriteOnly))f.write(MarkdownCore::toHtml(QString::fromUtf8(text()),css()).toUtf8());}}
void scheduleRender(){if(!dock||!dock->isVisible())return;if(!renderTimer){renderTimer=new QTimer(QApplication::instance());renderTimer->setSingleShot(true);QObject::connect(renderTimer,&QTimer::timeout,&render);}renderTimer->start(120);}
void syncPosition(){if(!syncCaret||!preview||!host||!host->get_current_view||!host->get_first_visible_line||!host->send_scintilla)return;int view=host->get_current_view(host->host_context);qint64 first=host->get_first_visible_line(host->host_context,view);qint64 lines=host->send_scintilla(host->host_context,view,2154,0,0);if(first<0||lines<=1)return;QScrollBar*bar=preview->verticalScrollBar();bar->setValue(int(double(first)/double(lines-1)*bar->maximum()));}
void create(){if(dock)return;QMainWindow*p=window();if(!p)return;QDockWidget*d=new QDockWidget(QStringLiteral("NppMarkdownPanel-qt"),p);d->setObjectName(QStringLiteral("NppMarkdownPanelQtDock"));QWidget*w=new QWidget(d);QVBoxLayout*l=new QVBoxLayout(w);l->setContentsMargins(0,0,0,0);QToolBar*t=new QToolBar(w);QAction*refresh=t->addAction(QStringLiteral("Refresh"));QAction*exportAction=t->addAction(QStringLiteral("Export HTML"));QTextBrowser*b=new QTextBrowser(w);b->setOpenExternalLinks(true);l->addWidget(t);l->addWidget(b,1);d->setWidget(w);p->addDockWidget(Qt::RightDockWidgetArea,d);dock=d;preview=b;toolbar=t;QObject::connect(refresh,&QAction::triggered,&render);QObject::connect(exportAction,&QAction::triggered,[]{QString file=QFileDialog::getSaveFileName(window(),QStringLiteral("Export HTML"),htmlFile,QStringLiteral("HTML (*.html)"));if(!file.isEmpty()){htmlFile=file;save();render();}});QObject::connect(d,&QObject::destroyed,[]{dock=nullptr;preview=nullptr;toolbar=nullptr;});render();d->hide();}
void about(void*){QMessageBox::about(window(),QStringLiteral("NppMarkdownPanel-qt"),QStringLiteral("Qt port of NppMarkdownPanel 0.6.2."));}
void toggle(void*){create();if(dock){dock->setVisible(!dock->isVisible());if(dock->isVisible())render();}}
void sync(void*){syncCaret=!syncCaret;save();}
void settings(void*){QDialog d(window());d.setWindowTitle(QStringLiteral("NppMarkdownPanel-qt Settings"));QFormLayout l(&d);QSpinBox z;z.setRange(25,400);z.setValue(zoom);QLineEdit c(cssFile),h(htmlFile);QCheckBox tools;tools.setChecked(showToolbar);l.addRow(QStringLiteral("Zoom"),&z);l.addRow(QStringLiteral("CSS file"),&c);l.addRow(QStringLiteral("Automatic HTML output"),&h);l.addRow(QStringLiteral("Show toolbar"),&tools);QDialogButtonBox b(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);l.addRow(&b);QObject::connect(&b,&QDialogButtonBox::accepted,&d,&QDialog::accept);QObject::connect(&b,&QDialogButtonBox::rejected,&d,&QDialog::reject);if(d.exec()==QDialog::Accepted){zoom=z.value();cssFile=c.text();htmlFile=h.text();showToolbar=tools.isChecked();save();render();}}
NppPluginFuncItem commands[]={{sizeof(NppPluginFuncItem),"About",about,nullptr,0,{},{}},{sizeof(NppPluginFuncItem),"Toggle Markdown Panel",toggle,nullptr,0,{},{}},{sizeof(NppPluginFuncItem),"Synchronize viewer with caret position",sync,nullptr,1,{},{}},{sizeof(NppPluginFuncItem),"Edit Settings",settings,nullptr,0,{},{}}};
}
extern "C" {NPP_PLUGIN_EXPORT uint32_t NPP_PLUGIN_CALL nppGetPluginAbiVersion(){return 1;}NPP_PLUGIN_EXPORT const char*NPP_PLUGIN_CALL nppGetName(){return "NppMarkdownPanel-qt";}NPP_PLUGIN_EXPORT int NPP_PLUGIN_CALL nppSetInfo(const NppPluginHostInfo*v){if(!v||v->abi_version!=1||v->struct_size<offsetof(NppPluginHostInfo,set_status_text)+sizeof(v->set_status_text))return 0;host=v;iniPath=QString::fromUtf8(v->plugin_config_path_utf8?v->plugin_config_path_utf8:"")+QStringLiteral("/NppMarkdownPanel.ini");QSettings s(iniPath,QSettings::IniFormat);syncCaret=s.value(QStringLiteral("Options/SyncViewWithCaretPosition"),true).toBool();cssFile=s.value(QStringLiteral("Options/CssFileName")).toString();zoom=s.value(QStringLiteral("Options/ZoomLevel"),100).toInt();htmlFile=s.value(QStringLiteral("Options/HtmlFileName")).toString();showToolbar=s.value(QStringLiteral("Options/ShowToolbar"),true).toBool();return v->get_current_document!=nullptr;}NPP_PLUGIN_EXPORT const NppPluginFuncItem*NPP_PLUGIN_CALL nppGetFuncsArray(uint32_t*c){if(c)*c=4;return commands;}NPP_PLUGIN_EXPORT void NPP_PLUGIN_CALL nppBeNotified(const NppPluginNotification*n){if(!n)return;if(n->code==NPP_PLUGIN_NOTIFICATION_READY||n->code==NPP_PLUGIN_NOTIFICATION_BUFFER_ACTIVATED||n->code==NPP_PLUGIN_NOTIFICATION_FILE_OPENED||n->code==NPP_PLUGIN_NOTIFICATION_TEXT_MODIFIED)scheduleRender();else if(n->code==NPP_PLUGIN_NOTIFICATION_UPDATE_UI)syncPosition();else if(n->code==NPP_PLUGIN_NOTIFICATION_SHUTDOWN){if(renderTimer)renderTimer->stop();save();}}NPP_PLUGIN_EXPORT intptr_t NPP_PLUGIN_CALL nppMessageProc(uint32_t,uintptr_t,intptr_t){return 0;}
#if defined(_WIN32)
struct LegacyNppData{void*npp;void*main;void*sub;};NPP_PLUGIN_EXPORT void NPP_PLUGIN_CALL setInfo(LegacyNppData){}NPP_PLUGIN_EXPORT const wchar_t*NPP_PLUGIN_CALL getName(){return L"NppMarkdownPanel-qt";}NPP_PLUGIN_EXPORT void*NPP_PLUGIN_CALL getFuncsArray(int*c){if(c)*c=0;return nullptr;}NPP_PLUGIN_EXPORT void NPP_PLUGIN_CALL beNotified(void*){}NPP_PLUGIN_EXPORT intptr_t NPP_PLUGIN_CALL messageProc(uint32_t,uintptr_t,intptr_t){return 0;}NPP_PLUGIN_EXPORT int NPP_PLUGIN_CALL isUnicode(){return 1;}
#endif
}
