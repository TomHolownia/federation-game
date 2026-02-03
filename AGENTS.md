# AI Agent Workflow Guide

> Reference document for AI agents. **Critical workflow steps are in `.cursor/rules/agent-rules.mdc`** - those are always loaded and must be followed.

---

## Workflow Summary

1. **Check the task board in Notion** → Use the Notion API to fetch the Task Board page; pick a task from **Ready**. See [docs/technical/notion-todo.md](docs/technical/notion-todo.md). Require `NOTION_INTEGRATION_SECRET` (injected, never committed).
2. **Create worktree & branch** → `git worktree add ../federation-game-FED-XXX -b feature/FED-XXX-desc origin/main`
3. **Work in worktree** → Make changes, commit frequently. Update the task to **In Progress** in Notion.
4. **Push & create PR** → `git push -u origin HEAD && gh pr create`
5. **After merge** → Update task to **Done** in Notion with PR number; clean up worktree

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

### Update Notion Task Board
- Move task to **Done** in Notion with PR number and completion date
- Add new tasks discovered to **Backlog** in Notion (see docs/technical/notion-todo.md)

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
5. Document blocker in the Notion task board (or ask the user)

---

## Safety Rules

- Never force push to main/develop
- Never commit secrets or passwords (including **NOTION_INTEGRATION_SECRET** – use env injection only)
- Always use feature branches
- Use Git LFS for binary files (*.uasset, *.umap, *.png, etc.)
