using CaseGenV3.Models;

namespace CaseGenV3.Simulation;

public static class ActionResolver
{
    public static void ResolveAction(Actor actor, ActorAction action, WorldState world)
    {
        switch (action)
        {
            case ActorAction.GoToWork:
                MoveActor(actor, actor.WorkLocationId, world);
                break;

            case ActorAction.GoHome:
                MoveActor(actor, actor.HomeLocationId, world);
                break;

            case ActorAction.Sleep:
                // Ensure actor is at home; if not, move them home
                if (actor.CurrentLocationId != actor.HomeLocationId)
                {
                    MoveActor(actor, actor.HomeLocationId, world);
                }
                // Rest satisfaction is handled by NeedSystem
                break;

            case ActorAction.Socialize:
                ResolveSocialize(actor, world);
                break;

            case ActorAction.VisitPartner:
                ResolveVisitPartner(actor, world);
                break;

            case ActorAction.Confront:
                ResolveConfront(actor, world);
                break;

            case ActorAction.Idle:
            case ActorAction.Hide:
                // Do nothing — stay in place
                break;

            // --- Post-crime actions ---
            case ActorAction.FleeScene:
                ResolveFleeScene(actor, world);
                break;

            case ActorAction.DisposeWeapon:
                ResolveDisposeWeapon(actor, world);
                break;

            case ActorAction.CleanUp:
                ResolveCleanUp(actor, world);
                break;

            case ActorAction.EstablishAlibi:
                ResolveEstablishAlibi(actor, world);
                break;

            case ActorAction.ConfideInFriend:
                ResolveConfideInFriend(actor, world);
                break;
        }
    }

    private static void ResolveSocialize(Actor actor, WorldState world)
    {
        // Pick a social location from connected locations
        var socialTypes = new[] { LocationType.Bar, LocationType.Restaurant, LocationType.Park };

        if (!world.Locations.TryGetValue(actor.CurrentLocationId, out var currentLoc))
            return;

        var socialLocations = currentLoc.ConnectedLocationIds
            .Where(id => world.Locations.TryGetValue(id, out var loc) && socialTypes.Contains(loc.Type))
            .ToList();

        // If no social location is directly connected, search all locations
        if (socialLocations.Count == 0)
        {
            socialLocations = world.Locations.Values
                .Where(l => socialTypes.Contains(l.Type))
                .Select(l => l.Id)
                .ToList();
        }

        if (socialLocations.Count > 0)
        {
            string targetId = socialLocations[world.Rng.Next(socialLocations.Count)];
            MoveActor(actor, targetId, world);
        }

        // Satisfy Social need
        if (actor.Needs.TryGetValue(NeedType.Social, out var socialNeed))
        {
            socialNeed.Satisfy(5f);
        }
    }

    private static void ResolveVisitPartner(Actor actor, WorldState world)
    {
        var partnerRel = world.GetActorRelationships(actor.Id)
            .FirstOrDefault(r => r.Type == RelationshipType.Partner
                              || r.Type == RelationshipType.Spouse);

        if (partnerRel == null) return;

        string partnerId = world.GetOtherActorId(partnerRel, actor.Id);
        if (!world.Actors.TryGetValue(partnerId, out var partner) || !partner.IsAlive) return;

        // Move to partner's location
        MoveActor(actor, partner.CurrentLocationId, world);

        // Satisfy Love need
        if (actor.Needs.TryGetValue(NeedType.Love, out var loveNeed))
        {
            loveNeed.Satisfy(8f);
        }

        world.Events.Add(new SimEvent
        {
            Id = $"evt-{(world.Events.Count + 1):D5}",
            Timestamp = world.CurrentTime,
            Type = SimEventType.VisitPartner,
            Description = $"{actor.Name} visited {partner.Name}",
            LocationId = partner.CurrentLocationId,
            ActorIds = [actor.Id, partnerId],
            Relevance = EventRelevance.Background
        });
    }

    private static void ResolveConfront(Actor actor, WorldState world)
    {
        // Find the actor at the same location with the highest tension relationship
        var tensionRels = world.GetActorRelationships(actor.Id)
            .Where(r => r.Tension > 0)
            .OrderByDescending(r => r.Tension)
            .ToList();

        foreach (var rel in tensionRels)
        {
            string otherId = world.GetOtherActorId(rel, actor.Id);
            if (!world.Actors.TryGetValue(otherId, out var other)) continue;
            if (!other.IsAlive || other.CurrentLocationId != actor.CurrentLocationId) continue;

            // Increase tension
            float tensionIncrease = 5f + (float)(world.Rng.NextDouble() * 10.0); // 5-15
            rel.Tension = Math.Min(100f, rel.Tension + tensionIncrease);
            rel.History.Add($"Confronted at {world.CurrentTime}");

            world.Events.Add(new SimEvent
            {
                Id = $"evt-{(world.Events.Count + 1):D5}",
                Timestamp = world.CurrentTime,
                Type = SimEventType.Confront,
                Description = $"{actor.Name} confronted {other.Name}",
                LocationId = actor.CurrentLocationId,
                ActorIds = [actor.Id, otherId],
                Relevance = EventRelevance.KeyEvent
            });

            break; // Only confront one person per tick
        }
    }

    // ========== Post-crime action resolvers ==========

    private static void ResolveFleeScene(Actor actor, WorldState world)
    {
        var state = actor.ActiveCrimeState!;

        // Flee to a random adjacent location (not home — too panicked to think straight)
        if (world.Locations.TryGetValue(actor.CurrentLocationId, out var currentLoc)
            && currentLoc.ConnectedLocationIds.Count > 0)
        {
            var candidates = currentLoc.ConnectedLocationIds
                .Where(id => id != actor.HomeLocationId && id != state.CrimeLocationId)
                .ToList();
            if (candidates.Count == 0) candidates = currentLoc.ConnectedLocationIds;

            string targetId = candidates[world.Rng.Next(candidates.Count)];
            MoveActor(actor, targetId, world);
        }

        state.FledScene = true;

        // Witnesses may see them running
        var witnesses = world.GetActorsAtLocation(actor.CurrentLocationId)
            .Where(a => a.Id != actor.Id).ToList();

        string locName = world.Locations.GetValueOrDefault(state.CrimeLocationId)?.Name ?? "the scene";

        world.Events.Add(new SimEvent
        {
            Id = $"evt-{(world.Events.Count + 1):D5}",
            Timestamp = world.CurrentTime,
            Type = SimEventType.FledScene,
            Description = $"{actor.Name} was seen running from {locName}, appearing distressed",
            LocationId = state.CrimeLocationId,
            ActorIds = [actor.Id],
            Relevance = EventRelevance.KeyEvent,
            RelatedCrimeId = state.CrimeId
        });
    }

    private static void ResolveDisposeWeapon(Actor actor, WorldState world)
    {
        var state = actor.ActiveCrimeState!;
        if (state.WeaponToDisposeId == null) { state.DisposedWeapon = true; return; }

        // Find an alley, park, or parking lot to dump the weapon
        var disposalTypes = new[] { LocationType.Alley, LocationType.Park, LocationType.Parking };
        var disposalLocations = world.Locations.Values
            .Where(l => disposalTypes.Contains(l.Type) && l.Id != state.CrimeLocationId)
            .ToList();

        if (disposalLocations.Count == 0)
        {
            // No good disposal site, just drop it wherever
            state.DisposedWeapon = true;
            return;
        }

        var disposalLoc = disposalLocations[world.Rng.Next(disposalLocations.Count)];
        MoveActor(actor, disposalLoc.Id, world);

        // Move weapon from actor inventory to disposal location
        if (world.Objects.TryGetValue(state.WeaponToDisposeId, out var weapon))
        {
            actor.Inventory.Remove(weapon.Id);
            weapon.HeldByActorId = null;
            weapon.LocationId = disposalLoc.Id;
            if (!disposalLoc.ObjectIds.Contains(weapon.Id))
                disposalLoc.ObjectIds.Add(weapon.Id);
        }

        state.DisposedWeapon = true;

        // Possible witness at the disposal location
        var witnesses = world.GetActorsAtLocation(disposalLoc.Id)
            .Where(a => a.Id != actor.Id).ToList();

        world.Events.Add(new SimEvent
        {
            Id = $"evt-{(world.Events.Count + 1):D5}",
            Timestamp = world.CurrentTime,
            Type = SimEventType.DisposedWeapon,
            Description = $"{actor.Name} was seen discarding something near {disposalLoc.Name}",
            LocationId = disposalLoc.Id,
            ActorIds = [actor.Id],
            ObjectIds = state.WeaponToDisposeId != null ? [state.WeaponToDisposeId] : [],
            Relevance = EventRelevance.KeyEvent,
            RelatedCrimeId = state.CrimeId
        });
    }

    private static void ResolveCleanUp(Actor actor, WorldState world)
    {
        var state = actor.ActiveCrimeState!;
        state.CleanedUp = true;

        int hour = world.CurrentTime.Hour;
        bool isOddHour = hour < 5 || hour > 23; // suspicious timing

        string desc = isOddHour
            ? $"A neighbor noticed {actor.Name} doing laundry at an unusual hour"
            : $"{actor.Name} was seen cleaning up at home";

        world.Events.Add(new SimEvent
        {
            Id = $"evt-{(world.Events.Count + 1):D5}",
            Timestamp = world.CurrentTime,
            Type = SimEventType.CleanedUp,
            Description = desc,
            LocationId = actor.HomeLocationId,
            ActorIds = [actor.Id],
            Relevance = isOddHour ? EventRelevance.Minor : EventRelevance.Background,
            RelatedCrimeId = state.CrimeId
        });
    }

    private static void ResolveEstablishAlibi(Actor actor, WorldState world)
    {
        var state = actor.ActiveCrimeState!;
        state.EstablishedAlibi = true;

        // Go to a public place to be "seen"
        var publicTypes = new[] { LocationType.Bar, LocationType.Restaurant, LocationType.Store };
        var publicLocations = world.Locations.Values
            .Where(l => publicTypes.Contains(l.Type) && l.Id != state.CrimeLocationId)
            .ToList();

        if (publicLocations.Count > 0)
        {
            var target = publicLocations[world.Rng.Next(publicLocations.Count)];
            MoveActor(actor, target.Id, world);

            world.Events.Add(new SimEvent
            {
                Id = $"evt-{(world.Events.Count + 1):D5}",
                Timestamp = world.CurrentTime,
                Type = SimEventType.EstablishedAlibi,
                Description = $"{actor.Name} made a point of being seen at {target.Name}",
                LocationId = target.Id,
                ActorIds = [actor.Id],
                Relevance = EventRelevance.Minor,
                RelatedCrimeId = state.CrimeId
            });
        }
    }

    private static void ResolveConfideInFriend(Actor actor, WorldState world)
    {
        var state = actor.ActiveCrimeState!;

        var friendRel = world.GetActorRelationships(actor.Id)
            .Where(r => r.Type == RelationshipType.Friend && r.Strength > 50f)
            .OrderByDescending(r => r.Strength)
            .FirstOrDefault();

        if (friendRel == null) return;

        string friendId = world.GetOtherActorId(friendRel, actor.Id);
        if (!world.Actors.TryGetValue(friendId, out var friend) || !friend.IsAlive) return;

        state.ConfidedInFriend = true;

        // Move to friend's location
        MoveActor(actor, friend.CurrentLocationId, world);

        world.Events.Add(new SimEvent
        {
            Id = $"evt-{(world.Events.Count + 1):D5}",
            Timestamp = world.CurrentTime,
            Type = SimEventType.ConfidedInFriend,
            Description = $"{actor.Name} had a private conversation with {friend.Name}",
            LocationId = friend.CurrentLocationId,
            ActorIds = [actor.Id, friendId],
            Relevance = EventRelevance.KeyEvent,
            RelatedCrimeId = state.CrimeId
        });
    }

    private static void MoveActor(Actor actor, string targetLocationId, WorldState world)
    {
        if (string.IsNullOrEmpty(targetLocationId)) return;
        if (actor.CurrentLocationId == targetLocationId) return;

        // Remove from old location
        if (!string.IsNullOrEmpty(actor.CurrentLocationId)
            && world.Locations.TryGetValue(actor.CurrentLocationId, out var oldLoc))
        {
            oldLoc.ActorIdsPresent.Remove(actor.Id);
        }

        // Add to new location
        if (world.Locations.TryGetValue(targetLocationId, out var newLoc))
        {
            if (!newLoc.ActorIdsPresent.Contains(actor.Id))
            {
                newLoc.ActorIdsPresent.Add(actor.Id);
            }

            // Fingerprints: 10% chance per object at location
            foreach (var objId in newLoc.ObjectIds)
            {
                if (world.Rng.NextDouble() < 0.10
                    && world.Objects.TryGetValue(objId, out var obj)
                    && !obj.Fingerprints.Contains(actor.Id))
                {
                    obj.Fingerprints.Add(actor.Id);
                }
            }
        }

        actor.CurrentLocationId = targetLocationId;

        // Only log movement events 5% of the time to keep event log manageable
        if (world.Rng.NextDouble() < 0.05)
        {
            world.Events.Add(new SimEvent
            {
                Id = $"evt-{(world.Events.Count + 1):D5}",
                Timestamp = world.CurrentTime,
                Type = SimEventType.Movement,
                Description = $"{actor.Name} moved to {(newLoc?.Name ?? targetLocationId)}",
                LocationId = targetLocationId,
                ActorIds = [actor.Id],
                Relevance = EventRelevance.Background
            });
        }
    }
}
