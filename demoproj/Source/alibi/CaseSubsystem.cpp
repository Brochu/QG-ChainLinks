// Fill out your copyright notice in the Description page of Project Settings.

#include "CaseSubsystem.h"
#include "JsonObjectConverter.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"

DEFINE_LOG_CATEGORY(LogCase);

/// <summary>
/// Loads and parses the case found at case_path
/// </summary>
/// <param name="case_path">Filepath of the case to load and make active</param>
void UCaseSubsystem::load_case_file(FString case_path) {
	FString contents;
	if (!FFileHelper::LoadFileToString(contents, *case_path)) {
		UE_LOG(LogCase, Error, TEXT("load_case_file: could not read case file '%s' (file missing or unreadable)."), *case_path);
		return;
	}

	TSharedPtr<FJsonObject> root;
	TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(contents);
	if (!FJsonSerializer::Deserialize(reader, root) || !root.IsValid()) {
		UE_LOG(LogCase, Error,
			TEXT("load_case_file: malformed JSON in '%s': %s"),
			*case_path, *reader->GetErrorMessage());
		return;
	}

	FCaseFile case_data;
	if (!FJsonObjectConverter::JsonObjectToUStruct(root.ToSharedRef(), &case_data)) {
		UE_LOG(LogCase, Error, TEXT("load_case_file: '%s' is valid JSON but does not match the case schema (field type mismatch?)."), *case_path);
		return;
	}

	if (case_data.locations.Num() == 0) {
		UE_LOG(LogCase, Error, TEXT("load_case_file: '%s' parsed but contains no locations; cannot start the case."), *case_path);
		return;
	}

	file = MoveTemp(case_data);
	save = FCaseSaveState();

	UE_LOG(LogCase, Log, TEXT("load_case_file: loaded '%s' — case '%s' (%d facts, %d actions, %d locations)."),
		*case_path,
		*file.meta.case_id.ToString(),
		file.facts.Num(),
		file.actions.Num(),
		file.locations.Num()
	);

	// Always start at first location in the case file
	save.active_locid = file.locations[0].location_id;
	discover_facts(file.meta.starting_facts);
}

/// <summary>
/// Gets a reference to the current active player's location
/// </summary>
/// <returns>reference to the player's active location</returns>
const FCaseLocation &UCaseSubsystem::get_active_location() const {
	const FCaseLocation *active_loc = file.locations.FindByPredicate([this](const FCaseLocation &other) { return other.location_id == save.active_locid; });

	if (active_loc == nullptr) {
		// Invariant break: active_locid should always name a loaded location.
		UE_LOG(LogCase, Error,
			TEXT("get_active_location: active_locid '%s' is not a loaded location; falling back to the first."),
			*save.active_locid.ToString());
		active_loc = &file.locations[0];
	}
	return *active_loc;
}

/// <summary>
/// Gets the current state for a specific case location
/// </summary>
/// <param name="loc_id">Location id to check the state of</param>
/// <returns>the current active state for the given location</returns>
FName UCaseSubsystem::get_location_state(FName loc_id) const {
	const FCaseLocation *found_loc= file.locations.FindByPredicate([this, loc_id](const FCaseLocation &other) { return other.location_id == loc_id; });
	if (found_loc == nullptr) {
		return NAME_None;
	}

	const FCaseLocation &loc = *found_loc;
	FName state = loc.states[0].state_id;

	for (int32 i = 1; i < loc.states.Num(); i++) {
		if (all_facts_known(loc.states[i].when)) {
			state = loc.states[i].state_id;
		}
	}
	return state;
}

/// <summary>
/// Gets an array of all available location indices
/// </summary>
/// <returns>List of indices into the location array parsed from the case file</returns>
TArray<int32> UCaseSubsystem::list_location_idx() const {
	TArray<int32> results;

	for (int32 i = 0; i < file.locations.Num(); i++) {
		results.Add(i);
	}

	return results;
}

/// <summary>
/// Gets an array of all avaiable action indices for a given location
/// </summary>
/// <param name="loc_id">location to query actions for</param>
/// <returns>List of all actions that are visible to the player at loc_id</returns>
TArray<int32> UCaseSubsystem::list_action_idx(FName loc_id) const {
	TArray<int32> results;

	for (int32 i = 0; i < file.actions.Num(); i++) {
		if (file.actions[i].location_id != loc_id) {
			continue;
		}

		const EActionVisibility vis = action_visibility(file.actions[i]);
		if (vis == EActionVisibility::Absent || vis == EActionVisibility::Secret) {
			continue;
		}
		results.Add(i);
	}

	return results;
}

/// <summary>
/// Check the visibility for a given action of the case
/// </summary>
/// <param name="act">Action to check</param>
/// <returns>Visibility level</returns>
EActionVisibility UCaseSubsystem::action_visibility(const FCaseAction &act) const {
	const bool in_state =
		act.location_states.Num() == 0 ||
		act.location_states.Contains(get_location_state(act.location_id));

	if (!in_state || !is_in_window(act)) {
		return EActionVisibility::Absent;
	}

	if (!all_facts_known(act.prerequisites)) {
		return act.hidden ? EActionVisibility::Secret : EActionVisibility::Locked;
	}

	return EActionVisibility::Unlocked;
}

/// <summary>
/// Check if a given action of the case is possible to commit with the current player state
/// </summary>
/// <param name="act">Action to check</param>
/// <returns>Potential rejection reason when trying to commit the action</returns>
ECommitActionResult UCaseSubsystem::can_commit(const FCaseAction &act) const {
	if (act.location_id != save.active_locid) {
		return ECommitActionResult::NotAtLocation;
	}

	if (act.location_states.Num() > 0 && !act.location_states.Contains(get_location_state(act.location_id))) {
		return ECommitActionResult::WrongLocationState;
	}

	if (!is_in_window(act)) {
		return ECommitActionResult::OutsideAvailabilityWindow;
	}

	if (!all_facts_known(act.prerequisites)) {
		return ECommitActionResult::PrerequisitesNotMet;
	}

	const int32 block_of_day = save.used_blocks % FMath::Max(1, file.meta.blocks_per_day);
	if (act.blocks_of_day.Num() > 0 && !act.blocks_of_day.Contains(block_of_day)) {
		return ECommitActionResult::WrongBlockOfDay;
	}

	if ((save.used_blocks + act.cost) > (file.meta.deadline_days * file.meta.blocks_per_day)) {
		return ECommitActionResult::NotEnoughBlocks;
	}

	if (act.verb == ECaseVerb::COLLECT && save.active_lab_requests.Num() >= file.meta.lab_queue_capacity) {
		return ECommitActionResult::LabQueueFull;
	}

	if (!act.repeatable && save.completed_actions.Contains(act.action_id)) {
		return ECommitActionResult::AlreadyCompleted;
	}

	return ECommitActionResult::Success;
}

/// <summary>
/// Moves the player to a new location
/// </summary>
/// <param name="new_loc_id">new location id to move to</param>
/// <returns>If the move was successful or not</returns>
bool UCaseSubsystem::move_location(FName new_loc_id) {
	//TODO: Overly simple for now, will need events and checks later to react to movement during a case
	save.active_locid = new_loc_id;
	return true;
}

/// <summary>
/// Commit action by the name of action_id
/// </summary>
/// <param name="action_id">name of the action to commit</param>
/// <returns>Potential rejection reason when trying to commit this action</returns>
ECommitActionResult UCaseSubsystem::commit_action(FName action_id) {
	const FCaseAction *chosen_action = file.actions.FindByPredicate([action_id](const FCaseAction &other) { return other.action_id == action_id; });
	if (chosen_action == nullptr) {
		return ECommitActionResult::UnknownAction;
	}

	const ECommitActionResult res = can_commit(*chosen_action);
	if (res != ECommitActionResult::Success) {
		return res;
	}

	if (chosen_action->verb == ECaseVerb::COLLECT) {
		// Lab request action; delayed results
		save.active_lab_requests.Push({ action_id, save.used_blocks });
		on_new_pending_request.Broadcast(save.active_lab_requests.Last());
		//TODO: Need to handle delayed action that are not lab requests as well
	}

	spend_blocks(chosen_action->cost);
	if (chosen_action->delay <= 0) {
		discover_facts(chosen_action->produces);
	}
	if (!chosen_action->repeatable) {
		save.completed_actions.Add(chosen_action->action_id);
	}
	return ECommitActionResult::Success;
}

// --------------------------------------------------
bool UCaseSubsystem::all_facts_known(const TArray<FName> &facts_to_check) const {
	for (auto &fact : facts_to_check) {
		if (!save.known_facts.Contains(fact)) {
			return false;
		}
	}

	return true;
}

bool UCaseSubsystem::is_in_window(const FCaseAction &act) const {
	const int32 from = act.available[0];
	const int32 to = act.available[1];
	return save.used_blocks >= from && ( to == -1 || save.used_blocks <= to);
}

void UCaseSubsystem::spend_blocks(int32 quantity) {
	for (int32 time = 0; time < quantity; time++) {
		save.used_blocks++;

		for (int32 i = save.active_lab_requests.Num() - 1; i >= 0; i--) {
			FPendingRequest &request = save.active_lab_requests[i];
			FCaseAction *action = file.actions.FindByPredicate([&request](const FCaseAction &a) {return a.action_id == request.action_id; });
			checkf(action != nullptr, TEXT("Could not find lab request's action_id = %s!"), *request.action_id.ToString());

			const int32 diff = save.used_blocks - request.block_started;
			if (diff >= action->delay) {
				discover_facts(action->produces);
				on_pending_request_complete.Broadcast(request);

				save.active_lab_requests.RemoveAt(i, EAllowShrinking::No);
			}
		}

		for (const auto &schedule : file.schedule) {
			if (schedule.at_block == save.used_blocks) {
				discover_facts(schedule.delivers);
				on_schedule_complete.Broadcast(schedule.pager);
			}
		}

		on_time_advance.Broadcast(save.used_blocks);
	}
}

void UCaseSubsystem::discover_facts(const TArray<FName> &new_facts) {
	for (auto &fact_name : new_facts) {
		bool already_known;
		save.known_facts.Add(fact_name, &already_known);

		if (!already_known) {
			FCaseFact *fact = file.facts.FindByPredicate([fact_name](const FCaseFact &f) { return f.fact_id == fact_name; });
			checkf(fact != nullptr, TEXT("Could not find discovered fact_name = %s!"), *fact_name.ToString());
			for (auto &tag : fact->tags) {
				save.known_tags.Add(tag);
			}

			on_fact_discovered.Broadcast(fact_name, save.used_blocks);
		}
	}
}
