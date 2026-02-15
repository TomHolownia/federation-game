# Federation Game

Huge sprawling space opera.

A 3D galaxy simulator built with Unreal Engine 5, designed for large-scale space and ground battles.

Detailed lore in wiki: https://federationgame.atlassian.net/

## Vision

Explore a living galaxy, build your fleet, and engage in epic battles across the stars and on planetary surfaces.

## Project Status

🚧 **Early Development** - Setting up core infrastructure

## Getting Started

- **First-time setup:** Full step-by-step (Git, Visual Studio, Unreal Engine, clone, open project) is in **[INSTALLATION.md](INSTALLATION.md)**.
- **Already have UE5, VS, Git LFS:** Use the quick start below.

### Quick Start (prerequisites installed)

```bash
# Clone the repository
git clone https://github.com/TomHolownia/federation-game.git
cd federation-game

# Ensure Git LFS files are downloaded
git lfs pull

# Generate project files (Windows)
# Right-click FederationGame.uproject > Generate Visual Studio project files

# Open in Unreal Editor
# Double-click FederationGame.uproject
```

### Prerequisites

| Requirement | Version |
|-------------|---------|
| Unreal Engine | 5.4+ |
| Visual Studio | 2022 (with C++ workload) |
| Git LFS | Latest |

See [INSTALLATION.md](INSTALLATION.md) for detailed installation instructions.

### Running Tests
```bash
# From Unreal Editor
# Window > Developer Tools > Session Frontend > Automation

# Filter by "FederationGame" and run
```

## Project Structure

```
federation-game/
├── docs/technical/        # Technical docs (task board is in Confluence only)
├── README.md              # This file
├── .cursor/               # Cursor IDE configuration
│   └── rules/             # Project-specific AI rules
├── Content/               # UE5 content (assets, blueprints)
│   ├── Core/              # Shared utilities
│   ├── Galaxy/            # Galaxy-level systems
│   ├── SolarSystem/       # Solar system scale
│   └── ...
├── Source/                # C++ source code
│   └── FederationGame/
│       ├── Core/          # Core systems
│       ├── Galaxy/        # Galaxy systems
│       └── Tests/         # Unit tests
└── docs/                  # Documentation
    └── technical/         # Technical documentation
```

## For AI Agents

1. **Follow `.cursor/rules/agent-rules.mdc`** — Workflow, commit format, quick ref (always applied in Cursor).
2. **Confluence task board** — federationgame.atlassian.net; Confluence MCP (`.cursor/mcp.json.example`); never commit Atlassian env vars.
3. **Follow `.cursor/rules/`** — Project conventions. Write tests for new code.

## Documentation

- [INSTALLATION.md](INSTALLATION.md) - Complete setup guide
- Task board is in **Confluence** only (federationgame.atlassian.net); use Confluence MCP (`.cursor/mcp.json.example`)
- `docs/technical/` - Technical documentation
  - [AI workflow & galaxy-scale](docs/technical/ai-workflow-and-galaxy-scale.md) - Placement, World Partition, and **level streaming** (galaxy → solar system → planet, transitions, atmosphere, cities)

## License

Private - All rights reserved

## Contact

Tom Holownia - tom.holownia15@gmail.com
