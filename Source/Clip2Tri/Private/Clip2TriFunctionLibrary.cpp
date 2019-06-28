#include "Clip2TriFunctionLibrary.h"

#include "Clip2Tri/Clip2Tri.h"
#include "Set.h"

using namespace c2t;

void UClip2TriFunctionLibrary::Triangulate(TArray<FVector> Points, TArray<FVector>& Vertices, TArray<int32>& Triangles, TArray<int32>& Bounds, bool bFlip /*= false*/)
{
	if (Points.Num() < 3)
		return;

	vector<vector<Point>> inputPoints;
	vector<Point> outputTriangles;
	vector<Point> boundingPolygon;

	inputPoints.push_back(vector<Point>());
	for (auto i = 0; i < Points.Num(); i++)
		inputPoints[0].push_back(Point(Points[i].X, Points[i].Y));

	clip2tri clip2tri;
	clip2tri.triangulate(inputPoints, outputTriangles, boundingPolygon);

	TSet<FVector> VertSet;
	for (auto i = 0; i < outputTriangles.size(); i++)
		VertSet.Add(FVector(outputTriangles[i].x, outputTriangles[i].y, Points[0].Z));

	Vertices = VertSet.Array();

	for (auto i = 0; i < outputTriangles.size(); i++)
		Triangles.Add(Vertices.IndexOfByPredicate([&](FVector V) { return FMath::IsNearlyEqual(V.X, outputTriangles[i].x) && FMath::IsNearlyEqual(V.Y, outputTriangles[i].y); }));

	for (auto i = 0; i < boundingPolygon.size(); i++)
		Bounds.Add(Vertices.IndexOfByPredicate([&](FVector V) { return FMath::IsNearlyEqual(V.X, boundingPolygon[i].x) && FMath::IsNearlyEqual(V.Y, boundingPolygon[i].y); }));
}
