#include "Poly2Tri/Common/Shapes.h"

namespace Poly2Tri
{
    namespace Shapes
    {
      FTriangle::FTriangle(const FPoint& InA, const FPoint& InB, const FPoint& InC)
      {
        Points[0] = &InA;
        Points[1] = &InB;
        Points[2] = &InC;
          
        Neighbors[0] = nullptr;
        Neighbors[1] = nullptr;
        Neighbors[2] = nullptr;

        ConstrainedEdge[0] = ConstrainedEdge[1] = ConstrainedEdge[2] = false;
        DelaunayEdge[0] = DelaunayEdge[1] = DelaunayEdge[2] = false;
        bIsInterior = false;
      }

      // Update neighbor pointers
      void FTriangle::MarkNeighbor(TSharedPtr<FPoint> InPointA, TSharedPtr<FPoint> InPointB, TSharedPtr<FTriangle> InTriangle)
      {
        if((InPointA == Points[2] && InPointB == Points[1]) || (InPointA == Points[1] && InPointB == Points[2]))
          Neighbors[0] = InTriangle;
        else if((InPointA == Points[0] && InPointB == Points[2]) || (InPointA == Points[2] && InPointB == Points[0]))
          Neighbors[1] = InTriangle;
        else if((InPointA == Points[0] && InPointB == Points[1]) || (InPointA == Points[1] && InPointB == Points[0]))
          Neighbors[2] = InTriangle;
        else
          assert(0);
      }

      // Exhaustive search to update neighbor pointers
      void FTriangle::MarkNeighbor(TSharedRef<FTriangle>& InTriangle)
      {
        if(InTriangle.Contains(Points[1], Points[2])) 
        {
          Neighbors[0] = &InTriangle;
          InTriangle.MarkNeighbor(Points[1], Points[2], this);
        }
        else if(InTriangle.Contains(Points[0], Points[2]))
        {
          Neighbors[1] = &InTriangle;
          InTriangle.MarkNeighbor(Points[0], Points[1], this);
        }
        else if(InTriangle.Contains(Points[0], Points[1]))
        {
          Neighbors[2] = &InTriangle;
          InTriangle.MarkNeighbor(Points[0], Points[1], this);
        }
      }

      /**
       * Clears all references to all other triangles and points
       */
      void FTriangle::Clear()
      {
          TSharedPtr<FTriangle> Triangle;
          for(auto i = 0; i < 3; i++)
          {
            Triangle = Neigbors[i];
            if(Triangle.IsValid())
              Triangle->ClearNeighbor(this);
          }

          ClearNeighbors();
          Points.Empty();
      }

      void FTriangle::ClearNeighbor(TSharedPtr<FTriangle> InTriangle)
      {
        if(Neighbors[0] == InTriangle)
          Neighbors[0].Reset();
        else if(Neighbors[1] == InTriangle)
          Neighbors[1].Reset();
        else if(Neighbors[2] == InTriangle)
          Neighbors[2].Reset();
      }

      void FTriangle::ClearNeighbors()
      {
        Neighbors.Empty();
      }

      void FTriangle::ClearDelunayEdges()
      {
        DelaunayEdges.Set(false);
      }

      TSharedPtr<FPoint> FTriangle::GetOppositePoint(TSharedRef<FTriangle> InTriangle, TSharedRef<FPoint>& InPoint)
      {
        auto Clockwise = InTriangle.GetPointCW(InPoint);
        auto X = Clockwise->X;
        auto Y = Clockwise->Y;
        X = InPoint.X;
        Y = InPoint.Y;
        return GetPointCW(*Clockwise);
      }

      // Legalized triangle by rotating clockwise around point(0)
      void FTriangle::Legalize(TSharedRef<FPoint> InPoint)
      {
        Points[1] = Points[0];
        Points[0] = Points[2];
        Points[2] = &InPoint;
      }

      // Legalize triagnle by rotating clockwise around oPoint
      void FTriangle::Legalize(TSharedRef<FPoint> InPointO, TSharedRef<FPoint> InPointN)
      {
        if(&InPointO == Points[0])
        {
          Points[1] = Points[0];
          Points[0] = Points[2];
          Points[2] = &InPointN;
        }
        else if(&InPointO == Points[1])
        {
          Points[2] = Points[1];
          Points[1] = Points[0];
          Points[0] = &InPointN;
        }
        else if(&InPointO == Points[2])
        {
          Points[0] = Points[2];
          Points[2] = Points[1];
          Points[1] = &InPointN;
        }
        else
        {
          assert(0);
        }
      }

      int32 FTriangle::GetPointIndex(const TSharedPtr<FPoint> InPoint)
      {
        if(InPoint == Points[0])
          return 0;
        else if(InPoint == Points[1])
          return 1;
        else if(InPoint == Points[2])
          return 2;

        assert(0);
        return -1;
      }

      int32 FTriangle::GetEdgeIndex(const TSharedPtr<FPoint> InPointA, const TSharedPtr<FPoint> InPointB)
      {
        if(Points[0] == InPointA)
        {
          if(Points[1] == InPointB)
            return 2;
          else if(Points[2] == InPointB)
            return 1;
        }
        else if(Points[1] == InPointA)
        {
          if(Points[2] == InPointB)
            return 0;
          else if(Points[0] == InPointB)
            return 2;
        }
        else if(Points[2] == InPointA)
        {
          if(Points[0] == InPointB)
            return 1;
          else if(Points[1] == InPointB)
            return 0;
        }
        return -1;
      }

      void FTriangle::MarkConstrainedEdge(const int32 InIndex)
      {
        ConstrainedEdge[InIndex] = true;
      }

      void FTriangle::MarkConstrainedEdge(TSharedRef<FEdge>& InEdge)
      {
        MarkConstrainedEdge(InEdge.Start, InEdge.End);
      }

      // Mark edge as constrained
      void FTriangle::MarkConstrainedEdge(TSharedPtr<FPoint> InStart, TSharedPtr<FPoint> InEnd)
      {
        if((InEnd == Points[0] && InStart == Points[1]) || (InEnd == Points[1] && InStart == Points[0]))
          ConstrainedEdge[2] = true;
        else if((InEnd == Points[0] && InStart == Points[2]) || (InEnd == Points[2] && InStart == Points[0]))
          ConstrainedEdge[1] = true;
        else if((InEnd == Points[1] && InStart == Points[2]) || (InEnd == Points[2] && InStart == Points[1]))
          ConstrainedEdge[0] = true;
      }

      // The point counter-clockwise to given point
      TSharedPtr<FPoint> FTriangle::GetPointCW(TSharedRef<FPoint> InPoint)
      {
        if(&InPoint == Points[0])
          return Points[2];
        else if(&InPoint == Points[1])
          return Points[0];
        else if(&InPoint == Points[2])
          return Points[1];

        assert(0);
        return nullptr;
      }

      // The point counter-clockwise to given point
      TSharedPtr<FPoint> FTriangle::GetPointCCW(TSharedRef<FPoint> InPoint)
      {
        if(&InPoint == Points[0])
          return Points[1];
        else if(&InPoint == Points[1])
          return Points[2];
        else if(&InPoint == Points[2])
          return Points[0];

        assert(0);
        return nullptr;
      }

      // The neighbor clockwise to given point
      TSharedPtr<FTriangle> FTriangle::NeighborCW(TSharedRef<FPoint> InPoint)
      {
        if(&InPoint == Points[0])
          return Neighbors[1];
        else if(&InPoint == Points[1])
          return Neigbhors[2];

        return Neighbors[0];
      }

      // The neighbor counter-clockwise to given point
      TSharedPtr<FTriangle> FTriangle::NeighborCCW(TSharedRef<FPoint> InPoint)
      {
        if(&InPoint == Points[0])
          return Neighbors[2];
        else if(&InPoint == Points[1])
          return Neighbors[0];

        return Neighbors[1];
      }

      bool FTriangle::GetConstrainedEdgeCCW(TSharedRef<FPoint> InPoint)
      {
        if(&InPoint == Points[0])
          return ConstrainedEdge[2];
        else if(&InPoint == Points[1])
          return ConstrainedEdge[0];

        return ConstrainedEdge[1];
      }

      bool FTriangle::GetConstrainedEdgeCW(TSharedRef<FPoint> InPoint)
      {
        if(&InPoint == Points[0])
          return ConstrainedEdge[1];
        else if(&InPoint == Points[1])
          return ConstrainedEdge[2];

        return ConstrainedEdge[0];
      }

      void FTriangle::SetConstrainedEdgeCCW(TSharedRef<FPoint> InPoint, bool bValue)
      {
        if(&InPoint == Points[0])
          ConstrainedEdge[2] = bValue;
        else if(&InPoint == Points[1])
          ConstrainedEdge[0] = bValue;
        else
          ConstrainedEdge[1] = bValue;
      }

      void FTriangle::SetConstrainedEdgeCW(TSharedRef<FPoint> InPoint, bool bValue)
      {
        if(&InPoint == Points[0])
          ConstrainedEdge[1] = bValue;
        else if(&InPoint == Points[1])
          ConstrainedEdge[2] = bValue;
        else
          ConstrainedEdge[0] = bValue;
      }

      bool FTriangle::GetDelunayEdgeCCW(TSharedRef<FPoint> InPoint)
      {
        if(&InPoint == Points[0])
          return DelaunayEdge[2];
        else if(&InPoint == Points[1])
          return DelaunayEdge[0];
        
        return DelaunayEdge[1];
      }

      bool FTriangle::GetDelunayEdgeCW(TSharedRef<FPoint> InPoint)
      {
        if(&InPoint == Points[0])
          return DelaunayEdge[1];
        else if(&InPoint == Points[1])
          return DelaunayEdge[2];
        
        return DelaunayEdge[0];
      }

      void FTriangle::SetDelunayEdgeCCW(TSharedRef<FPoint> InPoint, bool bValue)
      {
        if(&InPoint == Points[0])
          DelaunayEdge[2] = bValue;
        else if(&InPoint == Points[1])
          DelaunayEdge[0] = bValue;
        else
          DelaunayEdge[1] = bValue;
      }

      void FTriangle::SetDelunayEdgeCW(TSharedRef<FPoint> InPoint, bool bValue)
      {
        if(&InPoint == Points[0])
          DelaunayEdge[1] = bValue;
        else if(&InPoint == Points[1])
          DelaunayEdge[2] = bValue;
        else
          DelaunayEdge[0] = bValue;
      }

      // The neighbor across to given point
      TSharedRef<FTriangle> FTriangle::GetNeighborAcross(TSharedRef<FPoint> InOppositePoint)
      {
        if(&InOppositePoint == Points[0])
          return *Neighbors[0];
        else if(&InOppositePoint == Points[1])
          return *Neighbors[1];

        return *Neighbors[2];
      }
    }
}