#pragma once

#include "PCGSettings.h"
#include "PCGBoundaryDistance.generated.h"

/// <summary>
/// PCGBoundaryDistance.h
/// Custom PCG node that enriches input points with boundary proximity data.
/// For each point, computes:
///   - "BoundaryDistance" (float): distance to the nearest spline boundary
///   - "WallDirection" (Vector): outward-facing direction toward the boundary
///   - "WallCardinal" (String): quantized cardinal direction ("PosX", "NegX", "PosY", "NegY")
///
/// Does NOT filter or split points — use downstream Attribute Filter nodes for that.
/// </summary>
UCLASS(BlueprintType, ClassGroup = (Procedural))
class UPCGBoundaryDistanceSettings : public UPCGSettings
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return TEXT("BoundaryDistance"); }
	virtual FText GetDefaultNodeTitle() const override;
	virtual FText GetNodeTooltipText() const override;
	virtual EPCGSettingsType GetType() const override { return EPCGSettingsType::Spatial; }
#endif

	virtual EPCGDataType GetCurrentPinTypes(const UPCGPin* InPin) const override;

protected:
	virtual TArray<FPCGPinProperties> InputPinProperties() const override;
	virtual TArray<FPCGPinProperties> OutputPinProperties() const override;
	virtual FPCGElementPtr CreateElement() const override;

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Attributes", meta = (PCG_Overridable))
	FName DistanceAttributeName = TEXT("BoundaryDistance");

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Attributes", meta = (PCG_Overridable))
	FName DirectionAttributeName = TEXT("WallDirection");

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Attributes", meta = (PCG_Overridable))
	FName CardinalAttributeName = TEXT("WallCardinal");

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Attributes", meta = (PCG_Overridable))
	bool bWriteCardinalDirection = true;
};

class FPCGBoundaryDistanceElement : public IPCGElement
{
protected:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;
};
