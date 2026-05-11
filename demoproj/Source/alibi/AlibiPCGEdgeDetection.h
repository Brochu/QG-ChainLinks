#pragma once

#include "PCGSettings.h"
#include "AlibiPCGEdgeDetection.generated.h"

/// <summary>
/// Custom PCG node that detects edge points from a floor grid by measuring
/// distance to the nearest spline boundary. Outputs two point sets:
///   - "Edge Points" with rotation oriented toward the boundary (for walls)
///   - "Interior Points" for everything else (for furniture, evidence, etc.)
/// </summary>
UCLASS(BlueprintType, ClassGroup = (Procedural))
class UAlibiPCGEdgeDetectionSettings : public UPCGSettings
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return TEXT("EdgeDetection"); }
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
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable))
	/// <summary>
	/// The maximum distance from the spline boundary for a point to be
	/// considered an "edge" point. Should roughly equal half your tile width.
	/// Example: for 100-unit tiles, set this to ~55 to catch edge tiles.
	/// </summary>
	float EdgeDistanceThreshold = 55.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable))
	/// <summary>
	/// If true, wall rotation is quantized to the nearest 90 degrees
	/// (axis-aligned walls only). Set to false for free-angle rotation.
	/// </summary>
	bool bQuantizeRotation = true;
};

// ---- Element (execution logic) ----
class FPCGEdgeDetectionElement : public IPCGElement
{
protected:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;
};
