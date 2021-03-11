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

#include "MachineEnum.h"
#include "Transforms.h"

#include <gcode/Axes.h>
#include <gcode/Units.h>
#include <gcode/Addresses.h>

#include <cbang/Exception.h>
#include <cbang/LocationRange.h>


namespace GCode {
  class MachineInterface : public MachineEnum, public UnitsEnumerationBase {
  public:
    virtual ~MachineInterface() {}

    virtual void start() = 0;
    virtual void end() = 0;

    /// @return the currently programmed feed rate.
    virtual double getFeed() const = 0;

    /***
     * Set the feed rate.  Feed rate is measured in millimeters.  If the feed
     * rate is 0, the default, then no moves can be made.
     *
     * @throw cb::Exception if @param feed is negative.
     */
    virtual void setFeed(double feed) = 0;

    /// @return the currently programmed feed mode.
    virtual feed_mode_t getFeedMode() const = 0;

    /***
     * Set the feed rate mode.
     *
     * UNITS_PER_MINUTE mode, the default, indicates that the controlled point
     * should move @param feed millimeters per minute.
     *
     * INVERSE_TIME mode indicates that each move should be completed in
     * 1 / @param feed minutes.  For example, if @param feed is 2, the move
     * should be completed in half a minute.
     *
     * UNITS_PER_REVOLUTION mode indicates the controlled point should move a
     * certain number of millimeters per revolution of the spindle.
     *
     * @throw cb::Exception if @param mode is invalid.
     */
    virtual void setFeedMode(feed_mode_t mode) = 0;

    /// @return the currently programmed spindle speed.
    virtual double getSpeed() const = 0;

    /***
     * Set the spindle speed.
     *
     * @param speed A positive value indicates clockwise spin.  A negative value
     * counterclockwise spin.  0 indicates no spin.
     */
    virtual void setSpeed(double speed) = 0;

    /***
     * @return the currently programmed spin mode and optionally the max speed.
     */
    virtual spin_mode_t getSpinMode(double *max = 0) const = 0;

    /***
     * Set the spin mode.
     *
     * @param mode REVOLUTIONS_PER_MINUTE, the default, indicates spin
     * measured in revolutions per minute.  CONSTANT_SURFACE_SPEED sets the
     * spindle speed in meters per minute.
     *
     * If @param mode is CONSTANT_SURFACE_SPEED and @param max is greater than
     * zero then @param max is the maximum spindle revolutions per minute.
     *
     * @throw cb::Exception @param mode is invalid.
     */
    virtual void setSpinMode(spin_mode_t mode = REVOLUTIONS_PER_MINUTE,
                             double max = 0) = 0;

    /***
     * Set path mode.
     *
     * @param mode EXACT_PATH_MODE, EXACT_STOP_MODE or CONTINUOUS_MODE.
     * @param motionBlending Motion blending tolerance.
     * @param naiveCAM Niave CAM tolerance.
     */
    virtual void setPathMode(path_mode_t mode, double motionBlending,
                             double naiveCAM) = 0;

    /***
     * Select the active tool.
     * This may cause some machines to run a tool change routine and return to
     * the current location.
     */
    virtual void changeTool(unsigned tool) = 0;

    /***
     * Wait for input change.
     *
     * @param port the input port to input on.
     * @param mode one of IMMEDIATE, RISE, FALL, HIGH or LOW
     * @param timeout the maximum amount of time to input or zero to input
     *   indefinitely.  It is an error of the port does not change to the
     *   specified state before the timeout expires.
     */
    virtual void input(port_t port, input_mode_t mode, double timeout) = 0;

    /***
     * Causes the immediately following move to pause when the sought input
     * changes state.
     *
     * @param port the input port.
     * @param active seeking the active state if true, inactive otherwise.
     * @param error if true, signal an error if the port state is not found
     *   before the move ends.
     */
    virtual void seek(port_t port, bool active, bool error) = 0;

    /***
     * Analog or digital output.
     * @param port the port number.  Valid values depend on the machine.
     * @param value An analog or digital value.
     *
     * @throw cb::Exception if the port is invalid or the operation is invalid
     * for the specified port.
     */
    virtual void output(port_t port, double value) = 0;

    /// @return the current position of all axes.
    virtual Axes getPosition() const = 0;
    virtual cb::Vector3D getPosition(axes_t axes) const = 0;

    /// Set the current position of all axes.
    virtual void setPosition(const Axes &position) = 0;

    /***
     * Delay the next move for @param seconds.
     *
     * @throw cb::Exception if @parm seconds is less than zero.
     */
    virtual void dwell(double seconds) = 0;

    /***
     * Program a linear move.
     *
     * Move to the position indicated by @param position at the current feed
     * rate if @param rapid is false, otherwise move at maximum speed.
     * This function may queue the operation and return immediately.
     *
     * @throw cb::Exception if the feed rate is zero or the move would go
     * beyond the limits of the machine.
     */
    virtual void move(const Axes &position, int axes, bool rapid,
                      double time) = 0;

    /***
     * Program a helical move.
     *
     * @param offset the offset from the current position to the far center of
     * the helicial move.
     * @param target the end point of the arc.
     * @param angle radians of rotation.  A positive value
     * indicates a clockwise rotation, negative a counter-clockwise rotation.
     * @param plane the plane to which the helical axis is perpendicular.
     * Valid values are XY, XZ or YZ.
     *
     * @throw cb::Exception if the feed rate is zero, the move would go beyond
     * the limits of the machine or the plane is invalid.
     */
    virtual void arc(const cb::Vector3D &offset, const cb::Vector3D &target,
                     double angle, plane_t plane) = 0;

    virtual Transforms &getTransforms() = 0;

    virtual void pause(pause_t type) = 0;

    // Number parameters
    virtual double get(address_t addr, Units units) const = 0;
    virtual void set(address_t addr, double value, Units units) = 0;

    // Named parameters
    virtual bool has(const std::string &name) const = 0;
    virtual double get(const std::string &name, Units units) const = 0;
    virtual void set(const std::string &name, double value, Units units) = 0;
    virtual void clear(const std::string &name) = 0;

    /// Get program location
    virtual const cb::LocationRange &getLocation() const = 0;

    /// Set program location
    virtual void setLocation(const cb::LocationRange &location) = 0;

    /// Output comment
    virtual void comment(const std::string &s) const = 0;

    /// Output message
    virtual void message(const std::string &s) = 0;
  };
}
