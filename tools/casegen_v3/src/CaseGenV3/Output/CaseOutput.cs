namespace CaseGenV3.Output;

/// <summary>
/// DTO records for JSON serialization. Decoupled from simulation models.
/// </summary>

public record CaseOutputRoot(
    CaseInfo Case,
    List<LocationInfo> Locations,
    List<ActorInfo> Actors,
    List<ObjectInfo> Objects,
    List<EvidenceInfo> Evidence,
    List<TimelineEvent> Timeline,
    SolutionInfo Solution
);

public record CaseInfo(
    string Id,
    string Name,
    int Seed,
    string Difficulty,
    CrimeInfo Crime
);

public record CrimeInfo(
    string Type,
    DateTime Timestamp,
    string Location,
    string? Method,
    string Motive,
    string PerpetratorId,
    string VictimId
);

public record LocationInfo(
    string Id,
    string Name,
    string Type,
    List<string> ConnectedLocationIds
);

public record ActorInfo(
    string Id,
    string Name,
    string Role,
    string Occupation,
    ActorTraitsInfo Traits,
    List<ActorRelationshipInfo> Relationships
);

public record ActorTraitsInfo(
    float Temper,
    float Greed,
    float Jealousy,
    float Loyalty,
    float Impulsivity
);

public record ActorRelationshipInfo(
    string ActorId,
    string Type,
    float Strength
);

public record ObjectInfo(
    string Id,
    string Name,
    string Type,
    string SubType,
    string? LocationId,
    string? HeldByActorId,
    List<string> Fingerprints
);

public record EvidenceInfo(
    string Id,
    string Type,
    string Description,
    string LocationId,
    string? PointsToActorId,
    bool IsRedHerring,
    bool Discoverable,
    string? WitnessActorId
);

public record TimelineEvent(
    DateTime Timestamp,
    string Type,
    string Description,
    List<string> ActorsInvolved,
    string LocationId,
    string Relevance
);

public record SolutionInfo(
    string KillerId,
    string Motive,
    List<string> CriticalEvidenceIds,
    string EvidenceChain
);
