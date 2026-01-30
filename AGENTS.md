# AI Agent Workflow Guide

> Reference document for AI agents. **Critical workflow steps are in `.cursor/rules/agent-workflow.mdc`** - those are always loaded and must be followed.

---

## Workflow Summary

1. **Check TODO.md on main** → Pick a task from `Ready`
2. **Create worktree & branch** → `git worktree add ../federation-game-FED-XXX -b feature/FED-XXX-desc origin/main`
3. **Work in worktree** → Make changes, commit frequently
4. **Push & create PR** → `git push -u origin HEAD && gh pr create`
5. **After merge** → Clean up worktree

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
cd ../federation-game            # Return to main repo
git pull origin main             # Get latest
git worktree remove ../federation-game-FED-XXX
git worktree prune
```

### Update TODO.md
- Move task to `Done` with PR number
- Add new tasks discovered to `Backlog`

---

## Quick Reference

### Git Commands
```bash
git status                       # Current state
git worktree list                # All worktrees
git log --oneline -10            # Recent commits
git stash / git stash pop        # Temporarily store changes
```

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
5. Document blocker in TODO.md

---

## Safety Rules

- Never force push to main/develop
- Never commit secrets or passwords
- Always use feature branches
- Use Git LFS for binary files (*.uasset, *.umap, *.png, etc.)
