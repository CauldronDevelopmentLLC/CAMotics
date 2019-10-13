/*******************************************************************************
 *                                                                             *
 * Author    :  Angus Johnson                                                  *
 * Version   :  5.1.6                                                          *
 * Date      :  23 May 2013                                                    *
 * Website   :  http://www.angusj.com                                          *
 * Copyright :  Angus Johnson 2010-2013                                        *
 *                                                                             *
 * License:                                                                    *
 * Use, modification & distribution is subject to Boost Software License Ver 1.*
 * http://www.boost.org/LICENSE_1_0.txt                                        *
 *                                                                             *
 * Attributions:                                                               *
 * The code in this library is an extension of Bala Vatti's clipping algorithm:*
 * "A generic solution to polygon clipping"                                    *
 * Communications of the ACM, Vol 35, Issue 7 (July 1992) pp 56-63.            *
 * http://portal.acm.org/citation.cfm?id=129906                                *
 *                                                                             *
 * Computer graphics and geometric modeling: implementation and algorithms     *
 * By Max K. Agoston                                                           *
 * Springer; 1 edition (January 4, 2005)                                       *
 * http://books.google.com/books?q=vatti+clipping+agoston                      *
 *                                                                             *
 * See also:                                                                   *
 * "Polygon Offsetting by Computing Winding Numbers"                           *
 * Paper no. DETC2005-85513 pp. 565-575                                        *
 * ASME 2005 International Design Engineering Technical Conferences            *
 * and Computers and Information in Engineering Conference (IDETC/CIE2005)     *
 * September 24-28, 2005 , Long Beach, California, USA                         *
 * http://www.me.berkeley.edu/~mcmains/pubs/DAC05OffsetPolygon.pdf             *
 *                                                                             *
 ******************************************************************************/

#include "PolyNode.h"

using namespace ClipperLib;


void PolyNode::AddChild(PolyNode &child) {
  unsigned cnt = Childs.size();

  Childs.push_back(&child);
  child.Parent = this;
  child.Index = cnt;
}


PolyNode *PolyNode::GetNext() const {
  return Childs.empty() ? GetNextSiblingUp() : Childs[0];
}


PolyNode *PolyNode::GetNextSiblingUp() const {
  if (!Parent) return 0; // protects against PolyTree.GetNextSiblingUp()
  if (Index == Parent->Childs.size() - 1)
    return Parent->GetNextSiblingUp();
  return Parent->Childs[Index + 1];
}


bool PolyNode::IsHole() const {
  bool result = true;
  PolyNode *node = Parent;

  while (node) {
    result = !result;
    node = node->Parent;
  }

  return result;
}


void PolyNode::AddToPolygons(Polygons &polygons) const {
  if (!Contour.empty()) polygons.push_back(Contour);

  for (int i = 0; i < ChildCount(); i++)
    Childs[i]->AddToPolygons(polygons);
}
