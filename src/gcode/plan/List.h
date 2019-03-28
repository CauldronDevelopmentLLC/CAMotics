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

#include <cbang/Exception.h>


namespace GCode {
  template <typename T>
  class List {
    T *head;
    T *tail;
    unsigned length;

  public:
    List() : head(0), tail(0), length(0) {}
    ~List() {clear();}


    unsigned size() const {return length;}
    bool empty() const {return !head;}
    T *front() const {return head;}
    T *back() const {return tail;}
    void clear() {while (!empty()) delete pop_front();}


    T *pop_front() {
      if (empty()) THROW("Empty list");

      T *x = head;
      if (head == tail) tail = 0;
      head = x->next;
      if (x->next) x->next->prev = 0;
      x->next = 0;

      length--;
      return x;
    }


    T *pop_back() {
      if (empty()) THROW("Empty list");

      T *x = tail;
      if (tail == head) head = 0;
      tail = x->prev;
      if (x->prev) x->prev->next = 0;
      x->prev = 0;

      length--;
      return x;
    }


    void push_front(T *x) {
      if (!x) THROW("Cannot push null");
      if (x->next || x->prev) THROW("Item already in list");

      if (empty()) head = tail = x;
      else {
        x->next = head;
        head->prev = x;
        head = x;
      }

      length++;
    }


    void push_back(T *x) {
      if (!x) THROW("Cannot push null");
      if (x->next || x->prev) THROW("Item already in list");

      if (empty()) head = tail = x;
      else {
        x->prev = tail;
        tail->next = x;
        tail = x;
      }

      length++;
    }


    void remove(T *x) {
      if (!x) THROW("Cannot remove null");
      if (!x->next && x != tail) THROW("Item not at list tail");
      if (!x->prev && x != head) THROW("Item not at list head");
      if (!length) THROW("Cannot remove item from empty list");

      if (head == x) head = x->next;
      if (tail == x) tail = x->prev;

      if (x->next) x->next->prev = x->prev;
      if (x->prev) x->prev->next = x->next;
      x->next = x->prev = 0;
      length--;
    }
  };
}
