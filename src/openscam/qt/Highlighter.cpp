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

#include "Highlighter.h"

using namespace OpenSCAM;


Highlighter::Highlighter(QTextDocument *parent)
  : QSyntaxHighlighter(parent), m_markCaseSensitivity(Qt::CaseInsensitive) {

  // Default color scheme
  m_colors[ColorComponent::Normal]     = QColor("#000000");
  m_colors[ColorComponent::Comment]    = QColor("#808080");
  m_colors[ColorComponent::Number]     = QColor("#008000");
  m_colors[ColorComponent::String]     = QColor("#800000");
  m_colors[ColorComponent::Operator]   = QColor("#808000");
  m_colors[ColorComponent::Identifier] = QColor("#000020");
  m_colors[ColorComponent::Keyword]    = QColor("#000080");
  m_colors[ColorComponent::BuiltIn]    = QColor("#008080");
  m_colors[ColorComponent::Marker]     = QColor("#ffff00");
}


void Highlighter::setColor(ColorComponent component, const QColor &color) {
  m_colors[component] = color;
  rehighlight();
}


void Highlighter::mark(const QString &str,
                       Qt::CaseSensitivity caseSensitivity) {
  m_markString = str;
  m_markCaseSensitivity = caseSensitivity;
  rehighlight();
}


QStringList Highlighter::keywords() const {
  return m_keywords.toList();
}


void Highlighter::setKeywords(const QStringList &keywords) {
  m_keywords = QSet<QString>::fromList(keywords);
  rehighlight();
}
