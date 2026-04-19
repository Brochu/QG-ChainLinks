using CaseGenV3.Models;

namespace CaseGenV3.Simulation;

public static class ConflictSystem
{
    // Track last conflict time per relationship to enforce cooldowns
    private static readonly Dictionary<string, DateTime> _lastConflictTime = new();
    // Track last crime time per relationship (fallback cooldown for non-murder crimes)
    private static readonly Dictionary<string, DateTime> _lastCrimeTime = new();

    private static string RelKey(Relationship rel) =>
        string.Compare(rel.ActorAId, rel.ActorBId, StringComparison.Ordinal) < 0
            ? $"{rel.ActorAId}:{rel.ActorBId}" : $"{rel.ActorBId}:{rel.ActorAId}";

    private static bool IsOnCooldown(Relationship rel, WorldState world, TimeSpan cooldown)
    {
        var key = RelKey(rel);
        if (_lastConflictTime.TryGetValue(key, out var last))
            return (world.CurrentTime - last) < cooldown;
        return false;
    }

    private static bool IsOnCrimeCooldown(Relationship rel, WorldState world)
    {
        var key = RelKey(rel);
        // 7-day cooldown after any crime (assault or murder attempt)
        if (_lastCrimeTime.TryGetValue(key, out var last))
            return (world.CurrentTime - last) < TimeSpan.FromDays(7);
        return false;
    }

    private static void SetCooldown(Relationship rel, WorldState world)
    {
        _lastConflictTime[RelKey(rel)] = world.CurrentTime;
    }

    private static void SetCrimeCooldown(Relationship rel, WorldState world)
    {
        _lastCrimeTime[RelKey(rel)] = world.CurrentTime;
    }

    public static void ProcessConflicts(WorldState world)
    {
        // Work on a snapshot of relationships to avoid modification during iteration
        var relationships = world.Relationships.ToList();

        foreach (var rel in relationships)
        {
            if (rel.Tension <= 30f) continue;

            if (!world.Actors.TryGetValue(rel.ActorAId, out var actorA) || !actorA.IsAlive) continue;
            if (!world.Actors.TryGetValue(rel.ActorBId, out var actorB) || !actorB.IsAlive) continue;

            // Skip if either actor is in post-crime mode — they're busy hiding/covering up
            if (actorA.ActiveCrimeState != null || actorB.ActiveCrimeState != null) continue;

            bool sameLocation = actorA.CurrentLocationId == actorB.CurrentLocationId;

            // --- Escalation based on tension thresholds ---
            if (rel.Tension >= 90f)
            {
                rel.EscalationLevel = EscalationLevel.Violence;
                // Violence: 48h cooldown, 7-day crime cooldown, 5% chance
                if (sameLocation && !IsOnCooldown(rel, world, TimeSpan.FromHours(48))
                    && !IsOnCrimeCooldown(rel, world)
                    && world.Rng.NextDouble() < 0.05)
                {
                    SetCooldown(rel, world);
                    ProcessViolence(actorA, actorB, rel, world);
                }
            }
            else if (rel.Tension >= 80f)
            {
                rel.EscalationLevel = EscalationLevel.Confrontation;
                // Confrontation: 24h cooldown, 7-day crime cooldown, 5% chance
                if (sameLocation && !IsOnCooldown(rel, world, TimeSpan.FromHours(24))
                    && !IsOnCrimeCooldown(rel, world)
                    && world.Rng.NextDouble() < 0.05)
                {
                    SetCooldown(rel, world);
                    ProcessConfrontation(actorA, actorB, rel, world);
                }
            }
            else if (rel.Tension >= 65f)
            {
                rel.EscalationLevel = EscalationLevel.Threat;
                // Threats have 6-hour cooldown, 5% chance
                if (sameLocation && !IsOnCooldown(rel, world, TimeSpan.FromHours(6))
                    && world.Rng.NextDouble() < 0.05)
                {
                    SetCooldown(rel, world);
                    GenerateEvent(world, SimEventType.Threaten,
                        $"{actorA.Name} threatened {actorB.Name}",
                        actorA.CurrentLocationId, [actorA.Id, actorB.Id],
                        EventRelevance.KeyEvent);
                    rel.History.Add($"Threat at {world.CurrentTime}");
                }
            }
            else if (rel.Tension >= 50f)
            {
                rel.EscalationLevel = EscalationLevel.Argument;
                // Arguments have 2-hour cooldown, 5% chance
                if (sameLocation && !IsOnCooldown(rel, world, TimeSpan.FromHours(2))
                    && world.Rng.NextDouble() < 0.05)
                {
                    SetCooldown(rel, world);
                    GenerateEvent(world, SimEventType.Argue,
                        $"{actorA.Name} argued with {actorB.Name}",
                        actorA.CurrentLocationId, [actorA.Id, actorB.Id],
                        EventRelevance.Minor);
                    rel.History.Add($"Argument at {world.CurrentTime}");
                    rel.Tension = Math.Min(100f, rel.Tension + 2f);
                }
            }
            else // 30-50
            {
                rel.EscalationLevel = EscalationLevel.ColdShoulder;
            }
        }
    }

    private static void ProcessConfrontation(Actor actorA, Actor actorB, Relationship rel, WorldState world)
    {
        GenerateEvent(world, SimEventType.Confront,
            $"{actorA.Name} confronted {actorB.Name}",
            actorA.CurrentLocationId, [actorA.Id, actorB.Id],
            EventRelevance.KeyEvent);

        rel.History.Add($"Confrontation at {world.CurrentTime}");

        // Chance of attack during confrontation
        float attackChance = Math.Max(actorA.Traits.Impulsivity, actorB.Traits.Impulsivity) * 0.2f;
        if (world.Rng.NextDouble() < attackChance)
        {
            // Determine aggressor (higher temper + impulsivity)
            var aggressor = (actorA.Traits.Temper + actorA.Traits.Impulsivity)
                            >= (actorB.Traits.Temper + actorB.Traits.Impulsivity)
                            ? actorA : actorB;
            var victim = aggressor == actorA ? actorB : actorA;

            CreateAssaultCrime(aggressor, victim, rel, world);
        }
    }

    private static void ProcessViolence(Actor actorA, Actor actorB, Relationship rel, WorldState world)
    {
        // Determine aggressor
        var aggressor = (actorA.Traits.Temper + actorA.Traits.Impulsivity)
                        >= (actorB.Traits.Temper + actorB.Traits.Impulsivity)
                        ? actorA : actorB;
        var victim = aggressor == actorA ? actorB : actorA;

        // Murder check: Temper > 0.5 AND Impulsivity > 0.3 AND fails control check
        bool canMurder = aggressor.Traits.Temper > 0.5f && aggressor.Traits.Impulsivity > 0.3f;
        double controlCheck = world.Rng.NextDouble();
        double murderThreshold = aggressor.Traits.Impulsivity * aggressor.Traits.Temper;

        if (canMurder && controlCheck < murderThreshold)
        {
            CreateMurderCrime(aggressor, victim, rel, world);
        }
        else
        {
            // Attack but not murder
            CreateAssaultCrime(aggressor, victim, rel, world);
        }
    }

    private static void CreateMurderCrime(Actor aggressor, Actor victim, Relationship rel, WorldState world)
    {
        // Find weapon at location or in aggressor inventory
        var (weapon, method) = FindWeaponAndMethod(aggressor, world);

        string crimeId = $"crime-{(world.Crimes.Count + 1):D3}";

        // Determine motive from relationship history and tension
        string motive = DetermineMotive(rel, aggressor, world);

        // Find witnesses (other actors at the same location)
        var witnesses = world.GetActorsAtLocation(aggressor.CurrentLocationId)
            .Where(a => a.Id != aggressor.Id && a.Id != victim.Id)
            .Select(a => a.Id)
            .ToList();

        var crime = new Crime
        {
            Id = crimeId,
            Type = CrimeType.Murder,
            Timestamp = world.CurrentTime,
            LocationId = aggressor.CurrentLocationId,
            PerpetratorId = aggressor.Id,
            VictimId = victim.Id,
            Method = method,
            WeaponId = weapon?.Id,
            Motive = motive,
            MotiveFactors = BuildMotiveFactors(rel),
            WitnessIds = witnesses
        };

        world.Crimes.Add(crime);

        // Kill the victim
        victim.IsAlive = false;

        // Add fingerprints on weapon if used
        if (weapon != null && !weapon.Fingerprints.Contains(aggressor.Id))
        {
            weapon.Fingerprints.Add(aggressor.Id);
        }

        rel.EscalationLevel = EscalationLevel.Murderous;
        rel.History.Add($"Murder at {world.CurrentTime}");
        SetCrimeCooldown(rel, world);

        // Enter post-crime behavior mode
        aggressor.ActiveCrimeState = new CrimeState
        {
            CrimeId = crimeId,
            CrimeTime = world.CurrentTime,
            CrimeLocationId = aggressor.CurrentLocationId,
            Phase = CrimePhase.Panic,
            WeaponToDisposeId = weapon?.Id
        };

        // If aggressor picked up the weapon, add to inventory
        if (weapon != null && !aggressor.Inventory.Contains(weapon.Id))
        {
            aggressor.Inventory.Add(weapon.Id);
            weapon.HeldByActorId = aggressor.Id;
            if (world.Locations.TryGetValue(aggressor.CurrentLocationId, out var loc))
                loc.ObjectIds.Remove(weapon.Id);
        }

        GenerateEvent(world, SimEventType.Murder,
            $"{aggressor.Name} murdered {victim.Name}{(weapon != null ? $" with {weapon.Name}" : "")}",
            aggressor.CurrentLocationId, [aggressor.Id, victim.Id],
            EventRelevance.CrimeEvent,
            weapon != null ? [weapon.Id] : [],
            crimeId);
    }

    private static void CreateAssaultCrime(Actor aggressor, Actor victim, Relationship rel, WorldState world)
    {
        string crimeId = $"crime-{(world.Crimes.Count + 1):D3}";

        var crime = new Crime
        {
            Id = crimeId,
            Type = CrimeType.Assault,
            Timestamp = world.CurrentTime,
            LocationId = aggressor.CurrentLocationId,
            PerpetratorId = aggressor.Id,
            VictimId = victim.Id,
            Motive = DetermineMotive(rel, aggressor, world),
            MotiveFactors = BuildMotiveFactors(rel),
            WitnessIds = world.GetActorsAtLocation(aggressor.CurrentLocationId)
                .Where(a => a.Id != aggressor.Id && a.Id != victim.Id)
                .Select(a => a.Id).ToList()
        };

        world.Crimes.Add(crime);
        rel.History.Add($"Assault at {world.CurrentTime}");

        // Assault vents significant tension but relationship gets worse
        rel.Tension = Math.Max(40f, rel.Tension - 30f);
        rel.Strength = Math.Max(-100f, rel.Strength - 15f);
        SetCrimeCooldown(rel, world);

        // Enter post-crime behavior mode (shorter than murder — skip to Paranoia after brief panic)
        aggressor.ActiveCrimeState = new CrimeState
        {
            CrimeId = crimeId,
            CrimeTime = world.CurrentTime,
            CrimeLocationId = aggressor.CurrentLocationId,
            Phase = CrimePhase.Panic,
            // No weapon disposal needed for assault
            FledScene = false,
            DisposedWeapon = true, // nothing to dispose
            CleanedUp = true       // no cleanup needed
        };

        GenerateEvent(world, SimEventType.Attack,
            $"{aggressor.Name} attacked {victim.Name}",
            aggressor.CurrentLocationId, [aggressor.Id, victim.Id],
            EventRelevance.CrimeEvent, relatedCrimeId: crimeId);
    }

    private static (WorldObject? weapon, MurderMethod method) FindWeaponAndMethod(Actor aggressor, WorldState world)
    {
        // Check aggressor's inventory first
        foreach (var objId in aggressor.Inventory)
        {
            if (world.Objects.TryGetValue(objId, out var obj) && obj.Type == ObjectType.Weapon)
            {
                return (obj, SubTypeToMethod(obj.SubType));
            }
        }

        // Check objects at the location
        if (world.Locations.TryGetValue(aggressor.CurrentLocationId, out var location))
        {
            foreach (var objId in location.ObjectIds)
            {
                if (world.Objects.TryGetValue(objId, out var obj) && obj.Type == ObjectType.Weapon)
                {
                    return (obj, SubTypeToMethod(obj.SubType));
                }
            }
        }

        // No weapon found: strangulation
        return (null, MurderMethod.Strangulation);
    }

    private static MurderMethod SubTypeToMethod(ObjectSubType subType) => subType switch
    {
        ObjectSubType.Knife => MurderMethod.Stabbing,
        ObjectSubType.Gun => MurderMethod.Shooting,
        ObjectSubType.BluntObject => MurderMethod.Bludgeoning,
        ObjectSubType.Poison => MurderMethod.Poisoning,
        ObjectSubType.Rope => MurderMethod.Strangulation,
        _ => MurderMethod.Bludgeoning
    };

    private static string DetermineMotive(Relationship rel, Actor aggressor, WorldState world)
    {
        if (rel.History.Contains("Discovered cheating"))
            return "Jealous rage after discovering infidelity";
        if (rel.Type == RelationshipType.Rival)
            return "Rivalry escalated to violence";
        if (aggressor.Traits.Greed > 0.5f)
            return "Greed-driven conflict";
        if (rel.Tension > 80f)
            return "Long-standing tension boiled over";
        return "Escalating personal conflict";
    }

    private static List<string> BuildMotiveFactors(Relationship rel)
    {
        var factors = new List<string>();
        if (rel.Tension > 80f) factors.Add("High tension");
        if (rel.History.Contains("Discovered cheating")) factors.Add("Infidelity");
        if (rel.History.Any(h => h.Contains("Argument"))) factors.Add("Repeated arguments");
        if (rel.History.Any(h => h.Contains("Threat"))) factors.Add("Previous threats");
        if (rel.Type == RelationshipType.Rival) factors.Add("Rivalry");
        if (rel.Type == RelationshipType.Ex) factors.Add("Failed relationship");
        return factors;
    }

    private static void GenerateEvent(WorldState world, SimEventType type, string description,
        string locationId, List<string> actorIds, EventRelevance relevance,
        List<string>? objectIds = null, string? relatedCrimeId = null)
    {
        world.Events.Add(new SimEvent
        {
            Id = $"evt-{(world.Events.Count + 1):D5}",
            Timestamp = world.CurrentTime,
            Type = type,
            Description = description,
            LocationId = locationId,
            ActorIds = actorIds,
            ObjectIds = objectIds ?? [],
            Relevance = relevance,
            RelatedCrimeId = relatedCrimeId
        });
    }
}
