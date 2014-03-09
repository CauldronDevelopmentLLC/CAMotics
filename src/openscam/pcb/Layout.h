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

#ifndef OPENSCAM_PCB_LAYOUT_H
#define OPENSCAM_PCB_LAYOUT_H

#include <list>
#include <map>
#include <vector>

#include "Object.h"
#include "Factory.h"

#include <cbang/SmartPointer.h>

namespace OpenSCAM {
  namespace PCB {
    class Layer;

    class Layout : public Object {
      static bool initialized;
      typedef std::map<std::string, FactoryBase *> factory_map_t;
      static factory_map_t factories;

      static void initialize();
      static void addFactory(FactoryBase *factory);

    protected:
      typedef std::list<cb::SmartPointer<Object> > objects_t;
      objects_t objects;

      typedef objects_t::iterator iterator;
      typedef objects_t::const_iterator const_iterator;

    public:
      std::string suffix;

      static cb::SmartPointer<Object> create(const std::string &name);
      void findByName(const std::string &name,
                      std::vector<cb::SmartPointer<Object> > &objs) const;
      cb::SmartPointer<Layer> findLayer(int number) const;

      // From Object
      const char *getName() const {return "Layout";}
      void rotate(const Point &center, double angle);
      void translate(const Point &t);
      void multiply(double m);
      void round(int x);
      void write(std::ostream &stream) const;
      void read(std::istream &stream);
      void bounds(Point &min, Point &max) const;
      void flipX(double x);
      void flipY(double x);
      void lineSize(int size);
      void textScale(int scale);

      virtual void centerMark();
      virtual void subtractMask(int clear);

      Point findCenter();
      void center();
      void merge(const Layout &layout);
    };
  }
}

#endif // OPENSCAM_PCB_LAYOUT_H

