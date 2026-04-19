using CaseGenV3.Models;

namespace CaseGenV3.Generation;

public static class LocationGenerator
{
    private static readonly string[] BarNames =
    [
        "The Rusty Nail", "O'Malley's Pub", "The Blind Tiger", "Neon Lounge",
        "The Broken Clock", "Half Moon Tavern", "The Copper Mug", "Whiskey Row"
    ];

    private static readonly string[] ParkNames =
    [
        "Riverside Park", "Cedar Grove", "Millstone Park", "Sunset Gardens",
        "Willow Creek Park", "Heritage Green", "Oak Hollow Park"
    ];

    private static readonly string[] ResidenceNames =
    [
        "Maple Street Apartments", "Oakwood Condos", "Pinehurst Townhomes",
        "Elm Court Residence", "Birchwood Flats", "Cedar Lane House",
        "Hawthorn Terrace", "Spruce Row Apartments", "Ashford Villas",
        "Magnolia Heights", "Sycamore Place", "Laurel Ridge Homes",
        "Willow Park Apartments", "Chestnut Hill Condos", "Ivy Gate Residence"
    ];

    private static readonly string[] WorkplaceNames =
    [
        "Grayson & Associates", "Metro Construction", "City Hospital",
        "Pinnacle Tech", "Harbor Freight Warehouse", "Summit Legal Group",
        "Crossroads Auto Repair", "Sterling Insurance", "Bayview Accounting",
        "Ironclad Manufacturing", "Lakeside Dental", "Ridgeline Consulting"
    ];

    private static readonly string[] StoreNames =
    [
        "Quick Stop Convenience", "Valley Hardware", "Main Street Pharmacy",
        "Lucky's Pawn Shop", "Corner Bodega", "Greenfield Grocery"
    ];

    private static readonly string[] RestaurantNames =
    [
        "Golden Dragon", "Mama Rosa's Kitchen", "The Grillhouse",
        "Blue Plate Diner", "Sakura Sushi", "El Camino Taqueria"
    ];

    private static readonly string[] ChurchNames =
    [
        "St. Michael's Church", "Grace Community Chapel", "First Baptist Church",
        "Holy Cross Parish"
    ];

    private static readonly string[] GymNames =
    [
        "Iron Temple Gym", "CrossFit Downtown", "Planet Fitness", "Flex Zone"
    ];

    private static readonly string[] AlleyNames =
    [
        "Back Alley behind 5th St", "Warehouse District Alley",
        "Dockside Passage", "The Cut between Main and 2nd"
    ];

    private static readonly string[] ParkingNames =
    [
        "Municipal Parking Garage", "Lot B - Downtown", "Riverside Parking Deck",
        "Strip Mall Parking Lot"
    ];

    public static Dictionary<string, Location> Generate(Random rng, int count)
    {
        count = Math.Clamp(count, 10, 20);

        var locations = new Dictionary<string, Location>();
        var namePools = new Dictionary<LocationType, List<string>>
        {
            [LocationType.Residence] = new(ResidenceNames),
            [LocationType.Workplace] = new(WorkplaceNames),
            [LocationType.Bar] = new(BarNames),
            [LocationType.Park] = new(ParkNames),
            [LocationType.Store] = new(StoreNames),
            [LocationType.Restaurant] = new(RestaurantNames),
            [LocationType.Church] = new(ChurchNames),
            [LocationType.Gym] = new(GymNames),
            [LocationType.Alley] = new(AlleyNames),
            [LocationType.Parking] = new(ParkingNames)
        };

        // Guaranteed minimum locations
        var typeSequence = new List<LocationType>
        {
            LocationType.Residence, LocationType.Residence,
            LocationType.Residence, LocationType.Residence,
            LocationType.Workplace, LocationType.Workplace,
            LocationType.Bar,
            LocationType.Park,
            LocationType.Alley
        };

        // Fill remaining slots with random types
        var randomTypes = new[]
        {
            LocationType.Residence, LocationType.Workplace, LocationType.Store,
            LocationType.Restaurant, LocationType.Church, LocationType.Gym,
            LocationType.Parking, LocationType.Bar
        };

        while (typeSequence.Count < count)
        {
            typeSequence.Add(randomTypes[rng.Next(randomTypes.Length)]);
        }

        // Shuffle the sequence so guaranteed types aren't always first
        Shuffle(rng, typeSequence);

        for (int i = 0; i < count; i++)
        {
            var type = typeSequence[i];
            var pool = namePools[type];
            string name = pool.Count > 0
                ? pool[rng.Next(pool.Count)]
                : $"{type} #{i + 1}";
            pool.Remove(name);

            var id = $"loc-{(i + 1):D3}";
            var capacity = type switch
            {
                LocationType.Residence => rng.Next(2, 6),
                LocationType.Workplace => rng.Next(8, 20),
                LocationType.Bar => rng.Next(10, 30),
                LocationType.Park => rng.Next(20, 50),
                LocationType.Restaurant => rng.Next(10, 25),
                LocationType.Gym => rng.Next(10, 25),
                LocationType.Church => rng.Next(15, 40),
                LocationType.Alley => rng.Next(2, 5),
                LocationType.Parking => rng.Next(5, 15),
                LocationType.Store => rng.Next(5, 15),
                _ => 10
            };

            locations[id] = new Location
            {
                Id = id,
                Name = name,
                Type = type,
                Capacity = capacity
            };
        }

        // Build connected graph: each location connects to 2-4 neighbors
        var ids = locations.Keys.ToList();
        ConnectLocations(rng, locations, ids);

        return locations;
    }

    private static void ConnectLocations(Random rng, Dictionary<string, Location> locations, List<string> ids)
    {
        // First, create a spanning path so the graph is connected
        var shuffled = new List<string>(ids);
        Shuffle(rng, shuffled);

        for (int i = 0; i < shuffled.Count - 1; i++)
        {
            AddConnection(locations, shuffled[i], shuffled[i + 1]);
        }

        // Then add extra edges so each location has 2-4 connections
        foreach (var id in ids)
        {
            var loc = locations[id];
            int desired = rng.Next(2, 5); // 2 to 4
            while (loc.ConnectedLocationIds.Count < desired)
            {
                var candidate = ids[rng.Next(ids.Count)];
                if (candidate == id) continue;
                if (loc.ConnectedLocationIds.Contains(candidate)) continue;

                // Don't let the candidate exceed 4 connections
                if (locations[candidate].ConnectedLocationIds.Count >= 4) continue;

                AddConnection(locations, id, candidate);
            }
        }
    }

    private static void AddConnection(Dictionary<string, Location> locations, string a, string b)
    {
        if (!locations[a].ConnectedLocationIds.Contains(b))
            locations[a].ConnectedLocationIds.Add(b);

        if (!locations[b].ConnectedLocationIds.Contains(a))
            locations[b].ConnectedLocationIds.Add(a);
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
