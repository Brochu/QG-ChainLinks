using CaseGenV3.Models;

namespace CaseGenV3.Simulation;

public static class PostCrimeBehavior
{
    // Phase durations in ticks (1 tick = 15 min)
    private const int PanicDuration = 8;       // 2 hours
    private const int CoverupDuration = 88;    // 22 hours (total 24h from crime)
    private const int LieBuildingDuration = 192; // 2 more days (total ~3 days)
    private const int ParanoiaDuration = 672;  // 7 more days (total ~10 days)

    /// <summary>
    /// Decides what action an actor in post-crime mode should take.
    /// Called instead of BehaviorSystem.DecideAction when ActiveCrimeState != null.
    /// </summary>
    public static ActorAction DecideAction(Actor actor, WorldState world)
    {
        var state = actor.ActiveCrimeState!;

        return state.Phase switch
        {
            CrimePhase.Panic => DecidePanicAction(actor, state, world),
            CrimePhase.Coverup => DecideCoverupAction(actor, state, world),
            CrimePhase.LieBuilding => DecideLieBuildingAction(actor, state, world),
            CrimePhase.Paranoia => DecideParanoiaAction(actor, state, world),
            _ => ActorAction.Idle
        };
    }

    /// <summary>
    /// Advance phase timers and transition between phases.
    /// Called once per tick for the whole world.
    /// </summary>
    public static void AdvancePhases(WorldState world)
    {
        foreach (var actor in world.Actors.Values)
        {
            if (!actor.IsAlive || actor.ActiveCrimeState == null) continue;

            var state = actor.ActiveCrimeState;
            state.TicksInPhase++;

            var nextPhase = state.Phase switch
            {
                CrimePhase.Panic when state.TicksInPhase >= PanicDuration => CrimePhase.Coverup,
                CrimePhase.Coverup when state.TicksInPhase >= CoverupDuration => CrimePhase.LieBuilding,
                CrimePhase.LieBuilding when state.TicksInPhase >= LieBuildingDuration => CrimePhase.Paranoia,
                CrimePhase.Paranoia when state.TicksInPhase >= ParanoiaDuration => CrimePhase.Normal,
                _ => state.Phase
            };

            if (nextPhase != state.Phase)
            {
                state.Phase = nextPhase;
                state.TicksInPhase = 0;

                // Clear crime state when returning to normal
                if (nextPhase == CrimePhase.Normal)
                {
                    actor.ActiveCrimeState = null;
                }
            }
        }
    }

    // --- Panic phase: flee, be erratic ---
    private static ActorAction DecidePanicAction(Actor actor, CrimeState state, WorldState world)
    {
        // Tank safety need
        if (actor.Needs.TryGetValue(NeedType.Safety, out var safety))
            safety.Value = 0f;

        if (!state.FledScene)
        {
            return ActorAction.FleeScene;
        }

        // After fleeing, just hide wherever we ended up
        return ActorAction.Hide;
    }

    // --- Coverup phase: dispose weapon, clean up, stay hidden ---
    private static ActorAction DecideCoverupAction(Actor actor, CrimeState state, WorldState world)
    {
        // Priority 1: dispose weapon if we have one
        if (!state.DisposedWeapon && state.WeaponToDisposeId != null)
        {
            return ActorAction.DisposeWeapon;
        }

        // Priority 2: go home and clean up (wash clothes, etc.)
        if (!state.CleanedUp)
        {
            if (actor.CurrentLocationId != actor.HomeLocationId)
                return ActorAction.GoHome;
            return ActorAction.CleanUp;
        }

        // Otherwise hide at home
        if (actor.CurrentLocationId != actor.HomeLocationId)
            return ActorAction.GoHome;
        return ActorAction.Hide;
    }

    // --- Lie building: establish alibi, confide in friend, start acting normal ---
    private static ActorAction DecideLieBuildingAction(Actor actor, CrimeState state, WorldState world)
    {
        int hour = world.CurrentTime.Hour;
        bool isWorkHours = hour >= 9 && hour < 17
                           && world.CurrentTime.DayOfWeek != DayOfWeek.Saturday
                           && world.CurrentTime.DayOfWeek != DayOfWeek.Sunday;

        // Try to establish alibi once
        if (!state.EstablishedAlibi && world.Rng.NextDouble() < 0.15)
        {
            return ActorAction.EstablishAlibi;
        }

        // Confide in a trusted friend once (if they have one)
        if (!state.ConfidedInFriend && world.Rng.NextDouble() < 0.08)
        {
            var friendRel = world.GetActorRelationships(actor.Id)
                .FirstOrDefault(r => r.Type == RelationshipType.Friend && r.Strength > 50f);
            if (friendRel != null)
            {
                return ActorAction.ConfideInFriend;
            }
        }

        // Go to work to appear normal (but avoid crime scene)
        if (isWorkHours && !string.IsNullOrEmpty(actor.WorkLocationId)
            && actor.WorkLocationId != state.CrimeLocationId)
        {
            return ActorAction.GoToWork;
        }

        // Otherwise stay home
        if (actor.CurrentLocationId != actor.HomeLocationId)
            return ActorAction.GoHome;

        return ActorAction.Hide;
    }

    // --- Paranoia: mostly normal but avoids crime scene, occasional suspicious behavior ---
    private static ActorAction DecideParanoiaAction(Actor actor, CrimeState state, WorldState world)
    {
        // Occasionally generate suspicious behavior event
        if (world.Rng.NextDouble() < 0.005) // ~once every 2 days
        {
            var location = world.Locations.GetValueOrDefault(actor.CurrentLocationId);
            string locName = location?.Name ?? actor.CurrentLocationId;

            world.Events.Add(new SimEvent
            {
                Id = $"evt-{(world.Events.Count + 1):D5}",
                Timestamp = world.CurrentTime,
                Type = SimEventType.SuspiciousBehavior,
                Description = $"{actor.Name} was seen nervously looking around at {locName}",
                LocationId = actor.CurrentLocationId,
                ActorIds = [actor.Id],
                Relevance = EventRelevance.Minor,
                RelatedCrimeId = state.CrimeId
            });
        }

        // Use normal behavior but override if it would send us to crime scene
        var normalAction = BehaviorSystem.DecideAction(actor, world);

        // Never confront anyone while paranoid
        if (normalAction == ActorAction.Confront)
            return ActorAction.GoHome;

        // Avoid the crime scene
        if (WouldGoToLocation(normalAction, actor, state.CrimeLocationId, world))
            return ActorAction.GoHome;

        return normalAction;
    }

    /// <summary>
    /// Check if a given action would send the actor to the forbidden location.
    /// </summary>
    private static bool WouldGoToLocation(ActorAction action, Actor actor, string locationId, WorldState world)
    {
        return action switch
        {
            ActorAction.GoToWork => actor.WorkLocationId == locationId,
            ActorAction.Socialize => false, // can't easily predict, let ActionResolver handle avoidance
            _ => false
        };
    }
}
