/******************************************************************************\

    CAMotics is an Open-Source CAM software.
    Copyright (C) 2011-2015 Joseph Coffland <joseph@cauldrondevelopment.com>

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


#ifndef CAMOTICS_NCEDIT_H
#define CAMOTICS_NCEDIT_H

#include "ColorComponent.h"

#include <cbang/SmartPointer.h>

#include <QtGui>


namespace CAMotics {
  class NCFile;
  class NCDocLayout;
  class Highlighter;
  class SidebarWidget;

  class NCEdit: public QPlainTextEdit {
    Q_OBJECT;
    Q_PROPERTY(bool bracketsMatchingEnabled READ isBracketsMatchingEnabled
               WRITE setBracketsMatchingEnabled);
    Q_PROPERTY(bool codeFoldingEnabled READ isCodeFoldingEnabled
               WRITE setCodeFoldingEnabled);
    Q_PROPERTY(bool lineNumbersVisible READ isLineNumbersVisible
               WRITE setLineNumbersVisible);
    Q_PROPERTY(bool textWrapEnabled READ isTextWrapEnabled
               WRITE setTextWrapEnabled);

    cb::SmartPointer<NCFile> file;
    cb::SmartPointer<NCDocLayout> layout;
    cb::SmartPointer<Highlighter> highlighter;
    cb::SmartPointer<SidebarWidget> sidebar;

    bool showLineNumbers;
    bool textWrap;
    bool bracketsMatching;
    QColor cursorColor;
    QList<int> matchPositions;
    QColor bracketMatchColor;
    QList<int> errorPositions;
    QColor bracketErrorColor;
    bool codeFolding;

    Q_DISABLE_COPY(NCEdit);

  public:
    NCEdit(const cb::SmartPointer<NCFile> &file,
           const cb::SmartPointer<Highlighter> &highlighter,
           QWidget *parent = 0);
    ~NCEdit();

    const cb::SmartPointer<NCFile> &getFile() const {return file;}
    bool isTPL() const;

    void loadLightScheme();
    void loadDarkScheme();

    void setColor(ColorComponent component, const QColor &color);

    QStringList keywords() const;
    void setKeywords(const QStringList &keywords);

    bool isBracketsMatchingEnabled() const;
    bool isCodeFoldingEnabled() const;
    bool isLineNumbersVisible() const;
    bool isTextWrapEnabled() const;

    bool isFoldable(int line) const;
    bool isFolded(int line) const;

  public slots:
    void updateSidebar();
    void mark(const QString &str,
              Qt::CaseSensitivity sens = Qt::CaseInsensitive);
    void setBracketsMatchingEnabled(bool enable);
    void setCodeFoldingEnabled(bool enable);
    void setLineNumbersVisible(bool visible);
    void setTextWrapEnabled(bool enable);

    void fold(int line);
    void unfold(int line);
    void toggleFold(int line);

  protected:
    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);
    void resizeEvent(QResizeEvent *e);
    void wheelEvent(QWheelEvent *e);

  private slots:
    void updateCursor();
    void updateSidebar(const QRect &rect, int d);
  };
}

#endif // CAMOTICS_NCEDIT_H

