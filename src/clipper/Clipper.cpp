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

#include "Clipper.h"

#include "Int128.h"
#include "Exception.h"

#include <cmath>
#include <algorithm>
#include <cstring>
#include <cstdlib>

using namespace ClipperLib;


Clipper::Clipper() : ClipperBase() {
  m_Scanbeam = 0;
  m_ActiveEdges = 0;
  m_SortedEdges = 0;
  m_IntersectNodes = 0;
  m_ExecuteLocked = false;
  m_UseFullRange = false;
  m_ReverseOutput = false;
  m_ForceSimple = false;
}


Clipper::~Clipper() {
  Clear();
  DisposeScanbeamList();
}


void Clipper::Clear() {
  if (m_edges.empty()) return; // avoids problems with ClipperBase destructor
  DisposeAllPolyPts();
  ClipperBase::Clear();
}


void Clipper::DisposeScanbeamList() {
  while (m_Scanbeam) {
    Scanbeam *sb2 = m_Scanbeam->next;
    delete m_Scanbeam;
    m_Scanbeam = sb2;
  }
}


void Clipper::Reset() {
  ClipperBase::Reset();
  m_Scanbeam = 0;
  m_ActiveEdges = 0;
  m_SortedEdges = 0;
  DisposeAllPolyPts();

  LocalMinima *lm = m_MinimaList;

  while (lm) {
    InsertScanbeam(lm->Y);
    lm = lm->next;
  }
}

bool Clipper::Execute(ClipType clipType, Polygons &solution,
                      PolyFillType subjFillType, PolyFillType clipFillType) {
  if (m_ExecuteLocked) return false;

  m_ExecuteLocked = true;
  solution.resize(0);
  m_SubjFillType = subjFillType;
  m_ClipFillType = clipFillType;
  m_ClipType = clipType;
  m_UsingPolyTree = false;

  bool succeeded = ExecuteInternal();
  if (succeeded) BuildResult(solution);
  m_ExecuteLocked = false;

  return succeeded;
}


bool Clipper::Execute(ClipType clipType, PolyTree &polytree,
                      PolyFillType subjFillType, PolyFillType clipFillType) {
  if (m_ExecuteLocked) return false;

  m_ExecuteLocked = true;
  m_SubjFillType = subjFillType;
  m_ClipFillType = clipFillType;
  m_ClipType = clipType;
  m_UsingPolyTree = true;

  bool succeeded = ExecuteInternal();
  if (succeeded) BuildResult2(polytree);
  m_ExecuteLocked = false;

  return succeeded;
}


void Clipper::FixHoleLinkage(OutRec &outrec) {
  // skip OutRecs that (a) contain outermost polygons or
  // (b) already have the correct owner/child linkage
  if (!outrec.FirstLeft ||
      (outrec.isHole != outrec.FirstLeft->isHole && outrec.FirstLeft->pts))
    return;

  OutRec *orfl = outrec.FirstLeft;
  while (orfl && ((orfl->isHole == outrec.isHole) || !orfl->pts))
    orfl = orfl->FirstLeft;

  outrec.FirstLeft = orfl;
}


bool Clipper::ExecuteInternal() {
  bool succeeded;

  try {
    Reset();

    if (!m_CurrentLM) return true;

    int64_t botY = PopScanbeam();
    do {
      InsertLocalMinimaIntoAEL(botY);
      ClearHorzJoins();
      ProcessHorizontals();
      int64_t topY = PopScanbeam();
      succeeded = ProcessIntersections(botY, topY);
      if (!succeeded) break;
      ProcessEdgesAtTopOfScanbeam(topY);
      botY = topY;
    } while (m_Scanbeam || m_CurrentLM);

  } catch(...) {succeeded = false;}

  if (succeeded) {
    // tidy up output polygons and fix orientations where necessary
    for (PolyOutList::size_type i = 0; i < m_PolyOuts.size(); i++) {
      OutRec *outRec = m_PolyOuts[i];
      if (!outRec->pts) continue;
      FixupOutPolygon(*outRec);
      if (!outRec->pts) continue;

      if ((outRec->isHole ^ m_ReverseOutput) ==
          (outRec->Area(m_UseFullRange) > 0))
        outRec->pts->ReversePolyPtLinks();
    }

    if (!m_Joins.empty()) JoinCommonEdges();
    if (m_ForceSimple) DoSimplePolygons();
  }

  ClearJoins();
  ClearHorzJoins();

  return succeeded;
}


void Clipper::InsertScanbeam(const int64_t Y) {
  if (!m_Scanbeam) {
    m_Scanbeam = new Scanbeam;
    m_Scanbeam->next = 0;
    m_Scanbeam->Y = Y;

  } else if (Y > m_Scanbeam->Y) {
    Scanbeam *newSb = new Scanbeam;
    newSb->Y = Y;
    newSb->next = m_Scanbeam;
    m_Scanbeam = newSb;

  } else {
    Scanbeam *sb2 = m_Scanbeam;
    while (sb2->next  && (Y <= sb2->next->Y)) sb2 = sb2->next;
    if (Y == sb2->Y) return; // ie ignores duplicates

    Scanbeam *newSb = new Scanbeam;
    newSb->Y = Y;
    newSb->next = sb2->next;
    sb2->next = newSb;
  }
}


int64_t Clipper::PopScanbeam() {
  int64_t Y = m_Scanbeam->Y;
  Scanbeam *sb2 = m_Scanbeam;
  m_Scanbeam = m_Scanbeam->next;

  delete sb2;
  return Y;
}


void Clipper::DisposeAllPolyPts() {
  for (PolyOutList::size_type i = 0; i < m_PolyOuts.size(); i++)
    DisposeOutRec(i);

  m_PolyOuts.clear();
}


void Clipper::DisposeOutRec(PolyOutList::size_type index) {
  OutRec *outRec = m_PolyOuts[index];
  if (outRec->pts) outRec->pts->Dispose();
  delete outRec;
  m_PolyOuts[index] = 0;
}


void Clipper::SetWindingCount(TEdge &edge) {
  TEdge *e = edge.prevInAEL;
  // find the edge of same polytype that immediately preceeds 'edge' in AEL
  while (e  && e->polyType != edge.polyType) e = e->prevInAEL;

  if (!e) {
    edge.windCnt = edge.windDelta;
    edge.windCnt2 = 0;
    e = m_ActiveEdges; // ie get ready to calc windCnt2

  } else if (IsEvenOddFillType(edge)) {
    // EvenOdd filling
    edge.windCnt = 1;
    edge.windCnt2 = e->windCnt2;
    e = e->nextInAEL; // ie get ready to calc windCnt2

  } else {
    // nonZero, Positive or Negative filling
    if (e->windCnt * e->windDelta < 0) {
      if (Abs(e->windCnt) > 1) {
        if (e->windDelta * edge.windDelta < 0) edge.windCnt = e->windCnt;
        else edge.windCnt = e->windCnt + edge.windDelta;
      } else edge.windCnt = e->windCnt + e->windDelta + edge.windDelta;

    } else {
      if (Abs(e->windCnt) > 1 && e->windDelta * edge.windDelta < 0)
        edge.windCnt = e->windCnt;

      else if (e->windCnt + edge.windDelta == 0) edge.windCnt = e->windCnt;
      else edge.windCnt = e->windCnt + edge.windDelta;
    }

    edge.windCnt2 = e->windCnt2;
    e = e->nextInAEL; // ie get ready to calc windCnt2
  }

  // update windCnt2
  if (IsEvenOddAltFillType(edge)) {
    // EvenOdd filling
    while (e != &edge) {
      edge.windCnt2 = (edge.windCnt2 == 0) ? 1 : 0;
      e = e->nextInAEL;
    }

  } else {
    // nonZero, Positive or Negative filling
    while (e != &edge) {
      edge.windCnt2 += e->windDelta;
      e = e->nextInAEL;
    }
  }
}


bool Clipper::IsEvenOddFillType(const TEdge &edge) const {
  if (edge.polyType == ptSubject) return m_SubjFillType == pftEvenOdd;
  return m_ClipFillType == pftEvenOdd;
}


bool Clipper::IsEvenOddAltFillType(const TEdge &edge) const {
  if (edge.polyType == ptSubject) return m_ClipFillType == pftEvenOdd;
  return m_SubjFillType == pftEvenOdd;
}


bool Clipper::IsContributing(const TEdge &edge) const {
  PolyFillType pft, pft2;
  if (edge.polyType == ptSubject) {
    pft = m_SubjFillType;
    pft2 = m_ClipFillType;

  } else {
    pft = m_ClipFillType;
    pft2 = m_SubjFillType;
  }

  switch (pft) {
  case pftEvenOdd:
  case pftNonZero:
    if (Abs(edge.windCnt) != 1) return false;
    break;

  case pftPositive:
    if (edge.windCnt != 1) return false;
    break;

  default: // pftNegative
    if (edge.windCnt != -1) return false;
  }

  switch (m_ClipType) {
  case ctIntersection:
    switch (pft2) {
    case pftEvenOdd:
    case pftNonZero: return edge.windCnt2 != 0;
    case pftPositive: return edge.windCnt2 > 0;
    default: return edge.windCnt2 < 0;
    }
    break;

  case ctUnion:
    switch (pft2) {
    case pftEvenOdd:
    case pftNonZero: return edge.windCnt2 == 0;
    case pftPositive: return edge.windCnt2 <= 0;
    default: return edge.windCnt2 >= 0;
    }
    break;

  case ctDifference:
    if (edge.polyType == ptSubject)
      switch (pft2) {
      case pftEvenOdd:
      case pftNonZero: return edge.windCnt2 == 0;
      case pftPositive: return edge.windCnt2 <= 0;
      default: return edge.windCnt2 >= 0;
      }

    else switch (pft2) {
      case pftEvenOdd:
      case pftNonZero: return edge.windCnt2 != 0;
      case pftPositive: return edge.windCnt2 > 0;
      default: return edge.windCnt2 < 0;
      }
    break;

  default: return true;
  }
}


void Clipper::AddLocalMinPoly(TEdge *e1, TEdge *e2, const IntPoint &pt) {
  TEdge *e, *prevE;

  if (CLIPPER_NEAR_EQUAL(e2->dx, CLIPPER_HORIZONTAL) || (e1->dx > e2->dx)) {
    AddOutPt(e1, pt);
    e2->outIdx = e1->outIdx;
    e1->side = esLeft;
    e2->side = esRight;
    e = e1;

    if (e->prevInAEL == e2) prevE = e2->prevInAEL;
    else prevE = e->prevInAEL;

  } else {
    AddOutPt(e2, pt);
    e1->outIdx = e2->outIdx;
    e1->side = esRight;
    e2->side = esLeft;
    e = e2;

    if (e->prevInAEL == e1) prevE = e1->prevInAEL;
    else prevE = e->prevInAEL;
  }

  if (prevE && prevE->outIdx >= 0 && prevE->TopX(pt.Y) == e->TopX(pt.Y) &&
      e->SlopesEqual(*prevE, m_UseFullRange))
    AddJoin(e, prevE, -1, -1);
}


void Clipper::AddLocalMaxPoly(TEdge *e1, TEdge *e2, const IntPoint &pt) {
  AddOutPt(e1, pt);

  if (e1->outIdx == e2->outIdx) {
    e1->outIdx = -1;
    e2->outIdx = -1;

  } else if (e1->outIdx < e2->outIdx) AppendPolygon(e1, e2);
  else AppendPolygon(e2, e1);
}


void Clipper::AddEdgeToSEL(TEdge *edge) {
  // SEL pointers in PEdge are reused to build a list of horizontal edges.
  // However, we don't need to worry about order with horizontal edge
  // processing.

  if (!m_SortedEdges) {
    m_SortedEdges = edge;
    edge->prevInSEL = 0;
    edge->nextInSEL = 0;

  } else {
    edge->nextInSEL = m_SortedEdges;
    edge->prevInSEL = 0;
    m_SortedEdges->prevInSEL = edge;
    m_SortedEdges = edge;
  }
}


void Clipper::CopyAELToSEL() {
  TEdge *e = m_ActiveEdges;
  m_SortedEdges = e;

  while (e) {
    e->prevInSEL = e->prevInAEL;
    e->nextInSEL = e->nextInAEL;
    e = e->nextInAEL;
  }
}


void Clipper::AddJoin(TEdge *e1, TEdge *e2, int e1OutIdx, int e2OutIdx) {
  JoinRec *jr = new JoinRec;

  if (e1OutIdx >= 0) jr->poly1Idx = e1OutIdx;
  else jr->poly1Idx = e1->outIdx;

  jr->pt1a = IntPoint(e1->xcurr, e1->ycurr);
  jr->pt1b = IntPoint(e1->xtop, e1->ytop);

  if (e2OutIdx >= 0) jr->poly2Idx = e2OutIdx;
  else jr->poly2Idx = e2->outIdx;

  jr->pt2a = IntPoint(e2->xcurr, e2->ycurr);
  jr->pt2b = IntPoint(e2->xtop, e2->ytop);
  m_Joins.push_back(jr);
}


void Clipper::ClearJoins() {
  for (JoinList::size_type i = 0; i < m_Joins.size(); i++)
    delete m_Joins[i];
  m_Joins.resize(0);
}


void Clipper::AddHorzJoin(TEdge *e, int idx) {
  HorzJoinRec *hj = new HorzJoinRec;

  hj->edge = e;
  hj->savedIdx = idx;
  m_HorizJoins.push_back(hj);
}


void Clipper::ClearHorzJoins() {
  for (HorzJoinList::size_type i = 0; i < m_HorizJoins.size(); i++)
    delete m_HorizJoins[i];

  m_HorizJoins.resize(0);
}


void Clipper::InsertLocalMinimaIntoAEL(const int64_t botY) {
  while (m_CurrentLM && m_CurrentLM->Y == botY) {
    TEdge *lb = m_CurrentLM->leftBound;
    TEdge *rb = m_CurrentLM->rightBound;

    InsertEdgeIntoAEL(lb);
    InsertScanbeam(lb->ytop);
    InsertEdgeIntoAEL(rb);

    if (IsEvenOddFillType(*lb)) {
      lb->windDelta = 1;
      rb->windDelta = 1;

    } else rb->windDelta = -lb->windDelta;

    SetWindingCount(*lb);
    rb->windCnt = lb->windCnt;
    rb->windCnt2 = lb->windCnt2;

    if (CLIPPER_NEAR_EQUAL(rb->dx, CLIPPER_HORIZONTAL)) {
      // nb: only rightbounds can have a horizontal bottom edge
      AddEdgeToSEL(rb);
      InsertScanbeam(rb->nextInLML->ytop);

    } else InsertScanbeam(rb->ytop);

    if (IsContributing(*lb))
      AddLocalMinPoly(lb, rb, IntPoint(lb->xcurr, m_CurrentLM->Y));

    // if any output polygons share an edge, they'll need joining later
    if (rb->outIdx >= 0 && CLIPPER_NEAR_EQUAL(rb->dx, CLIPPER_HORIZONTAL)) {
      for (HorzJoinList::size_type i = 0; i < m_HorizJoins.size(); i++) {
        IntPoint pt, pt2; // returned by GetOverlapSegment() but unused here.
        HorzJoinRec *hj = m_HorizJoins[i];
        // if horizontals rb and hj.edge overlap, flag for joining later
        if (GetOverlapSegment(IntPoint(hj->edge->xbot, hj->edge->ybot),
                              IntPoint(hj->edge->xtop, hj->edge->ytop),
                              IntPoint(rb->xbot, rb->ybot),
                              IntPoint(rb->xtop, rb->ytop), pt, pt2))
          AddJoin(hj->edge, rb, hj->savedIdx);
      }
    }

    if (lb->nextInAEL != rb) {
      if (rb->outIdx >= 0 && rb->prevInAEL->outIdx >= 0 &&
          rb->prevInAEL->SlopesEqual(*rb, m_UseFullRange))
        AddJoin(rb, rb->prevInAEL);

      TEdge *e = lb->nextInAEL;
      IntPoint pt = IntPoint(lb->xcurr, lb->ycurr);

      while (e != rb) {
        if (!e) throw ClipperException
                  ("InsertLocalMinimaIntoAEL: missing rightbound!");

        // nb: For calculating winding counts etc, IntersectEdges() assumes
        // that param1 will be to the right of param2 ABOVE the intersection
        IntersectEdges(rb, e, pt, ipNone); // order important here
        e = e->nextInAEL;
      }
    }

    PopLocalMinima();
  }
}


void Clipper::DeleteFromAEL(TEdge *e) {
  TEdge *AelPrev = e->prevInAEL;
  TEdge *AelNext = e->nextInAEL;
  if (!AelPrev && !AelNext && e != m_ActiveEdges) return; // already deleted
  if (AelPrev) AelPrev->nextInAEL = AelNext;
  else m_ActiveEdges = AelNext;

  if (AelNext) AelNext->prevInAEL = AelPrev;
  e->nextInAEL = 0;
  e->prevInAEL = 0;
}


void Clipper::DeleteFromSEL(TEdge *e) {
  TEdge *SelPrev = e->prevInSEL;
  TEdge *SelNext = e->nextInSEL;

  if (!SelPrev && !SelNext && e != m_SortedEdges) return; // already deleted
  if (SelPrev) SelPrev->nextInSEL = SelNext;
  else m_SortedEdges = SelNext;
  if (SelNext) SelNext->prevInSEL = SelPrev;

  e->nextInSEL = 0;
  e->prevInSEL = 0;
}


void Clipper::IntersectEdges(TEdge *e1, TEdge *e2, const IntPoint &pt,
                             const IntersectProtects protects) {
  // e1 will be to the left of e2 BELOW the intersection. Therefore e1 is
  // before e2 in AEL except when e1 is being inserted at the intersection
  // point
  bool e1stops = !(ipLeft & protects) && !e1->nextInLML &&
    e1->xtop == pt.X && e1->ytop == pt.Y;
  bool e2stops = !(ipRight & protects) && !e2->nextInLML &&
    e2->xtop == pt.X && e2->ytop == pt.Y;
  bool e1Contributing = e1->outIdx >= 0;
  bool e2contributing = e2->outIdx >= 0;

  // update winding counts
  // assumes that e1 will be to the right of e2 ABOVE the intersection
  if (e1->polyType == e2->polyType) {
    if (IsEvenOddFillType(*e1)) {
      int oldE1WindCnt = e1->windCnt;

      e1->windCnt = e2->windCnt;
      e2->windCnt = oldE1WindCnt;

    } else {
      if (e1->windCnt + e2->windDelta == 0) e1->windCnt = -e1->windCnt;
      else e1->windCnt += e2->windDelta;

      if (e2->windCnt - e1->windDelta == 0) e2->windCnt = -e2->windCnt;
      else e2->windCnt -= e1->windDelta;
    }

  } else {
    if (!IsEvenOddFillType(*e2)) e1->windCnt2 += e2->windDelta;
    else e1->windCnt2 = e1->windCnt2 == 0 ? 1 : 0;

    if (!IsEvenOddFillType(*e1)) e2->windCnt2 -= e1->windDelta;
    else e2->windCnt2 = e2->windCnt2 == 0 ? 1 : 0;
  }

  PolyFillType e1FillType, e2FillType, e1FillType2, e2FillType2;
  if (e1->polyType == ptSubject) {
    e1FillType = m_SubjFillType;
    e1FillType2 = m_ClipFillType;

  } else {
    e1FillType = m_ClipFillType;
    e1FillType2 = m_SubjFillType;
  }

  if (e2->polyType == ptSubject) {
    e2FillType = m_SubjFillType;
    e2FillType2 = m_ClipFillType;

  } else {
    e2FillType = m_ClipFillType;
    e2FillType2 = m_SubjFillType;
  }

  int64_t e1Wc, e2Wc;
  switch (e1FillType) {
  case pftPositive: e1Wc = e1->windCnt; break;
  case pftNegative: e1Wc = -e1->windCnt; break;
  default: e1Wc = Abs(e1->windCnt);
  }

  switch (e2FillType) {
  case pftPositive: e2Wc = e2->windCnt; break;
  case pftNegative: e2Wc = -e2->windCnt; break;
  default: e2Wc = Abs(e2->windCnt);
  }

  if (e1Contributing && e2contributing) {
    if (e1stops || e2stops ||
        (e1Wc != 0 && e1Wc != 1) || (e2Wc != 0 && e2Wc != 1) ||
        (e1->polyType != e2->polyType && m_ClipType != ctXor))
      AddLocalMaxPoly(e1, e2, pt);

    else {
      AddOutPt(e1, pt);
      AddOutPt(e2, pt);
      e1->SwapSides(*e2);
      e1->SwapPolyIndexes(*e2);
    }

  } else if (e1Contributing) {
    if (e2Wc == 0 || e2Wc == 1) {
      AddOutPt(e1, pt);
      e1->SwapSides(*e2);
      e1->SwapPolyIndexes(*e2);
    }

  } else if (e2contributing) {
    if (e1Wc == 0 || e1Wc == 1) {
      AddOutPt(e2, pt);
      e1->SwapSides(*e2);
      e1->SwapPolyIndexes(*e2);
    }

  } else if ((e1Wc == 0 || e1Wc == 1) &&
             (e2Wc == 0 || e2Wc == 1) && !e1stops && !e2stops) {
    // neither edge is currently contributing

    int64_t e1Wc2, e2Wc2;
    switch (e1FillType2) {
    case pftPositive: e1Wc2 = e1->windCnt2; break;
    case pftNegative : e1Wc2 = -e1->windCnt2; break;
    default: e1Wc2 = Abs(e1->windCnt2);
    }

    switch (e2FillType2) {
    case pftPositive: e2Wc2 = e2->windCnt2; break;
    case pftNegative: e2Wc2 = -e2->windCnt2; break;
    default: e2Wc2 = Abs(e2->windCnt2);
    }

    if (e1->polyType != e2->polyType) AddLocalMinPoly(e1, e2, pt);
    else if (e1Wc == 1 && e2Wc == 1)
      switch (m_ClipType) {
      case ctIntersection:
        if (e1Wc2 > 0 && e2Wc2 > 0) AddLocalMinPoly(e1, e2, pt);
        break;

      case ctUnion:
        if (e1Wc2 <= 0 && e2Wc2 <= 0) AddLocalMinPoly(e1, e2, pt);
        break;

      case ctDifference:
        if (((e1->polyType == ptClip) && (e1Wc2 > 0) && (e2Wc2 > 0)) ||
            ((e1->polyType == ptSubject) && (e1Wc2 <= 0) && (e2Wc2 <= 0)))
          AddLocalMinPoly(e1, e2, pt);
        break;

      case ctXor: AddLocalMinPoly(e1, e2, pt);
      }

    else e1->SwapSides(*e2);
  }

  if ((e1stops != e2stops) &&
      ((e1stops && (e1->outIdx >= 0)) || (e2stops && (e2->outIdx >= 0)))) {
    e1->SwapSides(*e2);
    e1->SwapPolyIndexes(*e2);
  }

  // finally, delete any non-contributing maxima edges
  if (e1stops) DeleteFromAEL(e1);
  if (e2stops) DeleteFromAEL(e2);
}


void Clipper::SetHoleState(TEdge *e, OutRec *outrec) {
  bool isHole = false;
  TEdge *e2 = e->prevInAEL;

  while (e2) {
    if (e2->outIdx >= 0) {
      isHole = !isHole;
      if (!outrec->FirstLeft) outrec->FirstLeft = m_PolyOuts[e2->outIdx];
    }

    e2 = e2->prevInAEL;
  }

  if (isHole) outrec->isHole = true;
}


OutRec *Clipper::GetOutRec(int idx) {
  OutRec *outrec = m_PolyOuts[idx];

  while (outrec != m_PolyOuts[outrec->idx])
    outrec = m_PolyOuts[outrec->idx];

  return outrec;
}


void Clipper::AppendPolygon(TEdge *e1, TEdge *e2) {
  // get the start and ends of both output polygons
  OutRec *outRec1 = m_PolyOuts[e1->outIdx];
  OutRec *outRec2 = m_PolyOuts[e2->outIdx];

  OutRec *holeStateRec;
  if (outRec1->Param1RightOfParam2(outRec2)) holeStateRec = outRec2;
  else if (outRec2->Param1RightOfParam2(outRec1)) holeStateRec = outRec1;
  else holeStateRec = outRec1->GetLowermostRec(outRec2);

  OutPt *p1_lft = outRec1->pts;
  OutPt *p1_rt = p1_lft->prev;
  OutPt *p2_lft = outRec2->pts;
  OutPt *p2_rt = p2_lft->prev;
  EdgeSide side;

  // join e2 poly onto e1 poly and delete pointers to e2
  if (e1->side == esLeft) {
    if (e2->side == esLeft) {
      // z y x a b c
      p2_lft->ReversePolyPtLinks();
      p2_lft->next = p1_lft;
      p1_lft->prev = p2_lft;
      p1_rt->next = p2_rt;
      p2_rt->prev = p1_rt;
      outRec1->pts = p2_rt;

    } else {
      // x y z a b c
      p2_rt->next = p1_lft;
      p1_lft->prev = p2_rt;
      p2_lft->prev = p1_rt;
      p1_rt->next = p2_lft;
      outRec1->pts = p2_lft;
    }

    side = esLeft;
  } else {
    if (e2->side == esRight) {
      // a b c z y x
      p2_lft->ReversePolyPtLinks();
      p1_rt->next = p2_rt;
      p2_rt->prev = p1_rt;
      p2_lft->next = p1_lft;
      p1_lft->prev = p2_lft;

    } else {
      // a b c x y z
      p1_rt->next = p2_lft;
      p2_lft->prev = p1_rt;
      p1_lft->prev = p2_rt;
      p2_rt->next = p1_lft;
    }

    side = esRight;
  }

  outRec1->bottomPt = 0;

  if (holeStateRec == outRec2) {
    if (outRec2->FirstLeft != outRec1)
      outRec1->FirstLeft = outRec2->FirstLeft;
    outRec1->isHole = outRec2->isHole;
  }

  outRec2->pts = 0;
  outRec2->bottomPt = 0;

  outRec2->FirstLeft = outRec1;

  int OKIdx = e1->outIdx;
  int ObsoleteIdx = e2->outIdx;

  e1->outIdx = -1; // nb: safe because we only get here via AddLocalMaxPoly
  e2->outIdx = -1;

  TEdge *e = m_ActiveEdges;
  while (e) {
    if (e->outIdx == ObsoleteIdx) {
      e->outIdx = OKIdx;
      e->side = side;
      break;
    }

    e = e->nextInAEL;
  }

  outRec2->idx = outRec1->idx;
}


OutRec *Clipper::CreateOutRec() {
  OutRec *result = new OutRec((int)m_PolyOuts.size());
  m_PolyOuts.push_back(result);
  return result;
}


void Clipper::AddOutPt(TEdge *e, const IntPoint &pt) {
  bool ToFront = (e->side == esLeft);

  if (e->outIdx < 0) {
    OutRec *outRec = CreateOutRec();
    e->outIdx = outRec->idx;
    OutPt *newOp = new OutPt;
    outRec->pts = newOp;
    newOp->pt = pt;
    newOp->idx = outRec->idx;
    newOp->next = newOp;
    newOp->prev = newOp;
    SetHoleState(e, outRec);

  } else {
    OutRec *outRec = m_PolyOuts[e->outIdx];
    OutPt *op = outRec->pts;
    if ((ToFront && pt.Equal(op->pt)) ||
        (!ToFront && pt.Equal(op->prev->pt))) return;

    OutPt *newOp = new OutPt;
    newOp->pt = pt;
    newOp->idx = outRec->idx;
    newOp->next = op;
    newOp->prev = op->prev;
    newOp->prev->next = newOp;
    op->prev = newOp;

    if (ToFront) outRec->pts = newOp;
  }
}


void Clipper::ProcessHorizontals() {
  TEdge *horzEdge = m_SortedEdges;

  while (horzEdge) {
    DeleteFromSEL(horzEdge);
    ProcessHorizontal(horzEdge);
    horzEdge = m_SortedEdges;
  }
}


bool Clipper::IsTopHorz(const int64_t XPos) {
  TEdge *e = m_SortedEdges;

  while (e) {
    if ((XPos >= std::min(e->xcurr, e->xtop)) &&
        (XPos <= std::max(e->xcurr, e->xtop)))
      return false;

    e = e->nextInSEL;
  }

  return true;
}


void Clipper::SwapPositionsInAEL(TEdge *edge1, TEdge *edge2) {
  if (edge1->nextInAEL == edge2) {
    TEdge *next = edge2->nextInAEL;

    if (next) next->prevInAEL = edge1;
    TEdge *prev = edge1->prevInAEL;

    if (prev) prev->nextInAEL = edge2;
    edge2->prevInAEL = prev;
    edge2->nextInAEL = edge1;
    edge1->prevInAEL = edge2;
    edge1->nextInAEL = next;

  } else if (edge2->nextInAEL == edge1) {
    TEdge *next = edge1->nextInAEL;
    if (next) next->prevInAEL = edge2;

    TEdge *prev = edge2->prevInAEL;
    if (prev) prev->nextInAEL = edge1;

    edge1->prevInAEL = prev;
    edge1->nextInAEL = edge2;
    edge2->prevInAEL = edge1;
    edge2->nextInAEL = next;

  } else {
    TEdge *next = edge1->nextInAEL;
    TEdge *prev = edge1->prevInAEL;

    edge1->nextInAEL = edge2->nextInAEL;
    if (edge1->nextInAEL) edge1->nextInAEL->prevInAEL = edge1;
    edge1->prevInAEL = edge2->prevInAEL;
    if (edge1->prevInAEL) edge1->prevInAEL->nextInAEL = edge1;
    edge2->nextInAEL = next;
    if (edge2->nextInAEL) edge2->nextInAEL->prevInAEL = edge2;
    edge2->prevInAEL = prev;
    if (edge2->prevInAEL) edge2->prevInAEL->nextInAEL = edge2;
  }

  if (!edge1->prevInAEL) m_ActiveEdges = edge1;
  else if (!edge2->prevInAEL) m_ActiveEdges = edge2;
}


void Clipper::SwapPositionsInSEL(TEdge *edge1, TEdge *edge2) {
  if (!edge1->nextInSEL && !edge1->prevInSEL) return;
  if (!edge2->nextInSEL && !edge2->prevInSEL) return;

  if (edge1->nextInSEL == edge2) {
    TEdge *next = edge2->nextInSEL;
    if (next) next->prevInSEL = edge1;
    TEdge *prev = edge1->prevInSEL;
    if (prev) prev->nextInSEL = edge2;

    edge2->prevInSEL = prev;
    edge2->nextInSEL = edge1;
    edge1->prevInSEL = edge2;
    edge1->nextInSEL = next;

  } else if (edge2->nextInSEL == edge1) {
    TEdge *next = edge1->nextInSEL;
    if (next) next->prevInSEL = edge2;

    TEdge *prev = edge2->prevInSEL;
    if (prev) prev->nextInSEL = edge1;
    edge1->prevInSEL = prev;
    edge1->nextInSEL = edge2;
    edge2->prevInSEL = edge1;
    edge2->nextInSEL = next;

  } else {
    TEdge *next = edge1->nextInSEL;
    TEdge *prev = edge1->prevInSEL;

    edge1->nextInSEL = edge2->nextInSEL;
    if (edge1->nextInSEL) edge1->nextInSEL->prevInSEL = edge1;
    edge1->prevInSEL = edge2->prevInSEL;
    if (edge1->prevInSEL) edge1->prevInSEL->nextInSEL = edge1;
    edge2->nextInSEL = next;
    if (edge2->nextInSEL) edge2->nextInSEL->prevInSEL = edge2;
    edge2->prevInSEL = prev;
    if (edge2->prevInSEL) edge2->prevInSEL->nextInSEL = edge2;
  }

  if (!edge1->prevInSEL) m_SortedEdges = edge1;
  else if (!edge2->prevInSEL) m_SortedEdges = edge2;
}


void Clipper::ProcessHorizontal(TEdge *horzEdge) {
  Direction dir;
  int64_t horzLeft, horzRight;

  if (horzEdge->xcurr < horzEdge->xtop) {
    horzLeft = horzEdge->xcurr;
    horzRight = horzEdge->xtop;
    dir = dLeftToRight;

  } else {
    horzLeft = horzEdge->xtop;
    horzRight = horzEdge->xcurr;
    dir = dRightToLeft;
  }

  TEdge *eMaxPair;
  if (horzEdge->nextInLML) eMaxPair = 0;
  else eMaxPair = horzEdge->GetMaximaPair();

  TEdge *e = horzEdge->GetNextInAEL(dir);
  while (e) {
    if (e->xcurr == horzEdge->xtop && !eMaxPair) {
      if (e->SlopesEqual(*horzEdge->nextInLML, m_UseFullRange)) {
        // if output polygons share an edge, they'll need joining later
        if (horzEdge->outIdx >= 0 && e->outIdx >= 0)
          AddJoin(horzEdge->nextInLML, e, horzEdge->outIdx);
        break; // we've reached the end of the horizontal line

      } else if (e->dx < horzEdge->nextInLML->dx)
        // we really have got to the end of intermediate horz edge so quit.
        // nb: More -ve slopes follow more +ve slopes ABOVE the horizontal.
        break;
    }

    TEdge *eNext = e->GetNextInAEL(dir);

    if (eMaxPair || ((dir == dLeftToRight) && (e->xcurr < horzRight)) ||
        ((dir == dRightToLeft) && (e->xcurr > horzLeft))) {
      // so far we're still in range of the horizontal edge
      if (e == eMaxPair) {
        // horzEdge a maxima horizontal and we've arrived at its end.
        if (dir == dLeftToRight)
          IntersectEdges(horzEdge, e, IntPoint(e->xcurr, horzEdge->ycurr),
                         ipNone);
        else IntersectEdges(e, horzEdge, IntPoint(e->xcurr, horzEdge->ycurr),
                            ipNone);
        if (eMaxPair->outIdx >= 0)
          throw ClipperException("ProcessHorizontal error");

        return;

      }

      if (CLIPPER_NEAR_EQUAL(e->dx, CLIPPER_HORIZONTAL) && !e->IsMinima() &&
          !(e->xcurr > e->xtop)) {
        // An overlapping horizontal edge. Overlapping horizontal edges are
        // processed as if layered with current horizontal edge (horizEdge)
        // being infinitesimally lower that the next (e). Therfore, we
        // intersect with e only if e.xcurr is within the bounds of horzEdge
        if (dir == dLeftToRight)
          IntersectEdges(horzEdge, e, IntPoint(e->xcurr, horzEdge->ycurr),
                         (IsTopHorz(e->xcurr))? ipLeft : ipBoth);
        else IntersectEdges(e, horzEdge, IntPoint(e->xcurr, horzEdge->ycurr),
                            (IsTopHorz(e->xcurr))? ipRight : ipBoth);
      } else if (dir == dLeftToRight)
        IntersectEdges(horzEdge, e, IntPoint(e->xcurr, horzEdge->ycurr),
                       (IsTopHorz(e->xcurr))? ipLeft : ipBoth);
      else IntersectEdges(e, horzEdge, IntPoint(e->xcurr, horzEdge->ycurr),
                          (IsTopHorz(e->xcurr))? ipRight : ipBoth);

      SwapPositionsInAEL(horzEdge, e);

    } else if ((dir == dLeftToRight && e->xcurr >= horzRight) ||
               (dir == dRightToLeft && e->xcurr <= horzLeft)) break;
    e = eNext;
  }

  if (horzEdge->nextInLML) {
    if (horzEdge->outIdx >= 0)
      AddOutPt(horzEdge, IntPoint(horzEdge->xtop, horzEdge->ytop));

    UpdateEdgeIntoAEL(horzEdge);

  } else {
    if (horzEdge->outIdx >= 0)
      IntersectEdges(horzEdge, eMaxPair,
                     IntPoint(horzEdge->xtop, horzEdge->ycurr), ipBoth);

    if (eMaxPair->outIdx >= 0)
      throw ClipperException("ProcessHorizontal error");

    DeleteFromAEL(eMaxPair);
    DeleteFromAEL(horzEdge);
  }
}


void Clipper::UpdateEdgeIntoAEL(TEdge *&e) {
  if (!e->nextInLML)
    throw ClipperException("UpdateEdgeIntoAEL: invalid call");

  TEdge *AelPrev = e->prevInAEL;
  TEdge *AelNext = e->nextInAEL;

  e->nextInLML->outIdx = e->outIdx;

  if (AelPrev) AelPrev->nextInAEL = e->nextInLML;
  else m_ActiveEdges = e->nextInLML;
  if (AelNext) AelNext->prevInAEL = e->nextInLML;

  e->nextInLML->side = e->side;
  e->nextInLML->windDelta = e->windDelta;
  e->nextInLML->windCnt = e->windCnt;
  e->nextInLML->windCnt2 = e->windCnt2;
  e = e->nextInLML;
  e->prevInAEL = AelPrev;
  e->nextInAEL = AelNext;
  if (!CLIPPER_NEAR_EQUAL(e->dx, CLIPPER_HORIZONTAL)) InsertScanbeam(e->ytop);
}


bool Clipper::ProcessIntersections(const int64_t botY, const int64_t topY) {
  if (!m_ActiveEdges) return true;

  try {
    BuildIntersectList(botY, topY);
    if (!m_IntersectNodes) return true;
    if (!m_IntersectNodes->next || FixupIntersectionOrder())
      ProcessIntersectList();
    else return false;

  } catch(...) {
    m_SortedEdges = 0;
    DisposeIntersectNodes();
    throw ClipperException("ProcessIntersections error");
  }

  m_SortedEdges = 0;
  return true;
}


void Clipper::DisposeIntersectNodes() {
  while (m_IntersectNodes) {
    IntersectNode *iNode = m_IntersectNodes->next;
    delete m_IntersectNodes;
    m_IntersectNodes = iNode;
  }
}


void Clipper::BuildIntersectList(const int64_t botY, const int64_t topY) {
  if (!m_ActiveEdges) return;

  // prepare for sorting
  TEdge *e = m_ActiveEdges;
  m_SortedEdges = e;

  while (e) {
    e->prevInSEL = e->prevInAEL;
    e->nextInSEL = e->nextInAEL;
    e->xcurr = e->TopX(topY);
    e = e->nextInAEL;
  }

  // bubblesort
  bool isModified;
  do {
    isModified = false;
    e = m_SortedEdges;

    while (e->nextInSEL) {
      TEdge *eNext = e->nextInSEL;
      IntPoint pt;

      if (e->xcurr > eNext->xcurr) {
        if (!e->IntersectPoint(*eNext, pt, m_UseFullRange) &&
            e->xcurr > eNext->xcurr + 1)
          throw ClipperException("Intersection error");

        if (pt.Y > botY) {
          pt.Y = botY;
          pt.X = e->TopX(pt.Y);
        }

        InsertIntersectNode(e, eNext, pt);
        SwapPositionsInSEL(e, eNext);
        isModified = true;

      } else e = eNext;
    }

    if (e->prevInSEL) e->prevInSEL->nextInSEL = 0;
    else break;

  } while (isModified);

  m_SortedEdges = 0; // important
}


void Clipper::InsertIntersectNode(TEdge *e1, TEdge *e2, const IntPoint &pt) {
  IntersectNode *newNode = new IntersectNode;

  newNode->edge1 = e1;
  newNode->edge2 = e2;
  newNode->pt = pt;
  newNode->next = 0;

  if (!m_IntersectNodes) m_IntersectNodes = newNode;
  else if (newNode->pt.Y > m_IntersectNodes->pt.Y) {
    newNode->next = m_IntersectNodes;
    m_IntersectNodes = newNode;

  } else {
    IntersectNode *iNode = m_IntersectNodes;
    while (iNode->next  && newNode->pt.Y <= iNode->next->pt.Y)
      iNode = iNode->next;

    newNode->next = iNode->next;
    iNode->next = newNode;
  }
}


void Clipper::ProcessIntersectList() {
  while (m_IntersectNodes) {
    IntersectNode *iNode = m_IntersectNodes->next;

    IntersectEdges(m_IntersectNodes->edge1,
                   m_IntersectNodes->edge2, m_IntersectNodes->pt, ipBoth);
    SwapPositionsInAEL(m_IntersectNodes->edge1, m_IntersectNodes->edge2);

    delete m_IntersectNodes;
    m_IntersectNodes = iNode;
  }
}


void Clipper::DoMaxima(TEdge *e, int64_t topY) {
  TEdge *eMaxPair = e->GetMaximaPair();
  int64_t X = e->xtop;
  TEdge *eNext = e->nextInAEL;

  while (eNext != eMaxPair) {
    if (!eNext) throw ClipperException("DoMaxima error");
    IntersectEdges(e, eNext, IntPoint(X, topY), ipBoth);
    SwapPositionsInAEL(e, eNext);
    eNext = e->nextInAEL;
  }

  if (e->outIdx < 0 && eMaxPair->outIdx < 0) {
    DeleteFromAEL(e);
    DeleteFromAEL(eMaxPair);

  } else if (e->outIdx >= 0 && eMaxPair->outIdx >= 0)
    IntersectEdges(e, eMaxPair, IntPoint(X, topY), ipNone);

  else throw ClipperException("DoMaxima error");
}


void Clipper::ProcessEdgesAtTopOfScanbeam(const int64_t topY) {
  TEdge *e = m_ActiveEdges;

  while (e) {
    // 1. process maxima, treating them as if they're 'bent' horizontal edges,
    //  but exclude maxima with horizontal edges. nb: e can't be a horizontal.
    if (e->IsMaxima(topY) &&
        !CLIPPER_NEAR_EQUAL(e->GetMaximaPair()->dx, CLIPPER_HORIZONTAL)) {
      // 'e' might be removed from AEL, as may any following edges so
      TEdge *ePrev = e->prevInAEL;
      DoMaxima(e, topY);
      if (!ePrev) e = m_ActiveEdges;
      else e = ePrev->nextInAEL;

    } else {
      bool intermediateVert = e->IsIntermediate(topY);
      // 2. promote horizontal edges, otherwise update xcurr and ycurr
      if (intermediateVert &&
          CLIPPER_NEAR_EQUAL(e->nextInLML->dx, CLIPPER_HORIZONTAL)) {
        if (e->outIdx >= 0) {
          AddOutPt(e, IntPoint(e->xtop, e->ytop));

          for (HorzJoinList::size_type i = 0; i < m_HorizJoins.size(); i++) {
            IntPoint pt, pt2;
            HorzJoinRec *hj = m_HorizJoins[i];
            if (GetOverlapSegment
                (IntPoint(hj->edge->xbot, hj->edge->ybot),
                 IntPoint(hj->edge->xtop, hj->edge->ytop),
                 IntPoint(e->nextInLML->xbot, e->nextInLML->ybot),
                 IntPoint(e->nextInLML->xtop, e->nextInLML->ytop), pt, pt2))
              AddJoin(hj->edge, e->nextInLML, hj->savedIdx, e->outIdx);
          }

          AddHorzJoin(e->nextInLML, e->outIdx);
        }
        UpdateEdgeIntoAEL(e);
        AddEdgeToSEL(e);

      } else {
        e->xcurr = e->TopX(topY);
        e->ycurr = topY;

        if (m_ForceSimple && e->prevInAEL &&
            e->prevInAEL->xcurr == e->xcurr &&
            e->outIdx >= 0 && e->prevInAEL->outIdx >= 0) {
          if (intermediateVert)
            AddOutPt(e->prevInAEL, IntPoint(e->xcurr, topY));
          else AddOutPt(e, IntPoint(e->xcurr, topY));
        }
      }

      e = e->nextInAEL;
    }
  }

  // 3. Process horizontals at the top of the scanbeam
  ProcessHorizontals();

  // 4. Promote intermediate vertices
  e = m_ActiveEdges;
  while (e) {
    if (e->IsIntermediate(topY)) {
      if (e->outIdx >= 0) AddOutPt(e, IntPoint(e->xtop, e->ytop));
      UpdateEdgeIntoAEL(e);

      // if output polygons share an edge, they'll need joining later
      TEdge *ePrev = e->prevInAEL;
      TEdge *eNext = e->nextInAEL;
      if (ePrev && ePrev->xcurr == e->xbot &&
          ePrev->ycurr == e->ybot && e->outIdx >= 0 &&
          ePrev->outIdx >= 0 && ePrev->ycurr > ePrev->ytop &&
          e->SlopesEqual(*ePrev, m_UseFullRange)) {
        AddOutPt(ePrev, IntPoint(e->xbot, e->ybot));
        AddJoin(e, ePrev);

      } else if (eNext && eNext->xcurr == e->xbot &&
                 eNext->ycurr == e->ybot && e->outIdx >= 0 &&
                 eNext->outIdx >= 0 && eNext->ycurr > eNext->ytop &&
                 e->SlopesEqual(*eNext, m_UseFullRange)) {
        AddOutPt(eNext, IntPoint(e->xbot, e->ybot));
        AddJoin(e, eNext);
      }
    }

    e = e->nextInAEL;
  }
}


void Clipper::FixupOutPolygon(OutRec &outrec) {
  // FixupOutPolygon() - removes duplicate points and simplifies consecutive
  // parallel edges by removing the middle vertex.
  OutPt *lastOK = 0;
  outrec.bottomPt = 0;
  OutPt *pp = outrec.pts;

  while (true) {
    if (pp->prev == pp || pp->prev == pp->next) {
      pp->Dispose();
      outrec.pts = 0;
      return;
    }

    // test for duplicate points and for same slope (cross-product)
    if (pp->pt.Equal(pp->next->pt) ||
        SlopesEqual(pp->prev->pt, pp->pt, pp->next->pt, m_UseFullRange)) {
      lastOK = 0;

      OutPt *tmp = pp;
      pp->prev->next = pp->next;
      pp->next->prev = pp->prev;
      pp = pp->prev;
      delete tmp;

    } else if (pp == lastOK) break;
    else {
      if (!lastOK) lastOK = pp;
      pp = pp->next;
    }
  }

  outrec.pts = pp;
}


void Clipper::BuildResult(Polygons &polys) {
  polys.reserve(m_PolyOuts.size());
  for (PolyOutList::size_type i = 0; i < m_PolyOuts.size(); i++) {
    if (m_PolyOuts[i]->pts) {
      Polygon pg;
      OutPt *p = m_PolyOuts[i]->pts;

      do {
        pg.push_back(p->pt);
        p = p->prev;
      } while (p != m_PolyOuts[i]->pts);

      if (pg.size() > 2) polys.push_back(pg);
    }
  }
}


void Clipper::BuildResult2(PolyTree &polytree) {
  polytree.Clear();
  polytree.AllNodes.reserve(m_PolyOuts.size());

  // add each output polygon/contour to polytree
  for (PolyOutList::size_type i = 0; i < m_PolyOuts.size(); i++) {
    OutRec *outRec = m_PolyOuts[i];
    int cnt = outRec->pts->PointCount();
    if (cnt < 3) continue;

    FixHoleLinkage(*outRec);
    PolyNode *pn = new PolyNode();

    // nb: polytree takes ownership of all the PolyNodes
    polytree.AllNodes.push_back(pn);
    outRec->polyNode = pn;
    pn->Parent = 0;
    pn->Index = 0;
    pn->Contour.reserve(cnt);

    OutPt *op = outRec->pts;
    for (int j = 0; j < cnt; j++) {
      pn->Contour.push_back(op->pt);
      op = op->prev;
    }
  }

  // fixup PolyNode links etc
  polytree.Childs.reserve(m_PolyOuts.size());
  for (PolyOutList::size_type i = 0; i < m_PolyOuts.size(); i++) {
    OutRec *outRec = m_PolyOuts[i];

    if (!outRec->polyNode) continue;
    if (outRec->FirstLeft)
      outRec->FirstLeft->polyNode->AddChild(*outRec->polyNode);
    else polytree.AddChild(*outRec->polyNode);
  }
}


bool Clipper::FixupIntersectionOrder() {
  // pre-condition: intersections sorted bottom-most (then left-most) first.
  // Now it's crucial that intersections are made only between adjacent edges,
  // so to ensure this the order of intersections may need adjusting
  IntersectNode *inode = m_IntersectNodes;
  CopyAELToSEL();

  while (inode) {
    if (!inode->EdgesAdjacent()) {
      IntersectNode *nextNode = inode->next;

      while (nextNode && !nextNode->EdgesAdjacent())
        nextNode = nextNode->next;

      if (!nextNode) return false;

      inode->Swap(*nextNode);
    }

    SwapPositionsInSEL(inode->edge1, inode->edge2);
    inode = inode->next;
  }

  return true;
}


void Clipper::InsertEdgeIntoAEL(TEdge *edge) {
  edge->prevInAEL = 0;
  edge->nextInAEL = 0;

  if (!m_ActiveEdges) m_ActiveEdges = edge;
  else if (m_ActiveEdges->InsertsBefore(*edge)) {
    edge->nextInAEL = m_ActiveEdges;
    m_ActiveEdges->prevInAEL = edge;
    m_ActiveEdges = edge;

  } else {
    TEdge *e = m_ActiveEdges;
    while (e->nextInAEL  && !e->nextInAEL->InsertsBefore(*edge))
      e = e->nextInAEL;

    edge->nextInAEL = e->nextInAEL;
    if (e->nextInAEL) e->nextInAEL->prevInAEL = edge;
    edge->prevInAEL = e;
    e->nextInAEL = edge;
  }
}


bool Clipper::JoinPoints(const JoinRec *j, OutPt *&p1, OutPt *&p2) {
  OutRec *outRec1 = m_PolyOuts[j->poly1Idx];
  OutRec *outRec2 = m_PolyOuts[j->poly2Idx];
  if (!outRec1 || !outRec2)  return false;

  OutPt *pp1a = outRec1->pts;
  OutPt *pp2a = outRec2->pts;
  IntPoint pt1 = j->pt2a, pt2 = j->pt2b;
  IntPoint pt3 = j->pt1a, pt4 = j->pt1b;

  if (!(pp1a = pp1a->FindSegment(m_UseFullRange, pt1, pt2))) return false;

  if (outRec1 == outRec2) {
    // we're searching the same polygon for overlapping segments so
    // segment 2 mustn't be the same as segment 1
    pp2a = pp1a->next;
    if (!(pp2a = pp2a->FindSegment(m_UseFullRange, pt3, pt4)) || (pp2a == pp1a))
      return false;

  } else if (!(pp2a = pp2a->FindSegment(m_UseFullRange, pt3, pt4)))
    return false;

  if (!GetOverlapSegment(pt1, pt2, pt3, pt4, pt1, pt2)) return false;

  OutPt *p3, *p4, *prev = pp1a->prev;
  // get p1 & p2 polypts - the overlap start & endpoints on poly1
  if (pp1a->pt.Equal(pt1)) p1 = pp1a;
  else if (prev->pt.Equal(pt1)) p1 = prev;
  else p1 = pp1a->InsertPolyPtBetween(prev, pt1);

  if (pp1a->pt.Equal(pt2)) p2 = pp1a;
  else if (prev->pt.Equal(pt2)) p2 = prev;
  else if ((p1 == pp1a) || (p1 == prev))
    p2 = pp1a->InsertPolyPtBetween(prev, pt2);
  else if (pt2.IsBetween(pp1a->pt, p1->pt))
    p2 = pp1a->InsertPolyPtBetween(p1, pt2);
  else p2 = p1->InsertPolyPtBetween(prev, pt2);

  // get p3 & p4 polypts - the overlap start & endpoints on poly2
  prev = pp2a->prev;
  if (pp2a->pt.Equal(pt1)) p3 = pp2a;
  else if (prev->pt.Equal(pt1)) p3 = prev;
  else p3 = pp2a->InsertPolyPtBetween(prev, pt1);

  if (pp2a->pt.Equal(pt2)) p4 = pp2a;
  else if (prev->pt.Equal(pt2)) p4 = prev;
  else if ((p3 == pp2a) || (p3 == prev))
    p4 = pp2a->InsertPolyPtBetween(prev, pt2);
  else if (pt2.IsBetween(pp2a->pt, p3->pt))
    p4 = pp2a->InsertPolyPtBetween(p3, pt2);
  else p4 = p3->InsertPolyPtBetween(prev, pt2);

  // p1.pt == p3.pt and p2.pt == p4.pt so join p1 to p3 and p2 to p4
  if (p1->next == p2 && p3->prev == p4) {
    p1->next = p3;
    p3->prev = p1;
    p2->prev = p4;
    p4->next = p2;
    return true;
  }

  if (p1->prev == p2 && p3->next == p4) {
    p1->prev = p3;
    p3->next = p1;
    p2->next = p4;
    p4->prev = p2;
    return true;
  }

  return false; // an orientation is probably wrong
}


void Clipper::FixupJoinRecs(JoinRec *j, OutPt *pt, unsigned startIdx) {
  for (JoinList::size_type k = startIdx; k < m_Joins.size(); k++) {
    JoinRec *j2 = m_Joins[k];

    if (j2->poly1Idx == j->poly1Idx && pt->PointIsVertex(j2->pt1a))
      j2->poly1Idx = j->poly2Idx;

    if (j2->poly2Idx == j->poly1Idx && pt->PointIsVertex(j2->pt2a))
      j2->poly2Idx = j->poly2Idx;
  }
}


void Clipper::FixupFirstLefts1(OutRec *OldOutRec, OutRec *NewOutRec) {
  for (PolyOutList::size_type i = 0; i < m_PolyOuts.size(); i++) {
    OutRec *outRec = m_PolyOuts[i];

    if (outRec->pts && outRec->FirstLeft == OldOutRec) {
      if (outRec->pts->Contains(NewOutRec->pts, m_UseFullRange))
        outRec->FirstLeft = NewOutRec;
    }
  }
}


void Clipper::FixupFirstLefts2(OutRec *OldOutRec, OutRec *NewOutRec) {
  for (PolyOutList::size_type i = 0; i < m_PolyOuts.size(); i++) {
    OutRec *outRec = m_PolyOuts[i];
    if (outRec->FirstLeft == OldOutRec) outRec->FirstLeft = NewOutRec;
  }
}


void Clipper::JoinCommonEdges() {
  for (JoinList::size_type i = 0; i < m_Joins.size(); i++) {
    JoinRec *j = m_Joins[i];

    OutRec *outRec1 = GetOutRec(j->poly1Idx);
    OutRec *outRec2 = GetOutRec(j->poly2Idx);

    if (!outRec1->pts || !outRec2->pts) continue;

    // get the polygon fragment with the correct hole state (FirstLeft)
    // before calling JoinPoints()
    OutRec *holeStateRec;
    if (outRec1 == outRec2) holeStateRec = outRec1;
    else if (outRec1->Param1RightOfParam2(outRec2)) holeStateRec = outRec2;
    else if (outRec2->Param1RightOfParam2(outRec1)) holeStateRec = outRec1;
    else holeStateRec = outRec1->GetLowermostRec(outRec2);

    OutPt *p1, *p2;
    if (!JoinPoints(j, p1, p2)) continue;

    if (outRec1 == outRec2) {
      // instead of joining two polygons, we've just created a new one by
      // splitting one polygon into two.
      outRec1->pts = p1;
      outRec1->bottomPt = 0;
      outRec2 = CreateOutRec();
      outRec2->pts = p2;

      if (outRec2->pts->Contains(outRec1->pts, m_UseFullRange)) {
        // outRec2 is contained by outRec1
        outRec2->isHole = !outRec1->isHole;
        outRec2->FirstLeft = outRec1;

        FixupJoinRecs(j, p2, i + 1);

        // fixup FirstLeft pointers that may need reassigning to OutRec1
        if (m_UsingPolyTree) FixupFirstLefts2(outRec2, outRec1);

        FixupOutPolygon(*outRec1); // nb: do this BEFORE testing orientation
        FixupOutPolygon(*outRec2); //    but AFTER calling FixupJoinRecs()


        if ((outRec2->isHole ^ m_ReverseOutput) ==
            (outRec2->Area(m_UseFullRange) > 0))
          outRec2->pts->ReversePolyPtLinks();

      } else if (outRec1->pts->Contains(outRec2->pts, m_UseFullRange)) {
        // outRec1 is contained by outRec2
        outRec2->isHole = outRec1->isHole;
        outRec1->isHole = !outRec2->isHole;
        outRec2->FirstLeft = outRec1->FirstLeft;
        outRec1->FirstLeft = outRec2;

        FixupJoinRecs(j, p2, i + 1);

        // fixup FirstLeft pointers that may need reassigning to OutRec1
        if (m_UsingPolyTree) FixupFirstLefts2(outRec1, outRec2);

        FixupOutPolygon(*outRec1); // nb: do this BEFORE testing orientation
        FixupOutPolygon(*outRec2); //    but AFTER calling FixupJoinRecs()

        if ((outRec1->isHole ^ m_ReverseOutput) ==
            (outRec1->Area(m_UseFullRange) > 0))
          outRec1->pts->ReversePolyPtLinks();

      } else {
        // the 2 polygons are completely separate
        outRec2->isHole = outRec1->isHole;
        outRec2->FirstLeft = outRec1->FirstLeft;

        FixupJoinRecs(j, p2, i + 1);

        // fixup FirstLeft pointers that may need reassigning to OutRec2
        if (m_UsingPolyTree) FixupFirstLefts1(outRec1, outRec2);

        FixupOutPolygon(*outRec1); // nb: do this BEFORE testing orientation
        FixupOutPolygon(*outRec2); //    but AFTER calling FixupJoinRecs()
      }

    } else {
      // joined 2 polygons together

      // cleanup redundant edges
      FixupOutPolygon(*outRec1);

      outRec2->pts = 0;
      outRec2->bottomPt = 0;
      outRec2->idx = outRec1->idx;

      outRec1->isHole = holeStateRec->isHole;
      if (holeStateRec == outRec2) outRec1->FirstLeft = outRec2->FirstLeft;
      outRec2->FirstLeft = outRec1;

      // fixup FirstLeft pointers that may need reassigning to OutRec1
      if (m_UsingPolyTree) FixupFirstLefts2(outRec2, outRec1);
    }
  }
}


void Clipper::DoSimplePolygons() {
  PolyOutList::size_type i = 0;

  while (i < m_PolyOuts.size()) {
    OutRec *outrec = m_PolyOuts[i++];
    OutPt *op = outrec->pts;

    if (!op) continue;

    do { // for each Pt in Polygon until duplicate found do
      OutPt *op2 = op->next;

      while (op2 != outrec->pts) {
        if (op->pt.Equal(op2->pt) && op2->next != op &&
            op2->prev != op) {
          // split the polygon into two
          OutPt *op3 = op->prev;
          OutPt *op4 = op2->prev;
          op->prev = op4;
          op4->next = op;
          op2->prev = op3;
          op3->next = op2;

          outrec->pts = op;
          OutRec *outrec2 = CreateOutRec();
          outrec2->pts = op2;
          outrec2->UpdateOutPtIdxs();

          if (outrec2->pts->Contains(outrec->pts, m_UseFullRange)) {
            // OutRec2 is contained by OutRec1
            outrec2->isHole = !outrec->isHole;
            outrec2->FirstLeft = outrec;

          } else if (outrec->pts->Contains(outrec2->pts, m_UseFullRange)) {
            // OutRec1 is contained by OutRec2
            outrec2->isHole = outrec->isHole;
            outrec->isHole = !outrec2->isHole;
            outrec2->FirstLeft = outrec->FirstLeft;
            outrec->FirstLeft = outrec2;

          } else {
            // the 2 polygons are separate
            outrec2->isHole = outrec->isHole;
            outrec2->FirstLeft = outrec->FirstLeft;
          }

          op2 = op; // ie get ready for the next iteration
        }

        op2 = op2->next;
      }

      op = op->next;

    } while (op != outrec->pts);
  }
}
