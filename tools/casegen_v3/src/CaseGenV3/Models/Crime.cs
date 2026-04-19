namespace CaseGenV3.Models;

public enum CrimeType
{
    Murder,
    Assault,
    Theft
}

public enum MurderMethod
{
    Stabbing,
    Shooting,
    Strangulation,
    Bludgeoning,
    Poisoning
}

public class Crime
{
    public string Id { get; set; } = string.Empty;
    public CrimeType Type { get; set; }
    public DateTime Timestamp { get; set; }
    public string LocationId { get; set; } = string.Empty;
    public string PerpetratorId { get; set; } = string.Empty;
    public string VictimId { get; set; } = string.Empty;
    public MurderMethod? Method { get; set; }
    public string? WeaponId { get; set; }
    public string Motive { get; set; } = string.Empty;
    public List<string> MotiveFactors { get; set; } = [];
    public List<string> WitnessIds { get; set; } = [];
    public List<string> EvidenceIds { get; set; } = [];
    public float Score { get; set; } = 0f;
}
