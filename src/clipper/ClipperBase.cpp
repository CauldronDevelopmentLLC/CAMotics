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
 * September 24-28, 2005, Long Beach, California, USA                          *
 * http://www.me.berkeley.edu/~mcmains/pubs/DAC05OffsetPolygon.pdf             *
 *                                                                             *
 * This is a translation of the Delphi Clipper library and the naming style    *
 * used has retained a Delphi flavour.                                         *
 ******************************************************************************/

#include "ClipperBase.h"
#include "Clipper.h"

using namespace ClipperLib;


ClipperBase::ClipperBase() {
  m_MinimaList = 0;
  m_CurrentLM = 0;
  m_UseFullRange = true;
}


ClipperBase::~ClipperBase() {Clear();}


bool ClipperBase::AddPolygon(const Polygon &pg, PolyType polyType) {
  int len = (int)pg.size();
  if (len < 3) return false;

  int64_t maxVal;
  if (m_UseFullRange) maxVal = CLIPPER_HI; else maxVal = CLIPPER_LO;
  pg[0].RangeTest(maxVal);

  Polygon p(len);
  p[0] = pg[0];
  int j = 0;

  for (int i = 0; i < len; i++) {
    pg[i].RangeTest(maxVal);

    if (i == 0 || p[j].Equal(pg[i])) continue;
    else if (j > 0 && SlopesEqual(p[j - 1], p[j], pg[i], m_UseFullRange)) {
      if (p[j - 1].Equal(pg[i])) j--;
    } else j++;

    p[j] = pg[i];
  }

  if (j < 2) return false;

  len = j + 1;
  while (len > 2) {
    // nb: test for point equality before testing slopes
    if (p[j].Equal(p[0])) j--;
    else if (p[0].Equal(p[1]) ||
             SlopesEqual(p[j], p[0], p[1], m_UseFullRange))
      p[0] = p[j--];
    else if (SlopesEqual(p[j - 1], p[j], p[0], m_UseFullRange)) j--;
    else if (SlopesEqual(p[0], p[1], p[2], m_UseFullRange)) {
      for (int i = 2; i <= j; i++) p[i - 1] = p[i];
      j--;

    } else break;

    len--;
  }

  if (len < 3) return false;

  // create a new edge array
  TEdge *edges = new TEdge[len];
  m_edges.push_back(edges);

  // convert vertices to a double-linked-list of edges and initialize
  edges[0].xcurr = p[0].X;
  edges[0].ycurr = p[0].Y;
  edges[len - 1] = TEdge(&edges[0], &edges[len - 2], p[len - 1], polyType);

  for (int i = len - 2; i > 0; i--)
    edges[i] = TEdge(&edges[i + 1], &edges[i - 1], p[i], polyType);

  edges[0] = TEdge(&edges[1], &edges[len - 1], p[0], polyType);

  // reset xcurr & ycurr and find 'eHighest' (given the Y axis coordinates
  // increase downward so the 'highest' edge will have the smallest ytop)
  TEdge *e = &edges[0];
  TEdge *eHighest = e;
  do {
    e->xcurr = e->xbot;
    e->ycurr = e->ybot;
    if (e->ytop < eHighest->ytop) eHighest = e;
    e = e->next;
  } while (e != &edges[0]);

  // make sure eHighest is positioned so the following loop works safely
  if (eHighest->windDelta > 0) eHighest = eHighest->next;
  if (CLIPPER_NEAR_EQUAL(eHighest->dx, CLIPPER_HORIZONTAL))
    eHighest = eHighest->next;

  // finally insert each local minima
  e = eHighest;
  do {
    e = AddBoundsToLML(e);
  } while (e != eHighest);

  return true;
}


void ClipperBase::InsertLocalMinima(LocalMinima *newLm) {
  if (!m_MinimaList) m_MinimaList = newLm;
  else if (newLm->Y >= m_MinimaList->Y) {
    newLm->next = m_MinimaList;
    m_MinimaList = newLm;

  } else {
    LocalMinima *tmpLm = m_MinimaList;
    while (tmpLm->next && (newLm->Y < tmpLm->next->Y))
      tmpLm = tmpLm->next;

    newLm->next = tmpLm->next;
    tmpLm->next = newLm;
  }
}


TEdge *ClipperBase::AddBoundsToLML(TEdge *e) {
  // Starting at the top of one bound we progress to the bottom where there's
  // a local minima. We then go to the top of the next bound. These two bounds
  // form the left and right (or right and left) bounds of the local minima.
  e->nextInLML = 0;
  e = e->next;

  while (true) {
    if (CLIPPER_NEAR_EQUAL(e->dx, CLIPPER_HORIZONTAL)) {
      // nb: proceed through horizontals when approaching from their right,
      //    but break on horizontal minima if approaching from their left.
      //    This ensures 'local minima' are always on the left of horizontals.
      if (e->next->ytop < e->ytop && e->next->xbot > e->prev->xbot) break;
      if (e->xtop != e->prev->xbot) e->SwapX();
      e->nextInLML = e->prev;

    } else if (e->ycurr == e->prev->ycurr) break;

    else e->nextInLML = e->prev;

    e = e->next;
  }

  // e and e.prev are now at a local minima
  LocalMinima *newLm = new LocalMinima;
  newLm->next = 0;
  newLm->Y = e->prev->ybot;

  // horizontal edges never start a left bound
  if (CLIPPER_NEAR_EQUAL(e->dx, CLIPPER_HORIZONTAL)) {
    if (e->xbot != e->prev->xbot) e->SwapX();
    newLm->leftBound = e->prev;
    newLm->rightBound = e;

  } else if (e->dx < e->prev->dx) {
    newLm->leftBound = e->prev;
    newLm->rightBound = e;

  } else {
    newLm->leftBound = e;
    newLm->rightBound = e->prev;
  }

  newLm->leftBound->side = esLeft;
  newLm->rightBound->side = esRight;
  InsertLocalMinima(newLm);

  while (true) {
    if (e->next->ytop == e->ytop &&
        !CLIPPER_NEAR_EQUAL(e->next->dx, CLIPPER_HORIZONTAL))
      break;

    e->nextInLML = e->next;
    e = e->next;
    if (CLIPPER_NEAR_EQUAL(e->dx, CLIPPER_HORIZONTAL) &&
        e->xbot != e->prev->xtop) e->SwapX();
  }

  return e->next;
}


bool ClipperBase::AddPolygons(const Polygons &ppg, PolyType polyType) {
  bool result = false;

  for (Polygons::size_type i = 0; i < ppg.size(); i++)
    if (AddPolygon(ppg[i], polyType)) result = true;

  return result;
}


void ClipperBase::Clear() {
  DisposeLocalMinimaList();

  for (EdgeList::size_type i = 0; i < m_edges.size(); i++)
    delete [] m_edges[i];

  m_edges.clear();
  m_UseFullRange = false;
}


void ClipperBase::Reset() {
  m_CurrentLM = m_MinimaList;
  if (!m_CurrentLM) return; // ie nothing to process

  // reset all edges
  LocalMinima *lm = m_MinimaList;

  while (lm) {
    TEdge *e = lm->leftBound;

    while (e) {
      e->xcurr = e->xbot;
      e->ycurr = e->ybot;
      e->side = esLeft;
      e->outIdx = -1;
      e = e->nextInLML;
    }

    e = lm->rightBound;

    while (e) {
      e->xcurr = e->xbot;
      e->ycurr = e->ybot;
      e->side = esRight;
      e->outIdx = -1;
      e = e->nextInLML;
    }

    lm = lm->next;
  }
}


void ClipperBase::DisposeLocalMinimaList() {
  while (m_MinimaList) {
    LocalMinima *tmpLm = m_MinimaList->next;
    delete m_MinimaList;
    m_MinimaList = tmpLm;
  }

  m_CurrentLM = 0;
}


void ClipperBase::PopLocalMinima() {
  if (!m_CurrentLM) return;

  m_CurrentLM = m_CurrentLM->next;
}


Bounds ClipperBase::GetBounds() {
  Bounds result;
  LocalMinima *lm = m_MinimaList;

  if (!lm) {
    result.left = result.top = result.right = result.bottom = 0;
    return result;
  }

  result.left = lm->leftBound->xbot;
  result.top = lm->leftBound->ybot;
  result.right = lm->leftBound->xbot;
  result.bottom = lm->leftBound->ybot;

  while (lm) {
    if (lm->leftBound->ybot > result.bottom)
      result.bottom = lm->leftBound->ybot;

    TEdge *e = lm->leftBound;

    while (true) {
      TEdge *bottomE = e;

      while (e->nextInLML) {
        if (e->xbot < result.left) result.left = e->xbot;
        if (e->xbot > result.right) result.right = e->xbot;
        e = e->nextInLML;
      }

      if (e->xbot < result.left) result.left = e->xbot;
      if (e->xbot > result.right) result.right = e->xbot;
      if (e->xtop < result.left) result.left = e->xtop;
      if (e->xtop > result.right) result.right = e->xtop;
      if (e->ytop < result.top) result.top = e->ytop;

      if (bottomE == lm->leftBound) e = lm->rightBound;
      else break;
    }

    lm = lm->next;
  }

  return result;
}
