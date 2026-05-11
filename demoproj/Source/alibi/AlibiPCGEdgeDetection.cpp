#include "AlibiPCGEdgeDetection.h"

#include "PCGContext.h"
#include "PCGPin.h"
#include "Data/PCGPointData.h"
#include "Data/PCGSplineData.h"
#include "Components/SplineComponent.h"

#define LOCTEXT_NAMESPACE "PCGEdgeDetection"

// Pin name constants
static const FName EdgeOutputPinName   = TEXT("Edge Points");
static const FName InteriorOutputPinName = TEXT("Interior Points");
static const FName SplineInputPinName  = TEXT("Spline");

// ============================================================================
// UAlibiPCGEdgeDetectionSettings
// ============================================================================

#if WITH_EDITOR
FText UAlibiPCGEdgeDetectionSettings::GetDefaultNodeTitle() const
{
	return LOCTEXT("NodeTitle", "Edge Detection (Spline Distance)");
}

FText UAlibiPCGEdgeDetectionSettings::GetNodeTooltipText() const
{
	return LOCTEXT("NodeTooltip",
		"Classifies input points as Edge or Interior based on distance to "
		"the nearest spline boundary. Edge points receive a rotation facing "
		"outward toward the boundary (for wall placement).");
}
#endif

EPCGDataType UAlibiPCGEdgeDetectionSettings::GetCurrentPinTypes(const UPCGPin* InPin) const
{
	const FName PinName = InPin->GetFName();

	if (PinName == PCGPinConstants::DefaultInputLabel)
	{
		return EPCGDataType::Point;
	}
	if (PinName == SplineInputPinName)
	{
		return EPCGDataType::Spline;
	}

	return Super::GetCurrentPinTypes(InPin);
}

TArray<FPCGPinProperties> UAlibiPCGEdgeDetectionSettings::InputPinProperties() const
{
	TArray<FPCGPinProperties> Pins;
	Pins.SetNum(2);

	// Pin 0: The room spline (from Get Spline Data)
	Pins[0].Label = SplineInputPinName;
	Pins[0].AllowedTypes = EPCGDataType::Spline;
	//Pins[1].bAllowMultipleConnections = false;

	// Pin 1: Floor points (from your Spline Sampler set to "On Interior")
	Pins[1].Label = PCGPinConstants::DefaultInputLabel;
	Pins[1].AllowedTypes = EPCGDataType::Point;
	//Pins[0].bAllowMultipleConnections = false;

	return Pins;
}

TArray<FPCGPinProperties> UAlibiPCGEdgeDetectionSettings::OutputPinProperties() const
{
	TArray<FPCGPinProperties> Pins;
	Pins.SetNum(2);

	// Pin 0: Points identified as edges (for wall spawning)
	Pins[0].Label = EdgeOutputPinName;
	Pins[0].AllowedTypes = EPCGDataType::Point;

	// Pin 1: Points identified as interior (for floor/furniture)
	Pins[1].Label = InteriorOutputPinName;
	Pins[1].AllowedTypes = EPCGDataType::Point;

	return Pins;
}

FPCGElementPtr UAlibiPCGEdgeDetectionSettings::CreateElement() const
{
	return MakeShared<FPCGEdgeDetectionElement>();
}

// ============================================================================
// FPCGEdgeDetectionElement
// ============================================================================

bool FPCGEdgeDetectionElement::ExecuteInternal(FPCGContext* Context) const
{
	check(Context);

	const UAlibiPCGEdgeDetectionSettings* Settings = Context->GetInputSettings<UAlibiPCGEdgeDetectionSettings>();
	check(Settings);

	const float Threshold = Settings->EdgeDistanceThreshold;
	const bool bQuantize  = Settings->bQuantizeRotation;

	TArray<FPCGTaggedData> PointInputs = Context->InputData.GetInputsByPin(PCGPinConstants::DefaultInputLabel);
	if (PointInputs.IsEmpty())
	{
		return true;
	}

	TArray<FPCGTaggedData> SplineInputs = Context->InputData.GetInputsByPin(SplineInputPinName);
	if (SplineInputs.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("EdgeDetection: No spline data on Spline pin."));
		return true;
	}

	// Get the spline component from the PCG spline data.
	// UPCGSplineData wraps a spline; we need the underlying USplineComponent
	// for distance queries.
	const UPCGSplineData* SplineData = Cast<UPCGSplineData>(SplineInputs[0].Data);
	if (!SplineData)
	{
		UE_LOG(LogTemp, Warning, TEXT("EdgeDetection: Could not cast to UPCGSplineData."));
		return true;
	}

	// NOTE: Depending on your UE version, accessing the spline may differ.
	// In some versions you use SplineData->SplineStruct or GetSplineStruct().
	// You may need to adapt this to your engine version.
	// The key function you need is FindLocationClosestToWorldLocation
	// or an equivalent parametric query.

	// ------------------------------------------------------------------
	// 3. Create output point data containers
	// ------------------------------------------------------------------
	UPCGPointData* EdgePointData    = NewObject<UPCGPointData>();
	UPCGPointData* InteriorPointData = NewObject<UPCGPointData>();

	TArray<FPCGPoint>& EdgePoints    = EdgePointData->GetMutablePoints();
	TArray<FPCGPoint>& InteriorPoints = InteriorPointData->GetMutablePoints();

	// ------------------------------------------------------------------
	// 4. Process each input point
	// ------------------------------------------------------------------
	for (const FPCGTaggedData& TaggedData : PointInputs)
	{
		const UPCGPointData* InputPointData = Cast<UPCGPointData>(TaggedData.Data);
		if (!InputPointData) continue;

		const TArray<FPCGPoint>& Points = InputPointData->GetPoints();

		for (const FPCGPoint& Point : Points)
		{
			const FVector PointLocation = Point.Transform.GetLocation();
			FVector ClosestSplineLocation = FVector::ZeroVector;

			float key = SplineData->SplineStruct.FindInputKeyClosestToWorldLocation(PointLocation);
			ClosestSplineLocation = SplineData->SplineStruct.GetLocationAtSplineInputKey(key, ESplineCoordinateSpace::Type::World);

			// Direction from the floor point toward the spline boundary
			FVector ToBoundary = ClosestSplineLocation - PointLocation;

			// We only care about the 2D distance (XY plane) for floor-based detection
			ToBoundary.Z = 0.0f;

			const float Distance = ToBoundary.Size();

			if (Distance <= Threshold)
			{
				// --- This is an EDGE point ---
				FPCGPoint EdgePoint = Point; // Copy the original point

				// Compute wall rotation: facing outward (toward the boundary)
				FVector Direction = ToBoundary.GetSafeNormal();

				if (bQuantize)
				{
					// Snap to nearest 90-degree cardinal direction
					// This ensures walls are axis-aligned on the grid
					if (FMath::Abs(Direction.X) >= FMath::Abs(Direction.Y))
					{
						Direction = FVector(FMath::Sign(Direction.X), 0.0f, 0.0f);
					}
					else
					{
						Direction = FVector(0.0f, FMath::Sign(Direction.Y), 0.0f);
					}
				}

				// Build rotation from the direction vector
				// MakeFromX creates a rotation where X axis points along Direction
				const FRotator WallRotation = FRotationMatrix::MakeFromX(Direction).Rotator();
				EdgePoint.Transform.SetRotation(WallRotation.Quaternion());

				EdgePoints.Add(EdgePoint);
			}
			else
			{
				// --- This is an INTERIOR point ---
				InteriorPoints.Add(Point);
			}
		}
	}

	// ------------------------------------------------------------------
	// 5. Push results to output pins
	// ------------------------------------------------------------------
	FPCGTaggedData& EdgeOutput = Context->OutputData.TaggedData.Emplace_GetRef();
	EdgeOutput.Data = EdgePointData;
	EdgeOutput.Pin  = EdgeOutputPinName;

	FPCGTaggedData& InteriorOutput = Context->OutputData.TaggedData.Emplace_GetRef();
	InteriorOutput.Data = InteriorPointData;
	InteriorOutput.Pin  = InteriorOutputPinName;

	return true;
}

#undef LOCTEXT_NAMESPACE
