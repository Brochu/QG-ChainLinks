namespace CaseGenV3.Models;

public enum EvidenceType
{
    Physical,       // fingerprints, blood, fibers
    Testimonial,    // witness statements
    Circumstantial, // motive, opportunity, behavior
    Documentary,    // phone records, financial records
    Forensic        // DNA, ballistics, toxicology
}

public class Evidence
{
    public string Id { get; set; } = string.Empty;
    public EvidenceType Type { get; set; }
    public string Description { get; set; } = string.Empty;
    public string LocationId { get; set; } = string.Empty;
    public string? PointsToActorId { get; set; }
    public string? LinkedEvidenceId { get; set; }
    public bool IsRedHerring { get; set; } = false;
    public bool Discoverable { get; set; } = true;
    public string? RelatedCrimeId { get; set; }
    public string? WitnessActorId { get; set; }
}
