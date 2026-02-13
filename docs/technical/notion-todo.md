# Notion Task Board – API Usage for Agents

The project task board is maintained in the **Notion wiki** (Federation Homepage), not in `TODO.md`. AI agents must read and update tasks via the Notion API.

## Authentication (never commit the secret)

- **Integration secret** is required for all API calls. It must be **injected at runtime**, not stored in the repo.
- Set the environment variable before running any script or agent that talks to Notion:
  - **Variable name:** `NOTION_INTEGRATION_SECRET`
  - **Where to set:** Your shell profile, CI secrets, or Cursor/env config – never in a committed file.
- If you are an agent and don’t have the secret, ask the user to set `NOTION_INTEGRATION_SECRET` and then retry.

## API basics

- **Base URL:** `https://api.notion.com/v1`
- **Required headers:**
  - `Authorization: Bearer <NOTION_INTEGRATION_SECRET>`
  - `Notion-Version: 2022-06-28` (or a [supported version](https://developers.notion.com/reference/versioning))
  - `Content-Type: application/json` for request bodies

## Task Board location

- **Page title:** Task Board  
- **Page ID (stable):** `2fc64fbd-745b-8131-806a-ed0ebf6c7489`  
- **Wiki database ID:** `2fc64fbd-745b-813d-84da-c1ba45526b6f` (Federation Homepage)

You can also find the Task Board by searching the workspace and filtering for the page titled “Task Board”.

## Useful endpoints

1. **Get Task Board content (block children)**  
   `GET https://api.notion.com/v1/blocks/{page_id}/children`  
   Use the Task Board page ID above. Paginate with `?page_size=100` and `start_cursor` if needed.

2. **Search (list pages/databases shared with the integration)**  
   `POST https://api.notion.com/v1/search`  
   Body: `{}` or `{"query": "Task Board"}`. Use this to discover the Task Board page if you don’t have the ID.

3. **Update a block** (e.g. to mark a task done or edit text)  
   `PATCH https://api.notion.com/v1/blocks/{block_id}`  
   See [Notion API – Update block](https://developers.notion.com/reference/update-a-block).

4. **Append block children** (add new tasks or sections)  
   `PATCH https://api.notion.com/v1/blocks/{block_id}/children`  
   Body: `{"children": [...]}`. Max 100 blocks per request. See [Append block children](https://developers.notion.com/reference/patch-block-children).

## Task board structure

The Task Board page contains:

- A short **instructions** section for agents (workflow, format, priorities).
- Sections: **Backlog**, **Ready**, **In Progress**, **In Review**, **Done**, **Blocked**, **Notes & Task ID Counter**.

Agents should:

1. Read the page’s block children to get the current task list.
2. Pick a task from **Ready**, create a worktree in **`.worktrees/`** and a branch (see AGENTS.md), then update Notion to move that task to **In Progress** (or add a note with branch name).
3. When done, update the task to **Done** and add PR number / completion date in the block content.

## References

- [Notion API – Introduction](https://developers.notion.com/reference/intro)
- [Notion API – Blocks](https://developers.notion.com/reference/block)
- [Notion API – Retrieve block children](https://developers.notion.com/reference/get-block-children)
