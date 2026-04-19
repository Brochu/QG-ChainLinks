namespace CaseGenV3.Models;

public enum ObjectType
{
    Weapon,
    Valuable,
    Personal,
    Tool
}

public enum ObjectSubType
{
    // Weapons
    Knife,
    Gun,
    BluntObject,
    Poison,
    // Valuables
    Jewelry,
    Cash,
    Electronics,
    // Personal
    Phone,
    Diary,
    Keys,
    Wallet,
    // Tools
    Rope,
    Shovel,
    Lighter,
    Gloves
}

public class WorldObject
{
    public string Id { get; set; } = string.Empty;
    public string Name { get; set; } = string.Empty;
    public ObjectType Type { get; set; }
    public ObjectSubType SubType { get; set; }
    public string? LocationId { get; set; }
    public string? HeldByActorId { get; set; }
    public List<string> Fingerprints { get; set; } = [];
}
