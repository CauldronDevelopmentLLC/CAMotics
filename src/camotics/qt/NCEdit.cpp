/******************************************************************************\

  CAMotics is an Open-Source simulation and CAM software.
  Copyright (C) 2011-2019 Joseph Coffland <joseph@cauldrondevelopment.com>

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

\******************************************************************************/

/*
  This file was derived from JSEdit from the Ofi Labs X2 project.

  Copyright (C) 2011 Ariya Hidayat <ariya.hidayat@gmail.com>
  Copyright (C) 2010 Ariya Hidayat <ariya.hidayat@gmail.com>

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.
  * Neither the name of the <organization> nor the
  names of its contributors may be used to endorse or promote products
  derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "NCEdit.h"

#include "TPLHighlighter.h"
#include "SidebarWidget.h"
#include "FileTabManager.h"

#include <cbang/SmartPointer.h>
#include <cbang/log/Logger.h>

using namespace cb;
using namespace CAMotics;


static int findClosingMatch(const QTextDocument *doc, int cursorPosition) {
  QTextBlock block = doc->findBlock(cursorPosition);
  TextBlockData *blockData =
    reinterpret_cast<TextBlockData *>(block.userData());

  if (!blockData->bracketPositions.isEmpty()) {
    int depth = 1;

    while (block.isValid()) {
      blockData = reinterpret_cast<TextBlockData *>(block.userData());

      if (blockData && !blockData->bracketPositions.isEmpty()) {
        for (int c = 0; c < blockData->bracketPositions.count(); ++c) {
          int absPos = block.position() + blockData->bracketPositions.at(c);
          if (absPos <= cursorPosition)

            continue;
          if (doc->characterAt(absPos) == '{') depth++;
          else depth--;

          if (depth == 0) return absPos;
        }
      }
      block = block.next();
    }
  }

  return -1;
}


static int findOpeningMatch(const QTextDocument *doc, int cursorPosition) {
  QTextBlock block = doc->findBlock(cursorPosition);
  TextBlockData *blockData =
    reinterpret_cast<TextBlockData *>(block.userData());

  if (!blockData->bracketPositions.isEmpty()) {
    int depth = 1;

    while (block.isValid()) {
      blockData = reinterpret_cast<TextBlockData *>(block.userData());

      if (blockData && !blockData->bracketPositions.isEmpty()) {
        for (int c = blockData->bracketPositions.count() - 1; c >= 0; --c) {
          int absPos = block.position() + blockData->bracketPositions.at(c);

          if (absPos >= cursorPosition - 1) continue;
          if (doc->characterAt(absPos) == '}') depth++;
          else depth--;
          if (depth == 0) return absPos;
        }
      }
      block = block.previous();
    }
  }
  return -1;
}


namespace CAMotics {
  class NCDocLayout: public QPlainTextDocumentLayout {
  public:
    NCDocLayout(QTextDocument *doc);
    void forceUpdate();
  };


  NCDocLayout::NCDocLayout(QTextDocument *doc) :
    QPlainTextDocumentLayout(doc) {}


  void NCDocLayout::forceUpdate() {
    emit documentSizeChanged(documentSize());
  }
}


NCEdit::NCEdit(const SmartPointer<Project::File> &file,
               const SmartPointer<Highlighter> &highlighter,
               FileTabManager *parent) :
  QPlainTextEdit(parent), parent(parent), file(file),
  layout(new NCDocLayout(document())), highlighter(highlighter),
  sidebar(new SidebarWidget(this)), showLineNumbers(true), textWrap(true),
  bracketsMatching(true), cursorColor(QColor(255, 255, 192)),
  bracketMatchColor(QColor(180, 238, 180)),
  bracketErrorColor(QColor(224, 128, 128)), codeFolding(true) {

  highlighter->setDocument(document());
  document()->setDocumentLayout(layout.get());

  connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(updateCursor()));
  connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateSidebar()));
  connect(this, SIGNAL(updateRequest(QRect, int)), this,
          SLOT(updateSidebar(QRect, int)));
  connect(this, SIGNAL(modificationChanged(bool)),
          SLOT(modificationChanged(bool)));

#if defined(Q_OS_MAC)
  QFont textFont = font();
  textFont.setPointSize(12);
  textFont.setFamily("Monaco");
  setFont(textFont);

#elif defined(Q_OS_UNIX)
  QFont textFont = font();
  textFont.setFamily("Monospace");
  setFont(textFont);
#endif

  loadDarkScheme();
}


NCEdit::~NCEdit() {}


bool NCEdit::isTPL() const {
  return highlighter.isInstance<TPLHighlighter>();
}


void NCEdit::loadLightScheme() {
  setColor(ColorComponent::Normal, QColor("#000000"));
  setColor(ColorComponent::Comment, QColor("#808080"));
  setColor(ColorComponent::Number, QColor("#008000"));
  setColor(ColorComponent::String, QColor("#800000"));
  setColor(ColorComponent::Operator, QColor("#808000"));
  setColor(ColorComponent::Identifier, QColor("#000020"));
  setColor(ColorComponent::Keyword, QColor("#000080"));
  setColor(ColorComponent::BuiltIn, QColor("#008080"));
  setColor(ColorComponent::Marker, QColor("#ffff00"));
}


void NCEdit::loadDarkScheme() {
  setColor(ColorComponent::Background, QColor("#343434"));
  setColor(ColorComponent::Normal, QColor("#d9d9d9"));
  setColor(ColorComponent::Comment, QColor("#ff7f24"));
  setColor(ColorComponent::Number, QColor("#eedd82"));
  setColor(ColorComponent::String, QColor("#ffa07a"));
  setColor(ColorComponent::Operator, QColor("#ce6926"));
  setColor(ColorComponent::Identifier, QColor("#d9d9d9"));
  setColor(ColorComponent::Keyword, QColor("#00ffff"));
  setColor(ColorComponent::BuiltIn, QColor("#98fb98"));
  setColor(ColorComponent::Cursor, QColor("#222222"));
  setColor(ColorComponent::Marker, QColor("#d9d9d9"));
  setColor(ColorComponent::BracketMatch, QColor("#cd0000"));
}


void NCEdit::setColor(ColorComponent component, const QColor &color) {
  switch (component) {
  case ColorComponent::Background: {
    QPalette pal = palette();
    pal.setColor(QPalette::Base, color);
    setPalette(pal);
    sidebar->indicatorColor = color;
    updateSidebar();
    break;
  }

  case ColorComponent::Normal: {
    QPalette pal = palette();
    pal.setColor(QPalette::Text, color);
    setPalette(pal);
    break;
  }

  case ColorComponent::Sidebar:
    sidebar->backgroundColor = color;
    updateSidebar();
    break;

  case ColorComponent::LineNumber:
    sidebar->lineNumberColor = color;
    updateSidebar();
    break;

  case ColorComponent::Cursor:
    cursorColor = color;
    updateCursor();
    break;

  case ColorComponent::BracketMatch:
    bracketMatchColor = color;
    updateCursor();
    break;

  case ColorComponent::BracketError:
    bracketErrorColor = color;
    updateCursor();
    break;

  case ColorComponent::FoldIndicator:
    sidebar->foldIndicatorColor = color;
    updateSidebar();
    break;

  default:
    highlighter->setColor(component, color);
    updateCursor();
  }
}


QStringList NCEdit::keywords() const {
  return highlighter->getKeywords();
}


void NCEdit::setKeywords(const QStringList &keywords) {
  highlighter->setKeywords(keywords);
}


bool NCEdit::isLineNumbersVisible() const {
  return showLineNumbers;
}


void NCEdit::setLineNumbersVisible(bool visible) {
  showLineNumbers = visible;
  updateSidebar();
}


bool NCEdit::isTextWrapEnabled() const {
  return textWrap;
}


void NCEdit::setTextWrapEnabled(bool enable) {
  textWrap = enable;
  setLineWrapMode(enable ? WidgetWidth : NoWrap);
}


bool NCEdit::isBracketsMatchingEnabled() const {
  return bracketsMatching;
}


void NCEdit::setBracketsMatchingEnabled(bool enable) {
  bracketsMatching = enable;
  updateCursor();
}


bool NCEdit::isCodeFoldingEnabled() const {
  return codeFolding;
}


void NCEdit::setCodeFoldingEnabled(bool enable) {
  codeFolding = enable;
  updateSidebar();
}


static int findClosingConstruct(const QTextBlock &block) {
  if (!block.isValid()) return -1;

  TextBlockData *blockData = reinterpret_cast<TextBlockData*>(block.userData());
  if (!blockData) return -1;
  if (blockData->bracketPositions.isEmpty()) return -1;

  const QTextDocument *doc = block.document();
  int offset = block.position();

  foreach (int pos, blockData->bracketPositions) {
    int absPos = offset + pos;
    if (doc->characterAt(absPos) == '{') {
      int matchPos = findClosingMatch(doc, absPos);
      if (matchPos >= 0) return matchPos;
    }
  }

  return -1;
}


bool NCEdit::isFoldable(int line) const {
  int matchPos = findClosingConstruct(document()->findBlockByNumber(line - 1));
  if (matchPos >= 0) {
    QTextBlock matchBlock = document()->findBlock(matchPos);
    if (matchBlock.isValid() && matchBlock.blockNumber() > line) return true;
  }

  return false;
}


bool NCEdit::isFolded(int line) const {
  QTextBlock block = document()->findBlockByNumber(line - 1);

  if (!block.isValid()) return false;
  block = block.next();
  if (!block.isValid()) return false;

  return !block.isVisible();
}


bool NCEdit::findOnce(QString find, bool regex, int options) {
  QTextCursor cursor = textCursor();
  QTextCursor savedCursor = cursor;
  QTextDocument::FindFlag opts = (QTextDocument::FindFlag)options;

  if (regex) cursor = document()->find(QRegExp(find), cursor, opts);
  else cursor = document()->find(find, cursor, opts);

  if (!cursor.isNull()) {
    setTextCursor(cursor);
    return true;
  }

  setTextCursor(savedCursor);
  return false;
}


void NCEdit::find(QString find, bool regex, int options) {
  bool found = true;

  if (!findOnce(find, regex, options)) {
    QTextCursor savedCursor = textCursor();
    setTextCursor(QTextCursor()); // Wrap around
    if (!findOnce(find, regex, options)) {
      found = false;
      setTextCursor(savedCursor);
    }
  }

  emit findResult(found);
}


void NCEdit::replace(QString find, QString replace, bool regex, int options,
                     bool all) {
  bool found;

  do {
    textCursor().insertText(replace);
    found = findOnce(find, regex, options);
  } while (found && all);

  emit findResult(found);
}


void NCEdit::updateSidebar() {
  if (!showLineNumbers && !codeFolding) {
    sidebar->hide();
    setViewportMargins(0, 0, 0, 0);
    sidebar->setGeometry(3, 0, 0, height());
    return;
  }

  sidebar->foldIndicatorWidth = 0;
  sidebar->font = this->font();
  sidebar->show();

  int sw = 0;
  if (showLineNumbers) {
    int digits = 2;
    int maxLines = blockCount();
    for (int number = 10; number < maxLines; number *= 10)
      ++digits;
    sw += fontMetrics().width('w') * digits;
  }

  if (codeFolding) {
    int fh = fontMetrics().lineSpacing();
    int fw = fontMetrics().width('w');
    sidebar->foldIndicatorWidth = qMax(fw, fh);
    sw += sidebar->foldIndicatorWidth;
  }
  setViewportMargins(sw, 0, 0, 0);

  sidebar->setGeometry(0, 0, sw, height());
  QRectF sidebarRect(0, 0, sw, height());

  QTextBlock block = firstVisibleBlock();
  int index = 0;
  while (block.isValid()) {
    if (block.isVisible()) {
      QRectF rect = blockBoundingGeometry(block).translated(contentOffset());
      if (sidebarRect.intersects(rect)) {
        if (sidebar->lineNumbers.count() >= index)
          sidebar->lineNumbers.resize(index + 1);

        sidebar->lineNumbers[index].position = rect.top();
        sidebar->lineNumbers[index].number = block.blockNumber() + 1;
        sidebar->lineNumbers[index].foldable =
          codeFolding ? isFoldable(block.blockNumber() + 1) : false;
        sidebar->lineNumbers[index].folded =
          codeFolding ? isFolded(block.blockNumber() + 1) : false;

        index++;
      }

      if (rect.top() > sidebarRect.bottom()) break;
    }

    block = block.next();
  }

  sidebar->lineNumbers.resize(index);
  sidebar->update();
}


void NCEdit::mark(const QString &str, Qt::CaseSensitivity sens) {
  highlighter->mark(str, sens);
}


void NCEdit::fold(int line) {
  QTextBlock startBlock = document()->findBlockByNumber(line - 1);
  int endPos = findClosingConstruct(startBlock);
  if (endPos < 0) return;
  QTextBlock endBlock = document()->findBlock(endPos);

  QTextBlock block = startBlock.next();
  while (block.isValid() && block != endBlock) {
    block.setVisible(false);
    block.setLineCount(0);
    block = block.next();
  }

  document()->markContentsDirty(startBlock.position(),
                                endPos - startBlock.position() + 1);
  updateSidebar();
  update();

  NCDocLayout *layout =
    reinterpret_cast<NCDocLayout*>(document()->documentLayout());
  layout->forceUpdate();
}


void NCEdit::unfold(int line) {
  QTextBlock startBlock = document()->findBlockByNumber(line - 1);
  int endPos = findClosingConstruct(startBlock);

  QTextBlock block = startBlock.next();
  while (block.isValid() && !block.isVisible()) {
    block.setVisible(true);
    block.setLineCount(block.layout()->lineCount());
    endPos = block.position() + block.length();
    block = block.next();
  }

  document()->markContentsDirty(startBlock.position(),
                                endPos - startBlock.position() + 1);
  updateSidebar();
  update();

  NCDocLayout *layout =
    reinterpret_cast<NCDocLayout*>(document()->documentLayout());
  layout->forceUpdate();
}


void NCEdit::toggleFold(int line) {
  if (isFolded(line)) unfold(line);
  else fold(line);
}


void NCEdit::contextMenuEvent(QContextMenuEvent *e) {
  QMenu *menu = createStandardContextMenu();

  menu->addSeparator();
  menu->addAction(QIcon(":/icons/replace.png"), "&Find && Replace",
                  this, SIGNAL(find()))->setShortcut(QKeySequence::Find);
  menu->addAction(QIcon(":/icons/find.png"), "Find &Next", this,
                  SIGNAL(findNext()))->setShortcut(QKeySequence::FindNext);

  menu->exec(e->globalPos());
  delete menu;
}


void NCEdit::keyPressEvent(QKeyEvent *e) {
  if (e->key() == Qt::Key_Tab) insertPlainText("  ");
  else QPlainTextEdit::keyPressEvent(e);
}


void NCEdit::keyReleaseEvent(QKeyEvent *e) {
  if (e->key() != Qt::Key_Tab) QPlainTextEdit::keyReleaseEvent(e);

  if (e->matches(QKeySequence::Find)) emit find();
  if (e->matches(QKeySequence::FindNext)) emit findNext();
}


void NCEdit::resizeEvent(QResizeEvent *e) {
  QPlainTextEdit::resizeEvent(e);
  updateSidebar();
}


void NCEdit::wheelEvent(QWheelEvent *e) {
  if (e->modifiers() == Qt::ControlModifier) {
    int steps = e->delta() / 20;
    steps = qBound(-3, steps, 3);
    QFont textFont = font();
    int pointSize = textFont.pointSize() + steps;
    pointSize = qBound(10, pointSize, 40);
    textFont.setPointSize(pointSize);
    setFont(textFont);
    updateSidebar();
    e->accept();
    return;
  }

  QPlainTextEdit::wheelEvent(e);
}


void NCEdit::updateCursor() {
  if (isReadOnly()) setExtraSelections(QList<QTextEdit::ExtraSelection>());

  else {
    matchPositions.clear();
    errorPositions.clear();

    if (bracketsMatching && textCursor().block().userData()) {
      QTextCursor cursor = textCursor();
      int cursorPosition = cursor.position();

      if (document()->characterAt(cursorPosition) == '{') {
        int matchPos = findClosingMatch(document(), cursorPosition);

        if (matchPos < 0) errorPositions += cursorPosition;
        else {
          matchPositions += cursorPosition;
          matchPositions += matchPos;
        }
      }

      if (document()->characterAt(cursorPosition - 1) == '}') {
        int matchPos = findOpeningMatch(document(), cursorPosition);

        if (matchPos < 0) errorPositions += cursorPosition - 1;
        else {
          matchPositions += cursorPosition - 1;
          matchPositions += matchPos;
        }
      }
    }

    QTextEdit::ExtraSelection highlight;
    highlight.format.setBackground(cursorColor);
    highlight.format.setProperty(QTextFormat::FullWidthSelection, true);
    highlight.cursor = textCursor();
    highlight.cursor.clearSelection();

    QList<QTextEdit::ExtraSelection> extraSelections;
    extraSelections.append(highlight);

    for (int i = 0; i < matchPositions.count(); ++i) {
      int pos = matchPositions.at(i);
      QTextEdit::ExtraSelection matchHighlight;
      matchHighlight.format.setBackground(bracketMatchColor);
      matchHighlight.cursor = textCursor();
      matchHighlight.cursor.setPosition(pos);
      matchHighlight.cursor.setPosition(pos + 1, QTextCursor::KeepAnchor);
      extraSelections.append(matchHighlight);
    }

    for (int i = 0; i < errorPositions.count(); ++i) {
      int pos = errorPositions.at(i);
      QTextEdit::ExtraSelection errorHighlight;
      errorHighlight.format.setBackground(bracketErrorColor);
      errorHighlight.cursor = textCursor();
      errorHighlight.cursor.setPosition(pos);
      errorHighlight.cursor.setPosition(pos + 1, QTextCursor::KeepAnchor);
      extraSelections.append(errorHighlight);
    }

    setExtraSelections(extraSelections);
  }
}


void NCEdit::updateSidebar(const QRect &rect, int d) {
  Q_UNUSED(rect);
  if (d != 0) updateSidebar();
}


void NCEdit::modificationChanged(bool changed) {
  parent->on_modificationChanged(this, changed);
}
