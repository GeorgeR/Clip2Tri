#include "Poly2Tri/Sweep/AdvancingFront.h"

namespace Poly2Tri
{
  namespace Sweep
  {
    FAdvancingFront::FAdvancingFront(FNode& InHead, FNode& InTail)
      : Head(&InHead),
      Tail(&InTail) { }

    FNode* FAdvancingFront::LocateNode(const double& InPoint)
    {
      auto Node = SearchNode;

      if(InPoint < Node->Value)
      {
        while((Node = Node->Previous) != nullptr)
        {
          if(InPoint >= Node->Value)
          {
            SearchNode = Node;
            return Node;
          }
        }
      }
      else
      {
        while((Node = Node->Next) != nullptr)
        {
          if(InPoint < Node->Value)
          {
            SearchNode = Node->Previous;
            return Node->Previous;
          }
        }
      }

      return nullptr;
    }

    FNode* FAdvancingFront::LocatePoint(const FPoint* InPoint)
    {
      const auto PointX = InPoint->X;
      auto Node = FindSearchNode(PointX);
      const auto NodePointX = Node->Point->X;

      if(PointX == NodePointX)
      {
        if(InPoint != Node->Point)
        {
          if(InPoint == Node->Previous->Point)
            Node = Node->Previous;
          else if(InPoint == Node->Next->Point)
            Node = Node->Next;
          else
            assert(0); // ensure doesnt hit here
        }
      }
      else if(PointX < NodePointX)
      {
        while((Node = Node->Previous) != nullptr)
        {
          if(InPoint == Node->Point)
            break;
        }
      }
      else 
      {
        while((Node = Node->Next) != nullptr)
        {
          if(InPoint == Node->Point)
            break;
        }
      }

      if(Node)
        SearchNode = Node;
      
      return Node;
    }

    FNode* FAdvancingFront::FindSearchNode(const double& InPoint)
    {
      (void)InPoint;
      return SearchNode;
    }
  }
}