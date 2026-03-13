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

#pragma once

#ifdef _WIN32
#define EXPORT_API extern "C" __declspec(dllexport)
#else
#define EXPORT_API extern "C" __attribute__((visibility("default")))
#endif

#include <Standard_Handle.hxx>
#include <AIS_InteractiveContext.hxx>
#include <V3d_Viewer.hxx>
#include <V3d_View.hxx>
#include <Geom_BSplineSurface.hxx>
#include <AIS_Shape.hxx>

typedef void* OcctEngineHandle;
typedef void* OcctShapeHandle;

EXPORT_API OcctEngineHandle Occt_CreateEngine(void* nativeWindowHandle);
EXPORT_API void Occt_DestroyEngine(OcctEngineHandle engine);

EXPORT_API void Occt_LoadNurbsSurface(OcctEngineHandle engine);

EXPORT_API void Occt_View_FitAll(OcctEngineHandle engine);
EXPORT_API void Occt_View_Redraw(OcctEngineHandle engine);
EXPORT_API void Occt_View_Resize(OcctEngineHandle engine, int width, int height);
