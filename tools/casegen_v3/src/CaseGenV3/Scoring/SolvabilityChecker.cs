using CaseGenV3.Models;

namespace CaseGenV3.Scoring;

public static class SolvabilityChecker
{
    /// <summary>
    /// Determines if a crime can be solved by following discoverable evidence to the perpetrator.
    /// </summary>
    public static bool IsSolvable(Crime crime, WorldState world)
    {
        return BuildEvidenceChain(crime, world).Count > 0;
    }

    /// <summary>
    /// Builds the shortest valid evidence chain from discoverable evidence to the perpetrator.
    /// A valid chain must include at least:
    ///   - One Physical or Forensic evidence at the crime scene
    ///   - One Testimonial from a witness OR one Circumstantial pointing to motive
    /// Returns an empty list if no valid chain exists.
    /// </summary>
    public static List<Evidence> BuildEvidenceChain(Crime crime, WorldState world)
    {
        // Gather all crime evidence that is discoverable and not a red herring
        var usableEvidence = world.Evidence
            .Where(e => crime.EvidenceIds.Contains(e.Id) && e.Discoverable && !e.IsRedHerring)
            .ToList();

        // Find physical/forensic evidence at the crime scene
        var sceneEvidence = usableEvidence
            .Where(e => e.LocationId == crime.LocationId &&
                        (e.Type == EvidenceType.Physical || e.Type == EvidenceType.Forensic))
            .ToList();

        // Find testimonial evidence from witnesses
        var testimonialEvidence = usableEvidence
            .Where(e => e.Type == EvidenceType.Testimonial && e.WitnessActorId != null)
            .ToList();

        // Find circumstantial evidence pointing to motive
        var circumstantialEvidence = usableEvidence
            .Where(e => e.Type == EvidenceType.Circumstantial)
            .ToList();

        if (sceneEvidence.Count == 0)
            return [];

        // Check if there's a corroborating piece (testimonial or circumstantial)
        bool hasTestimonial = testimonialEvidence.Count > 0;
        bool hasCircumstantial = circumstantialEvidence.Count > 0;

        if (!hasTestimonial && !hasCircumstantial)
            return [];

        // Build shortest chain: try all combinations starting from scene evidence
        List<Evidence>? bestChain = null;

        foreach (var sceneEv in sceneEvidence)
        {
            // Try with testimonial first (typically shorter chains)
            foreach (var corrEv in testimonialEvidence)
            {
                var chain = TryBuildChain(sceneEv, corrEv, crime.PerpetratorId, usableEvidence);
                if (chain != null && (bestChain == null || chain.Count < bestChain.Count))
                    bestChain = chain;
            }

            // Try with circumstantial
            foreach (var corrEv in circumstantialEvidence)
            {
                var chain = TryBuildChain(sceneEv, corrEv, crime.PerpetratorId, usableEvidence);
                if (chain != null && (bestChain == null || chain.Count < bestChain.Count))
                    bestChain = chain;
            }
        }

        return bestChain ?? [];
    }

    /// <summary>
    /// Tries to build a chain from scene evidence + corroborating evidence to the perpetrator,
    /// following PointsToActorId and LinkedEvidenceId links.
    /// </summary>
    private static List<Evidence>? TryBuildChain(
        Evidence sceneEvidence,
        Evidence corroboratingEvidence,
        string perpetratorId,
        List<Evidence> allUsable)
    {
        var chain = new List<Evidence> { sceneEvidence };
        if (sceneEvidence.Id != corroboratingEvidence.Id)
            chain.Add(corroboratingEvidence);

        // Check if either piece already points to perpetrator
        if (chain.Any(e => e.PointsToActorId == perpetratorId))
            return chain;

        // Follow linked evidence from both starting points
        var visited = chain.Select(e => e.Id).ToHashSet();
        var queue = new Queue<string>();

        // Seed the queue with linked evidence IDs from our starting pieces
        foreach (var ev in chain)
        {
            if (ev.LinkedEvidenceId != null && !visited.Contains(ev.LinkedEvidenceId))
                queue.Enqueue(ev.LinkedEvidenceId);
        }

        var traceChain = new List<Evidence>(chain);

        while (queue.Count > 0)
        {
            var nextId = queue.Dequeue();
            if (visited.Contains(nextId)) continue;
            visited.Add(nextId);

            var nextEv = allUsable.FirstOrDefault(e => e.Id == nextId);
            if (nextEv == null) continue;

            traceChain.Add(nextEv);

            if (nextEv.PointsToActorId == perpetratorId)
                return traceChain;

            if (nextEv.LinkedEvidenceId != null && !visited.Contains(nextEv.LinkedEvidenceId))
                queue.Enqueue(nextEv.LinkedEvidenceId);
        }

        return null;
    }

    /// <summary>
    /// Retroactively generates evidence for a crime and adds it to the world state.
    /// Creates physical, testimonial, circumstantial, documentary, and forensic evidence.
    /// </summary>
    public static void GenerateCrimeEvidence(Crime crime, WorldState world)
    {
        int counter = world.Evidence.Count + 1;
        string NextId() => $"ev-{counter++:D3}";

        // --- Physical: fingerprints on weapon ---
        if (crime.WeaponId != null && world.Objects.TryGetValue(crime.WeaponId, out var weapon))
        {
            var fingerprintEv = new Evidence
            {
                Id = NextId(),
                Type = EvidenceType.Physical,
                Description = $"Fingerprints found on {weapon.Name} matching the perpetrator",
                LocationId = crime.LocationId,
                PointsToActorId = crime.PerpetratorId,
                RelatedCrimeId = crime.Id,
                Discoverable = true
            };
            AddEvidence(fingerprintEv, crime, world);
        }

        // --- Physical: blood/fibers at scene ---
        {
            var sceneTraceEv = new Evidence
            {
                Id = NextId(),
                Type = EvidenceType.Physical,
                Description = "Blood spatter and clothing fibers recovered from the crime scene",
                LocationId = crime.LocationId,
                RelatedCrimeId = crime.Id,
                Discoverable = true
            };
            AddEvidence(sceneTraceEv, crime, world);
        }

        // --- Testimonial: witness statements ---
        foreach (var witnessId in crime.WitnessIds)
        {
            if (!world.Actors.TryGetValue(witnessId, out var witness)) continue;

            var witnessEv = new Evidence
            {
                Id = NextId(),
                Type = EvidenceType.Testimonial,
                Description = $"Witness statement from {witness.Name} regarding events at the scene",
                LocationId = crime.LocationId,
                WitnessActorId = witnessId,
                PointsToActorId = crime.PerpetratorId,
                RelatedCrimeId = crime.Id,
                Discoverable = true
            };
            AddEvidence(witnessEv, crime, world);
        }

        // --- Circumstantial: motive from relationship history ---
        {
            var rel = world.GetRelationship(crime.PerpetratorId, crime.VictimId);
            string motiveDesc = rel != null && rel.History.Count > 0
                ? $"History of conflict between suspect and victim: {string.Join(", ", rel.History.TakeLast(3))}"
                : $"Motive established through {crime.Motive}";

            var motiveEv = new Evidence
            {
                Id = NextId(),
                Type = EvidenceType.Circumstantial,
                Description = motiveDesc,
                LocationId = crime.LocationId,
                PointsToActorId = crime.PerpetratorId,
                RelatedCrimeId = crime.Id,
                Discoverable = true
            };
            AddEvidence(motiveEv, crime, world);
        }

        // --- Documentary: phone location data ---
        {
            var phoneEv = new Evidence
            {
                Id = NextId(),
                Type = EvidenceType.Documentary,
                Description = "Cell tower records placing suspect's phone near the crime scene at the time of the murder",
                LocationId = crime.LocationId,
                PointsToActorId = crime.PerpetratorId,
                RelatedCrimeId = crime.Id,
                Discoverable = true
            };
            AddEvidence(phoneEv, crime, world);
        }

        // --- Forensic: DNA on victim ---
        {
            var dnaEv = new Evidence
            {
                Id = NextId(),
                Type = EvidenceType.Forensic,
                Description = "DNA traces recovered from the victim matching the perpetrator",
                LocationId = crime.LocationId,
                PointsToActorId = crime.PerpetratorId,
                RelatedCrimeId = crime.Id,
                Discoverable = true
            };
            AddEvidence(dnaEv, crime, world);
        }

        // --- Forensic: ballistics if gun was used ---
        if (crime.Method == MurderMethod.Shooting)
        {
            var ballisticsEv = new Evidence
            {
                Id = NextId(),
                Type = EvidenceType.Forensic,
                Description = "Ballistics analysis matching bullet to weapon found at the scene",
                LocationId = crime.LocationId,
                PointsToActorId = crime.PerpetratorId,
                LinkedEvidenceId = crime.EvidenceIds.FirstOrDefault(), // Link to fingerprint evidence on weapon
                RelatedCrimeId = crime.Id,
                Discoverable = true
            };
            AddEvidence(ballisticsEv, crime, world);
        }

        // --- Coverup trail: evidence from post-crime events ---
        GenerateCoverupEvidence(crime, world, NextId);
    }

    /// <summary>
    /// Converts post-crime simulation events (fled scene, disposed weapon, etc.)
    /// into formal evidence objects that can be part of the case.
    /// </summary>
    private static void GenerateCoverupEvidence(Crime crime, WorldState world, Func<string> NextId)
    {
        var coverupEventTypes = new[]
        {
            SimEventType.FledScene, SimEventType.DisposedWeapon, SimEventType.CleanedUp,
            SimEventType.EstablishedAlibi, SimEventType.ConfidedInFriend, SimEventType.SuspiciousBehavior
        };

        var coverupEvents = world.Events
            .Where(e => e.RelatedCrimeId == crime.Id && coverupEventTypes.Contains(e.Type))
            .ToList();

        foreach (var evt in coverupEvents)
        {
            var evidenceType = evt.Type switch
            {
                SimEventType.FledScene => EvidenceType.Testimonial,
                SimEventType.DisposedWeapon => EvidenceType.Physical,
                SimEventType.CleanedUp => EvidenceType.Circumstantial,
                SimEventType.EstablishedAlibi => EvidenceType.Documentary,
                SimEventType.ConfidedInFriend => EvidenceType.Testimonial,
                SimEventType.SuspiciousBehavior => EvidenceType.Circumstantial,
                _ => EvidenceType.Circumstantial
            };

            // For fled scene and disposed weapon, find witnesses at the event location
            string? witnessId = null;
            if (evt.Type is SimEventType.FledScene or SimEventType.DisposedWeapon)
            {
                var witnessAtLoc = world.GetActorsAtLocation(evt.LocationId)
                    .FirstOrDefault(a => a.Id != crime.PerpetratorId && a.IsAlive);
                witnessId = witnessAtLoc?.Id;
            }

            // For confided in friend, the friend is the witness
            if (evt.Type == SimEventType.ConfidedInFriend && evt.ActorIds.Count > 1)
            {
                witnessId = evt.ActorIds.FirstOrDefault(id => id != crime.PerpetratorId);
            }

            var evidence = new Evidence
            {
                Id = NextId(),
                Type = evidenceType,
                Description = evt.Description,
                LocationId = evt.LocationId,
                PointsToActorId = crime.PerpetratorId,
                WitnessActorId = witnessId,
                RelatedCrimeId = crime.Id,
                Discoverable = true
            };
            AddEvidence(evidence, crime, world);
        }
    }

    private static void AddEvidence(Evidence evidence, Crime crime, WorldState world)
    {
        world.Evidence.Add(evidence);
        crime.EvidenceIds.Add(evidence.Id);
    }
}
