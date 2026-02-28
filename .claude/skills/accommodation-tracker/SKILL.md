---
name: accommodation-tracker
description: Track unexpected workarounds, architectural accommodations, and repeated failed attempts (guttering) during implementation. Use after completing a major implementation stage, when the user asks to review accommodations, or when pattern-validator flags a CAUTION or RED FLAG. Also use when the user says "track accommodations", "what workarounds did we make", or "review deviations".
---

# Accommodation Tracker

Track and document unexpected workarounds, architectural changes, repeated failed attempts (guttering), and deviations from the original plan that accumulate during implementation work. This skill operates in two modes: **on-the-fly guttering detection** during implementation, and **post-stage audit** at stage boundaries.

## When to use this skill

### Post-stage audit (invoke explicitly)

Use this skill **after each major implementation stage** to audit what changed unexpectedly. Also invoke when:
- The user asks to review what workarounds were made
- You notice the implementation has drifted significantly from the original plan
- A task required 3+ unexpected changes to make something work

### Guttering detection (self-monitor continuously)

During implementation, count your attempts at solving any given problem. If you have taken **3 or more attempts** to solve the same problem without success, you are guttering. Examples:

- Adding a feature caused compile errors. Fixing those errors failed. The next fix also failed. That's 3 attempts — stop.
- A config change broke tests. You tried a different value. That also broke tests. You tried a third value. Stop.
- You refactored a function, it introduced a bug, you fixed the bug but introduced another, you fixed that but the original issue returned. Stop.

The specific mechanism doesn't matter — what matters is that you've tried and failed 3 times at the same underlying problem.

When you detect guttering:

1. **Stop** implementation of the current approach
2. **Record** the guttering event in the accommodations file as an in-progress item:
   ```markdown
   ## N. [GUTTERING] Short description of what you're stuck on

   **Status**: In progress — not yet resolved
   **Attempts**: [number of attempts so far]
   **Approach tried**: [brief description of what you've been repeating]
   ```
3. **Invoke `/pattern-validator`** proactively on the underlying assumption or approach to check whether the direction is sound
4. Based on the pattern-validator result, you may make up to **2 more attempts** to solve the problem
5. If still unresolved after those 2 attempts, or if the approach has **deviated significantly from the original plan** (e.g. introduced significant new architecture/dependencies, or significantly reworked existing architecture in ways the plan did not expect), **halt** and explain the situation to the user: what you tried, why it isn't working, and what the pattern-validator found
6. If eventually resolved, update the guttering entry to a full accommodation entry with what finally worked

## Instructions

### Step 1: Identify the accommodations file

Check if an accommodations file already exists for the current task using Glob:

```
pattern: "docs/*accommodations*" (case-insensitive)
```

Also check for a task-specific file based on the current branch name or task context.

If no file exists yet and there are accommodations to track, create one at `docs/<TASK_NAME>_ACCOMMODATIONS.md`.

### Step 2: Inventory unexpected changes

Review the current session and recent commits for changes that were NOT part of the original plan. Look for:

- **Schema changes** required by a library or service limitation
- **Infrastructure changes** (new services, config changes, port changes)
- **Workarounds** for bugs or missing features in dependencies
- **Architectural deviations** from the spec or original design
- **New dependencies** added to solve unexpected problems
- **Configuration that differs** from what documentation suggested

For each accommodation, determine:
- **What changed** and why
- **Impact level**: Low / Medium / High
- **Effort**: Low / Medium / High
- **Reversible?**: Yes (how) / No (why) / N/A
- **Was it validated?**: Did we check if this is a normal pattern? (link to pattern-validator finding if applicable)

### Step 3: Classify severity

Rate each accommodation:

| Severity | Meaning |
|----------|---------|
| **Low** | Minor config tweak, no architectural impact |
| **Medium** | Meaningful change but well-understood trade-off |
| **High** | Significant architectural deviation, affects future work |

### Step 4: Write or update the accommodations file

Use this format for each entry:

```markdown
## N. Short Title

**Impact**: [description]
**Effort**: Low / Medium / High

[2-3 sentences explaining what changed and why]

**What changed**:
- [bullet list of concrete changes]

**Trade-off**: [what we gained vs what we gave up]
```

End the file with a summary table:

```markdown
## Summary

| Accommodation | Severity | Reversible? |
|---|---|---|
| [title] | [severity] | [yes/no/n/a] |
```

### Step 5: Flag items needing community validation

If any accommodation has NOT been validated against community patterns, flag it:

```markdown
> **Needs validation**: This accommodation has not been checked against community patterns. Run `/pattern-validator [description]` to verify.
```

## Integration with pattern-validator

**Pattern-validator does not auto-invoke.** This skill is the only entry point (besides direct user invocation). It invokes pattern-validator as a child in two situations:

1. **Guttering**: When guttering is detected (see above), invoke `/pattern-validator` to check the underlying assumption before allowing further attempts.
2. **Post-stage audit**: When reviewing accommodations that have not been validated, invoke `/pattern-validator` to check them against community practices. Mark validated entries so they are not re-researched.

## Output

After running the post-stage audit, present a brief summary to the user:
- Total accommodations tracked (including any resolved guttering events)
- Breakdown by severity (Low/Medium/High)
- Any items flagged for validation
- Any unresolved guttering events still in progress
- Link to the accommodations file
