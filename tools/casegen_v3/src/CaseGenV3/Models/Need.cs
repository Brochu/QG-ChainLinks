namespace CaseGenV3.Models;

public enum NeedType
{
    Money,
    Social,
    Love,
    Safety,
    Status,
    Rest
}

public class Need
{
    public NeedType Type { get; set; }
    public float Value { get; set; } = 75f;
    public float DecayRate { get; set; } = 1f;
    public float CriticalThreshold { get; set; } = 20f;

    public bool IsCritical => Value <= CriticalThreshold;

    public void Decay(float modifier = 1f)
    {
        Value = Math.Max(0f, Value - DecayRate * modifier);
    }

    public void Satisfy(float amount)
    {
        Value = Math.Min(100f, Value + amount);
    }
}
