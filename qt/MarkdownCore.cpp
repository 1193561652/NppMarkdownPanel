// Independent Qt Markdown renderer. SPDX-License-Identifier: MIT
#include "MarkdownCore.h"
#include <QRegularExpression>
#include <QTextDocument>
#include <QUrl>
#include <functional>

namespace {
struct Expansion {
    QString text;
    QVector<QPair<QString,QString>> replacements;
    QHash<QString,QString> abbreviations;
    QHash<QString,QString> footnotes;
    QHash<QString,QString> headingAttributes;
    QStringList footnoteOrder;
    int next=0;
    QString token(const QString&html){const QString key=QStringLiteral("NPPQTX%1Z").arg(next++,6,10,QChar('0'));replacements.append({key,html});return key;}
};
QString attributes(QString value)
{
    QString id;QStringList classes;QStringList extra;
    const auto parts=value.split(QRegularExpression(QStringLiteral("\\s+")),Qt::SkipEmptyParts);
    for(const QString&part:parts){if(part.startsWith('#'))id=part.mid(1);else if(part.startsWith('.'))classes.append(part.mid(1));else if(part.contains('=')){const int at=part.indexOf('=');extra.append(QStringLiteral(" %1=\"%2\"").arg(part.left(at).toHtmlEscaped(),part.mid(at+1).remove('"').toHtmlEscaped()));}}
    QString result;if(!id.isEmpty())result+=QStringLiteral(" id=\"%1\"").arg(id.toHtmlEscaped());if(!classes.isEmpty())result+=QStringLiteral(" class=\"%1\"").arg(classes.join(' ').toHtmlEscaped());return result+extra.join(QString());
}
QString slug(QString value){value.remove(QRegularExpression(QStringLiteral("<[^>]+>")));value=value.toLower().trimmed();value.replace(QRegularExpression(QStringLiteral("[^\\p{L}\\p{N}]+")),QStringLiteral("-"));value.remove(QRegularExpression(QStringLiteral("^-+|-+$")));return value.isEmpty()?QStringLiteral("section"):value;}
QString processInline(QString text,Expansion&x)
{
    auto replace=[&](const QRegularExpression&re,const std::function<QString(const QRegularExpressionMatch&)>&fn){int offset=0;while(true){auto m=re.match(text,offset);if(!m.hasMatch())break;const QString token=fn(m);text.replace(m.capturedStart(),m.capturedLength(),token);offset=m.capturedStart()+token.size();}};
    replace(QRegularExpression(QStringLiteral("!\\[([^]]*)\\]\\(([^ )]+)(?: \\\"([^\\\"]*)\\\")?\\)\\{([^}]*)\\}")),[&](const auto&m){const QString url=m.captured(2),ext=QUrl(url).path().section('.',-1).toLower();QString html;if(QStringList({QStringLiteral("mp4"),QStringLiteral("webm"),QStringLiteral("ogv")}).contains(ext))html=QStringLiteral("<video controls%1 src=\"%2\">%3</video>").arg(attributes(m.captured(4)),url.toHtmlEscaped(),m.captured(1).toHtmlEscaped());else if(QStringList({QStringLiteral("mp3"),QStringLiteral("ogg"),QStringLiteral("wav")}).contains(ext))html=QStringLiteral("<audio controls%1 src=\"%2\">%3</audio>").arg(attributes(m.captured(4)),url.toHtmlEscaped(),m.captured(1).toHtmlEscaped());else html=QStringLiteral("<img%1 alt=\"%2\" src=\"%3\" title=\"%4\">").arg(attributes(m.captured(4)),m.captured(1).toHtmlEscaped(),url.toHtmlEscaped(),m.captured(3).toHtmlEscaped());return x.token(html);});
    replace(QRegularExpression(QStringLiteral("!\\[([^]]*)\\]\\(([^ )]+)(?: \\\"([^\\\"]*)\\\")?\\)")),[&](const auto&m){const QString url=m.captured(2),ext=QUrl(url).path().section('.',-1).toLower();if(QStringList({QStringLiteral("mp4"),QStringLiteral("webm"),QStringLiteral("ogv")}).contains(ext))return x.token(QStringLiteral("<video controls src=\"%1\">%2</video>").arg(url.toHtmlEscaped(),m.captured(1).toHtmlEscaped()));if(QStringList({QStringLiteral("mp3"),QStringLiteral("ogg"),QStringLiteral("wav")}).contains(ext))return x.token(QStringLiteral("<audio controls src=\"%1\">%2</audio>").arg(url.toHtmlEscaped(),m.captured(1).toHtmlEscaped()));return x.token(QStringLiteral("<img alt=\"%1\" src=\"%2\" title=\"%3\">").arg(m.captured(1).toHtmlEscaped(),url.toHtmlEscaped(),m.captured(3).toHtmlEscaped()));});
    replace(QRegularExpression(QStringLiteral("\\[\\^([^]]+)\\]")),[&](const auto&m){const QString id=m.captured(1);if(!x.footnoteOrder.contains(id))x.footnoteOrder.append(id);const int number=x.footnoteOrder.indexOf(id)+1;return x.token(QStringLiteral("<sup id=\"fnref:%1\"><a href=\"#fn:%1\">%2</a></sup>").arg(id.toHtmlEscaped()).arg(number));});
    replace(QRegularExpression(QStringLiteral("\\[@([^] ]+)\\]")),[&](const auto&m){return x.token(QStringLiteral("<cite data-cite=\"%1\">[%1]</cite>").arg(m.captured(1).toHtmlEscaped()));});
    replace(QRegularExpression(QStringLiteral("\\$([^$\\n]+)\\$")),[&](const auto&m){return x.token(QStringLiteral("<span class=\"math\">%1</span>").arg(m.captured(1).toHtmlEscaped()));});
    replace(QRegularExpression(QStringLiteral("==([^=\\n]+)==")),[&](const auto&m){return x.token(QStringLiteral("<mark>%1</mark>").arg(m.captured(1).toHtmlEscaped()));});
    replace(QRegularExpression(QStringLiteral("\\+\\+([^+\\n]+)\\+\\+")),[&](const auto&m){return x.token(QStringLiteral("<ins>%1</ins>").arg(m.captured(1).toHtmlEscaped()));});
    replace(QRegularExpression(QStringLiteral("(?<!~)~([^~\\n]+)~(?!~)")),[&](const auto&m){return x.token(QStringLiteral("<sub>%1</sub>").arg(m.captured(1).toHtmlEscaped()));});
    replace(QRegularExpression(QStringLiteral("\\^([^\\^\\n]+)\\^")),[&](const auto&m){return x.token(QStringLiteral("<sup>%1</sup>").arg(m.captured(1).toHtmlEscaped()));});
    return text;
}
Expansion expand(QString markdown)
{
    Expansion x;QStringList lines=markdown.replace(QStringLiteral("\r\n"),QStringLiteral("\n")).split('\n');QStringList out;
    for(int i=0;i<lines.size();++i){QString line=lines[i];auto fence=QRegularExpression(QStringLiteral("^\\s*(```+|~~~+)\\s*([^ ]*)")).match(line);if(fence.hasMatch()){const QString marker=fence.captured(1),language=fence.captured(2);QString code;while(++i<lines.size()&&!lines[i].trimmed().startsWith(marker.left(3)))code+=lines[i]+QLatin1Char('\n');out<<x.token(QStringLiteral("<pre><code class=\"language-%1\">%2</code></pre>").arg(language.toHtmlEscaped(),code.toHtmlEscaped()));continue;}auto abbr=QRegularExpression(QStringLiteral("^\\*\\[([^]]+)\\]:\\s*(.+)$")).match(line);if(abbr.hasMatch()){x.abbreviations.insert(abbr.captured(1),abbr.captured(2));continue;}auto foot=QRegularExpression(QStringLiteral("^\\[\\^([^]]+)\\]:\\s*(.*)$")).match(line);if(foot.hasMatch()){QString content=foot.captured(2);while(i+1<lines.size()&&QRegularExpression(QStringLiteral("^\\s{2,}")).match(lines[i+1]).hasMatch())content+=QLatin1Char(' ')+lines[++i].trimmed();x.footnotes.insert(foot.captured(1),content);continue;}
        auto headingAttrs=QRegularExpression(QStringLiteral("^(#{1,6}\\s+)(.*?)\\s+\\{([^}]*)\\}\\s*$")).match(line);if(headingAttrs.hasMatch()){line=headingAttrs.captured(1)+headingAttrs.captured(2);x.headingAttributes.insert(slug(headingAttrs.captured(2)),attributes(headingAttrs.captured(3)));}
        auto task=QRegularExpression(QStringLiteral("^(\\s*[-+*]\\s+)\\[([ xX])\\]\\s*(.*)$")).match(line);if(task.hasMatch()){out<<task.captured(1)+x.token(QStringLiteral("<input type=\"checkbox\" disabled%1>").arg(task.captured(2).trimmed().isEmpty()?QString():QStringLiteral(" checked")))+QLatin1Char(' ')+processInline(task.captured(3),x);continue;}
        auto container=QRegularExpression(QStringLiteral("^\\s*:::\\s*([\\w-]+)?(?:\\s*\\{([^}]*)\\})?\\s*$")).match(line);if(container.hasMatch()){if(container.captured(1).isEmpty())out<<x.token(QStringLiteral("</div>"));else out<<x.token(QStringLiteral("<div class=\"%1\"%2>").arg(container.captured(1).toHtmlEscaped(),attributes(container.captured(2))));continue;}
        if(line.trimmed()==QStringLiteral("$$")){QString math;while(++i<lines.size()&&lines[i].trimmed()!=QStringLiteral("$$"))math+=lines[i]+QLatin1Char('\n');out<<x.token(QStringLiteral("<div class=\"math math-display\">%1</div>").arg(math.trimmed().toHtmlEscaped()));continue;}
        if(i+1<lines.size()&&line.contains('|')&&QRegularExpression(QStringLiteral("^\\s*\\|?\\s*:?-{3,}:?\\s*(?:\\|\\s*:?-{3,}:?\\s*)+\\|?\\s*$")).match(lines[i+1]).hasMatch()){auto split=[](QString row){row=row.trimmed();if(row.startsWith('|'))row.remove(0,1);if(row.endsWith('|'))row.chop(1);return row.split('|');};const QStringList heads=split(line),rules=split(lines[++i]);QString html=QStringLiteral("<table><thead><tr>");for(int c=0;c<heads.size();++c){const QString rule=c<rules.size()?rules[c].trimmed():QString();const QString align=rule.startsWith(':')&&rule.endsWith(':')?QStringLiteral("center"):rule.endsWith(':')?QStringLiteral("right"):QStringLiteral("left");html+=QStringLiteral("<th style=\"text-align:%1\">%2</th>").arg(align,processInline(heads[c].trimmed(),x));}html+=QStringLiteral("</tr></thead><tbody>");while(i+1<lines.size()&&lines[i+1].contains('|')&&!lines[i+1].trimmed().isEmpty()){html+=QStringLiteral("<tr>");for(const QString&cell:split(lines[++i]))html+=QStringLiteral("<td>%1</td>").arg(processInline(cell.trimmed(),x));html+=QStringLiteral("</tr>");}html+=QStringLiteral("</tbody></table>");out<<x.token(html);continue;}
        if(i+1<lines.size()&&lines[i+1].startsWith(QStringLiteral(": "))&&!line.trimmed().isEmpty()){QString html=QStringLiteral("<dl><dt>%1</dt>").arg(processInline(line,x));while(i+1<lines.size()&&lines[i+1].startsWith(QStringLiteral(": "))){html+=QStringLiteral("<dd>%1</dd>").arg(processInline(lines[++i].mid(2),x));}html+=QStringLiteral("</dl>");out<<x.token(html);continue;}
        auto figure=QRegularExpression(QStringLiteral("^\\s*\\^{3,}\\s*(.*)$")).match(line);if(figure.hasMatch()){QString content;while(++i<lines.size()&&!QRegularExpression(QStringLiteral("^\\s*\\^{3,}")).match(lines[i]).hasMatch())content+=processInline(lines[i],x)+QLatin1Char('\n');QString caption=figure.captured(1);if(i<lines.size()){auto closing=QRegularExpression(QStringLiteral("^\\s*\\^{3,}\\s*(.*)$")).match(lines[i]);if(caption.isEmpty())caption=closing.captured(1);}out<<x.token(QStringLiteral("<figure>%1%2</figure>").arg(content.trimmed(),caption.isEmpty()?QString():QStringLiteral("<figcaption>%1</figcaption>").arg(processInline(caption,x))));continue;}
        if(QRegularExpression(QStringLiteral("^\\s*\\^\\^(?:\\s+|$)")).match(line).hasMatch()){QString footer;while(i<lines.size()&&QRegularExpression(QStringLiteral("^\\s*\\^\\^(?:\\s+|$)")).match(lines[i]).hasMatch()){footer+=processInline(lines[i].replace(QRegularExpression(QStringLiteral("^\\s*\\^\\^\\s?")),QString()),x)+QLatin1Char('\n');++i;}--i;out<<x.token(QStringLiteral("<footer>%1</footer>").arg(footer.trimmed()));continue;}
        auto grid=QRegularExpression(QStringLiteral("^\\s*\\+(?:[-=]+\\+)+\\s*$")).match(line);if(grid.hasMatch()&&i+1<lines.size()){QString html=QStringLiteral("<table class=\"grid-table\"><tbody>");while(++i<lines.size()){if(QRegularExpression(QStringLiteral("^\\s*\\+(?:[-=]+\\+)+\\s*$")).match(lines[i]).hasMatch())continue;if(!lines[i].trimmed().startsWith('|')){--i;break;}QString row=lines[i].trimmed();row.remove(0,1);if(row.endsWith('|'))row.chop(1);html+=QStringLiteral("<tr>");for(const QString&cell:row.split('|'))html+=QStringLiteral("<td>%1</td>").arg(processInline(cell.trimmed(),x));html+=QStringLiteral("</tr>");}html+=QStringLiteral("</tbody></table>");out<<x.token(html);continue;}
        out<<processInline(line,x);
    }
    if(!x.footnoteOrder.isEmpty()){QString html=QStringLiteral("<section class=\"footnotes\"><hr><ol>");for(const QString&id:x.footnoteOrder)html+=QStringLiteral("<li id=\"fn:%1\">%2 <a href=\"#fnref:%1\" class=\"footnote-backref\">↩</a></li>").arg(id.toHtmlEscaped(),processInline(x.footnotes.value(id),x));html+=QStringLiteral("</ol></section>");out<<x.token(html);}
    x.text=out.join('\n');return x;
}
QString restore(QString html,const Expansion&x)
{
    for(auto it=x.abbreviations.cbegin();it!=x.abbreviations.cend();++it)html.replace(QRegularExpression(QStringLiteral("\\b%1\\b").arg(QRegularExpression::escape(it.key()))),QStringLiteral("<abbr title=\"%1\">%2</abbr>").arg(it.value().toHtmlEscaped(),it.key().toHtmlEscaped()));
    for(const auto&replacement:x.replacements){html.replace(QStringLiteral("<p>%1</p>").arg(replacement.first),replacement.second);html.replace(replacement.first,replacement.second);}
    QRegularExpression heading(QStringLiteral("<h([1-6])([^>]*)>(.*?)</h\\1>"),QRegularExpression::DotMatchesEverythingOption);int offset=0;while(true){auto m=heading.match(html,offset);if(!m.hasMatch())break;QString attrs=m.captured(2),content=m.captured(3);const QString id=slug(content);if(x.headingAttributes.contains(id))attrs+=x.headingAttributes.value(id);if(!attrs.contains(QRegularExpression(QStringLiteral("\\bid="))))attrs+=QStringLiteral(" id=\"%1\"").arg(id);const QString replacement=QStringLiteral("<h%1%2>%3</h%1>").arg(m.captured(1),attrs,content);html.replace(m.capturedStart(),m.capturedLength(),replacement);offset=m.capturedStart()+replacement.size();}
    return html;
}
}
namespace MarkdownCore {
QString toHtml(const QString&markdown,const QString&css){Expansion x=expand(markdown);QTextDocument document;document.setMarkdown(x.text,QTextDocument::MarkdownDialectGitHub);QString html=restore(document.toHtml(),x);const QString style=css.isEmpty()?QStringLiteral("body{font-family:sans-serif;margin:20px;line-height:1.5}pre,code{font-family:monospace;background:#f3f3f3}pre{padding:10px;overflow:auto}img,video{max-width:100%}table{border-collapse:collapse}th,td{border:1px solid #bbb;padding:4px 8px}.task-list-item{list-style:none}.math{font-family:serif}.footnotes{font-size:smaller}"):css;html.replace(QStringLiteral("</head>"),QStringLiteral("<style>%1</style></head>").arg(style));return html;}
}
