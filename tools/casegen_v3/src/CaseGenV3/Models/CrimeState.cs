namespace CaseGenV3.Models;

public enum CrimePhase
{
    Panic,        // 0-2 hours: flee scene, erratic behavior
    Coverup,      // 2-24 hours: dispose weapon, clean up, avoid people
    LieBuilding,  // 1-3 days: establish alibi, act normal, seek trusted friend
    Paranoia,     // 3-14 days: avoid crime scene, suspicious behavior
    Normal        // cleared — back to regular behavior
}

public class CrimeState
{
    public string CrimeId { get; set; } = string.Empty;
    public DateTime CrimeTime { get; set; }
    public string CrimeLocationId { get; set; } = string.Empty;
    public CrimePhase Phase { get; set; } = CrimePhase.Panic;
    public string? WeaponToDisposeId { get; set; }
    public bool FledScene { get; set; }
    public bool DisposedWeapon { get; set; }
    public bool CleanedUp { get; set; }
    public bool EstablishedAlibi { get; set; }
    public bool ConfidedInFriend { get; set; }
    public int TicksInPhase { get; set; }
}
