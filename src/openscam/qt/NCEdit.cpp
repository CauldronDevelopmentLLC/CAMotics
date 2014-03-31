/******************************************************************************\

    OpenSCAM is an Open-Source CAM software.
    Copyright (C) 2011-2014 Joseph Coffland <joseph@cauldrondevelopment.com>

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

using namespace OpenSCAM;


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
  TextBlockData *blockData = reinterpret_cast<TextBlockData *>(block.userData());

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


namespace OpenSCAM {
  class NCDocLayout: public QPlainTextDocumentLayout {
  public:
    NCDocLayout(QTextDocument *doc);
    void forceUpdate();
  };


  NCDocLayout::NCDocLayout(QTextDocument *doc) : QPlainTextDocumentLayout(doc) {
  }


  void NCDocLayout::forceUpdate() {
    emit documentSizeChanged(documentSize());
  }


  class NCEditPrivate {
  public:
    NCEdit *editor;
    NCDocLayout *layout;
    Highlighter *highlighter;
    SidebarWidget *sidebar;
    bool showLineNumbers;
    bool textWrap;
    QColor cursorColor;
    bool bracketsMatching;
    QList<int> matchPositions;
    QColor bracketMatchColor;
    QList<int> errorPositions;
    QColor bracketErrorColor;
    bool codeFolding : 1;
  };
}


NCEdit::NCEdit(QWidget *parent) :
  QPlainTextEdit(parent) , d_ptr(new NCEditPrivate) {
  d_ptr->editor = this;
  d_ptr->layout = new NCDocLayout(document());
  d_ptr->highlighter = new TPLHighlighter(document());
  d_ptr->sidebar = new SidebarWidget(this);
  d_ptr->showLineNumbers = true;
  d_ptr->textWrap = true;
  d_ptr->bracketsMatching = true;
  d_ptr->cursorColor = QColor(255, 255, 192);
  d_ptr->bracketMatchColor = QColor(180, 238, 180);
  d_ptr->bracketErrorColor = QColor(224, 128, 128);
  d_ptr->codeFolding = true;

  document()->setDocumentLayout(d_ptr->layout);

  connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(updateCursor()));
  connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateSidebar()));
  connect(this, SIGNAL(updateRequest(QRect, int)), this,
          SLOT(updateSidebar(QRect, int)));

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
}


NCEdit::~NCEdit() {
  delete d_ptr->layout;
}


void NCEdit::setColor(ColorComponent component, const QColor &color) {
  Q_D(NCEdit);

  switch (component) {
  case ColorComponent::Background: {
    QPalette pal = palette();
    pal.setColor(QPalette::Base, color);
    setPalette(pal);
    d->sidebar->indicatorColor = color;
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
    d->sidebar->backgroundColor = color;
    updateSidebar();
    break;

  case ColorComponent::LineNumber:
    d->sidebar->lineNumberColor = color;
    updateSidebar();
    break;

  case ColorComponent::Cursor:
    d->cursorColor = color;
    updateCursor();
    break;

  case ColorComponent::BracketMatch:
    d->bracketMatchColor = color;
    updateCursor();
    break;

  case ColorComponent::BracketError:
    d->bracketErrorColor = color;
    updateCursor();
    break;

  case ColorComponent::FoldIndicator:
    d->sidebar->foldIndicatorColor = color;
    updateSidebar();
    break;

  default:
    d->highlighter->setColor(component, color);
    updateCursor();
  }
}


QStringList NCEdit::keywords() const {
  return d_ptr->highlighter->keywords();
}


void NCEdit::setKeywords(const QStringList &keywords) {
  d_ptr->highlighter->setKeywords(keywords);
}


bool NCEdit::isLineNumbersVisible() const {
  return d_ptr->showLineNumbers;
}


void NCEdit::setLineNumbersVisible(bool visible) {
  d_ptr->showLineNumbers = visible;
  updateSidebar();
}


bool NCEdit::isTextWrapEnabled() const {
  return d_ptr->textWrap;
}


void NCEdit::setTextWrapEnabled(bool enable) {
  d_ptr->textWrap = enable;
  setLineWrapMode(enable ? WidgetWidth : NoWrap);
}


bool NCEdit::isBracketsMatchingEnabled() const {
  return d_ptr->bracketsMatching;
}


void NCEdit::setBracketsMatchingEnabled(bool enable) {
  d_ptr->bracketsMatching = enable;
  updateCursor();
}


bool NCEdit::isCodeFoldingEnabled() const {
  return d_ptr->codeFolding;
}


void NCEdit::setCodeFoldingEnabled(bool enable) {
  d_ptr->codeFolding = enable;
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
  Q_D(NCEdit);

  if (isReadOnly()) setExtraSelections(QList<QTextEdit::ExtraSelection>());

  else {
    d->matchPositions.clear();
    d->errorPositions.clear();

    if (d->bracketsMatching && textCursor().block().userData()) {
      QTextCursor cursor = textCursor();
      int cursorPosition = cursor.position();

      if (document()->characterAt(cursorPosition) == '{') {
        int matchPos = findClosingMatch(document(), cursorPosition);
        if (matchPos < 0) d->errorPositions += cursorPosition;
        else {
          d->matchPositions += cursorPosition;
          d->matchPositions += matchPos;
        }
      }

      if (document()->characterAt(cursorPosition - 1) == '}') {
        int matchPos = findOpeningMatch(document(), cursorPosition);
        if (matchPos < 0) d->errorPositions += cursorPosition - 1;
        else {
          d->matchPositions += cursorPosition - 1;
          d->matchPositions += matchPos;
        }
      }
    }

    QTextEdit::ExtraSelection highlight;
    highlight.format.setBackground(d->cursorColor);
    highlight.format.setProperty(QTextFormat::FullWidthSelection, true);
    highlight.cursor = textCursor();
    highlight.cursor.clearSelection();

    QList<QTextEdit::ExtraSelection> extraSelections;
    extraSelections.append(highlight);

    for (int i = 0; i < d->matchPositions.count(); ++i) {
      int pos = d->matchPositions.at(i);
      QTextEdit::ExtraSelection matchHighlight;
      matchHighlight.format.setBackground(d->bracketMatchColor);
      matchHighlight.cursor = textCursor();
      matchHighlight.cursor.setPosition(pos);
      matchHighlight.cursor.setPosition(pos + 1, QTextCursor::KeepAnchor);
      extraSelections.append(matchHighlight);
    }

    for (int i = 0; i < d->errorPositions.count(); ++i) {
      int pos = d->errorPositions.at(i);
      QTextEdit::ExtraSelection errorHighlight;
      errorHighlight.format.setBackground(d->bracketErrorColor);
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


void NCEdit::updateSidebar() {
  Q_D(NCEdit);

  if (!d->showLineNumbers && !d->codeFolding) {
    d->sidebar->hide();
    setViewportMargins(0, 0, 0, 0);
    d->sidebar->setGeometry(3, 0, 0, height());
    return;
  }

  d->sidebar->foldIndicatorWidth = 0;
  d->sidebar->font = this->font();
  d->sidebar->show();

  int sw = 0;
  if (d->showLineNumbers) {
    int digits = 2;
    int maxLines = blockCount();
    for (int number = 10; number < maxLines; number *= 10)
      ++digits;
    sw += fontMetrics().width('w') * digits;
  }

  if (d->codeFolding) {
    int fh = fontMetrics().lineSpacing();
    int fw = fontMetrics().width('w');
    d->sidebar->foldIndicatorWidth = qMax(fw, fh);
    sw += d->sidebar->foldIndicatorWidth;
  }
  setViewportMargins(sw, 0, 0, 0);

  d->sidebar->setGeometry(0, 0, sw, height());
  QRectF sidebarRect(0, 0, sw, height());

  QTextBlock block = firstVisibleBlock();
  int index = 0;
  while (block.isValid()) {
    if (block.isVisible()) {
      QRectF rect = blockBoundingGeometry(block).translated(contentOffset());
      if (sidebarRect.intersects(rect)) {
        if (d->sidebar->lineNumbers.count() >= index)
          d->sidebar->lineNumbers.resize(index + 1);
        d->sidebar->lineNumbers[index].position = rect.top();
        d->sidebar->lineNumbers[index].number = block.blockNumber() + 1;
        d->sidebar->lineNumbers[index].foldable =
          d->codeFolding ? isFoldable(block.blockNumber() + 1) : false;
        d->sidebar->lineNumbers[index].folded =
          d->codeFolding ? isFolded(block.blockNumber() + 1) : false;
        ++index;
      }

      if (rect.top() > sidebarRect.bottom()) break;
    }

    block = block.next();
  }

  d->sidebar->lineNumbers.resize(index);
  d->sidebar->update();
}


void NCEdit::mark(const QString &str, Qt::CaseSensitivity sens) {
  d_ptr->highlighter->mark(str, sens);
}
