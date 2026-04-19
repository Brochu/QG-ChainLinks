using CaseGenV3.Models;

namespace CaseGenV3.PostProcess;

public static class FlairGenerator
{
    private static readonly Dictionary<LocationType, string[]> LocationAdjectives = new()
    {
        [LocationType.Residence] = ["Suburban", "Domestic", "Quiet Street", "Household"],
        [LocationType.Workplace] = ["Office", "Workplace", "Industrial", "Corporate"],
        [LocationType.Bar] = ["Barroom", "Tavern", "Late-Night", "Downtown"],
        [LocationType.Park] = ["Park District", "Lakeside", "Riverside", "Trailside"],
        [LocationType.Store] = ["Shopfront", "Back-Alley", "Market", "Corner Store"],
        [LocationType.Church] = ["Chapel", "Parish", "Steeple", "Sanctuary"],
        [LocationType.Alley] = ["Back-Alley", "Shadowed", "Dead-End", "Darkened"],
        [LocationType.Parking] = ["Parking Lot", "Garage", "Concrete", "Underground"],
        [LocationType.Restaurant] = ["Dining Room", "Kitchen", "Bistro", "Diner"],
        [LocationType.Gym] = ["Locker Room", "Gymnasium", "Fitness Center", "Ringside"],
    };

    private static readonly Dictionary<int, string[]> TimeOfDayAdjectives = new()
    {
        [0] = ["Midnight", "Dead-of-Night", "Witching Hour", "After-Hours"],  // 0-5
        [1] = ["Early Morning", "Dawn", "Pre-Dawn", "Daybreak"],              // 6-9
        [2] = ["Midday", "Broad-Daylight", "Noontime", "High-Noon"],          // 10-13
        [3] = ["Afternoon", "Late-Day", "Twilight", "Dusk"],                  // 14-17
        [4] = ["Evening", "Nightfall", "Sundown", "After-Dark"],              // 18-23
    };

    private static readonly Dictionary<MurderMethod, string[]> MethodNouns = new()
    {
        [MurderMethod.Stabbing] = ["Stabbing", "Knifing", "Blade Murder"],
        [MurderMethod.Shooting] = ["Shooting", "Gunshot Murder", "Execution"],
        [MurderMethod.Strangulation] = ["Strangling", "Strangulation", "Choking"],
        [MurderMethod.Bludgeoning] = ["Bludgeoning", "Beating", "Blunt-Force Murder"],
        [MurderMethod.Poisoning] = ["Poisoning", "Toxic Death", "Silent Killing"],
    };

    /// <summary>
    /// Generates a case name and marks event relevance for the given crime.
    /// Returns the generated case name.
    /// </summary>
    public static string GenerateFlair(Crime crime, WorldState world)
    {
        MarkEventRelevance(crime, world);
        return GenerateCaseName(crime, world);
    }

    private static string GenerateCaseName(Crime crime, WorldState world)
    {
        var rng = world.Rng;

        // Pick adjective from either location-based or time-based pool
        string adjective;
        bool useLocationAdj = rng.Next(2) == 0;

        if (useLocationAdj && world.Locations.TryGetValue(crime.LocationId, out var location))
        {
            var locationAdjs = LocationAdjectives.GetValueOrDefault(location.Type, ["Unknown"]);
            adjective = locationAdjs[rng.Next(locationAdjs.Length)];
        }
        else
        {
            int timeSlot = crime.Timestamp.Hour switch
            {
                >= 0 and < 6 => 0,
                >= 6 and < 10 => 1,
                >= 10 and < 14 => 2,
                >= 14 and < 18 => 3,
                _ => 4,
            };
            var timeAdjs = TimeOfDayAdjectives[timeSlot];
            adjective = timeAdjs[rng.Next(timeAdjs.Length)];
        }

        // Pick method noun
        string methodNoun = "Murder"; // fallback for non-murder crimes
        if (crime.Method.HasValue && MethodNouns.TryGetValue(crime.Method.Value, out var nouns))
        {
            methodNoun = nouns[rng.Next(nouns.Length)];
        }

        return $"The {adjective} {methodNoun}";
    }

    private static void MarkEventRelevance(Crime crime, WorldState world)
    {
        var crimeTime = crime.Timestamp;
        var fortyEightHoursBefore = crimeTime.AddHours(-48);
        var oneWeekBefore = crimeTime.AddDays(-7);

        foreach (var evt in world.Events)
        {
            bool involvesPerpOrVictim = evt.ActorIds.Contains(crime.PerpetratorId)
                                     || evt.ActorIds.Contains(crime.VictimId);

            // The crime event itself
            if (evt.RelatedCrimeId == crime.Id && evt.Type == SimEventType.Murder)
            {
                evt.Relevance = EventRelevance.CrimeEvent;
            }
            // Events within 48 hours involving perpetrator or victim
            else if (involvesPerpOrVictim
                     && evt.Timestamp >= fortyEightHoursBefore
                     && evt.Timestamp < crimeTime)
            {
                evt.Relevance = EventRelevance.KeyEvent;
            }
            // Events within the week before involving either party
            else if (involvesPerpOrVictim
                     && evt.Timestamp >= oneWeekBefore
                     && evt.Timestamp < fortyEightHoursBefore)
            {
                evt.Relevance = EventRelevance.Minor;
            }
        }
    }
}
