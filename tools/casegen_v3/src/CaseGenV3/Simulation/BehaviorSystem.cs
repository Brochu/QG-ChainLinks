using CaseGenV3.Models;

namespace CaseGenV3.Simulation;

public enum ActorAction
{
    GoToWork,
    GoHome,
    Sleep,
    Socialize,
    VisitPartner,
    Idle,
    Confront,
    // Post-crime actions
    FleeScene,
    DisposeWeapon,
    CleanUp,
    EstablishAlibi,
    ConfideInFriend,
    Hide
}

public static class BehaviorSystem
{
    public static ActorAction DecideAction(Actor actor, WorldState world)
    {
        int hour = world.CurrentTime.Hour;
        bool isNight = hour >= 22 || hour < 6;
        bool isWorkHours = hour >= 9 && hour < 17
                           && world.CurrentTime.DayOfWeek != DayOfWeek.Saturday
                           && world.CurrentTime.DayOfWeek != DayOfWeek.Sunday;

        // --- Check for Confront opportunity ---
        // If any relationship has high tension and actor is hot-headed/impulsive
        if (actor.Traits.Temper > 0.3f && actor.Traits.Impulsivity > 0f)
        {
            var tensionRels = world.GetActorRelationships(actor.Id)
                .Where(r => r.Tension > 70f)
                .ToList();

            if (tensionRels.Count > 0)
            {
                // Chance to confront scales with impulsivity
                double confrontChance = 0.1 + actor.Traits.Impulsivity * 0.3;
                if (world.Rng.NextDouble() < confrontChance)
                {
                    // Only confront if target is at same location
                    foreach (var rel in tensionRels)
                    {
                        string otherId = world.GetOtherActorId(rel, actor.Id);
                        if (world.Actors.TryGetValue(otherId, out var other)
                            && other.IsAlive
                            && other.CurrentLocationId == actor.CurrentLocationId)
                        {
                            return ActorAction.Confront;
                        }
                    }
                }
            }
        }

        // --- Sleep if Rest is low and it's night ---
        var restNeed = actor.Needs.GetValueOrDefault(NeedType.Rest);
        if (isNight && restNeed != null && restNeed.Value < 50f)
        {
            return ActorAction.Sleep;
        }

        // --- Go to work during work hours ---
        if (isWorkHours && !string.IsNullOrEmpty(actor.WorkLocationId))
        {
            return ActorAction.GoToWork;
        }

        // --- Need-driven decisions: pick lowest need ---
        var lowestNeed = actor.GetLowestNeed();

        switch (lowestNeed.Type)
        {
            case NeedType.Rest:
                if (isNight)
                    return ActorAction.Sleep;
                return ActorAction.GoHome; // rest at home even during day if desperate

            case NeedType.Money:
                if (!string.IsNullOrEmpty(actor.WorkLocationId))
                    return ActorAction.GoToWork;
                return ActorAction.Idle;

            case NeedType.Social:
                return ActorAction.Socialize;

            case NeedType.Love:
                // Check if actor has a partner to visit
                var partnerRel = world.GetActorRelationships(actor.Id)
                    .FirstOrDefault(r => r.Type == RelationshipType.Partner
                                      || r.Type == RelationshipType.Spouse);
                if (partnerRel != null)
                    return ActorAction.VisitPartner;
                return ActorAction.Socialize; // socialize to find love

            case NeedType.Safety:
                return ActorAction.GoHome;

            case NeedType.Status:
                if (isWorkHours && !string.IsNullOrEmpty(actor.WorkLocationId))
                    return ActorAction.GoToWork;
                return ActorAction.Socialize;

            default:
                return ActorAction.Idle;
        }
    }
}
