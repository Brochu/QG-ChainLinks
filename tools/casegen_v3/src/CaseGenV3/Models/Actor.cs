namespace CaseGenV3.Models;

public class ActorTraits
{
    public float Temper { get; set; }       // -1 to 1: calm to hot-headed
    public float Greed { get; set; }        // -1 to 1: generous to greedy
    public float Jealousy { get; set; }     // -1 to 1: secure to jealous
    public float Loyalty { get; set; }      // -1 to 1: disloyal to loyal
    public float Impulsivity { get; set; }  // -1 to 1: cautious to impulsive
}

public class Actor
{
    public string Id { get; set; } = string.Empty;
    public string Name { get; set; } = string.Empty;
    public int Age { get; set; }
    public string Occupation { get; set; } = string.Empty;
    public string HomeLocationId { get; set; } = string.Empty;
    public string WorkLocationId { get; set; } = string.Empty;
    public string CurrentLocationId { get; set; } = string.Empty;
    public bool IsAlive { get; set; } = true;

    public Dictionary<NeedType, Need> Needs { get; set; } = [];
    public ActorTraits Traits { get; set; } = new();
    public List<string> Inventory { get; set; } = []; // object IDs
    public CrimeState? ActiveCrimeState { get; set; }

    public Need GetLowestNeed()
    {
        return Needs.Values.MinBy(n => n.Value)!;
    }

    public Need GetNeed(NeedType type) => Needs[type];
}
