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

#include "AABBView.h"
#include "GLBox.h"

using namespace CAMotics;
using namespace cb;


AABBView::AABBView() : leaves(new GLComposite), nodes(new GLComposite) {
  add(leaves);
  add(nodes);
}


void AABBView::load(const AABBTree &tree) {
  leaves->clear();
  nodes->clear();
  if (tree.getRoot()) load(*tree.getRoot(), tree.getHeight(), 0);
}


void AABBView::showNodes(bool show) {nodes->setVisible(show);}


void AABBView::load(const AABB &aabb, unsigned height, unsigned depth) {
  SmartPointer<GLBox> box = new GLBox;
  box->setBounds(aabb);
  box->setColor(0.5, 0, (height - depth) / (double)height);

  if (aabb.isLeaf()) leaves->add(box);
  else nodes->add(box);

  if (aabb.getLeft()) load(*aabb.getLeft(), height, depth + 1);
  if (aabb.getRight()) load(*aabb.getRight(), height, depth + 1);
}
