#pragma once

#include "CoreMinimal.h"

#include "Poly2Tri/Sweep/AdvancingFront.h"
#include "Poly2Tri/Sweep/SweepContext.h"
#include "Poly2Tri/Sweep/Sweep.h"

namespace Poly2Tri
{
  namespace Sweep
  {
    class FCDT
    {
    public:
      /** Add polyline with non repeating points */
      FCDT(TArray<FPoint*> InPolyline);
      virtual ~FCDT() { }

      TArray<FTriangle*> GetTriangles();
      TArray<FTriangle*> GetMap();

      void AddHole(TArray<FPoint*> InPolyline);
      
      /** Add a steiner point */
      void AddPoint(FPoint* InPoint);

      void Triangulate();

    private:
      TUniquePtr<FSweepContext> SweepContext;
      TUniquePtr<FSweep> Sweep;
    };
  }
}