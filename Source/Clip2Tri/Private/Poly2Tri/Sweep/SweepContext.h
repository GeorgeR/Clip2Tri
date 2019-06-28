#pragma once

#include "CoreMinimal.h"

namespace Poly2Tri
{
  namespace Sweep
  {
    const double KAlpha = 0.3;

    class FSweepContext
    {
    public:
      FSweepContext(TArray<FPoint*> InPolyline);
      virtual ~FSweepContext();

      FPoint* GetHead() const { return Head; }
      void SetHead(FPoint* InPoint) { Head = InPoint; }

      FPoint* GetTail() const { return Tail; }
      void SetTail(FPoint* InPoint) { Tail = InPoint; }

      int32 GetPointNum() const { return Points.Num(); }

      FNode& LocateNode(FPoint& InPoint);
      void RemoveNode(FNode* InNode);

      void CreateAdvancingFront(TArray<FNode*> InNodes);

      void MapTriangleToNodes(FTriangle& InTriangle);
      void AddToMap(FTriangle* InTriangle);
      void RemoveFromMap(FTriangle* InTriangle);

      FPoint* GetPoint(const int32& InIndex);
      FPoint* GetPoints();

      void AddHole(TArray<FPoint*> InPolyline);
      void AddPoint(FPoint* InPoint);

      FAdvancingFront* GetAdvancingFront() const { return AdvancingFrontHead; }

      void MeshClean(FTriangle& InTriangle);

      TArray<FTriangle*> GetTriangles() const { return Triangles; }
      TArray<FTriangle*> GetMap() const { return Map; }
      TArray<FEdge*> EdgeList;

      struct FBasin
      {
      public:
        FNode* LeftNode;
        FNode* BottomNode;
        FNode* RightNode;
        double Width;
        bool bLeftHighest;

        FBasin()
          : LeftNode(nullptr),
          BottomNode(nullptr),
          RightNode(nullptr),
          Width(0.0),
          bLeftHighest(false) { }

        void Clear()
        {
          LeftNode = nullptr;
          BottomNode = nullptr;
          RightNode = nullptr;
          Width = 0.0;
          bLeftHighest = false;
        }
      };

      struct FEdgeEvent
      {
      public:
        FEdge* ConstrainedEdge;
        bool bRight;

        FEdgeEvent()
          : ConstrainedEdge(nullptr),
          bRight(false) { }
      };

      FBasin Basin;
      FEdgeEvent EdgeEvent;

    private:
      TArray<FTriangle*> Triangles;
      TArray<FTriangle*> Map;
      TArray<FPoint*> Points;

      FAdvancingFront* AdvancingFront;
      FPoint* Head;
      FPoint* Tail;

      FNode* AdvancingFrontHead;
      FNode* AdvancingFrontMiddle;
      FNode* AdvancingFrontTail;

      void InitializeTriangulation();
      void InitializeEdges(TArray<FPoint*> InPolyline);
    };
  }
}