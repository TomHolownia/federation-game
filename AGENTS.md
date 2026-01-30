# AI Agent Workflow Guide

> This document defines how AI agents should work on this project. Follow these instructions precisely.

---

## Before You Start Working

### 1. Check Your Context
```bash
# What branch am I on?
git branch --show-current

# What worktrees exist?
git worktree list

# What's the current state?
git status
```

### 2. Review the Task Board
- Open `TODO.md` and read through available tasks
- Pick a task from the `Ready` section
- Ensure you understand the acceptance criteria

### 3. Understand the Codebase
- Review `README.md` for project overview
- Check `.cursor/rules/` for project-specific guidelines
- Look at recent commits: `git log --oneline -20`

---

## Starting a New Task

### 1. Create a Worktree and Branch
```bash
# From the main repository, create a new worktree for your task
git worktree add ../federation-game-FED-XXX -b feature/FED-XXX-short-description

# Navigate to the worktree
cd ../federation-game-FED-XXX
```

### 2. Update the TODO.md
- Move the task from `Ready` to `In Progress`
- Add your branch name, worktree path, and start date
- Commit this change to main before starting work (or note it will be updated in PR)

---

## While Working on a Task

### Core Principles

#### Small, Modular Changes
- **Commit frequently** - Every logical unit of work should be a commit
- **Keep commits focused** - One concern per commit
- **Prefer many small PRs** over one large PR when possible

#### Code Reuse
- **ALWAYS search before creating** - Use grep/find to look for existing solutions
- **Check for existing utilities** - Don't reinvent helpers that exist
- **Reference existing patterns** - Match the style of surrounding code
- **Extract common code** - If you write something reusable, put it in a shared location

```bash
# Search for existing implementations
rg "function_name" --type cpp
rg "ClassName" --type cpp

# Find similar files
find . -name "*.cpp" | xargs grep -l "pattern"
```

#### Testing Requirements
- **Unit tests are mandatory** for all new code
- **Write tests first** when practical (TDD)
- **Test edge cases** - Empty inputs, nulls, boundaries
- **Run existing tests** before pushing to ensure no regressions

```bash
# Run all tests (UE5)
# From Editor: Window > Developer Tools > Session Frontend > Automation

# Command line
"UnrealEditor-Cmd.exe" "ProjectPath" -ExecCmds="Automation RunTests" -Unattended
```

### Commit Message Format
```
[FED-XXX] Short description (50 chars max)

- Detailed explanation of what changed
- Why this change was necessary
- Any notable decisions made

Co-authored-by: AI Assistant
```

---

## Completing a Task

### 1. Final Checks
```bash
# Ensure all tests pass
# Run the game and verify functionality
# Check for linting/formatting issues

# Review your changes
git diff main...HEAD

# Ensure commits are clean
git log --oneline main...HEAD
```

### 2. Push and Create PR
```bash
# Push your branch
git push -u origin feature/FED-XXX-short-description

# Create PR using GitHub CLI
gh pr create --title "[FED-XXX] Short description" --body "## Summary
- What this PR does

## Test Plan
- [ ] Unit tests pass
- [ ] Manual testing steps
- [ ] Verified in editor

## Related
- Closes #issue_number (if applicable)
- Related to FED-YYY
"
```

### 3. Suggest Testing to User
Always provide the user with:
- **How to test manually** - Step-by-step instructions
- **What to look for** - Expected behavior
- **Edge cases to try** - Unusual scenarios
- **Performance considerations** - If applicable

Example:
```markdown
## How to Test This Change

1. Open the project in Unreal Editor
2. Navigate to [location]
3. [Perform action]
4. **Expected:** [What should happen]

### Edge Cases to Verify
- [ ] Test with zero items
- [ ] Test with maximum items
- [ ] Test rapid repeated actions
```

### 4. After PR is Merged

```bash
# Return to main repository
cd ../federation-game

# Pull latest changes
git pull origin main

# Remove the worktree
git worktree remove ../federation-game-FED-XXX

# Prune any stale worktrees
git worktree prune

# Delete remote branch (if not auto-deleted)
git push origin --delete feature/FED-XXX-short-description
```

### 5. Update TODO.md
- Move task from `In Review` to `Done`
- Add PR number and merge date
- Add any **new tasks discovered** during work to `Backlog`
- Note any important implementation details

---

## Best Practices

### Code Quality
- **Follow existing conventions** - Match the style of the codebase
- **Use meaningful names** - Variables, functions, classes should be self-documenting
- **Add comments for "why"** - Code shows what, comments explain why
- **Keep functions small** - Single responsibility, < 50 lines ideal
- **Avoid deep nesting** - Extract to functions, use early returns

### Performance
- **Profile before optimizing** - Don't guess at bottlenecks
- **Consider scale** - Will this work with 1000x the data?
- **Cache expensive operations** - But invalidate correctly
- **Use appropriate data structures** - Maps vs arrays vs sets

### UE5 Specific
- **Use Blueprints for prototyping** - Quick iteration
- **Move to C++ for performance** - When needed
- **Prefer composition over inheritance** - Components are powerful
- **Use Data Assets** - Avoid hardcoding values
- **Leverage async loading** - For large content

### Safety
- **Never force push to main/develop**
- **Always create feature branches**
- **Review diffs before committing**
- **Don't commit secrets, keys, or passwords**
- **Don't commit large binary files** (use Git LFS)

---

## Git LFS Setup
```bash
# Track large file types
git lfs track "*.uasset"
git lfs track "*.umap"
git lfs track "*.png"
git lfs track "*.jpg"
git lfs track "*.wav"
git lfs track "*.mp3"
git lfs track "*.fbx"
```

---

## Useful Commands Reference

### Git
```bash
git status                          # Current state
git branch -a                       # All branches
git worktree list                   # All worktrees
git log --oneline -20               # Recent commits
git diff --stat main...HEAD         # Summary of changes from main
git stash                           # Temporarily store changes
git stash pop                       # Restore stashed changes
```

### GitHub CLI
```bash
gh pr list                          # Open PRs
gh pr view                          # View current branch's PR
gh pr checks                        # CI status
gh pr merge                         # Merge PR
gh issue list                       # Open issues
```

### Search
```bash
rg "pattern" --type cpp             # Search C++ files
rg "pattern" --type-add 'ue:*.h'    # Custom file type
fd "filename"                       # Find files by name
```

---

## When You're Stuck

1. **Re-read the task requirements** - Did you miss something?
2. **Search the codebase** - Similar problems may be solved
3. **Check UE5 documentation** - Official docs are comprehensive
4. **Look at existing tests** - They show how things should work
5. **Ask the user** - Clarify requirements if needed
6. **Document the blocker** - Move task to Blocked with details

---

## Starting New Work

After completing a task, you can:

1. **Pick another task from TODO.md** - Start from the top of `Ready`
2. **Create a new worktree** - Or reuse the existing one for related work
3. **Follow this workflow again** - From "Before You Start Working"

Remember: **Consistency and quality over speed.** Small, tested, well-documented changes compound into a great codebase.
