using CaseGenV3.Models;

namespace CaseGenV3.Simulation;

public static class NeedSystem
{
    public static void ProcessTick(WorldState world)
    {
        int hour = world.CurrentTime.Hour;
        bool isDaytime = hour >= 6 && hour < 22;
        bool isNight = !isDaytime;

        foreach (var actor in world.Actors.Values)
        {
            if (!actor.IsAlive) continue;

            // --- Rest: decays faster during the day ---
            if (actor.Needs.TryGetValue(NeedType.Rest, out var restNeed))
            {
                float restModifier = isDaytime ? 1.5f : 0.3f;
                restNeed.Decay(restModifier);

                // Sleeping at home during night hours satisfies Rest
                if (isNight && actor.CurrentLocationId == actor.HomeLocationId)
                {
                    restNeed.Satisfy(15f);
                }
            }

            // --- Money: decays always, faster with high Greed ---
            if (actor.Needs.TryGetValue(NeedType.Money, out var moneyNeed))
            {
                float greedModifier = 1f + Math.Max(0f, actor.Traits.Greed); // 1.0 to 2.0
                moneyNeed.Decay(greedModifier);

                // Working during work hours satisfies Money
                if (hour >= 9 && hour < 17
                    && actor.CurrentLocationId == actor.WorkLocationId
                    && world.CurrentTime.DayOfWeek != DayOfWeek.Saturday
                    && world.CurrentTime.DayOfWeek != DayOfWeek.Sunday)
                {
                    moneyNeed.Satisfy(2f);
                }
            }

            // --- Social: decays faster when alone ---
            if (actor.Needs.TryGetValue(NeedType.Social, out var socialNeed))
            {
                var othersHere = world.GetActorsAtLocation(actor.CurrentLocationId);
                bool isAlone = othersHere.Count <= 1; // only self
                float socialModifier = isAlone ? 1.8f : 0.5f;
                socialNeed.Decay(socialModifier);

                // Working satisfies Social slightly
                if (hour >= 9 && hour < 17
                    && actor.CurrentLocationId == actor.WorkLocationId
                    && world.CurrentTime.DayOfWeek != DayOfWeek.Saturday
                    && world.CurrentTime.DayOfWeek != DayOfWeek.Sunday)
                {
                    socialNeed.Satisfy(0.5f);
                }
            }

            // --- Love: decays if actor has a partner/spouse relationship ---
            if (actor.Needs.TryGetValue(NeedType.Love, out var loveNeed))
            {
                var partnerRel = world.GetActorRelationships(actor.Id)
                    .FirstOrDefault(r => r.Type == RelationshipType.Partner
                                      || r.Type == RelationshipType.Spouse);

                if (partnerRel != null)
                {
                    string partnerId = world.GetOtherActorId(partnerRel, actor.Id);
                    bool partnerPresent = world.Actors.TryGetValue(partnerId, out var partner)
                                          && partner.CurrentLocationId == actor.CurrentLocationId;

                    // High Jealousy = faster Love decay when partner is absent
                    float jealousyModifier = partnerPresent
                        ? 0.3f
                        : 1f + Math.Max(0f, actor.Traits.Jealousy);

                    loveNeed.Decay(jealousyModifier);
                }
            }

            // --- Safety: standard decay ---
            if (actor.Needs.TryGetValue(NeedType.Safety, out var safetyNeed))
            {
                safetyNeed.Decay(0.5f);
            }

            // --- Status: standard decay ---
            if (actor.Needs.TryGetValue(NeedType.Status, out var statusNeed))
            {
                statusNeed.Decay(0.3f);
            }

            // --- Organic tension buildup from unmet needs (once per hour) ---
            if (world.CurrentTime.Minute == 0)
            {
                BuildOrganicTension(actor, world);
            }
        }
    }

    /// <summary>
    /// Actors with critically low needs become irritable, building tension
    /// in their closest relationships.
    /// </summary>
    private static void BuildOrganicTension(Actor actor, WorldState world)
    {
        int criticalNeeds = actor.Needs.Values.Count(n => n.IsCritical);
        if (criticalNeeds == 0) return;

        // More critical needs = more irritable
        float irritability = criticalNeeds * 0.3f + Math.Max(0f, actor.Traits.Temper) * 0.5f;

        var rels = world.GetActorRelationships(actor.Id);
        foreach (var rel in rels)
        {
            // Only build tension with people the actor interacts with regularly
            if (rel.Type == RelationshipType.Stranger) continue;

            // Rivals and coworkers get more tension
            float typeMod = rel.Type switch
            {
                RelationshipType.Rival => 2.0f,
                RelationshipType.Coworker => 1.2f,
                RelationshipType.Ex => 1.5f,
                RelationshipType.Spouse when rel.Strength < 0 => 1.8f,
                _ => 0.5f
            };

            // Small tension increase, scaled by irritability and type
            float tensionBump = irritability * typeMod * 0.1f * (float)(0.5 + world.Rng.NextDouble() * 0.5);
            if (tensionBump > 0.01f)
            {
                rel.Tension = Math.Min(100f, rel.Tension + tensionBump);
            }
        }

        // Also slowly build tension between coworkers competing for status
        if (actor.Needs.TryGetValue(NeedType.Status, out var statusNeed) && statusNeed.IsCritical)
        {
            var coworkerRels = rels.Where(r => r.Type == RelationshipType.Coworker).ToList();
            foreach (var rel in coworkerRels)
            {
                rel.Tension = Math.Min(100f, rel.Tension + 0.15f);
            }
        }
    }
}
