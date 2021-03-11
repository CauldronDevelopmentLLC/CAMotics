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


#include "MachineAdapter.h"

#include <gcode/Units.h>


namespace GCode {
  class MachineUnitAdapter : virtual public MachineAdapter {
  protected:
    Units units;
    Units targetUnits;

  public:
    MachineUnitAdapter(Units units = METRIC, Units targetUnits = METRIC) :
      units(units), targetUnits(targetUnits) {}

    bool isMetric() const {return units == METRIC;}
    void setMetric() {setUnits(METRIC);}

    bool isImperial() const {return units == IMPERIAL;}
    void setImperial() {setUnits(IMPERIAL);}

    /// @return the currently programmed units.
    Units getUnits() const {return units;}

    /***
     * Set the active units, IMPERIAL or METRIC.
     * @throw cb::Exception if @param units is invalid.
     */
    void setUnits(Units units) {this->units = units;}
    void setTargetUnits(Units units) {targetUnits = units;}

    // From MachineInterface
    double getFeed() const;
    void setFeed(double feed);

    double getSpeed() const;
    void setSpeed(double speed);

    void setPathMode(path_mode_t mode, double motionBlending, double naiveCAM);

    Axes getPosition() const;
    cb::Vector3D getPosition(axes_t axes) const;
    void setPosition(const Axes &position);

    void move(const Axes &position, int axes, bool rapid, double time);
    void arc(const cb::Vector3D &offset, const cb::Vector3D &target,
             double angle, plane_t plane);

    double mmInchIn() const;
    double mmInchOut() const;
    double meterFootIn() const;
    double meterFootOut() const;
  };
}
