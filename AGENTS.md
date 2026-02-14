# AI Agent Workflow Guide

> **Reference only.** The mandatory workflow (task board → worktree → work → PR) is defined in **`.cursor/rules/agent-rules.mdc`** and is always applied by Cursor. This file adds detail and quick reference—do not duplicate or override the rule.

**Workflow order:** Task board (Confluence) → Create worktree & branch → Work in worktree, update task to In Progress → Push & create PR → After merge: Done in Confluence, remove worktree. Full instructions in agent-rules.mdc.

---

## Commit Message Format

```
[FED-XXX] Short description (50 chars max)

- What changed
- Why it was necessary

Co-authored-by: AI Assistant
```

---

## While Working

### Code Principles
- **Search before creating** - Use `rg "pattern"` to find existing solutions
- **Match existing style** - Follow conventions in surrounding code
- **Commit frequently** - Small, focused commits
- **Write tests** - Unit tests are mandatory for new code

### Testing (UE5)
```bash
# From Editor: Window > Developer Tools > Session Frontend > Automation
# Command line:
"UnrealEditor-Cmd.exe" "ProjectPath" -ExecCmds="Automation RunTests" -Unattended
```

---

## Completing a Task

### Before Creating PR
```bash
git diff origin/main...HEAD      # Review all changes
git log --oneline origin/main..HEAD  # Check commits
```

### PR Template
```markdown
## Summary
- What this PR does

## Test Plan
- [ ] Unit tests pass
- [ ] Manual testing done
- [ ] Verified in editor
```

### After PR Merged
```bash
cd ../federation-game            # Return to main repo (if you were in .worktrees/...)
git pull origin main             # Get latest
git worktree remove .worktrees/federation-game-FED-XXX
git worktree prune
```

### Update Confluence Task Board
- Move task to **Done** in Confluence with PR number and completion date
- Add new tasks discovered to **Backlog** in Confluence (via Confluence MCP or API)

---

## Quick Reference

### Git Commands
```bash
git status                       # Current state
git worktree list                # All worktrees
git worktree prune               # Remove stale worktree refs
git log --oneline -10            # Recent commits
git stash / git stash pop        # Temporarily store changes
```

**Worktrees** live under `.worktrees/` (gitignored). Create with  
`git worktree add .worktrees/federation-game-FED-XXX -b feature/FED-XXX-desc origin/main`

### GitHub CLI
```bash
gh pr list                       # Open PRs
gh pr create                     # Create PR
gh pr view                       # View current PR
```

### Search
```bash
rg "pattern" --type cpp          # Search C++ files
fd "filename"                    # Find files by name
```

---

## When Stuck

1. Re-read task requirements
2. Search codebase for similar solutions
3. Check UE5 documentation
4. Ask the user for clarification
5. Document blocker in the Confluence task board (or ask the user)

---

## Safety Rules

- Never force push to main/develop
- Never commit secrets or passwords (including **Atlassian API tokens** – use env injection only)
- Always use feature branches
- Use Git LFS for binary files (*.uasset, *.umap, *.png, etc.)
