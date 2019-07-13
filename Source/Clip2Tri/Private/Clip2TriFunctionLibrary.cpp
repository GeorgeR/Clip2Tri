#include "Clip2TriFunctionLibrary.h"

#include "Clip2Tri/Clip2Tri.h"
#include "Set.h"

using namespace c2t;

void UClip2TriFunctionLibrary::Triangulate(const TArray<FVector>& InPoints, TArray<FVector>& OutVertices, TArray<int32>& OutTriangles, TArray<int32>& OutBounds, bool bFlip /*= false*/)
{
	if (InPoints.Num() < 3)
		return;

	vector<vector<Point>> InputPoints;
	vector<Point> OutputTriangles;
	vector<Point> BoundingPolygon;

	InputPoints.push_back(vector<Point>());
	for (auto i = 0; i < InPoints.Num(); i++)
		InputPoints[0].push_back(Point(InPoints[i].X, InPoints[i].Y));

	clip2tri Clip2Tri;
	Clip2Tri.triangulate(InputPoints, OutputTriangles, BoundingPolygon);

	TSet<FVector> VertexSet;
	for (auto i = 0; i < OutputTriangles.size(); i++)
		VertexSet.Add(FVector(OutputTriangles[i].x, OutputTriangles[i].y, InPoints[0].Z));

	OutVertices = VertexSet.Array();

	for (auto i = 0; i < OutputTriangles.size(); i++)
		OutTriangles.Add(OutVertices.IndexOfByPredicate([&](const FVector Vertex) { return FMath::IsNearlyEqual(Vertex.X, OutputTriangles[i].x) && FMath::IsNearlyEqual(Vertex.Y, OutputTriangles[i].y); }));

	for (auto i = 0; i < BoundingPolygon.size(); i++)
		OutBounds.Add(OutVertices.IndexOfByPredicate([&](const FVector Vertex) { return FMath::IsNearlyEqual(Vertex.X, BoundingPolygon[i].x) && FMath::IsNearlyEqual(Vertex.Y, BoundingPolygon[i].y); }));
}
