#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "Clip2TriFunctionLibrary.generated.h"

UCLASS()
class CLIP2TRI_API UClip2TriFunctionLibrary
	: public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Mesh")
	static void Triangulate(const TArray<FVector>& InPoints, TArray<FVector>& OutVertices, TArray<int32>& OutTriangles, TArray<int32>& OutBounds, bool bFlip = false);
};
