#include "Poly2Tri/Sweep/CDT.h"

namespace Poly2Tri
{
  namespace Sweep
  {
    FCDT::FCDT(TArray<FPoint*> InPolyline)
      : SweepContext(new FSweepContext(InPolyline)),
      Sweep(new FSweep()) { }

    TArray<FTriangle*> FCDT::GetTriangles()
    {
      return SweepContext->GetTriangles();
    }

    TArray<FTriangle*> FCDT::GetMap()
    {
      return SweepContext->GetMap();
    }

    void FCDT::AddHole(TArray<FPoint*> InPolyline)
    {
      SweepContext->AddHole(InPolyline);
    }

    void FCDT::AddPoint(FPoint* InPoint)
    {
      SweepContext->AddPoint(InPoint);
    }

    void FCDT::Triangulate()
    {
      Sweep->Triangulate(*SweepContext);
    }
  }
}
