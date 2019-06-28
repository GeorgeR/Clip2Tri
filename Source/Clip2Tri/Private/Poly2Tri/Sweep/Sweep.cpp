#include "Poly2Tri/Sweep/Sweep.h"

#include "Poly2Tri/Sweep/SweepContext.h"
#include "Poly2Tri/Sweep/AdvancingFront.h"
#include "Poly2Tri/Common/Utils.h"

namespace Poly2Tri
{
    FSweep::~FSweep()
    {
      // @todo delete nodes
    }

    // Triangulate simple polygon with holes
    void FSweep::Triangulate(TSharedRef<FSweepContext> InContext)
    {
      InContext.InitializeTriangulation();
      InContext.CreateAdvancingFront(Nodes);

      // FSweep points; build mesh
      FSweepPoints(InContext);
      
      // Clean up
      FinalizationPolygon(InContext);
    }

    void FSweep::SweepPoints(TSharedRef<FSweepContext> InContext)
    {
      for(auto i = 1; i < InContext.GetPointNum(); i++)
      {
        auto& Point = *InContext.GetPoint(i);
        auto Node = &PointEvent(InContext, Point);
        for(auto j = 0; j < Point.EdgeList.Num(); j++)
          EdgeEvent(InContext, Point.EdgeList[j], Node);
      }
    }

    FNode& FSweep::PointEvent(TSharedRef<FSweepContext> InContext, TSharedRef<FPoint> InPoint)
    {
      auto& Node = InContext.LocatePoint(InPoint);
      auto& NewNode = NewFrontTriangle(InContext, InPoint, Node);

      // Only need to check +epsilon since point never have smaller
      // x value than node due to how we fetch nodes from the front
      if(InPoint.X <= Node.Point->X + EPSILON)
        Fill(InContext, Node);

      FillAdvancingFront(InContext, NewNode);
      return NewNode;
    }

    void FSweep::EdgeEvent(TSharedRef<FSweepContext> InContext, TSharedPtr<FEdge> InEdge, TSharedPtr<FNode> InNode)
    {
      InContext.EdgeEvent.ConstrainedEdge = InEdge;
      InContext.EdgeEvent.Right = (InEdge->Start->X > InEdge->End->X);

      if(IsEdgeSideOfTriangle(*InNode->Triangle, *InEdge->Start, *InEdge->End))
        return;

      // For now we will do all needed filling
      // TODO: integrate with flip process might give some better performance
      //       but for now this avoid the issue with cases that needs both flips and fills
      FillEdgeEvent(InContext, InEdge, InNode);
      EdgeEvent(InContext, *InEdge->Start, *InEdge->End, InNode->Triangle, *InEdge->End);
    }

    void FSweep::EdgeEvent(TSharedRef<FSweepContext> InContext, TSharedRef<FPoint> InEdgePointStart, TSharedRef<FPoint> InEdgePointEnd, FTriangle* InTriangle, TSharedRef<FPoint> InPoint)
    {
      if(IsEdgeSideOfTriangle(*InTriangle, InEdgePointStart, InEdgePointEnd))
        return;

      auto Point1 = InTriangle->GetPointCCW(InPoint);
      auto Orientation1 = Orient2D(InEdgePointEnd, *Point1, InEdgePointStart);
      if(Orientation1 == EEdgeOrientation::Collinear)
      {
        if(InTriangle->Contains(&InEdgePointEnd, Point1))
        {
          InTriangle->MarkConstrainedEdge(&InEdgePointEnd, Point1);

          // We are modifying the constraint maybe it would be better to 
          // not change the given constraint and just keep a variable for the new constraint
          InContext.EdgeEvent.ConstrainedEdge->End = Point1;
          InTriangle = &InTriangle->GetNeigbhorAcross(InPoint);
          EdgeEvent(InContext, InEdgePointStart, *Point1, InTriangle, *Point1);
        }
        else
        {
          // todo: assert -  std::runtime_error("EdgeEvent - collinear points not supported");
        }
        return;
      }

      auto Point2 = InTriangle->GetPointCW(InPoint);
      auto Orientation2 = Orient2D(InEdgePointEnd, *Point2, InEdgePointStart);
      if(Orientation2 == EEdgeOrientation::Collinear)
      {
        if(InTriangle->Contains(&InEdgePointEnd, Point2))
        {
          InTriangle->MarkConstrainedEdge(&InEdgePointEnd, Point);

          // We are modifying the constraint maybe it would be better to 
          // not change the given constraint and just keep a variable for the new constraint
          InContext.EdgeEvent.ConstrainedEdge->End = Point2;
          InTriangle = &InTriangle->GetNeigbhorAccess(InPoint);
          EdgeEvent(InContext, InEdgePointStart, *Point2, InTriangle, *Point2);
        }
        else
        {
          // todo: assert -  std::runtime_error("EdgeEvent - collinear points not supported");
        }
        return;
      }

      if(Orientation1 == Orientation2)
      {
        // Need to decide if we are rotating CW or CCW to get to a triangle
        // that will cross edge
        if(Orientation1 == EEdgeOrientation::Clockwise)
          InTriangle = InTriangle->GetNeighborCounterClockwise(InPoint);
        else
          InTriangle = InTriangle->GetNeighborClockwise(InPoint);
        EdgeEvent(InConteext, InEdgePointStart, InEdgePointEnd, InTriangle, InPoint);
      }
      else
        // This triangle crosses constraint so lets flippin start!
        FlipEdgeEvent(InContext, InEdgePointStart, InEdgePointEnd, InTriangle, InPoint);
    }

    FNode& FSweep::NewFrontTriangle(TSharedRef<FSweepContext> InContext, TSharedRef<FPoint> InPoint, FNode& InNode)
    {
      auto Triangle = new FTriangle(InPoint, *InNode.Point, *InNode.Next->Point);
      
      Triangle->MarkNeighbor(*InNode.Triangle);
      InContext.AddToMap(Triangle);

      auto NewNode = new FNode(InPoint);
      Nodes.PushBack(NewNode);

      NewNode->Next = InNode.Next;
      NewNode->Previous = &InNode;
      InNode.Next->Previous = NewNode;
      InNode.Next = NewNode;

      if(!Legalize(InContext, *Triangle))
        InContext.MapTriangleToNodes(*Triangle);

      return *NewNode;
    }

    void FSweep::Fill(TSharedRef<FSweepContext> InContext, FNode& InNode)
    {
      auto Triangle = new FTriangle(*InNode.Previous->Point, *InNode.Point, *InNode.Next->Point);

      Triangle->MarkNeighbor(*InNode.Previous->Triangle);
      Triangle->MarkNeighbor(*InNode.Triangle);

      InContext.AddToMap(Triangle);
 
      // Update the advancing front
      InNode.Previous->Next = InNode.Next;
      InNode.Next->Previous = InNode.Previous;

      // If it was legalized the triangle has already been mapped
      if (!Legalize(InContext, *Triangle))
        InContext.MapTriangleToNodes(*Triangle);
    }

    bool FSweep::Legalize(TSharedRef<FSweepContext> InContext, FTriangle& InTriangle)
    {
      // To legalize a triangle we start by finding if any of the three edges
      // violate the Delaunay condition
      for (auto i = 0; i < 3; i++) 
      {
        if(InTriangle.DelaunayEdge[i])
          continue;
        
        auto OppositeTriangle = InTriangle.GetNeighbor(i);
        if(OppositeTriangle)
        {
          auto Point = InTriangle.GetPoint(i);
          auto OppositePoint = OppositeTriangle->GetOppositePoint(InTriangle, *Point);
          auto OppositeIndex = OppositeTriangle->GetIndex(OppositePoint);

          // If this is a Constrained Edge or a Delaunay Edge(only during recursive legalization)
          // then we should not try to legalize
          if(OppositeTriangle->GetConstrainedEdge[OppositeIndex] || OppositeTriangle->GetDelaunayEdge[OppositeIndex])
          {
            InTriangle.SetConstrainedEdge[i] = OppositeTriangle->GetConstrainedEdge[OppositeIndex];
            continue;
          }

          bool bIsInside = Incircle(*Point, *InTriangle.GetPointCCW(*Point), *InTriangle.GetPointCW(*Point), *OppositePoint);
          if(bIsInside)
          {
            // Lets mark this shared edge as Delaunay
            InTriangle->SetIsDelaunayEdge[i] = true;
            OppositeTriangle->SetIsDelaunayEdge[OppositeIndex] = true;

            // Lets rotate shared edge one vertex CW to legalize it
            RotateTrianglePair(InTriangle, *Point, *OppositeTriangle, *OppositePoint);

            // We now got one valid Delaunay Edge shared by two triangles
            // This gives us 4 new edges to check for Delaunay

            // Make sure that triangle to node mapping is done only one time for a specific triangle
            bool bIsNotLegalized = !Legalize(InContext, InTriangle);
            if(bIsNotLegalized)
              InContext.MapTriangleToNodes(InTriangle);
            
            bIsNotLegalized = !Legalize(InContext, *OppositeTriangle);
            if(bIsNotLegalized)
              InContext.MapTriangleToNode(*OppositeTriangle);

            // Reset the Delaunay edges, since they only are valid Delaunay edges
            // until we add a new triangle or point.
            // XXX: need to think about this. Can these edges be tried after we
            //      return to previous recursive level?
            InTriangle.SetIsDelaunayEdge[i] = false;
            OppositeTriangle->SetIsDelaunayEdge[OppositeIndex] = false;

            // If triangle have been legalized no need to check the other edges since
            // the recursive legalization will handles those so we can end here.
            return true;
          }
        }
      }

      return false;
    }

    bool FSweep::Incircle(TSharedRef<FPoint> InPointA, TSharedRef<FPoint> InPointB, TSharedRef<FPoint> InPointC, TSharedRef<FPoint> InPointD)
    {
      auto ADX = InPointA.X - InPointD.X;
      auto ADY = InPointA.Y - InPointD.Y;
      auto BDX = InPointB.X - InPointD.X;
      auto BDY = InPointB.Y - InPointD.Y;

      auto ADX_BDY = ADX * BDY;
      auto BDX_ADY = BDX * ADY;
      auto OABD = ADX_BDY - BDX_ADY;

      if(OABD <= 0.0)
        return false;

      auto CDX = InPointC.X - InPointD.X;
      auto CDY = InPointC.Y - InPointD.Y;

      auto CDX_ADY = CDX * ADY;
      auto ADX_CDY = ADX * CDY;
      auto OCAD = CDX_ADY - ADX_CDY;

      if(OCAD <= 0.0)
        return false;

      auto BDX_CDY = BDX * CDY:
      auto CDX_BDY = CDX * BDY;

      auto LiftA = ADX * ADX + ADY * ADY;
      auto LiftB = BDX * BDX + BDY * BDY;
      auto LiftC = CDX * CDX + CDY * CDY;

      auto Determinant = LiftA * (BDX_CDY - CDX_BDY) + LiftB * OCAD + LiftC * OABD;

      return Determinant > 0.0;
    }

    void FSweep::RotateTrianglePair(FTriangle& InTriangle, TSharedRef<FPoint> InPoint, FTriangle& InOppositeTriangle, TSharedRef<FPoint> InOppositePoint)
    {
      auto Neighbor1 = InTriangle.GetNeighborCCW(InPoint);
      auto Neighbor2 = InTriangle.GetNeighborCW(InPoint);
      auto Neighbor3 = InOppositeTriangle.GetNeighborCCW(InOppositePoint);
      auto Neighbor4 = InOppositeTriangle.GetNeighborCW(InOppositePoint);
      
      auto ConstrainedEdge1 = InTriangle.GetConstrainedEdgeCCW(InPoint);
      auto ConstrainedEdge2 = InTriangle.GetConstrainedEdgeCW(InPoint);
      auto ConstrainedEdge3 = InOppositeTriangle.GetConstrainedEdgeCCW(InOppositePoint);
      auto ConstrainedEdge4 = InOppositeTriangle.GetConstrainedEdgeCW(InOppositePoint);

      auto DelaunayEdge1 = InTriangle.GetDelauanyEdgeCCW(InPoint);
      auto DelaunayEdge2 = InTriangle.GetDelauanyEdgeCW(InPoint);
      auto DelaunayEdge3 = InOppositeTriangle.GetDelauanyEdgeCCW(InOppositePoint);
      auto DelaunayEdge4 = InOppositeTriangle.GetDelauanyEdgeCW(InOppositePoint);

      InTriangle.Legalize(InPoint, InOppositePoint);
      InOppositeTriangle.Legalize(InOppositePoint, InPoint);
      
      // Remap delaunay_edge
      InOppositeTriangle.SetDelunayEdgeCCW(InPoint, DelaunayEdge1);
      InTriangle.SetDelunayEdgeCW(InPoint, DelaunayEdge12;
      InTriangle.SetDelunayEdgeCCW(InOppositePoint, DelaunayEdge3);
      InOppositeTriangle.SetDelunayEdgeCW(InOppositePoint, DelaunayEdge4);

      // Remap constrained_edge
      InOppositeTriangle.SetConstrainedEdgeCCW(InPoint, ConstrainedEdge1);
      InTriangle.SetConstrainedEdgeCW(InPoint, ConstrainedEdge2);
      InTriangle.SetConstrainedEdgeCCW(InOppositePoint, ConstrainedEdge3);
      InOppositeTriangle.SetConstrainedEdgeCW(InOppositePoint, ConstrainedEdge4);

      // Remap neighbors
      // XXX: might optimize the markNeighbor by keeping track of
      //      what side should be assigned to what neighbor after the
      //      rotation. Now mark neighbor does lots of testing to find
      //      the right side.
      InTriangle.ClearNeighbors();
      InOppositeTriangle.ClearNeighbors();

      if (Neighbor1) InOppositeTriangle.MarkNeighbor(*Neighbor1);
      if (Neighbor2) InTriangle.MarkNeighbor(*Neighbor2);
      if (Neighbor3) InTriangle.MarkNeighbor(*Neighbor3);
      if (Neighbor4) InOppositeTriangle.MarkNeighbor(*Neighbor4);
      InTriangle.MarkNeighbor(InOppositeTriangle);
    }

    void FSweep::FillAdvancingFront(TSharedRef<FSweepContext> InContext, FNode& InNode)
    {
      // Fill right holes
      auto Node = InNode.Next;
      while(Node->Next)
      {
        // if HoleAngle exceeds 90 degrees then break.
        if(LargeHole_DontFill(Node))
          break;

        Fill(InContext, *Node);
        Node = Node->Next;
      }

      // Fill left holes
      Node = Node.Previous;
      while (Node->Previous) 
      {
        // if HoleAngle exceeds 90 degrees then break.
        if (LargeHole_DontFill(Node)) 
          break;
        
        Fill(InContext, *Node);
        Node = Node->Previous;
      }

      // Fill right basins
      if(InNode.Next && InNode.Next->Next)
      {
        auto Angle = BasinAngle(InNode);
        if(Angle < PI_3Over4)
          FillBasin(InContext, InNode);
        }
      }
    }

    // True if HoleAngle exceeds 90 degrees.
    bool FSweep::LargeHole_DontFill(TSharedPtr<FNode> InNode)
    {
      auto NextNode = InNode->Next;
      auto PreviousNode = InNode->Previous;
      if(!AngleExceeds90Degrees(InNode->Point, NextNode->Point, PreviousNode->Point))
        return false
      
      // Check additional points on front.
      auto Next2Node = NextNode->Next;
      // "..Plus.." because only want angles on same side as point being added.
      if((Next2Node != nullptr) && !AngleExceedsPlus90DegreesOrIsNegative(InNode->Point, Next2Node->Point, PreviousNode->Point))
        return false;

      auto Previous2Node = PreviousNode->Previous;
      // "..Plus.." because only want angles on same side as point being added.
      if((Previous2Node != nullptr) && !AngleExceedsPlus90DegreesOrIsNegative(InNode->Point, NextNode->Point, Previous2Node->Point))
        return false;

      return true;
    }

    bool FSweep::AngleExceeds90Degrees(FPoint* InOrigin, FPoint* InPointA, FPoint* InPointB)
    {
      auto Angle = Angle(*InOrigin, *InPointA, *InPointB);
      auto bExceeds90Degrees = ((Angle > PIOver2) || (Angle <= -PIOver2));
      return bExceeds90Degrees;
    }

    bool FSweep::AngleExceedsPlus90DegreesOrIsNegative(FPoint* InOrigin, FPoint* InPointA, FPoint* InPointB)
    {
      auto Angle = Angle(*InOrigin, *InPointA, *InPointB);
      auto bExceedsPlus90DegreesOrIsNegative = (Angle > PIOver2) || (Angle < 0.0);
      return bExceedsPlus90DegreesOrIsNegative;
    }

    double FSweep::Angle(TSharedRef<FPoint> InOrigin, TSharedRef<FPoint> InPointA, TSharedRef<FPoint> InPointB) 
    {
      /* Complex plane
      * ab = cosA +i*sinA
      * ab = (ax + ay*i)(bx + by*i) = (ax*bx + ay*by) + i(ax*by-ay*bx)
      * atan2(y,x) computes the principal value of the argument function
      * applied to the complex number x+iy
      * Where x = ax*bx + ay*by
      *       y = ax*by - ay*bx
      */
      auto OriginX = InOrigin.X;
      auto OriginY = InOrigin.Y:

      auto PointAX = InPointA.X - OriginX;
      auto PointAY = InPointA.Y - OriginY;

      auto PointBX = InPointB.X - OriginX;
      auto PointBY = InPointB.Y - OriginY;

      auto X = PointAX * PointBY - PointAY * PointBX;
      auto Y + PointAX * PointBX + PointAY * PointBY;

      auto Angle = FMath::Atan2(X, Y);

      return Angle;
    }

    double FSweep::HoleAngle(FNode& InNode)
    {
      /* Complex plane
      * ab = cosA +i*sinA
      * ab = (ax + ay*i)(bx + by*i) = (ax*bx + ay*by) + i(ax*by-ay*bx)
      * atan2(y,x) computes the principal value of the argument function
      * applied to the complex number x+iy
      * Where x = ax*bx + ay*by
      *       y = ax*by - ay*bx
      */
      auto AX = InNode.Next->Point->X - InNode.Point->X;
      auto AY = InNode.Next->Point->Y - InNode.Point->Y;

      auto BX = InNode.Previous->Point->X - InNode.Point->X;
      auto BY = InNode.Previous->Point->Y - InNode.Point->Y;

      return FMath::Atan2(AX * BY - AY * BX, AX * BX + AY * BY);
    }

    double FSweep::BasinAngle(FNode& InNode)
    {
      auto AX = InNode.Point->X - InNode.Next->Next->Point->X;
      auto BY = InNode.Point->Y - InNode.Next->Next->Point->Y;
      return FMath::Atan2(AY, AX);
    }

    void FSweep::FillBasin(TSharedRef<FSweepContext> InContext, FNode& InNode)
    {
      if(Orient2D(*InNode.Point, *InNode.Next->Point, *InNode.Next->Next->Point) == EEdgeOrientation::CCW)
        InContext.Basin.LeftNode = InNode.Next->Next;
      else
        InContext.Basin.LeftNode = InNode.Next;
      
      // Find the bottom and right node
      InContext.Basin.BottomNode = InContext.Basin.LeftNode;
      while(InContext.Basin.BottomNode->Next && InContext.Basin.BottomNode->Point->Y >= InContext.Basin.BottomNode->Next->Point->Y)
        InContext.Basin.BottomNode = InContext.Basin.BottomNode->Next;
      
      if(InContext.Basin.BottomNode == InContext.Basin.LeftNode)
        // No valid basin
        return;

      InContext.Basin.RightNode = InContext.Basin.BottomNode;
      while(InContext.Basin.RightNode->Next && InContext.Basin.RightNode->Point->Y < InContext.Basin.RightNode->Next->Point->Y)
        InContext.Basin.RightNode = InContext.Basin.RightNode->Next;

      if(InContext.Basin.RightNode == InContext.Basin.BottomNode)
        // No valid basins
        return;

      InContext.Basin.Width = InContext.Basin.RightNode->Point->X - InContext.Basin.LeftNode->Point->X;
      InContext.Basin.LeftHighest = InContext.Basin.LeftNode->Point->Y > InContext.Basin.RightNode->Point->Y;
      
      FillBasinReq(InContext, InContext.Basin.BottomNode);
    }

    void FSweep::FillBasinReq(TSharedRef<FSweepContext> InContext, TSharedPtr<FNode> InNode)
    {
      // if shallow stop filling
      if(IsShallow(InContext, *InNode))
        return;
      
      Fill(InContext, *InNode)

      if(InNode->Previous == InContext.Basin.LeftNode && InNode->Next == InContext.Basin.RightNode)
        return;
      else if(InNode->Previous == InContext.Basin.LeftNode)
      {
        auto Orientation = Orient2D(*InNode->Point, *InNode->Next->Point, *InNode->Next->Next->Point);
        if(Orientation == EEdgeOrientation::CW)
          return;
        
        InNode = InNode->Next;
      } 
      else if(InNode->Next == InContext.Basin.RightNode) 
      {
        auto Orientation = Orient2D(*InNode->Point, *InNode->Previous->Point, *InNode->Previous->Previous->Point);
        if(Orientation == EEdgeOrientation::CCW)
          return;
        
        InNode = InNode->Previous;
      } 
      else 
      {
        // Continue with the neighbor node with lowest Y value
        if(InNode->Previous->Point->Y < InNode->Next->Point->Y)
          InNode = InNode->Previous;
        else
          InNode = InNode->Next;
      }

      FillBasinReq(InContext, InNode);
    }

    bool FSweep::IsShallow(TSharedRef<FSweepContext> InContext, FNode& InNode)
    {
      auto Height = 0.0;
      if(InContext.Basin.LeftHighest)
        Height = InContext.Basin.LeftNode->Point->Y - InNode.Point->Y;
      else
        Height = InContext.Basin.RightNode->Point->Y - InNode.Point->Y;

      // if shallow stop filling
      if(InContext.Basin.Width > Height)
        return true;
      
      return false;
    }

    bool FSweep::IsEdgeSideOfTriangle(FTriangle& InTriangle, TSharedRef<FPoint> InEdgePointStart, TSharedRef<FPoint> InEdgePointEnd)
    {
      auto EdgeIndex = InTriangle.GetEdgeIndex(&InEdgePointStart, &InEdgePointEnd);
      if(EdgeIndex != -1)
      {
        InTriangle.MarkConstrainedEdge(EdgeIndex);
        auto Triangle = InTriangle.GetNeighbor(EdgeIndex);
        if(Triangle)
          Triangle->MarkConstrainedEdge(&InEdgePointStart, &InEdgePointEnd);
        return true;
      }
      return false;
    }

    void FSweep::FillEdgeEvent(TSharedRef<FSweepContext> InContext, TSharedPtr<FEdge> InEdge, TSharedPtr<FNode> InNode)
    {
      if(InContext.EdgeEvent.Right)
        FillRightAboveEdgeEvent(InContext, InEdge, InNode);
      else
        FillLeftAboveEdgeEvent(InConext, InEdge, InNode);
    }

    void FSweep::FillRightAboveEdgeEvent(TSharedRef<FSweepContext> InContext, TSharedPtr<FEdge> InEdge, TSharedPtr<FNode> InNode)
    {
      while(InNode->Next->Point->X < InEdge->Start->X)
      {
        // Check if next node is below the edge
        if(Orient2D(*InEdge->End, *InNode->Next->Point, *InEdge->Start) == EEdgeOrientation::CCW)
          FillRightBelowEdgeEvent(InContext, InEdge, *InNode);
        else
          InNode = InNode->Next;
      }
    }

    void FSweep::FillRightBelowEdgeEvent(TSharedRef<FSweepContext> InContext, TSharedPtr<FEdge> InEdge, FNode& InNode)
    {
      if(InNode.Point->X < InEdge->Start->X)
      {
        if(Orient2D(*InNode.Point, *InNode.Next->Point, *InNode.Next->Next->Point) == EEdgeOrientation::CCW)
        {
          // Concave
          FillRightConcaveEdgeEvent(InContext, InEdge, InNode);
        } 
        else
        {
          // Convex
          FillRightConvexEdgeEvent(InContext, InEdge, InNode);
          // Retry this one
          FillRightBelowEdgeEvent(InContext, InEdge, InNode);
        }
      }
    }

    void FSweep::FillRightConcaveEdgeEvent(TSharedRef<FSweepContext> InContext, TSharedPtr<FEdge> InEdge, FNode& InNode)
    {
      Fill(InContext, *InNode.Next);
      if(InNode.Next->Point != InEdge->Start)
      {
        // Next above or below edge?
        if(Orient2D(*InEdge->End, *InNode.Next->Point, *InEdge->Start) == EEdgeOrientation::CCW)
        {
          // Below
          if (Orient2D(*InNode.Point, *InNode.Next->Point, *InNode.Next->Next->Point) == EEdgeOrientation::CCW)
            // Next is concave
            FillRightConcaveEdgeEvent(InContext, InEdge, InNode);
        }
      }
    }

    void FSweep::FillRightConvexEdgeEvent(TSharedRef<FSweepContext> InContext, TSharedPtr<FEdge> InEdge, TSharedRef<FNode> InNode)
    {
      // Next concave or convex?
      if (Orient2D(*InNode.Next->Point, *InNode.Next->Next->Point, *InNode.Next->Next->Next->Point) == EEdgeOrientation::CCW)
        // Concave
        FillRightConcaveEdgeEvent(InContext, InEdge, *InNode.next);
      else
      {
        // Convex
        // Next above or below edge?
        if (Orient2d(*InEdge->End, *InNode.Next->Next->Point, *InEdge->Start) == EEdgeOrientation::CCW)
          // Below
          FillRightConvexEdgeEvent(InContext, InEdge, *InNode.Next);
      }
    }

    void FSweep::FillLeftAboveEdgeEvent(TSharedRef<FSweepContext> InContext, TSharedPtr<FEdge> InEdge, TSharedPtr<FNode> InNode)
    {
      while (InNode->Previous->Point->X > InEdge->Start->X) 
      {
        // Check if next node is below the edge
        if (Orient2D(*InEdge->End, *InNode->Previous->Point, *InEdge->Start) == EEdgeOrientation::CW)
          FillLeftBelowEdgeEvent(InContext, InEdge, *InNode);
        else
          InNode = InNode->Previous;
      }
    }

    void FSweep::FillLeftBelowEdgeEvent(TSharedRef<FSweepContext> InContext, TSharedPtr<FEdge> InEdge, FNode& InNode)
    {
      if (InNode.Point->X > InEdge->Start->X) {
        if (Orient2D(*InNode.Point, *InNode.Previous->Point, *InNode.Previous->Previous->Point) == EEdgeOrientation::CW)
          // Concave
          FillLeftConcaveEdgeEvent(InContext, InEdge, InNode);
        else
        {
          // Convex
          FillLeftConvexEdgeEvent(InContext, InEdge, InNode);
          // Retry this one
          FillLeftBelowEdgeEvent(InContext, InEdge, InNode);
        }
      }
    }

    void FSweep::FillLeftConvexEdgeEvent(TSharedRef<FSweepContext> InContext, TSharedPtr<FEdge> InEdge, FNode& InNode)
    {
      // Next concave or convex?
      if (Orient2d(*InNode.Previous->Point, *InNode.Previous->Previous->Point, *InNode.Previous->Previous->Previous->Point) == EEdgeOrientation::CW)
        // Concave
        FillLeftConcaveEdgeEvent(InContext, InEdge, *InNode.Previous);
      else
      {
        // Convex
        // Next above or below edge?
        if (Orient2d(*InEdge->End, *InNode.Previous->Previous->Point, *InEdge->Start) == EEdgeOrientation::CW)
          // Below
          FillLeftConvexEdgeEvent(InContext, InEdge, *InNode.Previous);
      }
    }

    void FSweep::FillLeftConcaveEdgeEvent(TSharedRef<FSweepContext> InContext, TSharedPtr<FEdge> InEdge, FNode& InNode)
    {
      Fill(InContext, *InNode.Previous);
      if (InNode.Previous->Point != InEdge->Start) 
      {
        // Next above or below edge?
        if (Orient2D(*InEdge->End, *InNode.Previous->Point, *InEdge->Start) == EEdgeOrientation::CW) 
        {
          // Below
          if (Orient2D(*InNode.Point, *InNode.Previous->Point, *InNode.Previous->Previous->Point) == EEdgeOrientation::CW)
            // Next is concave
            FillLeftConcaveEdgeEvent(InContext, InEdge, InNode);
        }
      }
    }

    void FSweep::FlipEdgeEvent(TSharedRef<FSweepContext> InContext, TSharedRef<FPoint> InEdgePointStart, TSharedRef<FPoint> InEdgePointEnd, FTriangle* InTriangle, TSharedRef<FPoint> InPoint)
    {
      auto& OppositeTriangle = InTriangle->GetNeighborAcross(InPoint);
      auto& OppositePoint = *OppositeTriangle.GetOppositePoint(*InTriangle, InPoint);
      
      if(&OppositeTriangle == nullptr)
        // If we want to integrate the fillEdgeEvent do it here
        // With current implementation we should never get here
        //throw new RuntimeException( "[BUG:FIXME] FLIP failed due to missing triangle");
        assert(0);
      
      if(InScanArea(InPoint, *InTriangle->GetPointCCW(InPoint), *InTriangle->GetPointCW(InPoint), OppositePoint))
      {
        // Lets rotate shared edge one vertex CW
        RotateTrianglePair(*InTriangle, InPoint, OppositeTriangle, OppositePoint);
        InContext.MapTriangleToNodes(*InTriangle);
        InContext.MapTriangleToNodes(OppositeTriangle);

        if(InPoint == InEdgePointEnd && OppositePoint == InEdgePointStart)
        {
          if(InEdgePointEnd == *InContext.EdgeEvent.ConstrainedEdge->End && InEdgePointStart == *InContext.EdgeEvent.ConstrainedEdge->Start)
          {
            InTriangle->MarkConstrainedEdge(&InEdgePointStart, &InEdgePointEnd);
            OppositeTriangle.MarkConstrainedEdge(&InEdgePointStart, &InEdgePointEnd);
            Legalize(InContext, *InTriangle);
            Legalize(InContext, OppositeTriangle);
          }
        } 
        else 
        {
          auto Orientation = Orient2D(InEdgePointEnd, OppositePoint, InEdgePointStart);
          InTriangle = &NextFlipTriangle(InContext, Orientation, *InTriangle, OppositeTriangle, InPoint, OppositePoint);
          FlipEdgeEvent(InContext, InEdgePointStart, InEdgePointEnd, InTriangle, InPoint);
        }
      } 
      else 
      {
        auto& NewPoint = NextFlipPoint(InEdgePointStart, InEdgePointEnd, OppositeTriangle, OppositePoint);
        FlipScanEdgeEvent(InContext, InEdgePointStart, InEdgePointEnd, *InTriangle, OppositeTriangle, NewPoint);
        EdgeEvent(InContext, InEdgePointStart, InEdgePointEnd, InTriangle, InPoint);
      }
    }

    FTriangle& FSweep::NextFlipTriangle(TSharedRef<FSweepContext> InContext, EEdgeOrientation InOrientation, FTriangle& InTriangle, FTriangle& InOppositeTriangle, TSharedRef<FPoint> InPoint, TSharedRef<FPoint> InOppositePoint)
    {
      if(InOrientation == EEdgeOrientation::CCW)
      {
        // OppositeTriangle is not crossing edge after flip
        auto EdgeIndex = InOppositeTriangle.GetEdgeIndex(&InPoint, &InOppositePoint);
        OppositeTriangle.SetDelaunayEdge[EdgeIndex] = true;
        Legalize(InContext, OppositeTriangle);
        OppositeTriangle.ClearDelaunayEdges();
        return InTriangle;
      }

      // t is not crossing edge after flip
      auto EdgeIndex = InTriangle.GetEdgeIndex(&InPoint, &InOppositePoint);

      InTriangle.SetDelaunayEdge[EdgeIndex] = true;
      Legalize(InContext, InTriangle);
      InTriangle.ClearDelaunayEdges();
      return InOppositeTriangle;
    }

    Point& FSweep::NextFlipPoint(TSharedRef<FPoint> InEdgePointStart, TSharedRef<FPoint> InEdgePointEnd, FTriangle& InOppositeTriangle, TSharedRef<FPoint> InOppositePoint)
    {
      auto Orientation = Orient2D(InEdgePointEnd, InOppositePoint, InEdgePointStart);
      if(Orientation == EEdgeOrientation::CW)
        // Right
        return *InOppositeTriangle.GetPointCCW(InOppositePoint);
      else if (Orientation == EEdgeOrientation::CCW)
        // Left
        return *InOppositeTriangle.GetPointCW(InOppositePoint);
      else
      {
        //throw new RuntimeException("[Unsupported] Opposing point on constrained edge");
        assert(0);
        return InEdgePointStart;     // Arbitrary return val -- fixes warning
      }
    }

    void FSweep::FlipScanEdgeEvent(TSharedRef<FSweepContext> InContext, 
                                  TSharedRef<FPoint> InEdgePointStart, TSharedRef<FPoint> InEdgePointEnd, 
                                  FTriangle& InFlipTriangle,
                                  FTriangle& InTriangle, 
                                  TSharedRef<FPoint> InPoint)
    {
      auto& OppositeTriangle = InTriangle.GetNeighborAcross(InPoint);
      auto& OppositePoint = *OppositeTriangle.GetOppositePoiont(InTriangle, InPoint);
      if(&InTriangle.GetNeighborAcross(InPoint) == nullptr)
        // If we want to integrate the fillEdgeEvent do it here
        // With current implementation we should never get here
        //throw new RuntimeException( "[BUG:FIXME] FLIP failed due to missing triangle");
        assert(0);

      if(InScanArea(InEdgePointEnd, *InFlipTriangle.GetPointCCW(InEdgePointStart), *InFlipTriangle.GetPointCW(InEdgePointEnd), OppositePoint))
      {
        // flip with new edge op->eq
        FlipEdgeEvent(InContext, InEdgePointEnd, InEdgePointStart, &OppositeTriangle, OppositePoint);
        // TODO: Actually I just figured out that it should be possible to
        //       improve this by getting the next ot and op before the the above
        //       flip and continue the flipScanEdgeEvent here
        // set new ot and op here and loop back to inScanArea test
        // also need to set a new flip_triangle first
        // Turns out at first glance that this is somewhat complicated
        // so it will have to wait.
      } 
      else
      {
        auto& NewPoint = NextFlipPoint(InEdgePointStart, InEdgePointEnd, OppositeTriangle, OppositePoint);
        FlipScanEdgeEvent(InContext, InEdgePointStart, InEdgePointEnd, InFlipTriangle, OppositeTriangle, NewPoint);
      }
    }

    void FSweep::FinalizationPolygon(TSharedRef<FSweepContext> InContext)
    {
      // Get an Internal triangle to start with
      auto Triangle = InContext.GetFront()->GetHead()->Next->Triangle;
      auto Point = InContext.GetFront()->GetHead()->Next->Point;
      while(!Triangle->GetConstrainedEdgeCW(*Point))
        Triangle = Triangle->GetNeighborCCW(*Point);
      
      // Collect interior triangles constrained by edges
      InContext.MeshClean(*Triangle);
    }
}
