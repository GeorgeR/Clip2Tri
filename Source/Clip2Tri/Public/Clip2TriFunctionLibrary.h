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
	static void Triangulate(TArray<FVector> Points, TArray<FVector>& Vertices, TArray<int32>& Triangles, TArray<int32>& Bounds, bool bFlip = false);
};
