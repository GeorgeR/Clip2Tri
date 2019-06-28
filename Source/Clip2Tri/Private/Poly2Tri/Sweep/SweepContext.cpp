#include "Poly2Tri/Sweep/SweepContext.h"

#include "Poly2Tri/Sweep/AdvancingFront.h"

namespace Poly2Tri
{
  namespace Sweep
  {
    FSweepContext::FSweepContext(TArray<FPoint*> InPolyline)
      : Front(nullptr),
      Head(nullptr),
      Tail(nullptr),
      AdvancingFrontHead(nullptr),
      AdvancingFrontMiddle(nullptr),
      AdvancingFrontTail(nullptr),
      Basin(),
      EdgeEvent(),
      Points(InPolyline)
    {
      InitializeEdges(Points);
    }

    virtual FSweepContext::~FSweepContext()
    {
      delete Head;
      delete Tail;
      delete Front;
      delete AdvancingFrontHead;
      delete AdvancingFrontMiddle;
      delete AdvancingFrontTail;

      // @todo proper array delete
      delete Map;
      delete EdgeList;
    }

    FNode& FSweepContext::LocateNode(FPoint& InPoint)
    {
      // TODO implement search tree
      return *Front->LocateNode(InPoint.X);
    }

    void FSweepContext::RemoveNode(FNode* InNode)
    {
      delete InNode;

    }

    void FSweepContext::CreateAdvancingFront(TArray<FNode*> InNodes)
    {
      (void)InNodes; // what's this do?
      
      // Initial triangle
      auto InitialTriangle = new FTriangle(*Points[0], *Tail, *Head);
      Map.PushBack(InitialTriangle);
      
      AdvancingFrontHead = new FNode(*InitialTriangle->GetPoint(1), *InitialTriangle);
      AdvancingFrontMiddle = new FNode(*InitialTriangle->GetPoint(0), *InitialTriangle);
      AdvancingFrontTail = new FNode(*InitialTriangle->GetPoint(2));
      Front = new FAdvancingFront(*AdvancingFrontHead, *AdvancingHeadTail);

      AdvancingFrontHead->Next = AdvancingFrontMiddle;
      AdvancingFrontMiddle->Next = AdvancingFrontTail;
      AdvancingFrontMiddle->Previous = AdvancingFrontHead;
      AdvancingFrontTail->Previous = AdvancingFrontMiddle;
    }

    void FSweepContext::MapTriangleToNodes(FTriangle& InTriangle)
    {
      for(auto i = 0; i < 3; i++)
      {
        if(!InTriangle.GetNeighbor(i))
        {
          auto Node = Front->LocatePoint(InTriangle->GetPointClockwise(*InTriangle->GetPoint(i)));
          if(Node)
          Node->Triangle = &InTriangle;
        }
      }
    }

    void FSweepContext::AddToMap(FTriangle* InTriangle)
    {
      Map.PushBack(InTriangle);
    }

    void FSweepContext::RemoveFromMap(FTriangle* InTriangle)
    {
      Map.Remove(InTriangle);
    }

    FPoint* FSweepContext::GetPoint(const int32& InIndex)
    {
      return Points[InIndex];
    }

    void FSweepContext::AddHole(TArray<FPoint*> InPolyline)
    {
      InitializeEdges(InPolyline);
      for(auto i = 0; i < InPolyline.Num(); i++)
        Points.PushBack(InPolyline[i]);
    }

    void FSweepContext::AddPoint(FPoint* InPoint) 
    {
      Points.PushBack(InPoint);
    }

    void FSweepContext::MeshClean(FTriangle& InTriangle)
    {
      TArray<FTriangle*> Triangles;
      Triangles.PushBack(&InTriangle);

      while(!Triangles.Empty())
      {
        auto Triangle = InTriangle.Last();
        Triangles.PopBack();

        if(Triangle != nullptr && !Triangle->IsInterior())
        {
          Triangle->SetIsInterior(true);
          Triangles.PushBack(Triangle);
          for(auto i = 0; i < 3; i++)
          {
            if(!Triangle->ConstrainedEdge[i])
              Triangles.PushBack(Triangle->GetNeighbor(i));
          }
        }
      }
    }

    void FSweepContext::InitializeTriangulation()
    {
      auto MaxX = Points[0]->X;
      auto MinX = Points[0]->X;
      auto MaxY = Points[0]->Y;
      auto MinY = Points[0]->Y;

      for(auto i = 0; i < Points.Num(); i++)
      {
        auto& Point = *Points[i];
        if(Point.X > MaxX) MaxX = Point.X;
        if(Point.X < MinX) MinX = Point.X;
        if(Point.Y > MaxY) MaxY = Point.Y;
        if(Point.Y < MinY) MinY = Point.Y;
      }

      auto DeltaX = KAlpha * (MaxX - MinX);
      auto DeltaY = KAlpha * (MaxY - MinY);
      Head = new FPoint(MaxX + DeltaX, MinY - DeltaY);
      Tail = new FPoint(MinX - DeltaX, MinY - DeltaY);

      // @todo comparer?
      Points.Sort(cmp);
    }

    void FSweepContext::InitializeEdges(TArray<FPoint*> InPolyline)
    {
      auto PointNum = InPolyline.Num();
      for(auto i = 0; i < PointNum; i++)
      {
        auto j = i < PointNum - 1 ? i + 1 : 0;
        EdgeList.PushBack(new FEdge(*InPolyline[i], *InPolyline[j]));
      }
    }
  }
}