#pragma once

#include "CoreMinimal.h"

namespace Poly2Tri
{
	class FSweepContext;
	struct FNode;
	struct FPoint;
	struct FEdge;
	class FTriangle;

    class FSweep final
    {
    public:
      ~FSweep();

      void Triangulate(TSharedRef<FSweepContext> InContext);

    private:
      void SweepPoints(TSharedRef<FSweepContext> InContext);
      
      TSharedRef<FNode> PointEvent(TSharedRef<FSweepContext> InContext, TSharedRef<FPoint> InPoint);
      
      void EdgeEvent(TSharedRef<FSweepContext> InContext, TSharedPtr<FEdge> InEdge, TSharedPtr<FNode> InNode);
      void EdgeEvent(TSharedRef<FSweepContext> InContext, TSharedRef<FPoint> InStart, TSharedRef<FPoint> InEnd, TSharedPtr<FTriangle> InTriangle, TSharedRef<FPoint> InPoint);

      TSharedRef<FNode> NewFrontTriangle(TSharedRef<FSweepContext> InContext, TSharedRef<FPoint> InPoint, TSharedRef<FNode> InNode);
      void Fill(TSharedRef<FSweepContext> InContext, TSharedRef<FNode> InNode);
      bool Legalize(TSharedRef<FSweepContext> InContext, TSharedRef<FTriangle> InTriangle);
      bool InCircle(TSharedRef<FPoint> InPointA, TSharedRef<FPoint> InPointB, TSharedRef<FPoint> InPointC, TSharedRef<FPoint> InPointD);
      void RotateTrianglePair(TSharedRef<FTriangle> InTriangle, TSharedRef<FPoint> InPoint, TSharedRef<FTriangle> InOppositeTriangle, TSharedRef<FPoint> InOppositePoint);
      void FillAdvancingFront(TSharedRef<FSweepContext> InContext, TSharedRef<FNode> InNode);
      bool LargeHole_DontFill(TSharedPtr<FNode> InNode);
      bool AngleExceeds90Degrees(TSharedPtr<FPoint> InOrigin, TSharedPtr<FPoint> InPointA, TSharedPtr<FPoint> InPointB);
      bool AngleExceeds90DegreesOrIsNegative(TSharedPtr<FPoint> InOrigin, TSharedPtr<FPoint> InPointA, TSharedPtr<FPoint> InPointB);
      double Angle(TSharedRef<FPoint> InOrigin, TSharedRef<FPoint> InPointA, TSharedRef<FPoint> InPointB);
      double HoleAngle(TSharedRef<FNode> InNode);
      double BasinAngle(TSharedRef<FNode> InNode);

      void FillBasin(TSharedRef<FSweepContext> InContext, TSharedRef<FNode> InNode);
      void FillBasinReq(TSharedRef<FSweepContext> InContext, TSharedPtr<FNode> InNode);

      bool IsShallow(TSharedRef<FSweepContext> InContext, TSharedRef<FNode> InNode);
      bool IsEdgeSideOfTriangle(TSharedRef<FTriangle> InTriangle, TSharedRef<FPoint> InPointA, TSharedRef<FPoint> InPointB);

      void FillEdgeEvent(TSharedRef<FSweepContext> InContext, TSharedPtr<FEdge> InEdge, TSharedPtr<FNode> InNode);

      void FillRightAboveEdgeEvent(TSharedRef<FSweepContext> InContext, TSharedPtr<FEdge> InEdge, TSharedPtr<FNode> InNode);
      void FillRightBelowEdgeEvent(TSharedRef<FSweepContext> InContext, TSharedPtr<FEdge> InEdge, TSharedRef<FNode> InNode);

      void FillRightConcaveEdgeEvent(TSharedRef<FSweepContext> InContext, TSharedPtr<FEdge> InEdge, TSharedRef<FNode> InNode);
      void FillRightConvexEdgeEvent(TSharedRef<FSweepContext> InContext, TSharedPtr<FEdge> InEdge, TSharedRef<FNode> InNode);

      void FillLeftAboveEdgeEvent(TSharedRef<FSweepContext> InContext, TSharedPtr<FEdge> InEdge, TSharedPtr<FNode> InNode);
      void FillLeftBelowEdgeEvent(TSharedRef<FSweepContext> InContext, TSharedPtr<FEdge> InEdge, TSharedRef<FNode> InNode);

      void FillLeftConcaveEdgeEvent(TSharedRef<FSweepContext> InContext, TSharedPtr<FEdge> InEdge, TSharedRef<FNode> InNode);
      void FillLeftConvexEdgeEvent(TSharedRef<FSweepContext> InContext, TSharedPtr<FEdge> InEdge, TSharedRef<FNode> InNode);

      void FlipEdgeEvent(TSharedRef<FSweepContext> InContext, TSharedRef<FPoint> InPointA, TSharedRef<FPoint> InPointB, TSharedPtr<FTriangle> InTriangle, TSharedRef<FPoint> InPoint);

      TSharedRef<FTriangle> NextFlipTriangle(TSharedRef<FSweepContext> InContext, int32 InOrientation, TSharedRef<FTriangle> InTriangleA, TSharedRef<FTriangle> InTriangleB, TSharedRef<FPoint> InPointA, TSharedRef<FPoint> InPointB);
      TSharedRef<FPoint> NextFlipPoint(TSharedRef<FPoint> InPointA, TSharedRef<FPoint> InPointB, TSharedRef<FTriangle> InTriangleA, TSharedRef<FTriangle> InTriangleB);
      void FlipScanEdgeEvent(TSharedRef<FSweepContext> InContext, TSharedRef<FPoint> InPointA, TSharedRef<FPoint> InPointB, TSharedRef<FTriangle> InFlipTriangle, TSharedRef<FTriangle> InTriangle, TSharedRef<FPoint> InPoint);
      void FinalizationPolygon(TSharedRef<FSweepContext> InContext);
      TArray<TSharedPtr<FNode>> Nodes;
    };
}
