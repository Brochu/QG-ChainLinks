using CaseGenV3.Models;

namespace CaseGenV3.Simulation;

public static class RelationshipSystem
{
    private static int _eventCounter;

    public static void ProcessInteractions(WorldState world)
    {
        var processedPairs = new HashSet<string>();

        // --- Process co-located actors ---
        foreach (var location in world.Locations.Values)
        {
            var actorsHere = world.GetActorsAtLocation(location.Id);
            if (actorsHere.Count < 2) continue;

            for (int i = 0; i < actorsHere.Count; i++)
            {
                for (int j = i + 1; j < actorsHere.Count; j++)
                {
                    var a = actorsHere[i];
                    var b = actorsHere[j];

                    string pairKey = string.Compare(a.Id, b.Id, StringComparison.Ordinal) < 0
                        ? $"{a.Id}:{b.Id}" : $"{b.Id}:{a.Id}";

                    if (!processedPairs.Add(pairKey)) continue;

                    var rel = world.GetOrCreateRelationship(a.Id, b.Id);

                    // --- Socializing together strengthens bond (low probability per tick) ---
                    bool isSocialLocation = location.Type is LocationType.Bar
                                            or LocationType.Restaurant
                                            or LocationType.Park;

                    if (isSocialLocation && world.Rng.NextDouble() < 0.05) // 5% chance per tick
                    {
                        float bondAmount = 0.3f + (float)(world.Rng.NextDouble() * 0.7); // 0.3-1.0
                        rel.Strength = Math.Min(100f, rel.Strength + bondAmount);

                        // At a bar with low Social need -> small chance of forming friendship
                        if (location.Type == LocationType.Bar
                            && rel.Type == RelationshipType.Stranger)
                        {
                            bool aLowSocial = a.Needs.TryGetValue(NeedType.Social, out var aSoc) && aSoc.Value < 40f;
                            bool bLowSocial = b.Needs.TryGetValue(NeedType.Social, out var bSoc) && bSoc.Value < 40f;

                            if (aLowSocial && bLowSocial && world.Rng.NextDouble() < 0.02)
                            {
                                rel.Type = RelationshipType.Friend;
                                rel.Strength = Math.Max(rel.Strength, 20f);
                                rel.History.Add($"Became friends at {location.Name}");

                                world.Events.Add(new SimEvent
                                {
                                    Id = NextEventId(world),
                                    Timestamp = world.CurrentTime,
                                    Type = SimEventType.RelationshipFormed,
                                    Description = $"{a.Name} and {b.Name} became friends at {location.Name}",
                                    LocationId = location.Id,
                                    ActorIds = [a.Id, b.Id],
                                    Relevance = EventRelevance.Minor
                                });
                            }
                        }
                    }

                    // --- Jealousy: partner socializing with non-partner ---
                    CheckJealousy(a, b, rel, world, location);
                    CheckJealousy(b, a, rel, world, location);

                    // --- Degrade relationships with high tension ---
                    if (rel.Tension > 50f)
                    {
                        float degradeAmount = 1f + (float)(world.Rng.NextDouble() * 1.0); // 1-2
                        rel.Strength = Math.Max(-100f, rel.Strength - degradeAmount);
                    }
                }
            }
        }

        // --- Degrade relationships with high tension even when apart, and build tension from low strength ---
        foreach (var rel in world.Relationships)
        {
            if (!world.Actors.TryGetValue(rel.ActorAId, out var actA) || !actA.IsAlive) continue;
            if (!world.Actors.TryGetValue(rel.ActorBId, out var actB) || !actB.IsAlive) continue;

            if (rel.Tension > 50f && actA.CurrentLocationId != actB.CurrentLocationId)
            {
                float degradeAmount = 0.5f + (float)(world.Rng.NextDouble() * 0.5);
                rel.Strength = Math.Max(-100f, rel.Strength - degradeAmount);
            }

            // Low-strength relationships naturally build tension over time
            if (rel.Strength < -30f && rel.Type != RelationshipType.Stranger)
            {
                float tensionFromHatred = (-rel.Strength - 30f) / 700f; // 0 to 0.1 per tick
                rel.Tension = Math.Min(100f, rel.Tension + tensionFromHatred);
            }

            // Natural tension decay for low-tension relationships (people cool off)
            if (rel.Tension > 0f && rel.Tension < 30f)
            {
                rel.Tension = Math.Max(0f, rel.Tension - 0.01f);
            }
        }
    }

    private static void CheckJealousy(Actor actor, Actor other, Relationship theirRel,
        WorldState world, Location location)
    {
        // Find if actor has a spouse
        var spouseRel = world.GetActorRelationships(actor.Id)
            .FirstOrDefault(r => r.Type == RelationshipType.Spouse
                              && world.GetOtherActorId(r, actor.Id) != other.Id);

        if (spouseRel == null) return;

        string spouseId = world.GetOtherActorId(spouseRel, actor.Id);

        // Actor with spouse is socializing with someone else at a social location
        bool isSocialLocation = location.Type is LocationType.Bar
                                or LocationType.Restaurant
                                or LocationType.Park;
        if (!isSocialLocation) return;

        // Only trigger jealousy occasionally (1% per tick ≈ a few times per day at a social location)
        if (world.Rng.NextDouble() > 0.01) return;

        // Increase tension with spouse — scaled by spouse's jealousy trait
        if (!world.Actors.TryGetValue(spouseId, out var spouseActor) || !spouseActor.IsAlive) return;
        float jealousyMod = Math.Max(0.1f, (spouseActor.Traits.Jealousy + 1f) / 2f); // 0.1-1.0
        float tensionIncrease = (0.2f + (float)(world.Rng.NextDouble() * 0.5)) * jealousyMod;
        spouseRel.Tension = Math.Min(100f, spouseRel.Tension + tensionIncrease);

        // Detect cheating: if actor has a Partner-type relationship with the other person
        if (theirRel.Type == RelationshipType.Partner)
        {
            // Check if spouse is nearby (same location) or rare chance of "hearing about it"
            bool spouseNearby = spouseActor.CurrentLocationId == location.Id;
            bool heardsAboutIt = world.Rng.NextDouble() < 0.01; // 1% chance per tick

            if (spouseNearby || heardsAboutIt)
            {
                spouseRel.Tension = Math.Min(100f, spouseRel.Tension + 20f);
                spouseRel.History.Add("Discovered cheating");

                world.Events.Add(new SimEvent
                {
                    Id = NextEventId(world),
                    Timestamp = world.CurrentTime,
                    Type = SimEventType.CheatingDiscovered,
                    Description = $"{(spouseNearby ? "Witnessed" : "Heard about")} {actor.Name} cheating with {other.Name}",
                    LocationId = location.Id,
                    ActorIds = [actor.Id, other.Id, spouseId],
                    Relevance = EventRelevance.KeyEvent
                });
            }
        }
    }

    private static string NextEventId(WorldState world)
    {
        _eventCounter = world.Events.Count + 1;
        return $"evt-{_eventCounter:D5}";
    }
}
