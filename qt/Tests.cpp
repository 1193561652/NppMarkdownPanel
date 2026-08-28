#include "MarkdownCore.h"
#include "PluginInterface.h"
#include <QApplication>
#include <QLibrary>
#include <QTemporaryDir>
#include <cstdlib>
#include <iostream>
static size_t NPP_PLUGIN_CALL doc(void*,uint8_t*,size_t){return 0;}static void require(bool v,const char*m){if(!v){std::cerr<<m<<'\n';std::exit(1);}}
int main(int argc,char**argv){const QString rendered=MarkdownCore::toHtml(QStringLiteral("# X\n\nTerm\n: Definition\n\nText[^1], $x+y$, and ==mark==.\n\n```mermaid\ngraph TD; A-->B\n```\n\n$$\na+b\n$$\n\n[^1]: note"));require(rendered.contains(QStringLiteral("<h1"))&&rendered.contains(QStringLiteral("<dl>"))&&rendered.contains(QStringLiteral("footnotes"))&&rendered.contains(QStringLiteral("<mark>mark</mark>")),"advanced markdown core");require(rendered.contains(QStringLiteral("<div class=\"mermaid\">graph TD; A--&gt;B"))&&rendered.contains(QStringLiteral("\\(x+y\\)"))&&rendered.contains(QStringLiteral("\\[\na+b\n\\]")),"original static diagram and mathematics markup");QApplication app(argc,argv);QLibrary l(QString::fromUtf8(NPP_MARKDOWN_PANEL_PATH));require(l.load(),qPrintable(l.errorString()));auto name=reinterpret_cast<const char*(*)()>(l.resolve("nppGetName"));auto set=reinterpret_cast<int(*)(const NppPluginHostInfo*)>(l.resolve("nppSetInfo"));auto funcs=reinterpret_cast<const NppPluginFuncItem*(*)(uint32_t*)>(l.resolve("nppGetFuncsArray"));require(name&&set&&funcs,"cross-platform ABI");QTemporaryDir d;QByteArray p=d.path().toUtf8();NppPluginHostInfo h{};h.struct_size=sizeof(h);h.abi_version=1;h.plugin_config_path_utf8=p.constData();h.get_current_document=doc;require(set(&h),"setInfo");uint32_t c=0;require(funcs(&c)&&c==4&&QByteArray(name())=="NppMarkdownPanel-qt","identity");
#if defined(_WIN32)
auto legacyName=reinterpret_cast<const wchar_t*(*)()>(l.resolve("getName"));auto legacyFuncs=reinterpret_cast<void*(*)(int*)>(l.resolve("getFuncsArray"));require(legacyName&&legacyFuncs,"legacy ABI");int legacyCount=-1;require(legacyFuncs(&legacyCount)==nullptr&&legacyCount==0&&QString::fromWCharArray(legacyName())==QStringLiteral("NppMarkdownPanel-qt"),"empty legacy ABI");
#else
require(!l.resolve("setInfo")&&!l.resolve("getName")&&!l.resolve("getFuncsArray"),"legacy ABI must remain Windows-only");
#endif
return 0;}
