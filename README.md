# Federation Game

A 3D galaxy simulator built with Unreal Engine 5, designed for large-scale space and ground battles.

## Vision

Explore a living galaxy, build your fleet, and engage in epic battles across the stars and on planetary surfaces.

## Project Status

🚧 **Early Development** - Setting up core infrastructure

## Getting Started

**New to this project?** See the full **[Installation Guide](INSTALLATION.md)** for step-by-step setup instructions.

### Quick Start (if you have everything installed)

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
├── AGENTS.md              # AI agent workflow guide
├── docs/technical/
│   └── notion-todo.md     # Notion task board API usage (task board is in Notion wiki)
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

If you're an AI agent working on this project:

1. **Read `AGENTS.md` first** - Complete workflow guide
2. **Check the Notion task board** - Tasks live in the Federation wiki; use the Notion API (see `docs/technical/notion-todo.md`). Set `NOTION_INTEGRATION_SECRET` in the environment; never commit it.
3. **Follow `.cursor/rules/`** - Project-specific conventions
4. **Write tests** - All new code must be tested

## Documentation

- [INSTALLATION.md](INSTALLATION.md) - Complete setup guide
- [AGENTS.md](AGENTS.md) - AI agent workflow
- [docs/technical/notion-todo.md](docs/technical/notion-todo.md) - Notion task board API (task board is in Notion wiki)
- `docs/technical/` - Technical documentation

## License

Private - All rights reserved

## Contact

Tom Holownia - tom.holownia15@gmail.com
