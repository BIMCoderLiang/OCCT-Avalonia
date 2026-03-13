/*
 * OCCT-Avalonia
 *
 * Owner:
 * 2026/03/14 - Yuqing Liang (BIMCoder Liang)
 * bim.frankliang@foxmail.com
 *
 * COPYRIGHT:Yuqing Liang
 *
 * Use of this source code is governed by MIT LICENSE
 */

#include "OCCT_CAPI.h"

#include "LNObject.h"
#include "XYZ.h"
#include "XYZW.h"
#include "NurbsSurface.h"
#include "KnotVectorUtils.h"

#include <map>
#include <Geom_BSplineSurface.hxx>
#include <AIS_Shape.hxx>
#include <V3d_Viewer.hxx>
#include <V3d_View.hxx>
#include <AIS_InteractiveContext.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <Aspect_DisplayConnection.hxx>
#include <TColgp_Array2OfPnt.hxx>
#include <TColStd_Array2OfReal.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <TColStd_Array1OfInteger.hxx>

#ifdef _WIN32
#include <WNT_Window.hxx>
typedef WNT_Window PlatformWindow;
#elif __APPLE__
#include <Cocoa_Window.hxx>
typedef Cocoa_Window PlatformWindow;
#else
#include <Xw_Window.hxx>
typedef Xw_Window PlatformWindow;
#endif
#include <BRep_Builder.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>

struct OcctEngine {
    Handle(Aspect_DisplayConnection) DisplayConnection;
    Handle(OpenGl_GraphicDriver) Driver;
    Handle(V3d_Viewer) Viewer;
    Handle(V3d_View) View;
    Handle(AIS_InteractiveContext) Context;
    Handle(PlatformWindow) Window;
};

void ConvertToOpenCascadeSurface(const LNLib::LN_NurbsSurface& surface, Handle(Geom_BSplineSurface)& internalSurface)
{
    std::vector<double> knotU = surface.KnotVectorU;
    std::vector<double> knotV = surface.KnotVectorV;
    const int numPolesU = static_cast<int>(surface.ControlPoints.size());
    const int numPolesV = static_cast<int>(surface.ControlPoints[0].size());

    TColgp_Array2OfPnt poles(1, numPolesU, 1, numPolesV);
    TColStd_Array2OfReal weights(1, numPolesU, 1, numPolesV);

    for (int i = 0; i < numPolesU; i++) {
        for (int j = 0; j < numPolesV; j++) {
            const LNLib::XYZW& wcp = surface.ControlPoints[i][j];
            const LNLib::XYZ& cp = wcp.ToXYZ(true);
            poles.SetValue(i + 1, j + 1, gp_Pnt(cp.GetX(), cp.GetY(), cp.GetZ()));
            weights.SetValue(i + 1, j + 1, wcp.GetW());
        }
    }

    std::map<double, int> mapU = LNLib::KnotVectorUtils::GetKnotMultiplicityMap(knotU);
    TColStd_Array1OfReal knotsU(1, static_cast<int>(mapU.size()));
    TColStd_Array1OfInteger multsU(1, static_cast<int>(mapU.size()));
    int idx = 1;
    for (auto const& [key, val] : mapU) {
        knotsU.SetValue(idx, key);
        multsU.SetValue(idx, val);
        idx++;
    }

    std::map<double, int> mapV = LNLib::KnotVectorUtils::GetKnotMultiplicityMap(knotV);
    TColStd_Array1OfReal knotsV(1, static_cast<int>(mapV.size()));
    TColStd_Array1OfInteger multsV(1, static_cast<int>(mapV.size()));
    idx = 1;
    for (auto const& [key, val] : mapV) {
        knotsV.SetValue(idx, key);
        multsV.SetValue(idx, val);
        idx++;
    }

    internalSurface = new Geom_BSplineSurface(
        poles, weights, knotsU, knotsV,
        multsU, multsV,
        surface.DegreeU, surface.DegreeV);
}

OcctEngineHandle Occt_CreateEngine(void* nativeWindowHandle)
{
    OcctEngine* engine = new OcctEngine();

    engine->DisplayConnection = new Aspect_DisplayConnection();
    engine->Driver = new OpenGl_GraphicDriver(engine->DisplayConnection);

    engine->Viewer = new V3d_Viewer(engine->Driver);
    engine->Viewer->SetDefaultLights();
    engine->Viewer->SetLightOn();

    engine->View = engine->Viewer->CreateView();

    engine->Window = new PlatformWindow((Aspect_Drawable)nativeWindowHandle);
    engine->View->SetWindow(engine->Window);

    if (!engine->Window->IsMapped()) {
        engine->Window->Map();
    }

    engine->Context = new AIS_InteractiveContext(engine->Viewer);
    engine->Context->UpdateCurrentViewer();

    engine->View->Redraw();

    return engine;
}

void Occt_DestroyEngine(OcctEngineHandle handle)
{
    if (!handle) return;
    OcctEngine* engine = static_cast<OcctEngine*>(handle);

    engine->Context.Nullify();
    engine->View.Nullify();
    engine->Viewer.Nullify();
    engine->Window.Nullify();
    engine->Driver.Nullify();
    engine->DisplayConnection.Nullify();

    delete engine;
}

using namespace LNLib;
void Occt_LoadNurbsSurface(OcctEngineHandle handle)
{
    OcctEngine* engine = static_cast<OcctEngine*>(handle);
    if (!engine) return;

    //nurbs surface data ++++++++++++++++++++++++++++++++++++++++++++++++++

    int degreeU = 3;
    int degreeV = 3;
    std::vector<double> kvU = { 0,0,0,0,0.4,0.6,1,1,1,1 };
    std::vector<double> kvV = { 0,0,0,0,0.4,0.6,1,1,1,1 };
    std::vector<std::vector<XYZW>> controlPoints(6, std::vector<XYZW>(6));

    controlPoints[0][0] = XYZW(0, 0, 0, 1);
    controlPoints[0][1] = XYZW(6.666666, 0, 4, 1);
    controlPoints[0][2] = XYZW(16.666666, 0, 22, 1);
    controlPoints[0][3] = XYZW(33.333333, 0, 22, 1);
    controlPoints[0][4] = XYZW(43.333333, 0, 4, 1);
    controlPoints[0][5] = XYZW(50, 0, 0, 1);

    controlPoints[1][0] = XYZW(0, 6.666667, 0, 1);
    controlPoints[1][1] = XYZW(6.6666667, 6.666667, 9.950068, 1);
    controlPoints[1][2] = XYZW(16.6666666, 6.666667, 9.65541838, 1);
    controlPoints[1][3] = XYZW(33.3333333, 6.666667, 47.21371742, 1);
    controlPoints[1][4] = XYZW(43.3333333, 6.666667, -11.56982167, 1);
    controlPoints[1][5] = XYZW(50, 6.6666667, 0, 1);

    controlPoints[2][0] = XYZW(0, 16.666666, 0, 1);
    controlPoints[2][1] = XYZW(6.6666667, 16.666666, -7.9001371, 1);
    controlPoints[2][2] = XYZW(16.6666666, 16.666666, 46.6891632, 1);
    controlPoints[2][3] = XYZW(33.3333333, 16.666667, -28.4274348, 1);
    controlPoints[2][4] = XYZW(43.3333333, 16.666667, 35.1396433, 1);
    controlPoints[2][5] = XYZW(50, 16.6666667, 0, 1);

    controlPoints[3][0] = XYZW(0, 33.3333333, 0, 1);
    controlPoints[3][1] = XYZW(6.6666667, 33.3333333, 29.2877911, 1);
    controlPoints[3][2] = XYZW(16.6666666, 33.3333333, -30.4644718, 1);
    controlPoints[3][3] = XYZW(33.3333333, 33.3333333, 129.1582990, 1);
    controlPoints[3][4] = XYZW(43.3333333, 33.3333333, -62.1717142, 1);
    controlPoints[3][5] = XYZW(50, 33.333333, 0, 1);

    controlPoints[4][0] = XYZW(0, 43.333333, 0, 1);
    controlPoints[4][1] = XYZW(6.6666667, 43.333333, -10.384636, 1);
    controlPoints[4][2] = XYZW(16.6666666, 43.333333, 59.21371742, 1);
    controlPoints[4][3] = XYZW(33.3333333, 43.333333, -37.7272976, 1);
    controlPoints[4][4] = XYZW(43.3333333, 43.333333, 45.1599451, 1);
    controlPoints[4][5] = XYZW(50, 43.333333, 0, 1);

    controlPoints[5][0] = XYZW(0, 50, 0, 1);
    controlPoints[5][1] = XYZW(6.6666667, 50, 0, 1);
    controlPoints[5][2] = XYZW(16.6666666, 50, 0, 1);
    controlPoints[5][3] = XYZW(33.3333333, 50, 0, 1);
    controlPoints[5][4] = XYZW(43.3333333, 50, 0, 1);
    controlPoints[5][5] = XYZW(50, 50, 0, 1);

    LNLib::LN_NurbsSurface surface;
    surface.DegreeU = degreeU;
    surface.DegreeV = degreeV;
    surface.KnotVectorU = kvU;
    surface.KnotVectorV = kvV;
    surface.ControlPoints = controlPoints;

    //nurbs surface data +++++++++++++++++++++++++++++++++++++++++++++++

    Handle(Geom_BSplineSurface) geoSurface;
    ConvertToOpenCascadeSurface(surface, geoSurface);

    BRep_Builder builder;
    TopoDS_Compound topoDs;
    builder.MakeCompound(topoDs);

    TopoDS_Face face = BRepBuilderAPI_MakeFace(geoSurface, Precision::Confusion());
    if (!face.IsNull()) {
        builder.Add(topoDs, face);
    }

    Handle(AIS_Shape) aisShape = new AIS_Shape(topoDs);

    aisShape->SetColor(Quantity_Color(Quantity_NOC_LIGHTBLUE));
    aisShape->SetMaterial(Graphic3d_NOM_PLASTIC);
    aisShape->SetDisplayMode(AIS_Shaded);
    aisShape->Attributes()->SetFaceBoundaryDraw(Standard_True);

    engine->Context->Display(aisShape, Standard_True);
    Occt_View_FitAll(handle);
}

void Occt_View_FitAll(OcctEngineHandle handle) {
    OcctEngine* e = static_cast<OcctEngine*>(handle);
    if (e) { e->View->FitAll(); e->View->Redraw(); }
}
void Occt_View_Redraw(OcctEngineHandle handle) {
    OcctEngine* e = static_cast<OcctEngine*>(handle);
    if (e) e->View->Redraw();
}
void Occt_View_Resize(OcctEngineHandle handle, int w, int h) {
    OcctEngine* e = static_cast<OcctEngine*>(handle);
    if (e) {
        e->View->MustBeResized();
        e->View->Redraw();
    }
}