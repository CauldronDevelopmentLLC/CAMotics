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

#pragma once

#include "ClipperBase.h"
#include "OutPt.h"
#include "OutRec.h"
#include "IntersectNode.h"
#include "PolyTree.h"


namespace ClipperLib {
  class Clipper : public virtual ClipperBase {
    struct Scanbeam {
      int64_t    Y;
      Scanbeam *next;
    };


    struct JoinRec {
      IntPoint  pt1a;
      IntPoint  pt1b;
      int       poly1Idx;
      IntPoint  pt2a;
      IntPoint  pt2b;
      int       poly2Idx;
    };


    struct HorzJoinRec {
      TEdge    *edge;
      int       savedIdx;
    };

    typedef std::vector <OutRec *> PolyOutList;
    typedef std::vector <JoinRec *> JoinList;
    typedef std::vector <HorzJoinRec *> HorzJoinList;

    PolyOutList     m_PolyOuts;
    JoinList        m_Joins;
    HorzJoinList    m_HorizJoins;
    ClipType        m_ClipType;
    Scanbeam        *m_Scanbeam;
    TEdge           *m_ActiveEdges;
    TEdge           *m_SortedEdges;
    IntersectNode   *m_IntersectNodes;
    bool            m_ExecuteLocked;
    PolyFillType    m_ClipFillType;
    PolyFillType    m_SubjFillType;
    bool            m_ReverseOutput;
    bool            m_UsingPolyTree;
    bool            m_ForceSimple;

  public:
    Clipper();
    ~Clipper();

    bool Execute(ClipType clipType, Polygons &solution,
                 PolyFillType subjFillType = pftEvenOdd,
                 PolyFillType clipFillType = pftEvenOdd);
    bool Execute(ClipType clipType, PolyTree &polytree,
                 PolyFillType subjFillType = pftEvenOdd,
                 PolyFillType clipFillType = pftEvenOdd);
    void Clear();
    bool ReverseSolution() {return m_ReverseOutput;}
    void ReverseSolution(bool value) {m_ReverseOutput = value;}
    bool ForceSimple() {return m_ForceSimple;}
    void ForceSimple(bool value) {m_ForceSimple = value;}

  protected:
    void Reset();
    virtual bool ExecuteInternal();

  private:
    void DisposeScanbeamList();
    void SetWindingCount(TEdge &edge);
    bool IsEvenOddFillType(const TEdge &edge) const;
    bool IsEvenOddAltFillType(const TEdge &edge) const;
    void InsertScanbeam(const int64_t Y);
    int64_t PopScanbeam();
    void InsertLocalMinimaIntoAEL(const int64_t botY);
    void InsertEdgeIntoAEL(TEdge *edge);
    void AddEdgeToSEL(TEdge *edge);
    void CopyAELToSEL();
    void DeleteFromSEL(TEdge *e);
    void DeleteFromAEL(TEdge *e);
    void UpdateEdgeIntoAEL(TEdge *&e);
    void SwapPositionsInSEL(TEdge *edge1, TEdge *edge2);
    bool IsContributing(const TEdge &edge) const;
    bool IsTopHorz(const int64_t XPos);
    void SwapPositionsInAEL(TEdge *edge1, TEdge *edge2);
    void DoMaxima(TEdge *e, int64_t topY);
    void ProcessHorizontals();
    void ProcessHorizontal(TEdge *horzEdge);
    void AddLocalMaxPoly(TEdge *e1, TEdge *e2, const IntPoint &pt);
    void AddLocalMinPoly(TEdge *e1, TEdge *e2, const IntPoint &pt);
    OutRec *GetOutRec(int idx);
    void AppendPolygon(TEdge *e1, TEdge *e2);
    void IntersectEdges(TEdge *e1, TEdge *e2, const IntPoint &pt,
                        const IntersectProtects protects);
    OutRec *CreateOutRec();
    void AddOutPt(TEdge *e, const IntPoint &pt);
    void DisposeAllPolyPts();
    void DisposeOutRec(PolyOutList::size_type index);
    bool ProcessIntersections(const int64_t botY, const int64_t topY);
    void InsertIntersectNode(TEdge *e1, TEdge *e2, const IntPoint &pt);
    void BuildIntersectList(const int64_t botY, const int64_t topY);
    void ProcessIntersectList();
    void ProcessEdgesAtTopOfScanbeam(const int64_t topY);
    void BuildResult(Polygons &polys);
    void BuildResult2(PolyTree &polytree);
    void SetHoleState(TEdge *e, OutRec *outrec);
    void DisposeIntersectNodes();
    bool FixupIntersectionOrder();
    void FixupOutPolygon(OutRec &outrec);
    bool IsHole(TEdge *e);
    void FixHoleLinkage(OutRec &outrec);
    void AddJoin(TEdge *e1, TEdge *e2, int e1OutIdx = -1, int e2OutIdx = -1);
    void ClearJoins();
    void AddHorzJoin(TEdge *e, int idx);
    void ClearHorzJoins();
    bool JoinPoints(const JoinRec *j, OutPt *&p1, OutPt *&p2);
    void FixupJoinRecs(JoinRec *j, OutPt *pt, unsigned startIdx);
    void JoinCommonEdges();
    void DoSimplePolygons();
    void FixupFirstLefts1(OutRec *OldOutRec, OutRec *NewOutRec);
    void FixupFirstLefts2(OutRec *OldOutRec, OutRec *NewOutRec);
  };
}
