#include "MarkdownCore.h"
#include <QRegularExpression>
#include <QStringList>

namespace {
QString inlineMarkup(QString value)
{
    value = value.toHtmlEscaped();
    value.replace(QRegularExpression(QStringLiteral("`([^`]+)`")), QStringLiteral("<code>\\1</code>"));
    value.replace(QRegularExpression(QStringLiteral("!\\[([^]]*)\\]\\(([^ )]+)(?: \\\"([^\\\"]*)\\\")?\\)")), QStringLiteral("<img alt=\"\\1\" src=\"\\2\" title=\"\\3\">"));
    value.replace(QRegularExpression(QStringLiteral("\\[([^]]+)\\]\\(([^ )]+)(?: \\\"([^\\\"]*)\\\")?\\)")), QStringLiteral("<a href=\"\\2\" title=\"\\3\">\\1</a>"));
    value.replace(QRegularExpression(QStringLiteral("\\*\\*([^*]+)\\*\\*")), QStringLiteral("<strong>\\1</strong>"));
    value.replace(QRegularExpression(QStringLiteral("__([^_]+)__")), QStringLiteral("<strong>\\1</strong>"));
    value.replace(QRegularExpression(QStringLiteral("(?<!\\*)\\*([^*]+)\\*(?!\\*)")), QStringLiteral("<em>\\1</em>"));
    value.replace(QRegularExpression(QStringLiteral("~~([^~]+)~~")), QStringLiteral("<del>\\1</del>"));
    value.replace(QRegularExpression(QStringLiteral("(?<![=\"'])\\b(https?://[^\\s<]+)")), QStringLiteral("<a href=\"\\1\">\\1</a>"));
    return value;
}
bool isRule(const QString& line) { return QRegularExpression(QStringLiteral("^\\s*(?:[-*_]\\s*){3,}$")).match(line).hasMatch(); }
QStringList cells(QString line){line=line.trimmed();if(line.startsWith('|'))line.remove(0,1);if(line.endsWith('|'))line.chop(1);QStringList result=line.split('|');for(QString&cell:result)cell=cell.trimmed();return result;}
bool isTableRule(const QString&line){const QStringList parts=cells(line);if(parts.isEmpty())return false;for(const QString&part:parts)if(!QRegularExpression(QStringLiteral("^:?-{3,}:?$")).match(part).hasMatch())return false;return true;}
}
namespace MarkdownCore {
QString toHtml(const QString& markdown, const QString& css)
{
    QString body; QStringList lines = markdown.split('\n'); bool code = false, ul = false, ol = false, quote = false;
    auto closeLists = [&] { if (ul) { body += QStringLiteral("</ul>\n"); ul = false; } if (ol) { body += QStringLiteral("</ol>\n"); ol = false; } };
    for (int index=0;index<lines.size();++index) {
        QString line=lines[index];
        if (line.endsWith('\r')) line.chop(1);
        QRegularExpressionMatch fence=QRegularExpression(QStringLiteral("^\\s*(```+|~~~+)\\s*([^ ]*)")).match(line);
        if (fence.hasMatch()) { closeLists(); body += code ? QStringLiteral("</code></pre>\n") : QStringLiteral("<pre><code class=\"language-%1\">").arg(fence.captured(2).toHtmlEscaped()); code = !code; continue; }
        if (code) { body += line.toHtmlEscaped() + '\n'; continue; }
        if(index+1<lines.size()&&QRegularExpression(QStringLiteral("^\\s*(=+|-+)\\s*$")).match(lines[index+1]).hasMatch()&&!line.trimmed().isEmpty()){closeLists();int level=lines[index+1].contains('=')?1:2;body+=QStringLiteral("<h%1>%2</h%1>\n").arg(level).arg(inlineMarkup(line));++index;continue;}
        if(index+1<lines.size()&&line.contains('|')&&isTableRule(lines[index+1])){closeLists();QStringList headers=cells(line);body+=QStringLiteral("<table><thead><tr>");for(const QString&cell:headers)body+=QStringLiteral("<th>%1</th>").arg(inlineMarkup(cell));body+=QStringLiteral("</tr></thead><tbody>");index+=2;while(index<lines.size()&&lines[index].contains('|')&&!lines[index].trimmed().isEmpty()){body+=QStringLiteral("<tr>");for(const QString&cell:cells(lines[index]))body+=QStringLiteral("<td>%1</td>").arg(inlineMarkup(cell));body+=QStringLiteral("</tr>");++index;}--index;body+=QStringLiteral("</tbody></table>\n");continue;}
        if(line.startsWith(QStringLiteral("    "))||line.startsWith('\t')){closeLists();body+=QStringLiteral("<pre><code>%1</code></pre>\n").arg(line.mid(line.startsWith('\t')?1:4).toHtmlEscaped());continue;}
        QRegularExpressionMatch heading = QRegularExpression(QStringLiteral("^(#{1,6})\\s+(.+)$")).match(line);
        if (heading.hasMatch()) { closeLists(); const int level = heading.captured(1).size(); body += QStringLiteral("<h%1>%2</h%1>\n").arg(level).arg(inlineMarkup(heading.captured(2))); continue; }
        if (isRule(line)) { closeLists(); body += QStringLiteral("<hr>\n"); continue; }
        QRegularExpressionMatch bullet = QRegularExpression(QStringLiteral("^\\s*[-+*]\\s+(.+)$")).match(line);
        if (bullet.hasMatch()) { if (!ul) { closeLists(); ul = true; body += QStringLiteral("<ul>\n"); }QString item=bullet.captured(1);QRegularExpressionMatch task=QRegularExpression(QStringLiteral("^\\[([ xX])\\]\\s*(.*)$")).match(item);if(task.hasMatch())body+=QStringLiteral("<li class=\"task-list-item\"><input type=\"checkbox\" disabled %1> %2</li>\n").arg(task.captured(1).trimmed().isEmpty()?QString():QStringLiteral("checked"),inlineMarkup(task.captured(2)));else body += QStringLiteral("<li>%1</li>\n").arg(inlineMarkup(item)); continue; }
        QRegularExpressionMatch number = QRegularExpression(QStringLiteral("^\\s*\\d+[.)]\\s+(.+)$")).match(line);
        if (number.hasMatch()) { if (!ol) { closeLists(); ol = true; body += QStringLiteral("<ol>\n"); } body += QStringLiteral("<li>%1</li>\n").arg(inlineMarkup(number.captured(1))); continue; }
        closeLists();
        if (line.startsWith(QStringLiteral("> "))) { if (!quote) { body += QStringLiteral("<blockquote>"); quote = true; } body += inlineMarkup(line.mid(2)) + QStringLiteral("<br>"); continue; }
        if (quote) { body += QStringLiteral("</blockquote>\n"); quote = false; }
        if (line.trimmed().isEmpty()) body += QStringLiteral("<br>\n");
        else {QString paragraph=line;while(index+1<lines.size()&&!lines[index+1].trimmed().isEmpty()&&!QRegularExpression(QStringLiteral("^(#{1,6})\\s+|^\\s*[-+*]\\s+|^\\s*\\d+[.)]\\s+|^> ")).match(lines[index+1]).hasMatch()){paragraph+=QLatin1Char('\n')+lines[++index];}body += QStringLiteral("<p>%1</p>\n").arg(inlineMarkup(paragraph).replace(QLatin1Char('\n'),QStringLiteral("<br>\n")));}
    }
    closeLists(); if (quote) body += QStringLiteral("</blockquote>\n"); if (code) body += QStringLiteral("</code></pre>\n");
    const QString style = css.isEmpty() ? QStringLiteral("body{font-family:sans-serif;margin:20px;line-height:1.5}pre,code{font-family:monospace;background:#f3f3f3}pre{padding:10px;overflow:auto}img{max-width:100%}table{border-collapse:collapse}th,td{border:1px solid #bbb;padding:4px 8px}.task-list-item{list-style:none}") : css;
    return QStringLiteral("<!doctype html><html><head><meta charset=\"utf-8\"><style>%1</style></head><body>%2</body></html>").arg(style, body);
}
}
