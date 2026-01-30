# Federation Game - Task Board

> **For AI Agents:** This file replaces a traditional Jira board. Read the instructions below before working on any tasks.

---

## Instructions for AI Agents

### Before Starting Work
1. **Check this TODO.md on MAIN branch first** - `git show origin/main:TODO.md`
2. **Follow `.cursor/rules/agent-workflow.mdc`** - Critical steps are enforced there
3. **Create a worktree and branch** for your task
4. **Move the task to `In Progress`** before starting work

### Task Format
```markdown
- [ ] **[TASK-ID]** Task title `[priority]` `[estimate]`
  - Description of what needs to be done
  - Acceptance criteria
  - Related files/modules (if known)
```

### Priority Levels
- `[P0]` - Critical/Blocking
- `[P1]` - High priority
- `[P2]` - Medium priority
- `[P3]` - Low priority/Nice-to-have

### Estimate Levels
- `[XS]` - < 1 hour
- `[S]` - 1-4 hours
- `[M]` - 4-8 hours
- `[L]` - 1-3 days
- `[XL]` - 3+ days (should be broken down)

### When Completing Tasks
1. Move task to `Done` with completion date
2. Add any new tasks discovered during work to `Backlog`
3. Update related tasks if scope changed
4. Reference the PR number in the task

---

## Backlog
> Tasks that need refinement or are not yet ready to work on.

- [ ] **[FED-010]** Design inventory system architecture `[P2]` `[M]`
  - Define data structures for items, equipment, weapons
  - Reference design docs in `gameDocs/`
  - Consider weight system from design

- [ ] **[FED-011]** Create star system data model `[P2]` `[M]`
  - Define what data each star system contains
  - Consider procedural generation parameters
  - Plan for solar system detail levels

- [ ] **[FED-012]** Research UE5 Large World Coordinates `[P2]` `[S]`
  - Document how to enable and use LWC
  - Test precision at galaxy scale
  - Write findings to `docs/technical/`

- [ ] **[FED-002]** Create basic galaxy star field renderer `[P1]` `[M]`
  - Implement instanced static mesh for stars
  - Support 10,000+ stars with good performance
  - Basic star color variation
  - Acceptance: Galaxy view with visible stars, 60fps

- [ ] **[FED-003]** Implement galaxy camera controller `[P1]` `[M]`
  - Pan, zoom, rotate controls
  - Smooth movement with configurable speeds
  - Zoom limits (galaxy scale to system scale)
  - Acceptance: Can navigate around galaxy view smoothly

---

## Ready
> Tasks that are refined and ready to be picked up.

- [ ] **[FED-001]** Set up Unreal Engine 5 project structure `[P0]` `[S]`
  - Create new UE5 project with recommended settings
  - Set up folder structure (Content/Core, Content/Galaxy, etc.)
  - Enable Large World Coordinates
  - Configure version control settings (.gitignore, Git LFS)
  - Acceptance: Project opens, builds, and runs empty scene

- [ ] **[FED-004]** Set up automated testing framework `[P0]` `[S]`
  - Configure UE5 Automation Testing
  - Create example unit test
  - Document how to run tests
  - Acceptance: Can run tests from editor and command line

---

## In Progress
> Tasks currently being worked on. Include agent/branch info.

- [ ] **[FED-014]** Reprioritize tasks - move FED-002/003 to Backlog `[P3]` `[XS]`
  - Move galaxy renderer and camera tasks to Backlog
  - Mark FED-013 as Done
  - **Branch:** feature/FED-014-reprioritize-tasks
  - **Worktree:** ../federation-game-FED-014
  - **Started:** 2026-01-30
  - **Agent:** Cursor AI

---

## In Review
> Tasks with open PRs awaiting review.

<!-- Example:
- [ ] **[FED-XXX]** Task name `[P1]` `[M]`
  - **PR:** #123
  - **Branch:** feature/FED-XXX-task-name
-->

---

## Done
> Completed tasks. Keep last 10-20 for reference.

- [x] **[FED-013]** Improve AI agent workflow enforcement `[P0]` `[S]`
  - **PR:** #2 (merged 2026-01-30)
  - **Notes:** Created `.cursor/rules/agent-workflow.mdc` with critical steps, shortened AGENTS.md

---

## Blocked
> Tasks that cannot proceed due to dependencies or issues.

<!-- Example:
- [ ] **[FED-XXX]** Task name `[P1]` `[M]`
  - **Blocked by:** [FED-YYY] or external reason
  - **Since:** 2026-01-30
-->

---

## Notes & Decisions

### Architecture Decisions
> Document important technical decisions here.

- **2026-01-30:** Chose Unreal Engine 5 for large world support and visual quality

### Dependencies
> External dependencies or prerequisites.

- Unreal Engine 5.4+ (for latest Large World Coordinates improvements)
- Git LFS (for binary assets)

---

## Task ID Counter
> Increment this when creating new tasks: **FED-015**
