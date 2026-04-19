namespace CaseGenV3.Models;

public enum SimEventType
{
    Movement,
    Socialize,
    Work,
    Sleep,
    Argue,
    Threaten,
    Confront,
    Attack,
    Murder,
    Steal,
    RelationshipFormed,
    RelationshipBroken,
    CheatingDiscovered,
    VisitPartner,
    FledScene,
    DisposedWeapon,
    CleanedUp,
    EstablishedAlibi,
    ConfidedInFriend,
    SuspiciousBehavior
}

public class SimEvent
{
    public string Id { get; set; } = string.Empty;
    public DateTime Timestamp { get; set; }
    public SimEventType Type { get; set; }
    public string Description { get; set; } = string.Empty;
    public string LocationId { get; set; } = string.Empty;
    public List<string> ActorIds { get; set; } = [];
    public List<string> ObjectIds { get; set; } = [];
    public string? RelatedCrimeId { get; set; }

    /// <summary>
    /// How relevant this event is to the final case narrative.
    /// Set during post-processing.
    /// </summary>
    public EventRelevance Relevance { get; set; } = EventRelevance.Background;
}

public enum EventRelevance
{
    Background,
    Minor,
    KeyEvent,
    CrimeEvent
}
