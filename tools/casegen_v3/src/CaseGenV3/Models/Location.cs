namespace CaseGenV3.Models;

public enum LocationType
{
    Residence,
    Workplace,
    Bar,
    Park,
    Store,
    Church,
    Alley,
    Parking,
    Restaurant,
    Gym
}

public class Location
{
    public string Id { get; set; } = string.Empty;
    public string Name { get; set; } = string.Empty;
    public LocationType Type { get; set; }
    public int Capacity { get; set; } = 10;
    public List<string> ConnectedLocationIds { get; set; } = [];
    public List<string> ObjectIds { get; set; } = [];
    public List<string> ActorIdsPresent { get; set; } = [];
}
