using CaseGenV3.Models;

namespace CaseGenV3.PostProcess;

public static class RedHerringInjector
{
    private static readonly string[] SuspiciousBehaviors =
    [
        "was seen pacing nervously near {location}",
        "left {location} in a hurry shortly after the incident",
        "was spotted acting strangely outside {location}",
        "was overheard making a phone call near {location} and seemed agitated",
        "was seen looking around furtively near {location}",
    ];

    private static readonly string[] MisleadingObjectNames =
    [
        "Unmarked Knife",
        "Stained Gloves",
        "Discarded Lighter",
        "Broken Phone",
        "Torn Fabric",
        "Empty Vial",
        "Bloodied Rag",
    ];

    /// <summary>
    /// Injects red herring evidence into the world to make the case harder to solve.
    /// </summary>
    public static void InjectRedHerrings(Crime crime, WorldState world)
    {
        var rng = world.Rng;
        int nextEvidenceId = world.Evidence.Count;

        nextEvidenceId = InjectMotiveWithoutOpportunity(crime, world, rng, nextEvidenceId);
        nextEvidenceId = InjectOpportunityWithoutMotive(crime, world, rng, nextEvidenceId);
        InjectMisleadingObjects(crime, world, rng, nextEvidenceId);
    }

    /// <summary>
    /// Actors with motive (negative relationship with victim) but no opportunity
    /// (not near crime scene on the day of the crime).
    /// </summary>
    private static int InjectMotiveWithoutOpportunity(
        Crime crime, WorldState world, Random rng, int nextEvidenceId)
    {
        var crimeDate = crime.Timestamp.Date;

        // Find actors with negative relationships with the victim
        var actorsWithMotive = world.Relationships
            .Where(r => (r.ActorAId == crime.VictimId || r.ActorBId == crime.VictimId)
                        && r.Strength < -20f)
            .Select(r => r.ActorAId == crime.VictimId ? r.ActorBId : r.ActorAId)
            .Where(id => id != crime.PerpetratorId && id != crime.VictimId)
            .Where(id => world.Actors.ContainsKey(id) && world.Actors[id].IsAlive)
            .ToList();

        // Filter to those who were NOT near the crime scene that day
        var crimeLocation = crime.LocationId;
        var connectedLocations = world.Locations.TryGetValue(crimeLocation, out var loc)
            ? loc.ConnectedLocationIds.ToHashSet()
            : new HashSet<string>();
        connectedLocations.Add(crimeLocation);

        var actorsNearScene = world.Events
            .Where(e => e.Timestamp.Date == crimeDate && connectedLocations.Contains(e.LocationId))
            .SelectMany(e => e.ActorIds)
            .ToHashSet();

        var candidates = actorsWithMotive
            .Where(id => !actorsNearScene.Contains(id))
            .OrderBy(_ => rng.Next())
            .Take(3)
            .ToList();

        foreach (var actorId in candidates)
        {
            var actor = world.Actors[actorId];

            // Create circumstantial evidence pointing to this actor
            var circumstantialEvidence = new Evidence
            {
                Id = $"evidence_{nextEvidenceId++}",
                Type = EvidenceType.Circumstantial,
                Description = $"{actor.Name} had a known grudge against the victim and had recently made threatening remarks.",
                LocationId = crime.LocationId,
                PointsToActorId = actorId,
                IsRedHerring = true,
                Discoverable = true,
                RelatedCrimeId = crime.Id,
            };
            world.Evidence.Add(circumstantialEvidence);
            crime.EvidenceIds.Add(circumstantialEvidence.Id);

            // Create a fake testimonial from an unreliable witness
            var potentialWitnesses = world.Actors.Values
                .Where(a => a.IsAlive && a.Id != actorId && a.Id != crime.PerpetratorId && a.Id != crime.VictimId)
                .OrderBy(_ => rng.Next())
                .FirstOrDefault();

            if (potentialWitnesses != null)
            {
                var testimonial = new Evidence
                {
                    Id = $"evidence_{nextEvidenceId++}",
                    Type = EvidenceType.Testimonial,
                    Description = $"{potentialWitnesses.Name} claims to have seen {actor.Name} near the victim's area on the night in question, though their account is inconsistent.",
                    LocationId = potentialWitnesses.CurrentLocationId,
                    PointsToActorId = actorId,
                    IsRedHerring = true,
                    Discoverable = true,
                    RelatedCrimeId = crime.Id,
                    WitnessActorId = potentialWitnesses.Id,
                };
                world.Evidence.Add(testimonial);
                crime.EvidenceIds.Add(testimonial.Id);
            }
        }

        return nextEvidenceId;
    }

    /// <summary>
    /// Actors with opportunity (near crime scene) but no motive
    /// (no negative relationship with victim).
    /// </summary>
    private static int InjectOpportunityWithoutMotive(
        Crime crime, WorldState world, Random rng, int nextEvidenceId)
    {
        var crimeDate = crime.Timestamp.Date;
        var crimeLocation = crime.LocationId;
        var connectedLocations = world.Locations.TryGetValue(crimeLocation, out var loc)
            ? loc.ConnectedLocationIds.ToHashSet()
            : new HashSet<string>();
        connectedLocations.Add(crimeLocation);

        // Find actors who were near the crime scene that day
        var actorsNearScene = world.Events
            .Where(e => e.Timestamp.Date == crimeDate && connectedLocations.Contains(e.LocationId))
            .SelectMany(e => e.ActorIds)
            .Distinct()
            .Where(id => id != crime.PerpetratorId && id != crime.VictimId)
            .Where(id => world.Actors.ContainsKey(id) && world.Actors[id].IsAlive)
            .ToList();

        // Filter to those with no negative relationship with the victim
        var candidates = actorsNearScene
            .Where(id =>
            {
                var rel = world.GetRelationship(id, crime.VictimId);
                return rel == null || rel.Strength >= 0f;
            })
            .OrderBy(_ => rng.Next())
            .Take(2)
            .ToList();

        var locationName = world.Locations.TryGetValue(crimeLocation, out var crimeLoc)
            ? crimeLoc.Name
            : "the area";

        foreach (var actorId in candidates)
        {
            var actor = world.Actors[actorId];
            var behaviorTemplate = SuspiciousBehaviors[rng.Next(SuspiciousBehaviors.Length)];
            var behavior = behaviorTemplate.Replace("{location}", locationName);

            // Add a suspicious behavior event
            var suspiciousEvent = new SimEvent
            {
                Id = $"event_rh_{nextEvidenceId}",
                Timestamp = crime.Timestamp.AddMinutes(rng.Next(-120, 60)),
                Type = SimEventType.Movement,
                Description = $"{actor.Name} {behavior}.",
                LocationId = crimeLocation,
                ActorIds = [actorId],
                ObjectIds = [],
                RelatedCrimeId = crime.Id,
                Relevance = EventRelevance.KeyEvent,
            };
            world.Events.Add(suspiciousEvent);

            // Create circumstantial evidence for this suspicious behavior
            var evidence = new Evidence
            {
                Id = $"evidence_{nextEvidenceId++}",
                Type = EvidenceType.Circumstantial,
                Description = $"{actor.Name} {behavior}. Multiple witnesses confirm this unusual behavior.",
                LocationId = crimeLocation,
                PointsToActorId = actorId,
                IsRedHerring = true,
                Discoverable = true,
                RelatedCrimeId = crime.Id,
            };
            world.Evidence.Add(evidence);
            crime.EvidenceIds.Add(evidence.Id);
        }

        return nextEvidenceId;
    }

    /// <summary>
    /// Plant misleading physical objects at the crime scene.
    /// </summary>
    private static void InjectMisleadingObjects(
        Crime crime, WorldState world, Random rng, int nextEvidenceId)
    {
        int objectCount = rng.Next(1, 3); // 1 or 2 misleading objects
        int nextObjectId = world.Objects.Count;

        // Pick innocent actors to pin objects on
        var innocentActors = world.Actors.Values
            .Where(a => a.IsAlive && a.Id != crime.PerpetratorId && a.Id != crime.VictimId)
            .OrderBy(_ => rng.Next())
            .Take(objectCount)
            .ToList();

        for (int i = 0; i < objectCount; i++)
        {
            var objectName = MisleadingObjectNames[rng.Next(MisleadingObjectNames.Length)];
            var obj = new WorldObject
            {
                Id = $"obj_{nextObjectId++}",
                Name = objectName,
                Type = ObjectType.Personal,
                SubType = objectName.Contains("Knife") ? ObjectSubType.Knife
                    : objectName.Contains("Gloves") ? ObjectSubType.Gloves
                    : objectName.Contains("Lighter") ? ObjectSubType.Lighter
                    : objectName.Contains("Phone") ? ObjectSubType.Phone
                    : ObjectSubType.Wallet,
                LocationId = crime.LocationId,
                HeldByActorId = null,
            };

            // Add fingerprints from an innocent actor if available
            if (i < innocentActors.Count)
            {
                obj.Fingerprints.Add(innocentActors[i].Id);
            }

            world.Objects[obj.Id] = obj;
            if (world.Locations.TryGetValue(crime.LocationId, out var loc))
            {
                loc.ObjectIds.Add(obj.Id);
            }

            // Create physical evidence for this planted object
            var ownerDesc = i < innocentActors.Count
                ? $" Fingerprints belonging to {innocentActors[i].Name} were found on it."
                : "";

            var evidence = new Evidence
            {
                Id = $"evidence_{nextEvidenceId++}",
                Type = EvidenceType.Physical,
                Description = $"A {objectName.ToLower()} was found at the crime scene.{ownerDesc}",
                LocationId = crime.LocationId,
                PointsToActorId = i < innocentActors.Count ? innocentActors[i].Id : null,
                IsRedHerring = true,
                Discoverable = true,
                RelatedCrimeId = crime.Id,
            };
            world.Evidence.Add(evidence);
            crime.EvidenceIds.Add(evidence.Id);
        }
    }
}
