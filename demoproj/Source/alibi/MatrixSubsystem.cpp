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
		FDataFork& b = entry.forks[i];
		if (b.source.src_type == src.src_type && b.source.src_id == src.src_id) {
			b.reliability = reliability;
			b.history.Add(point);
			return i;
		}
	}

	entry.forks.Emplace();
	FDataFork &b = entry.forks[entry.forks.Num()-1];
	b.reliability = reliability;
	b.source = src;
	b.history.Add(point);

	return entry.forks.Num() - 1;
}

int32 UMatrixSubsystem::Matrix_NewPersonDataPoint(EPersonInfoType info_type, FDataPointInfo info) {
	FDataKey k = _Matrix_CreateKey(EEntityType::PERSON, info.entity_id, (uint8)info_type);
	FDataEntry &entry = sparse_entries[k.key];

	return _Matrix_ApplyDataPoint(entry, info.reliability, info.source, info.data);
}

int32 UMatrixSubsystem::Matrix_NewLocationDataPoint(ELocationInfoType info_type, FDataPointInfo info) {
	FDataKey k = _Matrix_CreateKey(EEntityType::LOCATION, info.entity_id, (uint8)info_type);
	FDataEntry &entry = sparse_entries[k.key];

	return _Matrix_ApplyDataPoint(entry, info.reliability, info.source, info.data);
}

int32 UMatrixSubsystem::Matrix_NewObjectDataPoint(EObjectInfoType info_type, FDataPointInfo info) {
	FDataKey k = _Matrix_CreateKey(EEntityType::OBJECT, info.entity_id, (uint8)info_type);
	FDataEntry &entry = sparse_entries[k.key];

	return _Matrix_ApplyDataPoint(entry, info.reliability, info.source, info.data);
}

int32 UMatrixSubsystem::Matrix_NewEventDataPoint(EEventInfoType info_type, FDataPointInfo info) {
	FDataKey k = _Matrix_CreateKey(EEntityType::EVENT, info.entity_id, (uint8)info_type);
	FDataEntry &entry = sparse_entries[k.key];

	return _Matrix_ApplyDataPoint(entry, info.reliability, info.source, info.data);
}

// =============================

FDataPoint UMatrixSubsystem::Matrix_GetCurrentValue(EEntityType entity_type, int32 entity_id, uint8 info_type_id, int32 fork_index) {
	FDataKey k = _Matrix_CreateKey(entity_type, entity_id, info_type_id);
	FDataEntry *entry = sparse_entries.Find(k.key);

	if (!entry || entry->forks.Num() == 0) {
		return FDataPoint {};
	}

	check(fork_index < entry->forks.Num());

	FDataFork &e = entry->forks[fork_index];
	return e.history.Last();
}

FDataEntry UMatrixSubsystem::Matrix_GetDataEntry(EEntityType entity_type, int32 entity_id, uint8 info_type_id) {
	FDataKey k = _Matrix_CreateKey(entity_type, entity_id, info_type_id);
	FDataEntry *entry = sparse_entries.Find(k.key);

	if (!entry) {
		return FDataEntry {};
	}

	return *entry;
}

TArray<FDataPoint> UMatrixSubsystem::Matrix_GetDataHistory(EEntityType entity_type, int32 entity_id, uint8 info_type_id, int32 fork_index) {
	FDataKey k = _Matrix_CreateKey(entity_type, entity_id, info_type_id);
	FDataEntry *entry = sparse_entries.Find(k.key);

	if (!entry || entry->forks.Num() == 0) {
		return {};
	}

	check(fork_index < entry->forks.Num());

	FDataFork &e = entry->forks[fork_index];
	return e.history;
}

TArray<FDataPoint> UMatrixSubsystem::Matrix_GetAllForksLatest(EEntityType entity_type, int32 entity_id, uint8 info_type_id) {
	FDataKey k = _Matrix_CreateKey(entity_type, entity_id, info_type_id);
	FDataEntry *entry = sparse_entries.Find(k.key);

	if (!entry || entry->forks.Num() == 0) {
		return {};
	}

	TArray<FDataPoint> latests;
	for (FDataFork& fork : entry->forks) {
		latests.Add(fork.history.Last());
	}

	return latests;
}

TArray<FDataEntry> UMatrixSubsystem::Matrix_GetAllDataForEntity(EEntityType entity_type, int32 entity_id) {
	TArray<FDataEntry> entries;

	for (auto& [k, v] : sparse_entries) {
		FDataKey current;
		current.key = k;

		if (current.entity_type == entity_type && current.entity_id == entity_id) {
			entries.Add(v);
		}
	}

	return entries;
}

// =============================

int32 UMatrixSubsystem::Matrix_GetForkCount(EEntityType entity_type, int32 entity_id, uint8 info_type_id) {
	FDataKey k = _Matrix_CreateKey(entity_type, entity_id, info_type_id);
	FDataEntry *entry = sparse_entries.Find(k.key);

	if (!entry) {
		return 0;
	}

	return entry->forks.Num();
}

TArray<int32> UMatrixSubsystem::Matrix_GetAllEntityIds(EEntityType entity_type) {
	TArray<int32> ids;

	for (auto& [k, _] : sparse_entries) {
		FDataKey current;
		current.key = k;

		if (current.entity_type == entity_type) {
			ids.Add(current.entity_id);
		}
	}

	return ids;
}
