using CaseGenV3.Models;

namespace CaseGenV3.Generation;

public static class ActorGenerator
{
    private static readonly string[] FirstNames =
    [
        "James", "Maria", "Robert", "Linda", "David", "Susan", "Michael", "Karen",
        "William", "Nancy", "Richard", "Betty", "Joseph", "Dorothy", "Thomas", "Sandra",
        "Charles", "Ashley", "Daniel", "Margaret", "Anthony", "Emily", "Mark", "Donna",
        "Steven", "Carol", "Paul", "Amanda", "Andrew", "Melissa", "Joshua", "Deborah",
        "Kenneth", "Stephanie", "Kevin", "Rebecca", "Brian", "Sharon", "George", "Laura",
        "Timothy", "Cynthia", "Ronald", "Kathleen", "Edward", "Amy", "Jason", "Angela",
        "Jeffrey", "Shirley", "Ryan", "Nicole", "Jacob", "Brenda", "Gary", "Teresa",
        "Nicholas", "Pamela", "Eric", "Samantha"
    ];

    private static readonly string[] LastNames =
    [
        "Smith", "Johnson", "Williams", "Brown", "Jones", "Garcia", "Miller", "Davis",
        "Rodriguez", "Martinez", "Hernandez", "Lopez", "Gonzalez", "Wilson", "Anderson",
        "Thomas", "Taylor", "Moore", "Jackson", "Martin", "Lee", "Perez", "Thompson",
        "White", "Harris", "Sanchez", "Clark", "Ramirez", "Lewis", "Robinson", "Walker",
        "Young", "Allen", "King", "Wright", "Scott", "Torres", "Nguyen", "Hill", "Flores",
        "Green", "Adams", "Nelson", "Baker", "Hall", "Rivera", "Campbell", "Mitchell",
        "Carter", "Roberts"
    ];

    private static readonly Dictionary<LocationType, string[]> OccupationsByWorkplace = new()
    {
        [LocationType.Workplace] =
        [
            "Accountant", "Office Manager", "Software Developer", "Attorney",
            "Insurance Agent", "Mechanic", "Construction Worker", "Warehouse Worker",
            "Dentist", "Consultant", "Nurse", "Doctor", "Receptionist", "Engineer"
        ],
        [LocationType.Bar] =
        [
            "Bartender", "Bouncer", "Bar Manager"
        ],
        [LocationType.Restaurant] =
        [
            "Chef", "Waiter", "Restaurant Manager", "Line Cook", "Dishwasher"
        ],
        [LocationType.Store] =
        [
            "Cashier", "Store Manager", "Stock Clerk"
        ],
        [LocationType.Gym] =
        [
            "Personal Trainer", "Gym Manager", "Fitness Instructor"
        ],
        [LocationType.Church] =
        [
            "Pastor", "Church Administrator"
        ]
    };

    private static readonly (NeedType type, float decayRate)[] NeedDecayRates =
    [
        (NeedType.Money, 0.3f),
        (NeedType.Social, 0.8f),
        (NeedType.Love, 0.5f),
        (NeedType.Safety, 0.2f),
        (NeedType.Status, 0.3f),
        (NeedType.Rest, 1.2f)
    ];

    public static Dictionary<string, Actor> Generate(
        Random rng,
        int count,
        Dictionary<string, Location> locations)
    {
        count = Math.Clamp(count, 25, 40);

        var actors = new Dictionary<string, Actor>();
        var usedNames = new HashSet<string>();

        var residences = locations.Values
            .Where(l => l.Type == LocationType.Residence)
            .ToList();

        var workplaces = locations.Values
            .Where(l => l.Type is LocationType.Workplace or LocationType.Bar
                or LocationType.Restaurant or LocationType.Store
                or LocationType.Gym or LocationType.Church)
            .ToList();

        for (int i = 0; i < count; i++)
        {
            var id = $"actor-{(i + 1):D3}";

            // Generate a unique name
            string fullName;
            do
            {
                var first = FirstNames[rng.Next(FirstNames.Length)];
                var last = LastNames[rng.Next(LastNames.Length)];
                fullName = $"{first} {last}";
            } while (!usedNames.Add(fullName));

            // Pick home and work locations
            var home = residences[rng.Next(residences.Count)];
            var work = workplaces[rng.Next(workplaces.Count)];

            // Pick occupation matching workplace type
            var occupation = PickOccupation(rng, work.Type);

            var actor = new Actor
            {
                Id = id,
                Name = fullName,
                Age = rng.Next(20, 66),
                Occupation = occupation,
                HomeLocationId = home.Id,
                WorkLocationId = work.Id,
                CurrentLocationId = home.Id,
                IsAlive = true,
                Traits = GenerateTraits(rng),
                Needs = GenerateNeeds(rng)
            };

            actors[id] = actor;

            // Register actor at their home location
            home.ActorIdsPresent.Add(id);
        }

        return actors;
    }

    private static string PickOccupation(Random rng, LocationType workplaceType)
    {
        if (OccupationsByWorkplace.TryGetValue(workplaceType, out var occupations))
        {
            return occupations[rng.Next(occupations.Length)];
        }

        // Fallback for workplace types not in the dictionary
        return OccupationsByWorkplace[LocationType.Workplace][
            rng.Next(OccupationsByWorkplace[LocationType.Workplace].Length)];
    }

    private static ActorTraits GenerateTraits(Random rng)
    {
        return new ActorTraits
        {
            Temper = RandomTrait(rng),
            Greed = RandomTrait(rng),
            Jealousy = RandomTrait(rng),
            Loyalty = RandomTrait(rng),
            Impulsivity = RandomTrait(rng)
        };
    }

    private static float RandomTrait(Random rng)
    {
        // Generate float between -1 and 1, rounded to 2 decimal places
        return MathF.Round((float)(rng.NextDouble() * 2.0 - 1.0), 2);
    }

    private static Dictionary<NeedType, Need> GenerateNeeds(Random rng)
    {
        var needs = new Dictionary<NeedType, Need>();

        foreach (var (type, decayRate) in NeedDecayRates)
        {
            needs[type] = new Need
            {
                Type = type,
                Value = rng.Next(60, 91),    // 60-90 inclusive
                DecayRate = decayRate,
                CriticalThreshold = 20f
            };
        }

        return needs;
    }
}
