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

#include "GCodeInterpreter.h"

#include <gcode/Codes.h>

#include <gcode/ast/Assign.h>
#include <gcode/ast/Word.h>
#include <gcode/ast/NamedReference.h>
#include <gcode/ast/Comment.h>

#include <cbang/Exception.h>
#include <cbang/String.h>

#include <cbang/log/Logger.h>
#include <cbang/util/SmartFunctor.h>
#include <cbang/os/SystemUtilities.h>

#include <cctype>

using namespace std;
using namespace cb;
using namespace GCode;


GCodeInterpreter::GCodeInterpreter(Controller &controller) :
  controller(controller) {}


void GCodeInterpreter::setReference(address_t addr, double value) {
  LOG_DEBUG(3, "Set global variable #" << addr << "=" << value);
  controller.set(addr, value);
}


void GCodeInterpreter::setReference(const string &name, double value) {
  LOG_DEBUG(3, "Set global variable #<" << name << "> = " << value);
  controller.set(canonical(name), value);
}


void GCodeInterpreter::clearReference(const string &name) {
  LOG_DEBUG(3, "Clear global variable #<" << name << ">");
  controller.clear(canonical(name));
}


void GCodeInterpreter::execute(const Code &code, int vars) {
  if (!controller.execute(code, vars)) LOG_WARNING("Not implemented: " << code);
  else if (code.group == MG_MOTION)
    controller.setCurrentMotionMode(code.number);
}


void GCodeInterpreter::specialComment(const string text) {
  string::const_iterator it = text.begin();
  string::const_iterator end = text.end();

  // Skip space
  while (it != end && isspace(*it)) it++;

  // Look for special comment code
  string code;
  while (it != end && *it != ',' && !isspace(*it) && code.length() < 9)
    code.append(1, tolower(*it++));

  // TODO handle PROBE*
  // See http://linuxcnc.org/docs/html/gcode/overview.html#gcode:comments
  if (code == "probeopen" && isspace(*it))
    LOG_WARNING("PROBEOPEN not supported");

  // Skip space
  while (it != end && isspace(*it)) it++;

  // Look for comma
  if (it != end && *it == ',') {
    it++;

    // Skip space
    while (it != end && isspace(*it)) it++;

    string content(it, end);

    if (code == "msg") controller.message(content);
    else if (code == "debug") controller.message(interpolate(content));
    else if (code == "print") LOG_INFO(1, interpolate(content));
    else if (code == "logopen") {
      try {
        log.push_back(SystemUtilities::oopen(String::trim(content)));
      } CATCH_ERROR;

    } else if (code == "logappend") {
      try {
        log.push_back(SystemUtilities::open(String::trim(content),
                                            ios::out | ios::ate));
      } CATCH_ERROR;

    } else if (code == "logclose") {
      if (!log.empty()) log.pop_back();

    } else if (code == "log") {
      if (!log.empty()) *log.back() << interpolate(content) << endl;
    }
  }
}


string GCodeInterpreter::interpolate(const string &s) {
  string result;

  enum {
    SCAN_STATE,
    HASH_STATE,
    NUM_STATE,
    NAME_STATE,
  } state = SCAN_STATE;

  string::const_iterator refStart;

  for (auto it = s.begin(); it != s.end(); it++)
    switch (state) {
    case SCAN_STATE:
      if (*it == '#') {
        state = HASH_STATE;
        refStart = it;

      } else result.append(1, *it);
      break;

    case HASH_STATE:
      if ('0' <= *it && *it <= '9') state = NUM_STATE;
      else if (*it == '<') state = NAME_STATE;
      else {
        result.append(1, '#');

        if (*it != '#') {
          result.append(1, *it);
          state = SCAN_STATE;
        }
      }
      break;

    case NUM_STATE:
      if (*it < '0' || '9' < *it) {
        address_t addr = (address_t)String::parseU32(string(refStart + 1, it));
        result += String(lookupReference(addr));
        if (*it == '#') state = HASH_STATE;
        else {
          result.append(1, *it);
          state = SCAN_STATE;
        }
      }
      break;

    case NAME_STATE:
      if (*it == '>') {
        string name(refStart + 2, it);
        result += String(lookupReference(name));
        state = SCAN_STATE;
      }
      break;
    }

  if (state == NUM_STATE) {
    address_t addr = (address_t)String::parseU32(string(refStart + 1, s.end()));
    result += String(lookupReference(addr));

  } else if (state != SCAN_STATE) result += string(refStart, s.end());

  return result;
}


string GCodeInterpreter::canonical(const string &name) const {
  return String::replace(String::toLower(name), " ", "");
}


void GCodeInterpreter::operator()(const SmartPointer<Block> &block) {
  if (block->isDeleted()) return;

  LOG_DEBUG(5, "Block: " << *block);

  Word *word;
  Assign *assign;
  int vars = 0;
  int groups = 0;
  unsigned lowestPriority = ~0;
  bool implicitMotion = true;
  vector<Word *> words;
  Comment *lastComment = 0;

  controller.startBlock();
  SmartFunctor<Controller> callEndBlock(&controller, &Controller::endBlock);

  // Evaluate all expressions and set variables
  for (auto it = block->begin(); it != block->end(); it++) {
    (*it)->eval(*this);

    if ((assign = (*it)->instance<Assign>())) { // Handle Assigns
      Reference *ref;
      NamedReference *nameRef;

      if ((ref = assign->getReference()->instance<Reference>()))
        setReference(ref->getAddress(), assign->getExprValue());

      else if ((nameRef = assign->getReference()->instance<NamedReference>()))
        setReference(nameRef->getName(), assign->getExprValue());

      else THROW("Invalid reference type in Assign");

    } else if ((word = (*it)->instance<Word>())) { // Handle Words
      double value = word->getValue(); // Must be after eval
      char c = word->getType();
      const Code *code = 0;

      switch (c) {
      case 'F': if (3 < lowestPriority) lowestPriority = 3; break;
      case 'S': if (4 < lowestPriority) lowestPriority = 4; break;
      case 'T': if (5 < lowestPriority) lowestPriority = 5; break;

      case 'G': case 'M':
        // Find word with lowest priority
        if (!(code = word->getCode())) // Must be after eval
          LOG_WARNING(word->getCol() << ':' << *word
                      << ": Invalid or unsupported code");

        else {
          // Check modal groups
          if (groups & code->group) {
            LOG_WARNING(word->getCol()
                        << ":Cannot have more than one word from modal group "
                        << ModalGroup(code->group) << ", Ignoring " << *code);
            continue;
          }

          groups |= code->group;

          // Implicit motion
          if (code->group == MG_MOTION || code->group == MG_ZERO)
            implicitMotion = false;

          // Find lowest priority word
          if (code->priority < lowestPriority) lowestPriority = code->priority;
        }
        break;

      case 'O': THROW("Unexpected O-code"); break;

      case 'N':
      default:
        if (c == 'N' || !isalpha(c))
          LOG_WARNING(word->getCol() << ':' << *word
                      << ": Invalid or unsupported code");

        else {
          int flag = 1 << (c - 'A');
          if (vars & flag)
            LOG_WARNING(word->getCol() << ":Word '" << c
                        << "' repeated in block, only the last value will be "
                        "recognized");

          vars |= flag; // Flag variable

          // Set variable
          controller.setVar(c, value);
        }
      }

      words.push_back(word);

    } else if ((*it)->instance<Comment>())
      lastComment = (*it)->instance<Comment>();

    else LOG_WARNING((*it)->getCol()
                     << ":Unsupported or unexpected entity: " << **it);
  }

  // Handle special comments
  if (lastComment) specialComment(lastComment->getText());

  // Process command words in order of priority
  while (true) {
    unsigned priority = lowestPriority;

    // Implicit motion
    const Code *activeMotion =
      Codes::find('G', controller.getCurrentMotionMode() / 10.0);
    if (implicitMotion && (vars & VT_AXIS) &&
        activeMotion->priority < priority) {
      SmartPointer<Word> implicitWord = new Word(activeMotion);
      implicitWord->getLocation() = block->getLocation();
      words.push_back(implicitWord.get());
      block->push_back(implicitWord); // Block owns Word
      implicitMotion = false;
      priority = activeMotion->priority;

    } else if (lowestPriority == (unsigned)~0) break;

    lowestPriority = ~0;

    for (unsigned i = 0; i < words.size(); i++) {
      word = words[i];
      unsigned wordPriority = ~0;

      switch (word->getType()) {
      case 'F':
        wordPriority = 3;
        if (priority == 3) controller.setFeed(word->getValue());
        break;

      case 'S':
        wordPriority = 4;
        if (priority == 4) controller.setSpeed(word->getValue());
        break;

      case 'T':
        wordPriority = 5;
        if (priority == 5) controller.setTool(word->getValue());
        break;

      case 'G': case 'M': {
        const Code *code = word->getCode();
        if (!code) continue; // Invalid or unsupported

        if (priority == code->priority) {
          controller.setLocation(word->getLocation());
          execute(*code, vars);
        }

        wordPriority = code->priority;
      }

      default: break;
      }

      // Next lowest priority
      if (priority < wordPriority && wordPriority < lowestPriority)
        lowestPriority = wordPriority;
    }
  }
}


double GCodeInterpreter::lookupReference(address_t addr) {
  return controller.get(addr);
}


double GCodeInterpreter::lookupReference(const string &name) {
  return controller.get(canonical(name));
}


bool GCodeInterpreter::hasReference(const string &name) {
  return controller.has(canonical(name));
}
