using CaseGenV3.Models;

namespace CaseGenV3.Generation;

public static class WorldGenerator
{
    public static WorldState Generate(Random rng, int locationCount = 15, int actorCount = 30)
    {
        locationCount = Math.Clamp(locationCount, 10, 20);
        actorCount = Math.Clamp(actorCount, 25, 40);

        var locations = LocationGenerator.Generate(rng, locationCount);
        var actors = ActorGenerator.Generate(rng, actorCount, locations);
        var objects = ObjectGenerator.Generate(rng, locations, actors);

        var world = new WorldState
        {
            Locations = locations,
            Actors = actors,
            Objects = objects,
            CurrentTime = new DateTime(2025, 6, 15, 8, 0, 0), // Sunday morning start
            Rng = rng
        };

        CreateInitialRelationships(rng, world);

        return world;
    }

    private static void CreateInitialRelationships(Random rng, WorldState world)
    {
        var actorList = world.Actors.Values.ToList();
        Shuffle(rng, actorList);

        int index = 0;

        // Create 3-5 spouse pairs
        int spousePairs = rng.Next(3, 6);
        for (int i = 0; i < spousePairs && index + 1 < actorList.Count; i++)
        {
            var a = actorList[index++];
            var b = actorList[index++];

            // Spouses share a home
            b.HomeLocationId = a.HomeLocationId;
            b.CurrentLocationId = a.HomeLocationId;

            world.Relationships.Add(new Relationship
            {
                ActorAId = a.Id,
                ActorBId = b.Id,
                Type = RelationshipType.Spouse,
                Strength = rng.Next(40, 90),
                Tension = rng.Next(0, 30)
            });
        }

        // Create friend pairs from remaining actors
        int friendPairs = rng.Next(6, 12);
        var friendCandidates = actorList.Skip(index).ToList();
        Shuffle(rng, friendCandidates);

        for (int i = 0; i + 1 < friendCandidates.Count && i / 2 < friendPairs; i += 2)
        {
            world.Relationships.Add(new Relationship
            {
                ActorAId = friendCandidates[i].Id,
                ActorBId = friendCandidates[i + 1].Id,
                Type = RelationshipType.Friend,
                Strength = rng.Next(20, 70),
                Tension = rng.Next(0, 15)
            });
        }

        // Create coworker relationships for actors who share a workplace
        var workplaceGroups = actorList
            .GroupBy(a => a.WorkLocationId)
            .Where(g => g.Count() >= 2);

        foreach (var group in workplaceGroups)
        {
            var coworkers = group.ToList();
            // Connect pairs within each workplace (limit to keep count reasonable)
            int pairsToCreate = Math.Min(coworkers.Count - 1, 4);
            for (int i = 0; i < pairsToCreate; i++)
            {
                var a = coworkers[i];
                var b = coworkers[i + 1];

                // Skip if they already have a relationship
                if (world.GetRelationship(a.Id, b.Id) != null) continue;

                world.Relationships.Add(new Relationship
                {
                    ActorAId = a.Id,
                    ActorBId = b.Id,
                    Type = RelationshipType.Coworker,
                    Strength = rng.Next(-10, 40),
                    Tension = rng.Next(0, 20)
                });
            }
        }

        // Add a couple of rival relationships for drama
        int rivalPairs = rng.Next(2, 4);
        for (int i = 0; i < rivalPairs; i++)
        {
            var a = actorList[rng.Next(actorList.Count)];
            var b = actorList[rng.Next(actorList.Count)];
            if (a.Id == b.Id) continue;
            if (world.GetRelationship(a.Id, b.Id) != null) continue;

            world.Relationships.Add(new Relationship
            {
                ActorAId = a.Id,
                ActorBId = b.Id,
                Type = RelationshipType.Rival,
                Strength = rng.Next(-60, -10),
                Tension = rng.Next(20, 60)
            });
        }
    }

    private static void Shuffle<T>(Random rng, List<T> list)
    {
        for (int i = list.Count - 1; i > 0; i--)
        {
            int j = rng.Next(i + 1);
            (list[i], list[j]) = (list[j], list[i]);
        }
    }
}
