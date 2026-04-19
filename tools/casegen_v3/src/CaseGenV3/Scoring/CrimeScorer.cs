using CaseGenV3.Models;

namespace CaseGenV3.Scoring;

public static class CrimeScorer
{
    /// <summary>
    /// Scores all Murder-type crimes in the world based on narrative complexity.
    /// Each factor contributes 0-20 points for a maximum score of 100.
    /// Returns crimes sorted by score descending.
    /// </summary>
    public static List<Crime> ScoreCrimes(WorldState world)
    {
        var murders = world.Crimes.Where(c => c.Type == CrimeType.Murder).ToList();

        foreach (var crime in murders)
        {
            float motiveScore = ScoreMotiveComplexity(crime);
            float suspectScore = ScoreSuspectPool(crime, world);
            float evidenceScore = ScoreEvidenceVariety(crime, world);
            float relationshipScore = ScoreRelationshipWeb(crime, world);
            float redHerringScore = ScoreRedHerringPotential(crime, world);

            crime.Score = motiveScore + suspectScore + evidenceScore + relationshipScore + redHerringScore;
        }

        return murders.OrderByDescending(c => c.Score).ToList();
    }

    /// <summary>
    /// +5 per motive factor, capped at 20.
    /// </summary>
    private static float ScoreMotiveComplexity(Crime crime)
    {
        return Math.Min(20f, crime.MotiveFactors.Count * 5f);
    }

    /// <summary>
    /// Count actors who had both motive (negative relationship with victim)
    /// AND opportunity (were at or near the crime location around the crime time).
    /// 1-2 suspects: 5, 3-4: 10, 5+: 20.
    /// </summary>
    private static float ScoreSuspectPool(Crime crime, WorldState world)
    {
        var crimeLocationId = crime.LocationId;
        var crimeDate = crime.Timestamp.Date;

        // Get connected locations (nearby)
        var nearbyLocationIds = new HashSet<string> { crimeLocationId };
        if (world.Locations.TryGetValue(crimeLocationId, out var crimeLoc))
        {
            foreach (var connId in crimeLoc.ConnectedLocationIds)
                nearbyLocationIds.Add(connId);
        }

        // Find actors who were at or near the crime location on the same day
        var actorsWithOpportunity = new HashSet<string>();
        foreach (var evt in world.Events)
        {
            if (evt.Timestamp.Date != crimeDate) continue;
            if (!nearbyLocationIds.Contains(evt.LocationId)) continue;

            foreach (var actorId in evt.ActorIds)
            {
                if (actorId != crime.VictimId && actorId != crime.PerpetratorId)
                    actorsWithOpportunity.Add(actorId);
            }
        }

        // Find actors with motive (negative relationship with victim)
        var actorsWithMotive = new HashSet<string>();
        var victimRels = world.GetActorRelationships(crime.VictimId);
        foreach (var rel in victimRels)
        {
            var otherId = world.GetOtherActorId(rel, crime.VictimId);
            if (otherId == crime.PerpetratorId) continue;
            if (rel.Strength < 0f || rel.Tension > 50f)
                actorsWithMotive.Add(otherId);
        }

        // Suspects = actors with BOTH motive and opportunity
        int suspectCount = actorsWithMotive.Intersect(actorsWithOpportunity).Count();

        return suspectCount switch
        {
            >= 5 => 20f,
            >= 3 => 10f,
            >= 1 => 5f,
            _ => 0f
        };
    }

    /// <summary>
    /// Count distinct EvidenceType values among this crime's evidence.
    /// 1 type: 5, 2: 10, 3: 15, 4-5: 20.
    /// </summary>
    private static float ScoreEvidenceVariety(Crime crime, WorldState world)
    {
        var crimeEvidence = world.Evidence
            .Where(e => crime.EvidenceIds.Contains(e.Id))
            .ToList();

        int distinctTypes = crimeEvidence.Select(e => e.Type).Distinct().Count();

        return distinctTypes switch
        {
            >= 4 => 20f,
            3 => 15f,
            2 => 10f,
            1 => 5f,
            _ => 0f
        };
    }

    /// <summary>
    /// How many actors are connected to BOTH the perpetrator AND the victim
    /// through relationships? More shared connections = higher score.
    /// 1-2: 5, 3-4: 10, 5+: 20.
    /// </summary>
    private static float ScoreRelationshipWeb(Crime crime, WorldState world)
    {
        var perpConnections = world.GetActorRelationships(crime.PerpetratorId)
            .Select(r => world.GetOtherActorId(r, crime.PerpetratorId))
            .ToHashSet();

        var victimConnections = world.GetActorRelationships(crime.VictimId)
            .Select(r => world.GetOtherActorId(r, crime.VictimId))
            .ToHashSet();

        // Exclude the perpetrator and victim themselves
        perpConnections.Remove(crime.VictimId);
        victimConnections.Remove(crime.PerpetratorId);

        int sharedCount = perpConnections.Intersect(victimConnections).Count();

        return sharedCount switch
        {
            >= 5 => 20f,
            >= 3 => 10f,
            >= 1 => 5f,
            _ => 0f
        };
    }

    /// <summary>
    /// Count actors who have motive but no opportunity, or opportunity but no motive.
    /// These are natural red herrings. More = higher score.
    /// 1-2: 5, 3-4: 10, 5+: 20.
    /// </summary>
    private static float ScoreRedHerringPotential(Crime crime, WorldState world)
    {
        var crimeLocationId = crime.LocationId;
        var crimeDate = crime.Timestamp.Date;

        var nearbyLocationIds = new HashSet<string> { crimeLocationId };
        if (world.Locations.TryGetValue(crimeLocationId, out var crimeLoc))
        {
            foreach (var connId in crimeLoc.ConnectedLocationIds)
                nearbyLocationIds.Add(connId);
        }

        // Opportunity set
        var actorsWithOpportunity = new HashSet<string>();
        foreach (var evt in world.Events)
        {
            if (evt.Timestamp.Date != crimeDate) continue;
            if (!nearbyLocationIds.Contains(evt.LocationId)) continue;

            foreach (var actorId in evt.ActorIds)
            {
                if (actorId != crime.VictimId && actorId != crime.PerpetratorId)
                    actorsWithOpportunity.Add(actorId);
            }
        }

        // Motive set
        var actorsWithMotive = new HashSet<string>();
        var victimRels = world.GetActorRelationships(crime.VictimId);
        foreach (var rel in victimRels)
        {
            var otherId = world.GetOtherActorId(rel, crime.VictimId);
            if (otherId == crime.PerpetratorId) continue;
            if (rel.Strength < 0f || rel.Tension > 50f)
                actorsWithMotive.Add(otherId);
        }

        // Red herrings: motive XOR opportunity
        int motiveOnly = actorsWithMotive.Except(actorsWithOpportunity).Count();
        int opportunityOnly = actorsWithOpportunity.Except(actorsWithMotive).Count();
        int redHerringCount = motiveOnly + opportunityOnly;

        return redHerringCount switch
        {
            >= 5 => 20f,
            >= 3 => 10f,
            >= 1 => 5f,
            _ => 0f
        };
    }
}
