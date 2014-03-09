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

#ifndef TPLANG_MACHINE_UNIT_ADAPTER_H
#define TPLANG_MACHINE_UNIT_ADAPTER_H

#include "MachineAdapter.h"


namespace tplang {
  class MachineUnitAdapter : virtual public MachineAdapter {
  public:
    typedef enum {IMPERIAL, METRIC} units_t;

  protected:
    units_t units;

  public:
    MachineUnitAdapter(units_t units = METRIC) : units(units) {}

    bool isMetric() const {return units == METRIC;}
    void setMetric() {setUnits(METRIC);}

    bool isImperial() const {return units == IMPERIAL;}
    void setImperial() {setUnits(IMPERIAL);}

    /// @return the currently programed units.
    units_t getUnits() const {return units;}

    /***
     * Set the active units, IMPERIAL or METRIC.
     * @throw cb::Exception if @param units is invalid.
     */
    void setUnits(units_t units) {this->units = units;}

    // From tplang::MachineInterface
    double getFeed(feed_mode_t *mode = 0) const;
    void setFeed(double feed, feed_mode_t mode = MM_PER_MINUTE);

    double getSpeed(spin_mode_t *mode = 0, double *max = 0) const;
    void setSpeed(double speed, spin_mode_t mode = REVOLUTIONS_PER_MINUTE,
                  double max = 0);

    Axes getPosition() const;
    cb::Vector3D getPosition(axes_t axes) const;

    void move(const Axes &axes, bool rapid = false);
    void arc(const cb::Vector3D &offset, double angle, plane_t plane = XY);
  };
}

#endif // TPLANG_MACHINE_UNIT_ADAPTER_H

