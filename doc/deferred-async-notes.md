# Deferred Feature — Colleague Notes (async player traces)
### Status: **deferred, post-launch nice-to-have** · no impact on the slice or the v2.1 format

> Deferred multiplayer in the Dark Souls tradition: rare traces of other players'
> judgment surfacing in your case. No real-time multiplayer, ever. The game must be
> fully playable — mechanically identical — with this feature absent or disabled.

---

## 1. The idea

Very rarely (~one case in 5–10), a teletype arrives at the Field Office from a
colleague at another field office: another *player*, whose judgment tag on the same
case has been circulated to you. *"RE: the clerk's statement — wouldn't put weight
on it."* It might be insight; it might be a stranger's bad hunch. Nothing tells you
which. Following it or ignoring it is the same gamble as any testimony — the same
doubt muscle the rest of the game trains.

## 2. Decided constraints

- **Overlay, never facts.** A note never enters the discovered set, carries no tags,
  satisfies no prerequisite, and never appears in reconstruction dropdowns. The
  rules engine never sees notes; they are presentation-side only — which makes the
  on/off toggle trivial.
- **No freeform text.** A note is auto-derived from another player's judgment tag:
  the shared record is just `{tag_type, target_id, block_index}`. The reader's game
  renders it through small authored phrase pools per tag type, plus a generated
  period-flavored agent name. No text travels → nothing to moderate.
- **No composition step for the writer.** Players just play and tag as normal;
  behind a consent toggle, their tags feed the pool.
- **Rare by design: ~1 note per 5–10 cases.** An event, not an information layer.
  Empty pool = the event silently never fires = a normal case. No seeded fake notes
  needed.
- **Delivery filter:** a note only renders if its referenced nouns ⊆ the reader's
  discovered set, plus a couple of blocks of lag so it never echoes a discovery
  instantly. Arrives through the existing pager/fax rhythm — no new UI surface.
- **Sample honestly.** The candidate tag is drawn randomly across eligible tags —
  color facts and dead-end suspects included. If notes only pointed at load-bearing
  facts, receiving one would itself be a hint.
- **No metadata that becomes an oracle.** No writer case outcome, no ratings, no
  seniority. Every note arrives with identical, unearnable authority.
- **No mechanical bonus/malus.** Time is the only resource; a bad note costs blocks
  organically, a good one saves them.
- **Reader can judgment-tag the note itself** (DOUBTED/CLEARED); tags are pure
  player state, so this is free, and the derived post-game review can acknowledge
  it ("you doubted SA Whoever's tip; you were right").
- **Fiction: parallel colleagues** (VICAP-era Bureau information sharing between
  field offices), *not* "previous agents on a cold file" — the parallel framing
  doesn't constrain case fiction or the game's title.

## 3. Explicitly rejected

- Diorama-placed hints from other players — spoiler risk, threatens the
  "game never tells you if you're right" pillar.
- Freeform note text — moderation burden, spoiler vector.
- Ratings/appraisals — crowd signal is statistically true, i.e. an oracle.
- Bundling the writer's case outcome with the note.

## 4. Open questions

- **Eligible tags:** judgments only (DOUBTED / CLEARED) — leaning yes; KEY is
  "important to me," which reads as a hint rather than an opinion.
- **Writer-side echo:** a one-line beat next session — *"your assessment on the
  Ashford matter was circulated to another field office."* One extra field + one
  pager line; cuttable.
- **First-contact clarity:** a player may see their first note a dozen cases in,
  so the presentation must be self-explanatory with zero learned literacy. This is
  the main design work in the feature.
- Cheapest appetite test if ever unsure: post-game community stats in the derived
  review ("38% of agents accused Reyes") — zero spoiler risk, same tiny backend.

## 5. Backend note (Steam)

If the game ships on Steam, Steamworks likely covers the entire backend with no
server of our own: the shared records are tiny fixed-size blobs, a good fit for
lightweight Steamworks services (e.g. UGC/remote storage or leaderboard-style
attachments), with the consent toggle mapping onto a normal in-game option. Worth
a proper SDK survey when the time comes; the design deliberately requires nothing
more than "upload tiny record / fetch a few random records for case X."
