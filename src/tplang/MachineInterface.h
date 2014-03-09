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

#ifndef TPLANG_MACHINE_INTERFACE_H
#define TPLANG_MACHINE_INTERFACE_H

#include "Axes.h"
#include "MachineEnum.h"

#include <cbang/Exception.h>
#include <cbang/LocationRange.h>
#include <cbang/geom/Matrix.h>


namespace tplang {
  class MachineMatrix;

  class MachineInterface : public MachineEnum {
  public:
    virtual ~MachineInterface() {}

    virtual void reset() = 0;
    virtual void start() = 0;
    virtual void end() = 0;

    /// @return the currently programed feed rate and optionaly the feed mode.
    virtual double getFeed(feed_mode_t *mode = 0) const = 0;

    /***
     * Set the feed rate.
     * A positive feed rate is measured in millimeters.  If the feed rate is 0,
     * the default, then no moves can be made.
     *
     * Positive feed rates depend on the feed mode.
     *
     * MM_PER_MINUTE mode, the default, indicates that the controlled point
     * should move @param feed millimeters per minute.
     *
     * INVERSE_TIME mode indicates that each move should be completed in
     * 1 / @param feed minutes.  For example, if @param feed is 2, the move
     * should be completed in half a minute.
     *
     * MM_PER_REVOLUTION mode indicates the controlled point should move a
     * certain number of millimeters per revolution of the spindle.
     *
     * @throw cb::Exception if @param mode is invalid or @param feed is
     * negative.
     */
    virtual void setFeed(double feed, feed_mode_t mode = MM_PER_MINUTE) = 0;

    /***
     * @return the currently programed spindle speed and optionaly the spin
     * mode and max speed.
     */
    virtual double getSpeed(spin_mode_t *mode = 0, double *max = 0) const = 0;

    /***
     * Set the spindle speed.
     *
     * @param speed A positive value indicates clockwise spin.  A negative value
     * counterclockwise spin.  0 indicates no spin.
     *
     * @param mode REVOLUTIONS_PER_MINUTE, the default, indicates spin
     * measured in revolutions per minute.  CONSTANT_SURFACE_SPEED sets the
     * spindle speed in meters per minute.
     *
     * If @param mode is CONSTANT_SURFACE_SPEED and @param max is greater than
     * zero then @param max is the maxiumum spindle revolutions per minute.
     *
     * @throw cb::Exception if there are any pending errors or @param mode is
     * invalid.
     */
    virtual void setSpeed(double speed,
                          spin_mode_t mode = REVOLUTIONS_PER_MINUTE,
                          double max = 0) = 0;

    /// @return the currently programed tool number.
    virtual unsigned getTool() const = 0;

    /***
     * Select the active tool.
     * This may cause some machines to run a tool change routine and return to
     * the current location.
     *
     * @throw cb::Exception if there are any pending errors, or if the tool
     * number is invalid.
     */
    virtual void setTool(unsigned tool) = 0;

    /***
     * Query port by type.
     * @return The nuber of the @param index'th with @param type if it exists
     * or -1 otherwise.
     */
    virtual int findPort(port_t type, unsigned index = 0) = 0;

    /***
     * Analog or digital input.
     *
     * @param port the port number.  Valid values depend on the machine.
     *
     * @param mode If IMMEDIATE read and return the value immediately.
     *
     * If one of the START_* modes then delay the next move until the
     * indicated condition occurs.
     *
     * If one of the STOP_* modes then stop the the next move if the indicated
     * condition occurs before the end of the
     * move.
     *
     * If @param mode is not IMMEDIATE this function will still return
     * immediately but with a 0 value.
     *
     * If @param mode is IMMEDIATE then @param timeout has no effect.
     * If @param mode is one of the START_* modes then a non-zero @param timeout
     * indicates the maximum number of seconds to delay the next move.
     *
     * If @param mode is one of the STOP_* modes then a non-zero timeout
     * indicates the maximum number of seconds before aborting the next move
     * before either the move ends on its own or the target condition occurs.
     *
     * The timeout always begins from the start of the next move.
     *
     * If @param timeout is less than or equal to zero then timeout is infinite.
     *
     * If @param error is true and the timeout is reached before either the
     * condition is met or the move ends then the error TIMEOUT is set.
     *
     * If @param error is true and neither the timeout or the input condition
     * is are met before the move ends then the error CONDITION is set.
     *
     * @throw cb::Exception if there are any pending errors, the port is
     * invalid or the operation is invalid for the specified port.
     */
    virtual double input(unsigned port, input_mode_t mode = IMMEDIATE,
                         double timeout = 0, bool error = true) = 0;

    /***
     * Analog or digital output.
     * @param port the port number.  Valid values depend on the machine.
     * @param value An analog or digital value.
     * @param sync If true synchronize the output with the next move.
     *
     * @throw cb::Exception if there are any pending errors, the port is invalid
     * or the operation is invalid for the specified port.
     */
    virtual void output(unsigned port, double value, bool sync = true) = 0;

    /***
     * @return the current position of all axes.  A call to this function
     * implies a call to synchronize(0) and will return the position of the
     * axes after completion of the final pending move.
     */
    virtual Axes getPosition() const = 0;
    virtual cb::Vector3D getPosition(axes_t axes) const = 0;

    /***
     * Delay the next move for @param seconds.
     *
     * @throw cb::Exception if there are any pending errors or @parm seconds
     * is less than zero.
     */
    virtual void dwell(double seconds) = 0;

    /***
     * Program a linear move.
     *
     * Move to the position indicated by @param axes at the current feed rate if
     * @param rapid is false, otherwise move at maximum speed.
     * This function may queue the operation and return immediately.
     *
     * If a machine limit is hit then the LIMIT error is set and the move
     * is aborted and any pending operations are canceled.
     *
     * @throw cb::Exception if there are any pending errors, the feed rate is
     * zero or the move would go beyond the limits of the machine.
     */
    virtual void move(const Axes &axes, bool rapid = false) = 0;

    /***
     * Program a helical move.
     *
     * @param offset the offset from the current position to the far center of
     * the helicial move in the selected plane.  If the third value is zero then
     * them move is a simple arc in the selected plane, otherwise it is
     * a true helical move with an axis with length equal to the absolute value
     * of the third value either above or below the selected plane through the
     * current control point depending on the sign.
     * @param angle radians of rotation.  A positive value
     * indicates a clockwise rotation, negative a counter-clockwise rotation.
     * @param plane the plane to which the helical axis is perpendicular.
     * Valid values are XY (the default), XZ or YZ.
     *
     * If a machine limit is hit then the LIMIT error is set and the move
     * is aborted and any pending operations are canceled.
     *
     * @throw cb::Exception if there are any pending errors, the feed rate is
     * zero, the move would go beyond the limits of the machine or the plane
     * is invalid.
     */
    virtual void arc(const cb::Vector3D &offset, double angle,
                     plane_t plane = XY) = 0;

    virtual const cb::Matrix4x4D &getMatrix(axes_t matrix = XYZ) const = 0;

    virtual void setMatrix(const cb::Matrix4x4D &m, axes_t matrix = XYZ) = 0;

    virtual void pause(bool optional = true) const = 0;

    /***
     * Returns once all outstanding moves have completed unless @param timeout
     * is non-zero, in which case false will be returned if the timeout, in
     * seconds, is reached sooner.  Reaching the timeout will not set the
     * TIMEOUT error.
     */
    virtual bool synchronize(double timeout = 0) const = 0;

    /// Halt all movement immediately and cancel any pending operations.
    virtual void abort() = 0;

    /***
     * Read any pending errors.
     * @return OK when there are no more errors, the next error code otherwise.
     */
    virtual async_error_t readAsyncError() = 0;

    /// Clear any pending errors.
    virtual void clearAsyncErrors() = 0;

    /// Set program location
    virtual const cb::LocationRange &getLocation() const = 0;

    /// Get program location
    virtual void setLocation(const cb::LocationRange &location) = 0;
  };
}

#endif // TPLANG_MACHINE_INTERFACE_H

