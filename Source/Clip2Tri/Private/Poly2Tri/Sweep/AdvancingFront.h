#pragma once

#include "CoreMinimal.h"

#include "Poly2Tri/Common/Shapes.h"

namespace Poly2Tri
{
  namespace Sweep
  {
    struct FNode
    {
    public:
      FPoint* Point;
      FTriangle* Triangle;
      FNode* Next;
      FNode* Previous;
      double Value;

      FNode(FPoint& InPoint)
        : Point(&InPoint),
        Triangle(nullptr),
        Next(nullptr),
        Previous(nullptr),
        Value(InPoint.X) { }

      FNode(FPoint& InPoint, FTriangle& InTriangle)
        : Point(&InPoint),
        Triangle(&InTriangle),
        Next(nullptr),
        Previous(nullptr),
        Value(InPoint.X) { }
    };

    class FAdvancingFront
    {
    public:
      FAdvancingFront(FNode& InHead, FNode& InTail);
      virtual ~FAdvancingFront() { }

      FNode* GetHead() const { return Head; }
      void SetHead(FNode* InNode) { Head = InNode; }

      FNode* GetTail() const { return Tail; }
      void SetTail(FNode* InNode) { Tail = InNode; }

      FNode* GetSearch() const { return SearchNode; }
      void SetSearch(FNode* InNode) { SearchNode = InNode; }

      /** Locate insertion point along advancing front */
      FNode* LocateNode(const double& InPoint);

      FNode* LocatePoint(const FPoint* InPoint);

    private:
      FNode* Head;
      FNode* Tail;
      FNode* SearchNode;

      FNode* FindSearchNode(const double& InPoint);
    };
  }
}

#ifndef ADVANCED_FRONT_H
#define ADVANCED_FRONT_H

#include "../common/shapes.h"

namespace p2t {

struct Node;

// Advancing front node
struct Node {
  Point* point;
  Triangle* triangle;

  Node* next;
  Node* prev;

  double value;

  Node(Point& p) : point(&p), triangle(NULL), next(NULL), prev(NULL), value(p.x)
  {
  }

  Node(Point& p, Triangle& t) : point(&p), triangle(&t), next(NULL), prev(NULL), value(p.x)
  {
  }

};

// Advancing front
class AdvancingFront {
public:

AdvancingFront(Node& head, Node& tail);
// Destructor
~AdvancingFront();

Node* head();
void set_head(Node* node);
Node* tail();
void set_tail(Node* node);
Node* search();
void set_search(Node* node);

/// Locate insertion point along advancing front
Node* LocateNode(const double& x);

Node* LocatePoint(const Point* point);

private:

Node* head_, *tail_, *search_node_;

Node* FindSearchNode(const double& x);
};

inline Node* AdvancingFront::head()
{
  return head_;
}
inline void AdvancingFront::set_head(Node* node)
{
  head_ = node;
}

inline Node* AdvancingFront::tail()
{
  return tail_;
}
inline void AdvancingFront::set_tail(Node* node)
{
  tail_ = node;
}

inline Node* AdvancingFront::search()
{
  return search_node_;
}

inline void AdvancingFront::set_search(Node* node)
{
  search_node_ = node;
}

}

#endif
