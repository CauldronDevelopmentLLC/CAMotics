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

#include "Layout.h"

#include "PCB.h"

#include <cbang/Exception.h>

#include <set>
#include <limits>

using namespace std;
using namespace cb;
using namespace OpenSCAM::PCB;


bool Layout::initialized = false;
Layout::factory_map_t Layout::factories;


void Layout::addFactory(FactoryBase *factory) {
  Object *tmp = factory->create();
  factories[tmp->getName()] = factory;
  delete tmp;
}


void Layout::initialize() {
  if (initialized) return;

  addFactory(new Factory<Arc>);
  addFactory(new Factory<ElementArc>);
  addFactory(new Factory<Element>);
  addFactory(new Factory<ElementLine>);
  addFactory(new Factory<Layer>);
  addFactory(new Factory<Layout>);
  addFactory(new Factory<Line>);
  addFactory(new Factory<NetList>);
  addFactory(new Factory<Pad>);
  addFactory(new Factory<Pin>);
  addFactory(new Factory<Point>);
  addFactory(new Factory<Polygon>);
  addFactory(new Factory<Symbol>);
  addFactory(new Factory<Text>);
  addFactory(new Factory<Via>);

  initialized = true;
}


SmartPointer<Object> Layout::create(const string &name) {
  initialize();

  factory_map_t::iterator it = factories.find(name);
  if (it == factories.end()) return 0;
  return it->second->create();
}


void Layout::findByName(const string &name,
                        vector<SmartPointer<Object> > &objs) const {
  for (const_iterator it = objects.begin(); it != objects.end(); it++)
    if ((*it)->getName() == name) objs.push_back(*it);
}


SmartPointer<Layer> Layout::findLayer(int number) const {
  for (const_iterator it = objects.begin(); it != objects.end(); it++) {
    if (string((*it)->getName()) != "Layer") continue;
    SmartPointer<Layer> layer = it->cast<Layer>();
    if (layer->number == number) return layer;
  }

  return 0;
}


void Layout::rotate(const Point &center, double angle) {
  for (iterator it = objects.begin(); it != objects.end(); it++)
    (*it)->rotate(center, angle);
}


void Layout::translate(const Point &t) {
  for (iterator it = objects.begin(); it != objects.end(); it++)
    (*it)->translate(t);
}


void Layout::multiply(double m) {
  for (iterator it = objects.begin(); it != objects.end(); it++)
    (*it)->multiply(m);
}


void Layout::round(int x) {
  for (const_iterator it = objects.begin(); it != objects.end(); it++)
    (*it)->round(x); 
}


void Layout::write(ostream &stream) const {
  for (const_iterator it = objects.begin(); it != objects.end(); it++) {
    stream << (*it)->prefix;
    (*it)->write(stream);
  }

  stream << suffix;
}


void Layout::read(istream &stream) {
  string s;
  bool comment = false;
  int start = -1;
  int depth = 0;

  while (!stream.fail()) {
    int c = stream.get();
    if (c == -1) break;
    s += c;

    switch (c) {
    case '\n':
    case '\r':
      comment = false;
      break;

    case '#': comment = true; break;

    default:
      if (!comment) {
        if (c == ')') depth--;
        if (c == '(') depth++;

        if (start == -1) {
          if (isalpha(c)) start = s.length() - 1;

        } else if (!isalpha(c)) {
          string word = s.substr(start, s.length() - 1 - start);
          SmartPointer<Object> o = create(word);

          if (!o.isNull()) {
            o->prefix = s.substr(0, start);
            s = "";

            if (isspace(c)) {
              readWS(stream);
              c = stream.get();
            }

            if (c != '[' && c != '(') THROWS("missing '[' or '(' on " << word);
            if (c == '(') depth--;

            o->read(stream);

            readWS(stream);
            c = stream.get();
            if (c != -1 && c != ')' && c != ']')
              THROWS("missing ']' or ')' on " << word);

            objects.push_back(o);
          }

          start = -1;
        }
      }
    }

    if (depth < 0) {
      stream.unget();
      if (s.length()) s = s.substr(0, s.length() - 1);
      break;
    }
  }

  if (s.length()) suffix = s;
}


void Layout::bounds(Point &min, Point &max) const {
  for (const_iterator it = objects.begin(); it != objects.end(); it++)
    (*it)->bounds(min, max);
}


void Layout::centerMark() {
  for (iterator it = objects.begin(); it != objects.end(); it++)
    if (it->isInstance<Layout>()) it->cast<Layout>()->centerMark();
}


void Layout::subtractMask(int clear) {
  for (iterator it = objects.begin(); it != objects.end(); it++)
    if (it->isInstance<Layout>()) it->cast<Layout>()->subtractMask(clear);
}


Point Layout::findCenter() {
  Point min(numeric_limits<int>::max(), numeric_limits<int>::max());
  Point max(-numeric_limits<int>::max(), -numeric_limits<int>::max());
  bounds(min, max);
  return (max - min) / 2 + min;
}


void Layout::center() {
  translate(-findCenter());
}


void Layout::flipX(double x) {
  for (const_iterator it = objects.begin(); it != objects.end(); it++)
    (*it)->flipX(x);
}


void Layout::flipY(double y) {
  for (const_iterator it = objects.begin(); it != objects.end(); it++)
    (*it)->flipY(y);
}


void Layout::lineSize(int size) {
  for (const_iterator it = objects.begin(); it != objects.end(); it++)
    (*it)->lineSize(size);
}


void Layout::textScale(int scale) {
  for (const_iterator it = objects.begin(); it != objects.end(); it++)
    (*it)->textScale(scale);
}


void Layout::merge(const Layout &layout) {
  string last;
  set<int> foundLayers;
  bool foundNetList = false;

  for (iterator it = objects.begin(); it != objects.end(); it++) {
    SmartPointer<Object> o = *it;

    if (string(o->getName()) != "Via" && last == "Via") {
      vector<SmartPointer<Object> > objs;
      layout.findByName("Via", objs);
      objects.insert(it, objs.begin(), objs.end());
    }

    if (string(o->getName()) != "Element" && last == "Element") {
      vector<SmartPointer<Object> > objs;
      layout.findByName("Element", objs);
      objects.insert(it, objs.begin(), objs.end());
    }

    if (string(o->getName()) == "Layer") {
      SmartPointer<Layer> layer1 = o.cast<Layer>();
      SmartPointer<Layer> layer2 = layout.findLayer(layer1->number);
      if (!layer2.isNull()) layer1->merge(*layer2);

      foundLayers.insert(layer1->number);
    }

    if (string(o->getName()) != "Layer" && last == "Layer") {
      vector<SmartPointer<Object> > layers;
      vector<SmartPointer<Object> > objs;
      layout.findByName("Layer", layers);

      for (unsigned int i = 0; i < layers.size(); i++) {
        SmartPointer<Layer> layer = layers[i].cast<Layer>();
        if (foundLayers.find(layer->number) == foundLayers.end())
          objs.push_back(layer);
      }

      objects.insert(it, objs.begin(), objs.end());
    }

    if (string(o->getName()) == "NetList") {
      SmartPointer<NetList> netList1 = o.cast<NetList>();
      vector<SmartPointer<Object> > objs;
      layout.findByName("NetList", objs);

      if (objs.size()) {
        SmartPointer<NetList> netList2 = objs[0].cast<NetList>();
        netList1->merge(*netList2);
        foundNetList = true;
      }
    }

    last = o->getName();
  }

  if (!foundNetList) {
    vector<SmartPointer<Object> > objs;
    layout.findByName("NetList", objs);
    objects.insert(objects.end(), objs.begin(), objs.end());
  }
}
