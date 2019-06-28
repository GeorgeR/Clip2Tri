#pragma

#include "CoreMinimal.h"
#include "StaticArray.h"

namespace Poly2Tri
{
	struct FEdge;

	struct FPoint : TSharedFromThis<FPoint>
	{
	public:
		double X;
		double Y;

		/// The edges this point constitutes an upper ending point
		TArray<TSharedPtr<FEdge>> Edges;

		/// Default constructor does nothing (for performance).
		FPoint() = default;

		/// Construct using coordinates.
		FPoint(const double& InX, const double& InY)
			: X(InX),
			Y(InY) { }

		/// Set this point to all zeros.
		void SetZero()
		{
			X = 0.0;
			Y = 0.0;
		}

		/// Set this point to some specified coordinates.
		void Set(const double InX, const double InY)
		{
			X = InX;
			Y = InY;
		}

		/// Negate this point.
		FPoint operator-() const
		{
			FPoint Result;
			Result.Set(-X, -Y);
			return Result;
		}

		/// Add a point to this point.
		void operator +=(const FPoint& InOther)
		{
			X += InOther.X;
			Y += InOther.Y;
		}

		friend FPoint operator+(const FPoint& InLeft, const FPoint& InRight)
		{
			return FPoint(InLeft.X + InRight.X, InLeft.Y + InRight.Y);
		}

		/// Subtract a point from this point.
		void operator -=(const FPoint& InOther)
		{
			X -= InOther.X;
			Y -= InOther.Y;
		}

		friend FPoint operator-(const FPoint& InLeft, const FPoint& InRight)
		{
			return FPoint(InLeft.X - InRight.X, InLeft.Y - InRight.Y);
		}

		/// Multiply this point by a scalar.
		void operator *=(const double InValue)
		{
			X *= InValue;
			Y *= InValue;
		}

		friend FPoint operator *(const FPoint& InPoint, const double InValue)
		{
			return FPoint(InPoint.X * InValue, InPoint.Y * InValue);
		}

        friend bool operator==(const FPoint& InLeft, const FPoint& InRight)
        {
			return InLeft.X == InRight.X && InLeft.Y == InRight.Y;
		}

		friend bool operator!=(const FPoint& InLeft, const FPoint& InRight)
		{
			return !(InLeft.X == InRight.X) && !(InLeft.Y == InRight.Y);
		}

		/// Peform the dot product on two vectors.
		static double DotProduct(const FPoint& InLeft, const FPoint& InRight)
		{
			return InLeft.X * InRight.X + InLeft.Y * InRight.Y;
		}

		/// Perform the cross product on two vectors. In 2D this produces a scalar.
		static double CrossProduct(const FPoint& InLeft, const FPoint& InRight)
		{
			return InLeft.X * InRight.X - InLeft.Y * InRight.Y;
		}

		/// Perform the cross product on a point and a scalar. In 2D this produces
		/// a point.
		static FPoint CrossProduct(const FPoint& InPoint, const double InValue)
		{
			return FPoint(InPoint.Y * InValue, InPoint.X * -InValue);
		}

		/// Perform the cross product on a scalar and a point. In 2D this produces
		/// a point.
		static FPoint CrossProduct(const double InValue, const FPoint& InPoint)
		{
			return FPoint(-InValue * InPoint.Y, InValue * InPoint.X);
		}

		/// Get the length of this point (the norm).
		double Length() const
		{
			return FMath::Sqrt(X * X + Y * Y);
		}

		/// Convert this point into a unit point. Returns the Length.
		double Normalize()
		{
            const auto Length = this->Length();
			X /= Length;
			Y /= Length;
			return Length;
		}

		friend bool operator<(const FPoint* InLeft, const FPoint* InRight)
		{
			if (InLeft->Y < InRight->Y)
				return true;
			else if (InLeft->Y == InRight->Y)
			{
				if (InLeft->X < InRight->X)
					return true;
			}
			return false;
		}
	};

	// Represents a simple polygon's edge
	struct FEdge : TSharedFromThis<FEdge>
	{
	public:
		TSharedPtr<FPoint> Start;
		TSharedPtr<FPoint> End;

		FEdge(const TSharedRef<FPoint> InStart, const TSharedRef<FPoint> InEnd)
			: Start(InStart),
			End(InEnd)
		{
			if (InStart->Y > InEnd->Y)
			{
				End = InStart;
				Start = InEnd;
			}
			else if (InStart->Y == InEnd->Y)
			{
				if (InStart->X > InEnd->X)
				{
					End = InStart;
					Start = InEnd;
				}
				else if (InStart->X == InEnd->X)
				{
					// Repeat points
					checkNoEntry();
				}
			}

            // in stl this is push_back
			End->Edges.Emplace(this->AsShared());
		}
	};

	class FTriangle : TSharedFromThis<FTriangle>
	{
	public:
		TStaticArray<bool, 3> ConstrainedEdge;
		TStaticArray<bool, 3> DelaunayEdge;

		FTriangle(const TSharedRef<FPoint> InPointA, const TSharedRef<FPoint> InPointB, const TSharedRef<FPoint> InPointC);

		TSharedPtr<FPoint> GetPoint(const int32& InIndex) { return Points[InIndex]; }
		TSharedPtr<FPoint> GetPointClockwise(const TSharedRef<FPoint> InPoint);
		TSharedPtr<FPoint> GetPointCounterClockwise(const TSharedRef<FPoint> InPoint);
		TSharedPtr<FPoint> GetOppositePoint(const TSharedRef<FTriangle> InTriangle, const TSharedRef<FPoint> InPoint);

		TSharedPtr<FTriangle> GetNeighbor(const int32& InIndex) { return Neighbors[InIndex]; }
		void MarkNeighbour(TSharedPtr<FPoint> InPointA, TSharedPtr<FPoint> InPointB, TSharedPtr<FTriangle> InTriangle);
		void MarkNeighbour(FTriangle& InTriangle);

		void MarkConstrainedEdge(const int32& InIndex);
		void MarkConstrainedEdge(const TSharedRef<FEdge> InEdge);
		void MarkConstrainedEdge(TSharedPtr<FPoint> InP, TSharedPtr<FPoint> InQ);

		int32 GetIndex(const TSharedPtr<FPoint> InPoint);
		int32 GetEdgeIndex(const TSharedPtr<FPoint> InPoint1, const TSharedPtr<FPoint> InPoint2);

		TSharedPtr<FTriangle> GetNeighborClockwise(const TSharedRef<FPoint> InPoint);
		TSharedPtr<FTriangle> GetNeighborCounterClockwise(const TSharedRef<FPoint> InPoint);

		bool GetConstrainedEdgeClockwise(TSharedRef<const FPoint> InPoint);
		void SetConstrainedEdgeClockwise(TSharedRef<FPoint> InPoint, bool bIsConstrained);

		bool GetConstrainedEdgeCounterClockwise(const TSharedRef<FPoint> InPoint);
		void SetConstrainedEdgeCounterClockwise(TSharedRef<FPoint> InPoint, bool bIsConstrained);

		bool GetDelaunayEdgeClockwise(const TSharedRef<FPoint> InPoint);
		void SetDelaunayEdgeClockwise(TSharedRef<FPoint> InPoint, bool bValue);

		bool GetDelaunayEdgeCounterClockwise(const FPoint& InPoint);
		void SetDelaunayEdgeCounterClockwise(FPoint& InPoint, bool bValue);

		bool Contains(const TSharedPtr<FPoint> InPoint) { return InPoint == Points[0] || InPoint == Points[1] || InPoint == Points[2]; }
		bool Contains(const TSharedRef<FEdge> InEdge) { return Contains(InEdge->Start) && Contains(InEdge->End); }
		bool Contains(const TSharedPtr<FPoint> InPointStart, const TSharedPtr<FPoint> InPointEnd) { return Contains(InPointStart) && Contains(InPointEnd); }

		void Legalize(TSharedRef<FPoint> InPoint);
		void Legalize(TSharedRef<FPoint> InPointO, TSharedRef<FPoint> InPointN);

		/**Clears all references to all other triangles and points */
		void Clear();
		void ClearNeighbor(TSharedPtr<FTriangle> InTriangle);
		void ClearNeighbors();
		void ClearDelaunayEdges();

		inline bool GetIsInterior() const { return bIsInterior; }
		inline void SetIsInterior(bool bValue) { bIsInterior = bValue; }

		FTriangle& GetNeighborAcross(TSharedRef<FPoint> InOppositePoint);

	private:
		// Triangle points
		TStaticArray<TSharedPtr<FPoint>, 3> Points;

		// Neighbors
		TStaticArray<TSharedPtr<FTriangle>, 3> Neighbors;

		bool bIsInterior;
	};
}
