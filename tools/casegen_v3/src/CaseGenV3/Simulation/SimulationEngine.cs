using CaseGenV3.Models;

namespace CaseGenV3.Simulation;

public static class SimulationEngine
{
    /// <summary>
    /// Runs the simulation for the given number of months.
    /// Each tick represents a 15-minute interval.
    /// </summary>
    public static WorldState Run(WorldState world, int months)
    {
        int totalTicks = months * 30 * 24 * 4; // 15-min intervals
        Console.WriteLine($"[Simulation] Starting: {totalTicks} ticks ({months} month(s))");
        Console.WriteLine($"[Simulation] Actors: {world.Actors.Count}, Locations: {world.Locations.Count}");

        for (int tick = 0; tick < totalTicks; tick++)
        {
            // Advance time by 15 minutes
            world.CurrentTime = world.CurrentTime.AddMinutes(15);

            // 1. Process need decay and passive satisfaction
            NeedSystem.ProcessTick(world);

            // 2. Each alive actor decides and executes an action
            foreach (var actor in world.Actors.Values)
            {
                if (!actor.IsAlive) continue;

                // Actors in post-crime mode use PostCrimeBehavior instead of normal behavior
                ActorAction action;
                if (actor.ActiveCrimeState != null)
                    action = PostCrimeBehavior.DecideAction(actor, world);
                else
                    action = BehaviorSystem.DecideAction(actor, world);

                ActionResolver.ResolveAction(actor, action, world);
            }

            // 3. Advance post-crime phase timers
            PostCrimeBehavior.AdvancePhases(world);

            // 4. Process social interactions and relationship changes
            RelationshipSystem.ProcessInteractions(world);

            // 5. Process conflict escalation
            ConflictSystem.ProcessConflicts(world);

            // Progress logging
            if ((tick + 1) % 1000 == 0)
            {
                int aliveCount = world.Actors.Values.Count(a => a.IsAlive);
                Console.WriteLine($"[Simulation] Tick {tick + 1}/{totalTicks} | " +
                    $"Time: {world.CurrentTime:yyyy-MM-dd HH:mm} | " +
                    $"Alive: {aliveCount}/{world.Actors.Count} | " +
                    $"Events: {world.Events.Count} | " +
                    $"Crimes: {world.Crimes.Count}");
            }
        }

        Console.WriteLine($"[Simulation] Complete. Events: {world.Events.Count}, Crimes: {world.Crimes.Count}");
        return world;
    }
}
