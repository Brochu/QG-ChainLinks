// ============================================================================
//  sawyer_ridge.ink  —  prose & interview flow for case "sawyer-ridge"
//  STUBS ONLY. Knot structure + discoverFact() contract wired to the fact
//  manifests in case.json. Real dialogue/documents/outcome memos come later.
//  Wholly fictional; any resemblance to real persons or events is unintended.
//
//  Each interview knot emits exactly the facts in that interview's
//  fact_manifest (case.json). Keep them in sync — the validator cross-checks
//  manifest <-> discoverFact() calls for reachability.
// ============================================================================

// --- Engine <-> ink contract (kept tiny & stable; see design doc 9.1) --------
EXTERNAL discoverFact(id)
EXTERNAL hasFact(id)
EXTERNAL contradictionActive(id)
EXTERNAL day()
EXTERNAL spendBlock()

// Library file: no linear main flow. The engine jumps straight to a knot via
// ChoosePathString(...). This root divert just keeps the story valid.
-> END

// Fallback definitions so this file compiles & runs standalone under inklecate
// when the engine isn't bound. The real implementations live in the runtime.
=== function discoverFact(id) ===
~ return
=== function hasFact(id) ===
~ return false
=== function contradictionActive(id) ===
~ return false
=== function day() ===
~ return 0
=== function spendBlock() ===
~ return


// ============================================================================
//  BRIEFING
//  Starting facts (F-001..F-004) are seeded by the loader from
//  case.starting_facts — the briefing only narrates them, so no discoverFact
//  calls here (avoids double-delivery).
// ============================================================================
=== briefing ===
# TODO: opening memo — three Cranes missing off Sawyer Ridge; Boyd's warning.
-> DONE


// ============================================================================
//  INTERVIEWS
//  Pattern reminder for later authoring: gate topics on evidence/leverage, e.g.
//    { hasFact("F-041") or contradictionActive("C-03"):
//        ~ discoverFact("F-053")
//    }
//  For now each stub emits its full manifest unconditionally.
// ============================================================================

=== interview_boyd ===
# TODO: Boyd Reems — the alarmed in-law who kicks the case off.
~ discoverFact("F-040")   // Wade packed & armed; liar; raised on Ottis's guns
~ discoverFact("F-041")   // forged "gone to Arkansas —Dale" note
~ discoverFact("F-005")   // the forged note is grounds for a farmhouse warrant (shared-fact idiom)
-> DONE

=== interview_lorraine ===
# TODO: Lorraine Reems — grief + names the accomplice.
~ discoverFact("F-043")   // names Rhonda as Wade's half-sister / likely helper
-> DONE

=== interview_rhonda_motel ===
# TODO: Rhonda at the Blue Spruce — the lies.
~ discoverFact("F-050")   // "only just reconnected with Wade"
~ discoverFact("F-051")   // "green Bronco doesn't run"
~ discoverFact("F-052")   // accuses Boyd
~ discoverFact("F-053")   // slip: refers to them as dead
~ discoverFact("F-005")   // her slip is also grounds for a farmhouse warrant (shared-fact idiom)
-> DONE

=== interview_wade_voluntary ===
# TODO: Wade walks in — free to leave; probes & deflects.
~ discoverFact("F-110")   // "Boyd fired on me on the 21st"
-> DONE

=== interview_rhonda_confession ===
# TODO: confront with contradictions — she gives up the cleanup account.
~ discoverFact("F-120")   // Wade went in alone; she helped scrub & paint
~ discoverFact("F-121")   // log-chain + tractor + tipping the truck
~ discoverFact("F-122")   // admits tampering; "he said they were already dead"
-> DONE

=== interview_wade_interrogation ===
# TODO: Wade in custody — partial confession, blames the victims.
~ discoverFact("F-111")   // "Dale and Cody shot each other"
~ discoverFact("F-112")   // covering & burning the truck ("a pyre")
-> DONE


// ============================================================================
//  OUTCOME MEMOS  (one knot per grading tier; see case.outcome_tiers)
// ============================================================================
=== outcome_case_closed ===
# TODO: Case Closed — conviction, commendation.
-> DONE

=== outcome_plea_bargain ===
# TODO: Plea Bargain — gaps let the defense deal down.
-> DONE

=== outcome_mistrial ===
# TODO: Mistrial — right suspect, case didn't hold.
-> DONE

=== outcome_unsolved ===
# TODO: Wrong Man / Unsolved — honest consequences.
-> DONE
