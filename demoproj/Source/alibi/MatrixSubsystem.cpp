// Fill out your copyright notice in the Description page of Project Settings.


#include "MatrixSubsystem.h"

void UMatrixSubsystem::Matrix_InitNewCase() {
	//TODO: Clear matrix data; prepare for next case
	// Setup the default data for the case
}

static FDataKey _Matrix_CreateKey(EEntityType entity_type, int32 entity_id, uint8 info_type_id) {
	FDataKey key {};
	key.entity_type = entity_type;

	switch (entity_type) {
	case EEntityType::PERSON:
		key.person_info_type = (EPersonInfoType)info_type_id;
		break;
	case EEntityType::LOCATION:
		key.location_info_type = (ELocationInfoType)info_type_id;
		break;
	case EEntityType::OBJECT:
		key.object_info_type = (EObjectInfoType)info_type_id;
		break;
	case EEntityType::EVENT:
		key.event_info_type = (EEventInfoType)info_type_id;
		break;
	}

	key.pad0 = 0;
	key.entity_id = entity_id;

	return key;
}

static int32 _Matrix_ApplyDataPoint(FDataEntry &entry, EReliabilityCategory reliability, const FDataSource &src, const FDataPoint &point) {
	for (int32 i = 0; i < entry.forks.Num(); i++) {
		FDataBranch& b = entry.forks[i];
		if (b.source.src_type == src.src_type && b.source.src_id == src.src_id) {
			b.reliability = reliability;
			b.history.Add(point);
			return i;
		}
	}

	entry.forks.Emplace();
	FDataBranch& b = entry.forks[entry.forks.Num()-1];
	b.reliability = reliability;
	b.source = src;
	b.history.Add(point);

	return entry.forks.Num() - 1;
}

int32 UMatrixSubsystem::Matrix_NewPersonDataPoint(EPersonInfoType info_type, FDataPointInfo info) {
	FDataKey k = _Matrix_CreateKey(EEntityType::PERSON, info.entity_id, (uint8)info_type);
	FDataEntry& entry = sparse_entries[k.key];

	return _Matrix_ApplyDataPoint(entry, info.reliability, info.source, info.data);
}

int32 UMatrixSubsystem::Matrix_NewLocationDataPoint(ELocationInfoType info_type, FDataPointInfo info) {
	FDataKey k = _Matrix_CreateKey(EEntityType::LOCATION, info.entity_id, (uint8)info_type);
	FDataEntry& entry = sparse_entries[k.key];

	return _Matrix_ApplyDataPoint(entry, info.reliability, info.source, info.data);
}

int32 UMatrixSubsystem::Matrix_NewObjectDataPoint(EObjectInfoType info_type, FDataPointInfo info) {
	FDataKey k = _Matrix_CreateKey(EEntityType::OBJECT, info.entity_id, (uint8)info_type);
	FDataEntry& entry = sparse_entries[k.key];

	return _Matrix_ApplyDataPoint(entry, info.reliability, info.source, info.data);
}

int32 UMatrixSubsystem::Matrix_NewEventDataPoint(EEventInfoType info_type, FDataPointInfo info) {
	FDataKey k = _Matrix_CreateKey(EEntityType::EVENT, info.entity_id, (uint8)info_type);
	FDataEntry& entry = sparse_entries[k.key];

	return _Matrix_ApplyDataPoint(entry, info.reliability, info.source, info.data);
}
