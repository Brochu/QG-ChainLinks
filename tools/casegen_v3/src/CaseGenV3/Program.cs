using CaseGenV3.Generation;
using CaseGenV3.Simulation;
using CaseGenV3.Scoring;
using CaseGenV3.PostProcess;
using CaseGenV3.Output;
using CaseGenV3.Models;

// Parse CLI arguments
int seed = Environment.TickCount;
string outputPath = "case.json";
int months = 2;
int actorCount = 30;
int locationCount = 12;

var cliArgs = Environment.GetCommandLineArgs().Skip(1).ToArray();
for (int i = 0; i < cliArgs.Length; i++)
{
    switch (cliArgs[i])
    {
        case "--seed" when i + 1 < cliArgs.Length:
            seed = int.Parse(args[++i]);
            break;
        case "--output" when i + 1 < cliArgs.Length:
            outputPath = args[++i];
            break;
        case "--months" when i + 1 < cliArgs.Length:
            months = int.Parse(args[++i]);
            break;
        case "--actors" when i + 1 < cliArgs.Length:
            actorCount = int.Parse(args[++i]);
            break;
        case "--locations" when i + 1 < cliArgs.Length:
            locationCount = int.Parse(args[++i]);
            break;
        case "--help":
            Console.WriteLine("CaseGenV3 - Police Case Generator");
            Console.WriteLine("Usage: dotnet run -- [options]");
            Console.WriteLine("  --seed <int>       Random seed (default: random)");
            Console.WriteLine("  --output <path>    Output JSON file (default: case.json)");
            Console.WriteLine("  --months <int>     Simulation months (default: 2)");
            Console.WriteLine("  --actors <int>     Number of actors (default: 30)");
            Console.WriteLine("  --locations <int>  Number of locations (default: 12)");
            return;
    }
}

Console.WriteLine($"CaseGenV3 - Police Case Generator");
Console.WriteLine($"Seed: {seed}");
Console.WriteLine($"Simulating {months} months with {actorCount} actors in {locationCount} locations...");
Console.WriteLine();

var rng = new Random(seed);

// Phase 1: Generate world
Console.WriteLine("[1/6] Generating world...");
var world = WorldGenerator.Generate(rng, locationCount, actorCount);
Console.WriteLine($"  Created {world.Locations.Count} locations, {world.Actors.Count} actors, {world.Objects.Count} objects");
Console.WriteLine($"  Initial relationships: {world.Relationships.Count}");
Console.WriteLine();

// Phase 2: Run simulation
Console.WriteLine($"[2/6] Running simulation ({months} months at 15-min granularity)...");
SimulationEngine.Run(world, months);
Console.WriteLine($"  Simulation complete. {world.Events.Count} events generated.");
Console.WriteLine($"  Crimes committed: {world.Crimes.Count}");
var crimesByType = world.Crimes.GroupBy(c => c.Type).OrderByDescending(g => g.Count());
foreach (var group in crimesByType)
    Console.WriteLine($"    {group.Key}: {group.Count()}");
Console.WriteLine();

// Phase 3: Generate evidence for crimes
Console.WriteLine("[3/6] Generating evidence...");
foreach (var crime in world.Crimes.Where(c => c.Type == CrimeType.Murder))
{
    SolvabilityChecker.GenerateCrimeEvidence(crime, world);
}
Console.WriteLine($"  Total evidence pieces: {world.Evidence.Count}");
Console.WriteLine();

// Phase 4: Score and select best crime
Console.WriteLine("[4/6] Scoring crimes...");
var rankedCrimes = CrimeScorer.ScoreCrimes(world);
var murders = rankedCrimes.Where(c => c.Type == CrimeType.Murder).ToList();

if (murders.Count == 0)
{
    Console.WriteLine("  No murders occurred during the simulation. Try a different seed or longer duration.");
    Console.WriteLine($"  Suggestion: dotnet run -- --seed {seed + 1} --months {months + 1}");
    return;
}

foreach (var m in murders)
{
    var perp = world.Actors[m.PerpetratorId];
    var victim = world.Actors[m.VictimId];
    Console.WriteLine($"  Murder: {perp.Name} killed {victim.Name} (score: {m.Score:F1})");
}

// Select best solvable crime
Crime? selectedCrime = null;
foreach (var crime in murders)
{
    if (SolvabilityChecker.IsSolvable(crime, world))
    {
        selectedCrime = crime;
        break;
    }
}

if (selectedCrime == null)
{
    Console.WriteLine("  No solvable murders found. Selecting best murder and enhancing evidence...");
    selectedCrime = murders[0];
    // Add extra evidence to make it solvable
    SolvabilityChecker.GenerateCrimeEvidence(selectedCrime, world);
}

var selectedPerp = world.Actors[selectedCrime.PerpetratorId];
var selectedVictim = world.Actors[selectedCrime.VictimId];
Console.WriteLine($"  Selected: {selectedPerp.Name} → {selectedVictim.Name} (score: {selectedCrime.Score:F1})");
Console.WriteLine();

// Phase 5: Post-processing
Console.WriteLine("[5/6] Adding flair and red herrings...");
var caseName = FlairGenerator.GenerateFlair(selectedCrime, world);
RedHerringInjector.InjectRedHerrings(selectedCrime, world);
Console.WriteLine($"  Case name: \"{caseName}\"");
Console.WriteLine($"  Total evidence (with red herrings): {world.Evidence.Count(e => e.RelatedCrimeId == selectedCrime.Id)}");
Console.WriteLine();

// Phase 6: Output
Console.WriteLine("[6/6] Writing case file...");
CaseJsonWriter.WriteCaseJson(selectedCrime, world, caseName, seed, outputPath);
Console.WriteLine($"  Output written to: {outputPath}");
Console.WriteLine();
Console.WriteLine("Done!");
