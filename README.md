# Federation Game

A 3D galaxy simulator built with Unreal Engine 5, designed for large-scale space and ground battles.

## Vision

Explore a living galaxy, build your fleet, and engage in epic battles across the stars and on planetary surfaces.

## Project Status

🚧 **Early Development** - Setting up core infrastructure

## Getting Started

### Prerequisites
- Unreal Engine 5.4+
- Visual Studio 2022 (Windows) or Xcode (Mac)
- Git with Git LFS

### Setup
```bash
# Clone the repository
git clone https://github.com/yourusername/federation-game.git
cd federation-game

# Initialize Git LFS
git lfs install
git lfs pull

# Generate project files (Windows)
# Right-click FederationGame.uproject > Generate Visual Studio project files

# Open in Unreal Editor
# Double-click FederationGame.uproject
```

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
├── TODO.md                # Task board (Jira replacement)
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
2. **Check `TODO.md`** - Pick tasks from the Ready section
3. **Follow `.cursor/rules/`** - Project-specific conventions
4. **Write tests** - All new code must be tested

## Documentation

- [AGENTS.md](AGENTS.md) - AI agent workflow
- [TODO.md](TODO.md) - Task board
- `docs/technical/` - Technical documentation (coming soon)

## License

Private - All rights reserved

## Contact

Tom Holownia - tom.holownia15@gmail.com
