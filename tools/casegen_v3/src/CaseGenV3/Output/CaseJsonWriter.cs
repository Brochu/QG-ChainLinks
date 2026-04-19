using System.Text.Json;
using System.Text.Json.Serialization;
using CaseGenV3.Models;

namespace CaseGenV3.Output;

public static class CaseJsonWriter
{
    private static readonly JsonSerializerOptions JsonOptions = new()
    {
        WriteIndented = true,
        PropertyNamingPolicy = JsonNamingPolicy.SnakeCaseLower,
        DefaultIgnoreCondition = JsonIgnoreCondition.WhenWritingNull,
    };

    /// <summary>
    /// Builds and writes the full case JSON to the given output path.
    /// </summary>
    public static void WriteCaseJson(
        Crime selectedCrime, WorldState world, string caseName, int seed, string outputPath)
    {
        var output = BuildCaseOutput(selectedCrime, world, caseName, seed);
        var json = JsonSerializer.Serialize(output, JsonOptions);
        File.WriteAllText(outputPath, json);
    }

    private static CaseOutputRoot BuildCaseOutput(
        Crime crime, WorldState world, string caseName, int seed)
    {
        return new CaseOutputRoot(
            Case: BuildCaseInfo(crime, world, caseName, seed),
            Locations: BuildLocations(crime, world),
            Actors: BuildActors(crime, world),
            Objects: BuildObjects(crime, world),
            Evidence: BuildEvidence(crime, world),
            Timeline: BuildTimeline(crime, world),
            Solution: BuildSolution(crime, world)
        );
    }

    private static CaseInfo BuildCaseInfo(Crime crime, WorldState world, string caseName, int seed)
    {
        string difficulty = crime.Score switch
        {
            < 40f => "easy",
            < 70f => "medium",
            _ => "hard",
        };

        var locationName = world.Locations.TryGetValue(crime.LocationId, out var loc)
            ? loc.Name
            : crime.LocationId;

        return new CaseInfo(
            Id: crime.Id,
            Name: caseName,
            Seed: seed,
            Difficulty: difficulty,
            Crime: new CrimeInfo(
                Type: crime.Type.ToString(),
                Timestamp: crime.Timestamp,
                Location: locationName,
                Method: crime.Method?.ToString(),
                Motive: crime.Motive,
                PerpetratorId: crime.PerpetratorId,
                VictimId: crime.VictimId
            )
        );
    }

    private static List<LocationInfo> BuildLocations(Crime crime, WorldState world)
    {
        var relevantLocationIds = new HashSet<string> { crime.LocationId };

        // Add connected locations to the crime scene
        if (world.Locations.TryGetValue(crime.LocationId, out var crimeLoc))
        {
            foreach (var connId in crimeLoc.ConnectedLocationIds)
                relevantLocationIds.Add(connId);
        }

        // Add locations where key events happened
        var keyEvents = world.Events
            .Where(e => e.Relevance is EventRelevance.KeyEvent or EventRelevance.CrimeEvent);
        foreach (var evt in keyEvents)
        {
            if (!string.IsNullOrEmpty(evt.LocationId))
                relevantLocationIds.Add(evt.LocationId);
        }

        return relevantLocationIds
            .Where(id => world.Locations.ContainsKey(id))
            .Select(id =>
            {
                var loc = world.Locations[id];
                return new LocationInfo(
                    Id: loc.Id,
                    Name: loc.Name,
                    Type: loc.Type.ToString(),
                    ConnectedLocationIds: loc.ConnectedLocationIds
                );
            })
            .ToList();
    }

    private static List<ActorInfo> BuildActors(Crime crime, WorldState world)
    {
        // Collect all involved actor IDs
        var involvedIds = new HashSet<string> { crime.PerpetratorId, crime.VictimId };
        foreach (var wId in crime.WitnessIds)
            involvedIds.Add(wId);

        // Add actors pointed to by evidence (suspects / red herring targets)
        foreach (var evId in crime.EvidenceIds)
        {
            var ev = world.Evidence.FirstOrDefault(e => e.Id == evId);
            if (ev?.PointsToActorId != null)
                involvedIds.Add(ev.PointsToActorId);
        }

        // Filter to only actors that exist in the world
        var involvedActors = involvedIds
            .Where(id => world.Actors.ContainsKey(id))
            .Select(id => world.Actors[id])
            .ToList();

        return involvedActors.Select(actor =>
        {
            string role = actor.Id == crime.VictimId ? "victim"
                : actor.Id == crime.PerpetratorId ? "suspect"
                : crime.WitnessIds.Contains(actor.Id) ? "witness"
                : "person_of_interest";

            // Build relationships to other involved actors
            var rels = world.Relationships
                .Where(r => (r.ActorAId == actor.Id || r.ActorBId == actor.Id)
                            && involvedIds.Contains(r.ActorAId)
                            && involvedIds.Contains(r.ActorBId))
                .Select(r => new ActorRelationshipInfo(
                    ActorId: world.GetOtherActorId(r, actor.Id),
                    Type: r.Type.ToString(),
                    Strength: r.Strength
                ))
                .ToList();

            return new ActorInfo(
                Id: actor.Id,
                Name: actor.Name,
                Role: role,
                Occupation: actor.Occupation,
                Traits: new ActorTraitsInfo(
                    Temper: actor.Traits.Temper,
                    Greed: actor.Traits.Greed,
                    Jealousy: actor.Traits.Jealousy,
                    Loyalty: actor.Traits.Loyalty,
                    Impulsivity: actor.Traits.Impulsivity
                ),
                Relationships: rels
            );
        }).ToList();
    }

    private static List<ObjectInfo> BuildObjects(Crime crime, WorldState world)
    {
        var relevantObjectIds = new HashSet<string>();

        // Weapon
        if (!string.IsNullOrEmpty(crime.WeaponId))
            relevantObjectIds.Add(crime.WeaponId);

        // Objects referenced by evidence
        foreach (var evId in crime.EvidenceIds)
        {
            var ev = world.Evidence.FirstOrDefault(e => e.Id == evId);
            if (ev?.Type == EvidenceType.Physical && ev.LocationId == crime.LocationId)
            {
                // Find objects at the crime scene that this evidence may relate to
                if (world.Locations.TryGetValue(crime.LocationId, out var loc))
                {
                    foreach (var objId in loc.ObjectIds)
                        relevantObjectIds.Add(objId);
                }
            }
        }

        // Also include objects involved in key/crime events
        foreach (var evt in world.Events.Where(e =>
            e.Relevance is EventRelevance.KeyEvent or EventRelevance.CrimeEvent))
        {
            foreach (var objId in evt.ObjectIds)
                relevantObjectIds.Add(objId);
        }

        return relevantObjectIds
            .Where(id => world.Objects.ContainsKey(id))
            .Select(id =>
            {
                var obj = world.Objects[id];
                return new ObjectInfo(
                    Id: obj.Id,
                    Name: obj.Name,
                    Type: obj.Type.ToString(),
                    SubType: obj.SubType.ToString(),
                    LocationId: obj.LocationId,
                    HeldByActorId: obj.HeldByActorId,
                    Fingerprints: obj.Fingerprints
                );
            })
            .ToList();
    }

    private static List<EvidenceInfo> BuildEvidence(Crime crime, WorldState world)
    {
        return crime.EvidenceIds
            .Select(id => world.Evidence.FirstOrDefault(e => e.Id == id))
            .Where(e => e != null)
            .Select(e => new EvidenceInfo(
                Id: e!.Id,
                Type: e.Type.ToString(),
                Description: e.Description,
                LocationId: e.LocationId,
                PointsToActorId: e.PointsToActorId,
                IsRedHerring: e.IsRedHerring,
                Discoverable: e.Discoverable,
                WitnessActorId: e.WitnessActorId
            ))
            .ToList();
    }

    private static List<TimelineEvent> BuildTimeline(Crime crime, WorldState world)
    {
        return world.Events
            .Where(e => e.Relevance is not EventRelevance.Background)
            .Where(e => e.RelatedCrimeId == crime.Id
                        || e.ActorIds.Contains(crime.PerpetratorId)
                        || e.ActorIds.Contains(crime.VictimId))
            .OrderBy(e => e.Timestamp)
            .Select(e => new TimelineEvent(
                Timestamp: e.Timestamp,
                Type: e.Type.ToString(),
                Description: e.Description,
                ActorsInvolved: e.ActorIds,
                LocationId: e.LocationId,
                Relevance: e.Relevance.ToString()
            ))
            .ToList();
    }

    private static SolutionInfo BuildSolution(Crime crime, WorldState world)
    {
        // Critical evidence: non-red-herring evidence that forms the proof chain
        var criticalEvidenceIds = crime.EvidenceIds
            .Select(id => world.Evidence.FirstOrDefault(e => e.Id == id))
            .Where(e => e != null && !e.IsRedHerring)
            .Select(e => e!.Id)
            .ToList();

        // Build evidence chain description
        var chainParts = new List<string>();
        foreach (var evId in criticalEvidenceIds)
        {
            var ev = world.Evidence.First(e => e.Id == evId);
            chainParts.Add($"[{ev.Type}] {ev.Description}");
        }

        var perpetratorName = world.Actors.TryGetValue(crime.PerpetratorId, out var perp)
            ? perp.Name
            : crime.PerpetratorId;

        string evidenceChain = chainParts.Count > 0
            ? $"{perpetratorName} committed the crime. The evidence chain: {string.Join(" -> ", chainParts)}"
            : $"{perpetratorName} committed the crime. Motive: {crime.Motive}.";

        return new SolutionInfo(
            KillerId: crime.PerpetratorId,
            Motive: crime.Motive,
            CriticalEvidenceIds: criticalEvidenceIds,
            EvidenceChain: evidenceChain
        );
    }
}
