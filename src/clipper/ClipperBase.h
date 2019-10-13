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

#include "Polygons.h"
#include "TEdge.h"


namespace ClipperLib {
  struct LocalMinima {
    int64_t      Y;
    TEdge        *leftBound;
    TEdge        *rightBound;
    LocalMinima  *next;
  };


  struct Bounds {
    int64_t left;
    int64_t top;
    int64_t right;
    int64_t bottom;
  };


  // ClipperBase is the ancestor to the Clipper class. It should not be
  // instantiated directly. This class simply abstracts the conversion of sets
  // of polygon coordinates into edge objects that are stored in a LocalMinima
  // list.
  class ClipperBase {
  protected:
    LocalMinima      *m_CurrentLM;
    LocalMinima      *m_MinimaList;
    bool              m_UseFullRange;
    EdgeList          m_edges;

  public:
    ClipperBase();
    virtual ~ClipperBase();

    bool AddPolygon(const Polygon &pg, PolyType polyType);
    bool AddPolygons(const Polygons &ppg, PolyType polyType);
    virtual void Clear();
    Bounds GetBounds();

  protected:
    void DisposeLocalMinimaList();
    TEdge *AddBoundsToLML(TEdge *e);
    void PopLocalMinima();
    virtual void Reset();
    void InsertLocalMinima(LocalMinima *newLm);
  };
}
