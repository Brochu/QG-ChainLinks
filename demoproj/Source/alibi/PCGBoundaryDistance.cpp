#include "PCGBoundaryDistance.h"

#include "PCGContext.h"
#include "PCGPin.h"
#include "Data/PCGPointData.h"
#include "Data/PCGSplineData.h"
#include "Metadata/PCGMetadata.h"
#include "Metadata/PCGMetadataAttribute.h"

#define LOCTEXT_NAMESPACE "PCGBoundaryDistance"

static const FName SplineInputPinName = TEXT("Spline");

// UPCGBoundaryDistanceSettings
#if WITH_EDITOR
FText UPCGBoundaryDistanceSettings::GetDefaultNodeTitle() const
{
	return LOCTEXT("NodeTitle", "Boundary Distance");
}

FText UPCGBoundaryDistanceSettings::GetNodeTooltipText() const
{
	return LOCTEXT("NodeTooltip",
		"Enriches each point with its distance and direction to the nearest "
		"spline boundary. Use downstream Attribute Filter nodes to split "
		"points by proximity (e.g. edge vs interior).");
}
#endif

EPCGDataType UPCGBoundaryDistanceSettings::GetCurrentPinTypes(const UPCGPin* InPin) const
{
	const FName PinName = InPin->GetFName();

	if (PinName == PCGPinConstants::DefaultInputLabel)
	{
		return EPCGDataType::Point;
	}
	if (PinName == PCGPinConstants::DefaultOutputLabel)
	{
		return EPCGDataType::Point;
	}
	if (PinName == SplineInputPinName)
	{
		return EPCGDataType::Spline;
	}

	return Super::GetCurrentPinTypes(InPin);
}

TArray<FPCGPinProperties> UPCGBoundaryDistanceSettings::InputPinProperties() const
{
	TArray<FPCGPinProperties> Pins;
	Pins.SetNum(2);

	// Pin 0: Room boundary spline
	Pins[0].Label = SplineInputPinName;
	Pins[0].AllowedTypes = EPCGDataType::Spline;

	// Pin 1: Floor points
	Pins[1].Label = PCGPinConstants::DefaultInputLabel;
	Pins[1].AllowedTypes = EPCGDataType::Point;

	return Pins;
}

TArray<FPCGPinProperties> UPCGBoundaryDistanceSettings::OutputPinProperties() const
{
	TArray<FPCGPinProperties> Pins;
	Pins.SetNum(1);

	Pins[0].Label = PCGPinConstants::DefaultOutputLabel;
	Pins[0].AllowedTypes = EPCGDataType::Point;

	return Pins;
}

FPCGElementPtr UPCGBoundaryDistanceSettings::CreateElement() const
{
	return MakeShared<FPCGBoundaryDistanceElement>();
}

namespace PCGBoundaryDistanceHelpers
{
	struct FClosestResult
	{
		FVector ClosestPoint = FVector::ZeroVector;
		float Distance2D = FLT_MAX;
		FVector Direction2D = FVector::ZeroVector;
	};

	/// <summary>
	/// Quantizes a 2D direction to the nearest cardinal axis.
	/// Returns "PosX", "NegX", "PosY", "NegY", or "None" if the direction is zero.
	/// </summary>
	/// <param name="Direction"></param>
	/// <returns></returns>
	FString QuantizeToCardinal(const FVector& Direction)
	{
		if (Direction.IsNearlyZero()) return TEXT("None");

		if (FMath::Abs(Direction.X) >= FMath::Abs(Direction.Y))
		{
			return (Direction.X >= 0.0f) ? TEXT("PosX") : TEXT("NegX");
		}
		else
		{
			return (Direction.Y >= 0.0f) ? TEXT("PosY") : TEXT("NegY");
		}
	}
}

bool FPCGBoundaryDistanceElement::ExecuteInternal(FPCGContext* Context) const
{
	check(Context);

	const UPCGBoundaryDistanceSettings* Settings = Context->GetInputSettings<UPCGBoundaryDistanceSettings>();
	check(Settings);

	TArray<FPCGTaggedData> PointInputs = Context->InputData.GetInputsByPin(PCGPinConstants::DefaultInputLabel);
	TArray<FPCGTaggedData> SplineInputs = Context->InputData.GetInputsByPin(SplineInputPinName);
	if (PointInputs.IsEmpty() || SplineInputs.IsEmpty())
	{
		return true;
	}

	const UPCGSplineData* SplineData = Cast<UPCGSplineData>(SplineInputs[0].Data);
	if (!SplineData)
	{
		UE_LOG(LogTemp, Warning, TEXT("BoundaryDistance: Could not cast to UPCGSplineData."));
		return true;
	}

	for (const FPCGTaggedData& TaggedData : PointInputs)
	{
		const UPCGPointData* InputPointData = Cast<UPCGPointData>(TaggedData.Data);
		if (!InputPointData) continue;

		const TArray<FPCGPoint>& InputPoints = InputPointData->GetPoints();

		// PCG convention: don't mutate inputs
		UPCGPointData* OutputPointData = NewObject<UPCGPointData>();
		OutputPointData->InitializeFromData(InputPointData);
		TArray<FPCGPoint>& OutputPoints = OutputPointData->GetMutablePoints();
		OutputPoints.Reserve(InputPoints.Num());

		UPCGMetadata* Metadata = OutputPointData->MutableMetadata();

		FPCGMetadataAttribute<float>* DistAttr = Metadata->FindOrCreateAttribute<float>(
				Settings->DistanceAttributeName,
				0.0f,
				true,   // Allows interpolation
				true    // Override parent
		);

		FPCGMetadataAttribute<FVector>* DirAttr = Metadata->FindOrCreateAttribute<FVector>(
				Settings->DirectionAttributeName,
				FVector::ZeroVector,
				true,
				true
		);

		FPCGMetadataAttribute<FString>* CardinalAttr = nullptr;
		if (Settings->bWriteCardinalDirection)
		{
			CardinalAttr = Metadata->FindOrCreateAttribute<FString>(
				Settings->CardinalAttributeName,
				TEXT("None"),
				false,
				true
			);
		}

		for (const FPCGPoint& Point : InputPoints)
		{
			FPCGPoint NewPoint = Point;
			const FVector PointLocation = Point.Transform.GetLocation();

			PCGBoundaryDistanceHelpers::FClosestResult result;
			float key = SplineData->SplineStruct.FindInputKeyClosestToWorldLocation(PointLocation);
			result.ClosestPoint = SplineData->SplineStruct.GetLocationAtSplineInputKey(key, ESplineCoordinateSpace::World);

			FVector Dir = result.ClosestPoint - PointLocation;
			Dir.Z = 0.f;
			result.Distance2D = Dir.Size();
			result.Direction2D = (result.Distance2D > SMALL_NUMBER) ? Dir.GetSafeNormal() : FVector::ZeroVector;

			NewPoint.MetadataEntry = Metadata->AddEntry();

			DistAttr->SetValue(NewPoint.MetadataEntry, result.Distance2D);
			DirAttr->SetValue(NewPoint.MetadataEntry, result.Direction2D);

			if (CardinalAttr)
			{
				FString Cardinal = PCGBoundaryDistanceHelpers::QuantizeToCardinal(result.Direction2D);
				CardinalAttr->SetValue(NewPoint.MetadataEntry, MoveTemp(Cardinal));
			}

			OutputPoints.Add(NewPoint);
		}

		FPCGTaggedData& Output = Context->OutputData.TaggedData.Emplace_GetRef();
		Output.Data = OutputPointData;
		Output.Pin = PCGPinConstants::DefaultOutputLabel;
	}

	return true;
}

#undef LOCTEXT_NAMESPACE
