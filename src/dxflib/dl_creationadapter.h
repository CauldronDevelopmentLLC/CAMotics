/****************************************************************************
** Copyright (C) 2001-2013 RibbonSoft, GmbH. All rights reserved.
**
** This file is part of the dxflib project.
**
** This file is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** Licensees holding valid dxflib Professional Edition licenses may use
** this file in accordance with the dxflib Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.ribbonsoft.com for further details.
**
** Contact info@ribbonsoft.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef DL_CREATIONADAPTER_H
#define DL_CREATIONADAPTER_H

#include "dl_global.h"

#include "dl_creationinterface.h"

/**
 * An abstract adapter class for receiving DXF events when a DXF file is being read.
 * The methods in this class are empty. This class exists as convenience for creating
 * listener objects.
 *
 * @author Andrew Mustun
 */
class DXFLIB_EXPORT DL_CreationAdapter : public DL_CreationInterface {
public:
    DL_CreationAdapter() {}
    ~DL_CreationAdapter() {}
    void processCodeValuePair(unsigned int, const std::string&) override {}
    void endSection() override {}
    void addLayer(const DL_LayerData&) override {}
    void addLinetype(const DL_LinetypeData&) override {}
    void addLinetypeDash(double) override {}
    void addBlock(const DL_BlockData&) override {}
    void endBlock() override {}
    void addTextStyle(const DL_StyleData&) override {}
    void addPoint(const DL_PointData&) override {}
    void addLine(const DL_LineData&) override {}
    void addXLine(const DL_XLineData&) override {}
    void addRay(const DL_RayData&) override {}

    void addArc(const DL_ArcData&) override {}
    void addCircle(const DL_CircleData&) override {}
    void addEllipse(const DL_EllipseData&) override {}

    void addPolyline(const DL_PolylineData&) override {}
    void addVertex(const DL_VertexData&) override {}

    void addSpline(const DL_SplineData&) override {}
    void addControlPoint(const DL_ControlPointData&) override {}
    void addFitPoint(const DL_FitPointData&) override {}
    void addKnot(const DL_KnotData&) override {}

    void addInsert(const DL_InsertData&) override {}

    void addMText(const DL_MTextData&) override {}
    void addMTextChunk(const std::string&) override {}
    void addText(const DL_TextData&) override {}
    void addArcAlignedText(const DL_ArcAlignedTextData&) override {}
    void addAttribute(const DL_AttributeData&) override {}

    void addDimAlign(const DL_DimensionData&,
                             const DL_DimAlignedData&) override {}
    void addDimLinear(const DL_DimensionData&,
                              const DL_DimLinearData&) override {}
    void addDimRadial(const DL_DimensionData&,
                              const DL_DimRadialData&) override {}
    void addDimDiametric(const DL_DimensionData&,
                              const DL_DimDiametricData&) override {}
    void addDimAngular(const DL_DimensionData&,
                              const DL_DimAngularData&) override {}
    void addDimAngular3P(const DL_DimensionData&,
                              const DL_DimAngular3PData&) override {}
    void addDimOrdinate(const DL_DimensionData&,
                             const DL_DimOrdinateData&) override {}
    void addLeader(const DL_LeaderData&) override {}
    void addLeaderVertex(const DL_LeaderVertexData&) override {}

    void addHatch(const DL_HatchData&) override {}

    void addTrace(const DL_TraceData&) override {}
    void add3dFace(const DL_3dFaceData&) override {}
    void addSolid(const DL_SolidData&) override {}

    void addImage(const DL_ImageData&) override {}
    void linkImage(const DL_ImageDefData&) override {}
    void addHatchLoop(const DL_HatchLoopData&) override {}
    void addHatchEdge(const DL_HatchEdgeData&) override {}

    void addXRecord(const std::string&) override {}
    void addXRecordString(int, const std::string&) override {}
    void addXRecordReal(int, double) override {}
    void addXRecordInt(int, int) override {}
    void addXRecordBool(int, bool) override {}

    void addXDataApp(const std::string&) override {}
    void addXDataString(int, const std::string&) override {}
    void addXDataReal(int, double) override {}
    void addXDataInt(int, int) override {}

    void addDictionary(const DL_DictionaryData&) override {}
    void addDictionaryEntry(const DL_DictionaryEntryData&) override {}

    void endEntity() override {}

    void addComment(const std::string&) override {}

    void setVariableVector(const std::string&,  double, double, double, int) override {}
    void setVariableString(const std::string&, const std::string&, int) override {}
    void setVariableInt(const std::string&, int, int) override {}
    void setVariableDouble(const std::string&, double, int) override {}
#ifdef DL_COMPAT
    void setVariableVector(const char*,  double, double, double, int) override {}
    void setVariableString(const char*, const char*, int) override {}
    void setVariableInt(const char*, int, int) override {}
    void setVariableDouble(const char*, double, int) override {}
    void processCodeValuePair(unsigned int, char*) override {}
    void addComment(const char*) override {}
    void addMTextChunk(const char*) override {}
#endif
    void endSequence() override {}
};

#endif
