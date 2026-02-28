---
name: pattern-validator
description: Validate unfamiliar implementation patterns against community practices. Called by accommodation-tracker during guttering and post-stage audits. Also use when the user says "validate this pattern", "is this normal", or "check if this approach is standard".
---

# Pattern Validator

Validate unfamiliar implementation patterns against community practices. This skill does **not** auto-invoke. It is called by the **accommodation-tracker** (during guttering and post-stage audits) or directly by the user.

## When this skill is used

- **Called by accommodation-tracker**: During guttering detection (to check assumptions) and post-stage audits (to validate unresearched accommodations). See accommodation-tracker for details.
- **User-invoked**: When the user explicitly asks to validate a pattern (e.g., `/pattern-validator`, "is this normal", "check if this approach is standard").

Do NOT invoke for:
- Standard CRUD operations
- Well-documented, routine library usage (e.g., basic Express routes, standard React hooks)
- Patterns you have implemented many times with high confidence
- Simple configuration following official quickstart guides

## Instructions

### Step 1: Articulate the pattern

Before searching, clearly state:
1. **What** you are about to do (one sentence)
2. **Why** it feels unfamiliar (one sentence)
3. **What library/service** is involved

Example: "I need to add user_id to every child table because PowerSync sync rules cannot JOIN. This is unusual because it denormalizes the schema. Library: PowerSync."

### Step 2: Quick web search

Perform 1-2 targeted web searches. Keep this fast -- spend no more than 2-3 search queries.

Good search patterns:
- `"[library name]" [specific pattern] site:github.com/[org]`
- `"[library name]" [pattern] best practice`
- `"[library name]" [limitation] workaround`
- `[library name] [pattern] discussion` (for forum results)

Look for:
- Official documentation confirming or denying the pattern
- GitHub issues or discussions where others hit the same situation
- Community blog posts or tutorials showing the same approach
- Stack Overflow answers with high votes

### Step 3: Classify the finding

Based on search results, classify as one of:

**NORMAL** -- Community widely uses this pattern. Proceed.
> Format: `Pattern check: NORMAL -- [one-line summary of evidence]`

**CAUTION** -- Pattern is used but has known trade-offs or alternatives. Proceed but document.
> Format: `Pattern check: CAUTION -- [one-line summary]. [Trade-off or alternative to note]`

**RED FLAG** -- Pattern contradicts community practice, has known issues, or a clearly better alternative exists. Stop and discuss with user.
> Format: `Pattern check: RED FLAG -- [one-line summary]. Recommended alternative: [brief description]`

### Step 4: Report inline

Report the finding concisely inline with your implementation work. Do not create a separate document just for this -- the finding should appear naturally in your response.

For NORMAL findings: state the verdict in one line and continue implementing.

For CAUTION findings:
1. State the verdict with the trade-off
2. Note it in the accommodations file if one exists (see integration below)
3. Continue implementing

For RED FLAG findings:
1. State the verdict and the recommended alternative
2. Stop implementation of that specific pattern
3. Present the alternative to the user
4. Wait for user decision before proceeding

### Step 5: Log to accommodation tracker (CAUTION and RED FLAG only)

If the current task has an accommodations file (check `docs/*ACCOMMODATIONS*`), add the finding:

```markdown
## N. [Short title]

**Impact**: [description]
**Effort**: [estimate]

[What the pattern is and why it was flagged]

**Pattern validation**: [CAUTION/RED FLAG] -- [summary from Step 3]
**Evidence**: [link or source from search results]

**What changed**:
- [concrete changes made or proposed]

**Trade-off**: [what we gain vs give up]
```

If no accommodations file exists and the finding is a RED FLAG, create one. For CAUTION findings without an existing file, mention the finding in your response but do not create a new file just for one item.

## Integration with accommodation-tracker

- **Accommodation-tracker** is the parent skill -- it runs DURING and AFTER implementation stages, detecting guttering and auditing workarounds
- **Pattern-validator** is always called by accommodation-tracker or the user, never auto-invoked
- When pattern-validator logs a CAUTION or RED FLAG to the accommodations file, it marks the entry as `validated` so the accommodation-tracker does not re-research it

## Speed guidance

This skill should be FAST. Target:
- 1-2 web searches (30 seconds of research, not 5 minutes)
- 1-3 lines of output for NORMAL findings
- 3-5 lines of output for CAUTION findings
- Brief paragraph + stop for RED FLAG findings

If search results are ambiguous after 2-3 queries, classify as CAUTION and move on. Do not get stuck researching.
