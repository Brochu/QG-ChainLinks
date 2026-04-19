namespace CaseGenV3.Models;

public enum RelationshipType
{
    Spouse,
    Partner,
    Friend,
    Coworker,
    Rival,
    Stranger,
    Ex,
    Family
}

public class Relationship
{
    public string ActorAId { get; set; } = string.Empty;
    public string ActorBId { get; set; } = string.Empty;
    public RelationshipType Type { get; set; }
    public float Strength { get; set; } = 0f; // -100 (hatred) to +100 (devotion)
    public float Tension { get; set; } = 0f;  // 0 to 100, drives conflict escalation
    public EscalationLevel EscalationLevel { get; set; } = EscalationLevel.None;
    public List<string> History { get; set; } = [];
}

public enum EscalationLevel
{
    None,
    ColdShoulder,
    Argument,
    Threat,
    Confrontation,
    Violence,
    Murderous
}
