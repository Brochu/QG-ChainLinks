namespace CaseGenV3.Models;

public class WorldState
{
    public Dictionary<string, Actor> Actors { get; set; } = [];
    public Dictionary<string, Location> Locations { get; set; } = [];
    public Dictionary<string, WorldObject> Objects { get; set; } = [];
    public List<Relationship> Relationships { get; set; } = [];
    public List<SimEvent> Events { get; set; } = [];
    public List<Crime> Crimes { get; set; } = [];
    public List<Evidence> Evidence { get; set; } = [];
    public DateTime CurrentTime { get; set; }
    public Random Rng { get; set; } = new();

    public Relationship? GetRelationship(string actorAId, string actorBId)
    {
        return Relationships.FirstOrDefault(r =>
            (r.ActorAId == actorAId && r.ActorBId == actorBId) ||
            (r.ActorAId == actorBId && r.ActorBId == actorAId));
    }

    public Relationship GetOrCreateRelationship(string actorAId, string actorBId)
    {
        var rel = GetRelationship(actorAId, actorBId);
        if (rel == null)
        {
            rel = new Relationship
            {
                ActorAId = actorAId,
                ActorBId = actorBId,
                Type = RelationshipType.Stranger,
                Strength = 0f
            };
            Relationships.Add(rel);
        }
        return rel;
    }

    public List<Actor> GetActorsAtLocation(string locationId)
    {
        return Actors.Values.Where(a => a.IsAlive && a.CurrentLocationId == locationId).ToList();
    }

    public List<Relationship> GetActorRelationships(string actorId)
    {
        return Relationships.Where(r => r.ActorAId == actorId || r.ActorBId == actorId).ToList();
    }

    public string GetOtherActorId(Relationship rel, string actorId)
    {
        return rel.ActorAId == actorId ? rel.ActorBId : rel.ActorAId;
    }
}
