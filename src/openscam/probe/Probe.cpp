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

#include "Probe.h"

#include <cbang/Math.h>
#include <cbang/log/Logger.h>
#include <cbang/config/Options.h>

#include <openscam/gcode/Parser.h>
#include <openscam/gcode/Interpreter.h>
#include <openscam/gcode/Codes.h>

#include <openscam/gcode/ast/Program.h>
#include <openscam/gcode/ast/BinaryOp.h>
#include <openscam/gcode/ast/Number.h>
#include <openscam/gcode/ast/Reference.h>
#include <openscam/gcode/ast/QuotedExpr.h>

using namespace std;
using namespace cb;
using namespace OpenSCAM;


Probe::Probe(Options &options, std::ostream &stream) :
  Machine(options), Controller((Machine &)*this), Printer(stream),
  interp(*this), gridSize(5), clearHeight(1), probeDepth(-1), probeFeed(5),
  liftOff(false), liftOffFeed(0.5), minMem(2000), maxMem(5400),
  useLastZExpression(true), pass(0), didOutputProbe(false) {

  options.pushCategory("Probe");
  options.addTarget("grid-size", gridSize, "Set max probe grid size.");
  options.addTarget("clear-height", clearHeight, "Set probe Z clearance.");
  options.addTarget("probe-depth", probeDepth, "Set probe Z depth.");
  options.addTarget("probe-feed", probeFeed, "Set probe feed rate.");
  options.addTarget("lift-off", liftOff, "Enable lift off probing.");
  options.addTarget("lift-off-feed", liftOffFeed, "Set probe lift off speed.");
  options.addTarget("min-mem", minMem, "Set probe starting memory address.  "
                    "This is where the probed values will be stored in the "
                    "controller's memory.");
  options.addTarget("max-mem", maxMem, "Set probe maximum memory address.");
  options.addTarget("use-last-z-expression", useLastZExpression, "Always use "
                    "the last Z word expression when computing Z heights "
                    "rather than using the current computed value.  This "
                    "option makes it possible to preserve a Z height variable "
                    "reference.");
  options.addTarget("probe-prefix", probePrefix,
                    "GCode to emit before the probe");
  options.addTarget("probe-suffix", probeSuffix,
                    "GCode to emit after the probe");
  options.popCategory();
}


void Probe::read(const InputSource &source) {
  // Parse program
  Program program;
  Parser().parse(source, program);

  // Compute bounding box
  pass = 1;
  try {
    program.process(interp);
  } catch (const EndProgram &) {}
  LOG_DEBUG(1, "Bounding box: " << bbox);

  // Create probe grid
  pass = 2;
  Vector2D divisions(ceil(bbox.getWidth() / gridSize),
                     ceil(bbox.getLength() / gridSize));
  grid = new ProbeGrid(bbox, divisions);
  try {
    program.process(interp);
  } catch (const EndProgram &) {}

  // Output program with probe
  didOutputProbe = false;
  program.process(*this);
}


void Probe::outputProbe(ProbePoint &pt, unsigned address, unsigned count) {
  if (maxMem <= address)
    THROW("Too many probes, ran out of address space in controller");
  
  pt.address = address;
  
  stream
    << "G0 X" << pt.x() << " Y" << pt.y() << " Z"
    << clearHeight << '\n'
    << "G38.2 Z" << probeDepth << " F" << probeFeed << '\n';
  
  if (liftOff)
    stream << "G38.4 Z" << clearHeight << " F" << liftOffFeed << '\n';
  
  stream << '#' << address << "=#5063\n";
  stream << "G0 Z" << clearHeight << '\n';
}


void Probe::outputProbe() {
  unsigned count = 0;

  stream << probePrefix << '\n';

  stream << "; Start probe\n";
  stream << "G0 Z" << clearHeight << '\n';

  unsigned address = minMem;
  for (ProbeGrid::row_iterator row = grid->begin(); row != grid->end(); row++) {
    // Down
    for (ProbeGrid::col_iterator it = row->begin(); it != row->end(); it++)
      if (it->probe) outputProbe(*it, address++, count++);

    if (++row == grid->end()) break;

    // Back
    ProbeGrid::reverse_col_iterator it;
    for (it = row->rbegin(); it != row->rend(); it++)
      if (it->probe) outputProbe(*it, address++, count++);
  }

  stream << "M0\n"; // Pause

  stream << "; End probe\n\n";

  stream << probeSuffix << '\n';

  LOG_INFO(1, "Output " << count << " probes");
}


void Probe::execute(Word *word, int vars) {
  const Code &code = *word->getCode();
  Controller::setLocation(word->getLocation());
  Controller::execute(code, vars);

  // TODO This should be absolute position & we should account for offsets etc.
  Vector2D pos(getAxisPosition('X'), getAxisPosition('Y'));

  switch (pass) {
  case 1:
    if (code.type == 'G' && code.number == 1) bbox.add(pos);
    break;

  case 2:
    if (code.type == 'G' && code.number == 1) {
      vector<ProbePoint *> pt = grid->find(pos);
      for (int i = 0; i < 4; i++) pt[i]->probe = true;
    }
    break;
  }
}


void Probe::operator()(const SmartPointer<Block> &block) {
  interp(block);

  if (!block->isDeleted()) {
    if (!didOutputProbe && block->findWord('M', 3)) {
      outputProbe();
      didOutputProbe = true;
    }
    
    if (didOutputProbe && block->findWord('G', 1) &&
        getAxisPosition('Z') < clearHeight) {
      double x = getAxisPosition('X');
      double y = getAxisPosition('Y');
      double z = getAxisPosition('Z');

      vector<ProbePoint *> pt = grid->find(Vector2D(x, y));
      double x1 = pt[0]->x();
      double x2 = pt[1]->x();
      double y1 = pt[0]->y();
      double y2 = pt[2]->y();
      double denom = (x2 - x1) * (y2 - y1);
      double v[4] = {
        ((x2 - x) * (y2 - y)) / denom,
        ((x - x1) * (y2 - y)) / denom,
        ((x2 - x) * (y - y1)) / denom,
        ((x - x1) * (y - y1)) / denom,
      };

      SmartPointer<Entity> expr;
      Word *zWord = block->findWord('Z');
      if (useLastZExpression) expr = getVarExpr('Z');
      else if (zWord) expr = zWord->getExpression();
      else expr = new Number(z);

      for (int i = 0; i < 4; i++) {
        if (!pt[i]->address)
          THROWS("Point " << pt[i] << " does not have address");

        SmartPointer<Entity> ref = new Reference(new Number(pt[i]->address));
        SmartPointer<Entity> num = new Number(v[i]);

        expr = new BinaryOp(Operator::ADD_OP, expr,
                            new BinaryOp(Operator::MUL_OP, ref, num));
      }
      expr = new QuotedExpr(expr);

      if (!zWord) {
        zWord = new Word('Z', expr);
        block->push_back(zWord);

      } else zWord->setExpression(expr);
    }
  }

  Printer::operator()(block);
}
