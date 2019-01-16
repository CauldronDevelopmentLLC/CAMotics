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

#include "SidebarWidget.h"

#include "NCEdit.h"

using namespace CAMotics;


Q_DECLARE_TYPEINFO(CAMotics::BlockInfo, Q_PRIMITIVE_TYPE);


SidebarWidget::SidebarWidget(NCEdit *editor)
  : QWidget(editor), foldIndicatorWidth(0) {
  backgroundColor = Qt::lightGray;
  lineNumberColor = Qt::black;
  indicatorColor = Qt::white;
  foldIndicatorColor = Qt::lightGray;
}


void SidebarWidget::mousePressEvent(QMouseEvent *event) {
  if (foldIndicatorWidth > 0) {
    int xofs = width() - foldIndicatorWidth;
    int lineNo = -1;
    int fh = fontMetrics().lineSpacing();
    int ys = event->pos().y();
    if (event->pos().x() > xofs) {
      foreach (BlockInfo ln, lineNumbers)
        if (ln.position < ys && (ln.position + fh) > ys) {
          if (ln.foldable) lineNo = ln.number;
          break;
        }
    }
    if (lineNo >= 0) {
      NCEdit *editor = qobject_cast<NCEdit*>(parent());
      if (editor) editor->toggleFold(lineNo);
    }
  }
}

void SidebarWidget::paintEvent(QPaintEvent *event) {
  QPainter p(this);
  p.fillRect(event->rect(), backgroundColor);
  p.setPen(lineNumberColor);
  p.setFont(font);
  int fh = QFontMetrics(font).height();
  foreach (BlockInfo ln, lineNumbers)
    p.drawText(0, ln.position, width() - 4 - foldIndicatorWidth, fh,
               Qt::AlignRight, QString::number(ln.number));

  if (foldIndicatorWidth > 0) {
    int xofs = width() - foldIndicatorWidth;
    p.fillRect(xofs, 0, foldIndicatorWidth, height(), indicatorColor);

    // initialize (or recreate) the arrow icons whenever necessary
    if (foldIndicatorWidth != rightArrowIcon.width()) {
      QPainter iconPainter;
      QPolygonF polygon;

      int dim = foldIndicatorWidth;
      rightArrowIcon = QPixmap(dim, dim);
      rightArrowIcon.fill(Qt::transparent);
      downArrowIcon = rightArrowIcon;

      polygon << QPointF(dim * 0.4, dim * 0.25);
      polygon << QPointF(dim * 0.4, dim * 0.75);
      polygon << QPointF(dim * 0.8, dim * 0.5);
      iconPainter.begin(&rightArrowIcon);
      iconPainter.setRenderHint(QPainter::Antialiasing);
      iconPainter.setPen(Qt::NoPen);
      iconPainter.setBrush(foldIndicatorColor);
      iconPainter.drawPolygon(polygon);
      iconPainter.end();

      polygon.clear();
      polygon << QPointF(dim * 0.25, dim * 0.4);
      polygon << QPointF(dim * 0.75, dim * 0.4);
      polygon << QPointF(dim * 0.5, dim * 0.8);
      iconPainter.begin(&downArrowIcon);
      iconPainter.setRenderHint(QPainter::Antialiasing);
      iconPainter.setPen(Qt::NoPen);
      iconPainter.setBrush(foldIndicatorColor);
      iconPainter.drawPolygon(polygon);
      iconPainter.end();
    }

    foreach (BlockInfo ln, lineNumbers)
      if (ln.foldable) {
        if (ln.folded) p.drawPixmap(xofs, ln.position, rightArrowIcon);
        else p.drawPixmap(xofs, ln.position, downArrowIcon);
      }
  }
}
