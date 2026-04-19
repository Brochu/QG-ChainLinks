using CaseGenV3.Models;

namespace CaseGenV3.Generation;

public static class ObjectGenerator
{
    private static int _objectCounter;

    public static Dictionary<string, WorldObject> Generate(
        Random rng,
        Dictionary<string, Location> locations,
        Dictionary<string, Actor> actors)
    {
        _objectCounter = 0;
        var objects = new Dictionary<string, WorldObject>();

        // Populate each location with objects
        foreach (var location in locations.Values)
        {
            var locationObjects = GenerateObjectsForLocation(rng, location);
            foreach (var obj in locationObjects)
            {
                objects[obj.Id] = obj;
                location.ObjectIds.Add(obj.Id);
            }
        }

        // Give actors personal items (phone, wallet, keys)
        foreach (var actor in actors.Values)
        {
            var personalItems = GeneratePersonalItems(rng, actor);
            foreach (var obj in personalItems)
            {
                objects[obj.Id] = obj;
                actor.Inventory.Add(obj.Id);
            }
        }

        return objects;
    }

    private static List<WorldObject> GenerateObjectsForLocation(Random rng, Location location)
    {
        var items = new List<WorldObject>();
        int count = rng.Next(2, 6); // 2-5 objects per location

        switch (location.Type)
        {
            case LocationType.Residence:
                items.AddRange(GenerateResidenceObjects(rng, location, count));
                break;
            case LocationType.Bar:
                items.AddRange(GenerateBarObjects(rng, location, count));
                break;
            case LocationType.Restaurant:
                items.AddRange(GenerateRestaurantObjects(rng, location, count));
                break;
            case LocationType.Alley:
                items.AddRange(GenerateAlleyObjects(rng, location, count));
                break;
            case LocationType.Workplace:
                items.AddRange(GenerateWorkplaceObjects(rng, location, count));
                break;
            case LocationType.Park:
                items.AddRange(GenerateParkObjects(rng, location, count));
                break;
            case LocationType.Store:
                items.AddRange(GenerateStoreObjects(rng, location, count));
                break;
            default:
                items.AddRange(GenerateGenericObjects(rng, location, count));
                break;
        }

        return items;
    }

    private static List<WorldObject> GenerateResidenceObjects(Random rng, Location location, int count)
    {
        var items = new List<WorldObject>();

        // Always at least one personal item
        items.Add(CreateObject(location.Id, "Kitchen Knife", ObjectType.Weapon, ObjectSubType.Knife));

        // Maybe a weapon (30% chance)
        if (rng.NextDouble() < 0.3)
        {
            var weaponChoice = rng.Next(3);
            items.Add(weaponChoice switch
            {
                0 => CreateObject(location.Id, "Baseball Bat", ObjectType.Weapon, ObjectSubType.BluntObject),
                1 => CreateObject(location.Id, "Hunting Rifle", ObjectType.Weapon, ObjectSubType.Gun),
                _ => CreateObject(location.Id, "Rat Poison", ObjectType.Weapon, ObjectSubType.Poison)
            });
        }

        // Personal/valuable items to fill count
        var residenceItems = new (string name, ObjectType type, ObjectSubType sub)[]
        {
            ("Diary", ObjectType.Personal, ObjectSubType.Diary),
            ("Jewelry Box", ObjectType.Valuable, ObjectSubType.Jewelry),
            ("Laptop", ObjectType.Valuable, ObjectSubType.Electronics),
            ("Cash Stash", ObjectType.Valuable, ObjectSubType.Cash),
            ("Lighter", ObjectType.Tool, ObjectSubType.Lighter),
            ("Spare Keys", ObjectType.Personal, ObjectSubType.Keys),
            ("Garden Gloves", ObjectType.Tool, ObjectSubType.Gloves),
            ("Rope", ObjectType.Tool, ObjectSubType.Rope)
        };

        while (items.Count < count)
        {
            var pick = residenceItems[rng.Next(residenceItems.Length)];
            items.Add(CreateObject(location.Id, pick.name, pick.type, pick.sub));
        }

        return items;
    }

    private static List<WorldObject> GenerateBarObjects(Random rng, Location location, int count)
    {
        var items = new List<WorldObject>
        {
            CreateObject(location.Id, "Bar Knife", ObjectType.Weapon, ObjectSubType.Knife),
            CreateObject(location.Id, "Cash Register Money", ObjectType.Valuable, ObjectSubType.Cash)
        };

        var barExtras = new (string name, ObjectType type, ObjectSubType sub)[]
        {
            ("Broken Bottle", ObjectType.Weapon, ObjectSubType.BluntObject),
            ("Tip Jar Cash", ObjectType.Valuable, ObjectSubType.Cash),
            ("Lighter", ObjectType.Tool, ObjectSubType.Lighter),
            ("Bar Stool Leg", ObjectType.Weapon, ObjectSubType.BluntObject)
        };

        while (items.Count < count)
        {
            var pick = barExtras[rng.Next(barExtras.Length)];
            items.Add(CreateObject(location.Id, pick.name, pick.type, pick.sub));
        }

        return items;
    }

    private static List<WorldObject> GenerateRestaurantObjects(Random rng, Location location, int count)
    {
        var items = new List<WorldObject>
        {
            CreateObject(location.Id, "Chef's Knife", ObjectType.Weapon, ObjectSubType.Knife),
            CreateObject(location.Id, "Cash Register", ObjectType.Valuable, ObjectSubType.Cash)
        };

        var extras = new (string name, ObjectType type, ObjectSubType sub)[]
        {
            ("Paring Knife", ObjectType.Weapon, ObjectSubType.Knife),
            ("Rolling Pin", ObjectType.Weapon, ObjectSubType.BluntObject),
            ("Cleaning Gloves", ObjectType.Tool, ObjectSubType.Gloves),
            ("Lighter", ObjectType.Tool, ObjectSubType.Lighter)
        };

        while (items.Count < count)
        {
            var pick = extras[rng.Next(extras.Length)];
            items.Add(CreateObject(location.Id, pick.name, pick.type, pick.sub));
        }

        return items;
    }

    private static List<WorldObject> GenerateAlleyObjects(Random rng, Location location, int count)
    {
        var items = new List<WorldObject>();

        // Alleys often have blunt objects and discarded items
        items.Add(CreateObject(location.Id, "Metal Pipe", ObjectType.Weapon, ObjectSubType.BluntObject));

        var alleyItems = new (string name, ObjectType type, ObjectSubType sub)[]
        {
            ("Brick", ObjectType.Weapon, ObjectSubType.BluntObject),
            ("Rusty Knife", ObjectType.Weapon, ObjectSubType.Knife),
            ("Discarded Rope", ObjectType.Tool, ObjectSubType.Rope),
            ("Broken Shovel", ObjectType.Tool, ObjectSubType.Shovel),
            ("Old Lighter", ObjectType.Tool, ObjectSubType.Lighter)
        };

        while (items.Count < count)
        {
            var pick = alleyItems[rng.Next(alleyItems.Length)];
            items.Add(CreateObject(location.Id, pick.name, pick.type, pick.sub));
        }

        return items;
    }

    private static List<WorldObject> GenerateWorkplaceObjects(Random rng, Location location, int count)
    {
        var items = new List<WorldObject>
        {
            CreateObject(location.Id, "Office Electronics", ObjectType.Valuable, ObjectSubType.Electronics)
        };

        var workItems = new (string name, ObjectType type, ObjectSubType sub)[]
        {
            ("Box Cutter", ObjectType.Weapon, ObjectSubType.Knife),
            ("Heavy Stapler", ObjectType.Weapon, ObjectSubType.BluntObject),
            ("Petty Cash", ObjectType.Valuable, ObjectSubType.Cash),
            ("Tool Kit", ObjectType.Tool, ObjectSubType.Gloves),
            ("Work Gloves", ObjectType.Tool, ObjectSubType.Gloves),
            ("Rope", ObjectType.Tool, ObjectSubType.Rope),
            ("Utility Lighter", ObjectType.Tool, ObjectSubType.Lighter),
            ("Shovel", ObjectType.Tool, ObjectSubType.Shovel)
        };

        while (items.Count < count)
        {
            var pick = workItems[rng.Next(workItems.Length)];
            items.Add(CreateObject(location.Id, pick.name, pick.type, pick.sub));
        }

        return items;
    }

    private static List<WorldObject> GenerateParkObjects(Random rng, Location location, int count)
    {
        var items = new List<WorldObject>();

        var parkItems = new (string name, ObjectType type, ObjectSubType sub)[]
        {
            ("Park Bench Slat", ObjectType.Weapon, ObjectSubType.BluntObject),
            ("Gardening Shovel", ObjectType.Tool, ObjectSubType.Shovel),
            ("Rope Swing", ObjectType.Tool, ObjectSubType.Rope),
            ("Lighter", ObjectType.Tool, ObjectSubType.Lighter),
            ("Garden Gloves", ObjectType.Tool, ObjectSubType.Gloves)
        };

        while (items.Count < count)
        {
            var pick = parkItems[rng.Next(parkItems.Length)];
            items.Add(CreateObject(location.Id, pick.name, pick.type, pick.sub));
        }

        return items;
    }

    private static List<WorldObject> GenerateStoreObjects(Random rng, Location location, int count)
    {
        var items = new List<WorldObject>
        {
            CreateObject(location.Id, "Cash Register", ObjectType.Valuable, ObjectSubType.Cash)
        };

        var storeItems = new (string name, ObjectType type, ObjectSubType sub)[]
        {
            ("Display Electronics", ObjectType.Valuable, ObjectSubType.Electronics),
            ("Jewelry Display", ObjectType.Valuable, ObjectSubType.Jewelry),
            ("Box Cutter", ObjectType.Weapon, ObjectSubType.Knife),
            ("Utility Knife", ObjectType.Weapon, ObjectSubType.Knife),
            ("Work Gloves", ObjectType.Tool, ObjectSubType.Gloves)
        };

        while (items.Count < count)
        {
            var pick = storeItems[rng.Next(storeItems.Length)];
            items.Add(CreateObject(location.Id, pick.name, pick.type, pick.sub));
        }

        return items;
    }

    private static List<WorldObject> GenerateGenericObjects(Random rng, Location location, int count)
    {
        var items = new List<WorldObject>();

        var genericItems = new (string name, ObjectType type, ObjectSubType sub)[]
        {
            ("Keys", ObjectType.Personal, ObjectSubType.Keys),
            ("Cash", ObjectType.Valuable, ObjectSubType.Cash),
            ("Electronics", ObjectType.Valuable, ObjectSubType.Electronics),
            ("Lighter", ObjectType.Tool, ObjectSubType.Lighter),
            ("Gloves", ObjectType.Tool, ObjectSubType.Gloves)
        };

        while (items.Count < count)
        {
            var pick = genericItems[rng.Next(genericItems.Length)];
            items.Add(CreateObject(location.Id, pick.name, pick.type, pick.sub));
        }

        return items;
    }

    private static List<WorldObject> GeneratePersonalItems(Random rng, Actor actor)
    {
        var items = new List<WorldObject>();

        // Everyone gets a phone
        items.Add(CreateObject(null, $"{actor.Name}'s Phone", ObjectType.Personal, ObjectSubType.Phone, actor.Id));

        // Most people have a wallet (90%)
        if (rng.NextDouble() < 0.9)
        {
            items.Add(CreateObject(null, $"{actor.Name}'s Wallet", ObjectType.Personal, ObjectSubType.Wallet, actor.Id));
        }

        // Most people have keys (85%)
        if (rng.NextDouble() < 0.85)
        {
            items.Add(CreateObject(null, $"{actor.Name}'s Keys", ObjectType.Personal, ObjectSubType.Keys, actor.Id));
        }

        return items;
    }

    private static WorldObject CreateObject(
        string? locationId,
        string name,
        ObjectType type,
        ObjectSubType subType,
        string? heldByActorId = null)
    {
        _objectCounter++;
        return new WorldObject
        {
            Id = $"obj-{_objectCounter:D3}",
            Name = name,
            Type = type,
            SubType = subType,
            LocationId = locationId,
            HeldByActorId = heldByActorId
        };
    }
}
