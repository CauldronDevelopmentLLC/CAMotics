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

#pragma once


#include <boost/iostreams/categories.hpp>   // sink_tag
#include <boost/iostreams/positioning.hpp>  // stream_offset
#include <boost/iostreams/stream.hpp>

#include <QPlainTextEdit>


namespace CAMotics {
  class QTextDevice {
    QPlainTextEdit *textEdit;

  public:
    typedef char char_type;
    typedef boost::iostreams::sink_tag category;

    QTextDevice(QPlainTextEdit *textEdit) : textEdit(textEdit) {}

    std::streamsize write(const char_type *s, std::streamsize n) {
      textEdit->appendPlainText(QByteArray(s, n));
      return n;
    }
  };

  typedef boost::iostreams::stream<QTextDevice> QTextStream;
}
