# EROEOREOREOR Context Documentation

> **📖 Complete codebase context documentation for AI-assisted development**  
> Generated: August 30, 2025  
> Project: EROEOREOREOR (Unreal Engine 5 Action Game)

## 📁 Documentation Files Overview

This directory contains comprehensive context documentation for the EROEOREOREOR project, designed to provide AI assistants with deep understanding of the codebase architecture, patterns, and conventions.

### 🗂️ File Index

| File | Purpose | When to Use | Update Frequency |
|------|---------|-------------|------------------|
| **[CODEBASE_MAP.md](CODEBASE_MAP.md)** | Directory structure and system organization | Starting new features, onboarding | When adding new systems |
| **[CLASS_HIERARCHY.md](CLASS_HIERARCHY.md)** | Inheritance chains and component relationships | Understanding class structure, refactoring | When adding new classes |
| **[SYSTEM_INTERFACES.md](SYSTEM_INTERFACES.md)** | Delegates, event flows, and system communication | Integrating systems, debugging events | When changing system interfaces |
| **[DATA_ARCHITECTURE.md](DATA_ARCHITECTURE.md)** | DataTables, structs, and data flow patterns | Working with game data, balancing | When changing data structures |
| **[COMMON_OPERATIONS.md](COMMON_OPERATIONS.md)** | Code snippets for frequent tasks | Daily development, code examples | When adding new patterns |
| **[NAMING_CONVENTIONS.md](NAMING_CONVENTIONS.md)** | Project naming standards and patterns | Creating new files, code review | Rarely (establish standards) |
| **[BUILD_SETTINGS.md](BUILD_SETTINGS.md)** | Module dependencies and build configuration | Build issues, performance tuning | When changing dependencies |
| **[CURRENT_WORK_CONTEXT.md](CURRENT_WORK_CONTEXT.md)** | Active work tracking template | Every development session | **Daily/Per session** |

## 🚀 Quick Start Guide

### For New Developers

1. **Start with**: `CODEBASE_MAP.md` - Get overview of project structure
2. **Then read**: `CLASS_HIERARCHY.md` - Understand how classes relate
3. **Review**: `NAMING_CONVENTIONS.md` - Learn project standards
4. **Use**: `COMMON_OPERATIONS.md` - Copy-paste common code patterns

### For AI-Assisted Development

1. **Always update**: `CURRENT_WORK_CONTEXT.md` before starting
2. **Reference**: Relevant documentation files based on your task
3. **Share context**: Provide AI with relevant documentation sections
4. **Update documentation** if you find gaps or outdated information

### For Code Reviews

1. **Validate**: New code follows `NAMING_CONVENTIONS.md` standards  
2. **Check**: System integration matches `SYSTEM_INTERFACES.md` patterns
3. **Verify**: Data structures align with `DATA_ARCHITECTURE.md` principles
4. **Ensure**: Build changes are documented in `BUILD_SETTINGS.md`

## 🎯 Using These Files with AI Assistants

### 💬 Effective AI Collaboration Workflow

#### 1. **Context Preparation**
```markdown
Before asking AI for help:
✅ Update CURRENT_WORK_CONTEXT.md with your active work
✅ Identify which documentation files are relevant to your task
✅ Note any known issues or constraints from CURRENT_WORK_CONTEXT.md
```

#### 2. **Providing Context to AI**
```markdown
When requesting AI assistance, include:
🔗 Link to relevant documentation sections
📋 Current work status from CURRENT_WORK_CONTEXT.md
🎯 Specific file locations from CODEBASE_MAP.md
⚠️ Any known issues or constraints
```

#### 3. **Task-Specific Documentation Usage**

| Task Type | Primary Documentation | Supporting Files |
|-----------|----------------------|------------------|
| **Adding new combat action** | `DATA_ARCHITECTURE.md` | `COMMON_OPERATIONS.md`, `NAMING_CONVENTIONS.md` |
| **Creating new ability** | `CLASS_HIERARCHY.md` | `SYSTEM_INTERFACES.md`, `COMMON_OPERATIONS.md` |
| **Debugging system integration** | `SYSTEM_INTERFACES.md` | `CODEBASE_MAP.md`, `BUILD_SETTINGS.md` |
| **Performance optimization** | `BUILD_SETTINGS.md` | `CODEBASE_MAP.md`, `CLASS_HIERARCHY.md` |
| **Adding new component** | `CLASS_HIERARCHY.md` | `NAMING_CONVENTIONS.md`, `COMMON_OPERATIONS.md` |
| **Data table modifications** | `DATA_ARCHITECTURE.md` | `NAMING_CONVENTIONS.md` |

#### 4. **Example AI Prompt with Context**
```markdown
I'm working on adding a new wall-run ability to the movement system. 

**Context from CURRENT_WORK_CONTEXT.md:**
- Active Feature: Advanced Movement System Expansion
- Current Files: GameplayAbility_Dash.h/.cpp (reference implementation)
- Known Issues: None related to movement abilities

**Requirements from CODEBASE_MAP.md:**
- Should integrate with existing VelocitySnapshotComponent
- Follow GAS patterns like GameplayAbility_Dash
- Add to MyCharacter input bindings

**From NAMING_CONVENTIONS.md:**
- Class name: UGameplayAbility_WallRun  
- Input action: IA_WallRun
- Gameplay tag: Ability.WallRun

**From COMMON_OPERATIONS.md:**
- Use the "Creating New Gameplay Abilities" section as reference
- Follow the ability registration pattern in PossessedBy()

Please help me implement this new wall-run ability following the project's established patterns.
```

## 📋 When to Update Each File

### 🔄 High-Frequency Updates (Weekly/As Needed)

#### **CURRENT_WORK_CONTEXT.md** 
- **When**: Start of each development session
- **What**: Active feature, recent changes, current issues, next tasks
- **Owner**: Individual developer
- **Critical**: Keep this current for effective AI assistance

### 🔄 Medium-Frequency Updates (Monthly/Major Features)

#### **CODEBASE_MAP.md**
- **When**: Adding new systems, major refactoring
- **What**: New directories, system groups, integration points
- **Owner**: Tech lead or senior developer
- **Examples**: Adding AI system, new gameplay mechanics

#### **COMMON_OPERATIONS.md**
- **When**: Establishing new coding patterns, adding utilities
- **What**: New code snippets, improved patterns, common tasks
- **Owner**: Team (collaborative)
- **Examples**: New ability creation pattern, improved debugging utilities

### 🔄 Low-Frequency Updates (Quarterly/Architecture Changes)

#### **CLASS_HIERARCHY.md**
- **When**: Adding new base classes, major refactoring
- **What**: New inheritance chains, component compositions
- **Owner**: Architecture team
- **Examples**: New base classes, component restructuring

#### **SYSTEM_INTERFACES.md**
- **When**: Adding new delegates, changing system communication
- **What**: New event systems, communication patterns
- **Owner**: System designers
- **Examples**: New event types, system integration changes

#### **DATA_ARCHITECTURE.md**
- **When**: Adding new data structures, changing data flow
- **What**: New DataTables, struct definitions, data patterns
- **Owner**: Data designers
- **Examples**: New combat data, save system changes

#### **BUILD_SETTINGS.md**
- **When**: Adding dependencies, changing build configuration
- **What**: Module dependencies, platform settings, optimization flags
- **Owner**: Build engineer
- **Examples**: New modules, platform support, performance settings

### 🔄 Rare Updates (Major Versions/Standards Changes)

#### **NAMING_CONVENTIONS.md**
- **When**: Establishing project standards, major style changes
- **What**: Naming patterns, code organization standards
- **Owner**: Technical director
- **Examples**: Project style guide changes, new coding standards

## 🛠️ Maintenance Procedures

### 📅 Weekly Maintenance

```markdown
[ ] Review CURRENT_WORK_CONTEXT.md for accuracy
[ ] Update CODEBASE_MAP.md if new systems were added
[ ] Add new patterns to COMMON_OPERATIONS.md if discovered
[ ] Validate all documentation links and references
```

### 📅 Monthly Maintenance

```markdown
[ ] Comprehensive review of all documentation files
[ ] Remove outdated information and dead links
[ ] Add missing systems or components
[ ] Update performance data and benchmarks
[ ] Verify naming conventions are being followed
[ ] Check that build settings reflect current configuration
```

### 📅 Major Release Maintenance

```markdown
[ ] Complete audit of all documentation accuracy
[ ] Update version numbers and generation dates
[ ] Archive old documentation versions
[ ] Create new baseline for next development cycle
[ ] Review and update maintenance procedures
```

## 🎯 Best Practices for AI-Assisted Development

### ✅ DO

**Before Starting Development:**
- Update `CURRENT_WORK_CONTEXT.md` with your current focus
- Identify which documentation files are relevant to your task
- Review existing patterns in `COMMON_OPERATIONS.md`
- Check `SYSTEM_INTERFACES.md` for integration points

**When Working with AI:**
- Provide specific file references from `CODEBASE_MAP.md`
- Share relevant sections (not entire files) for focused assistance
- Reference naming conventions to ensure consistency
- Mention any constraints from `CURRENT_WORK_CONTEXT.md`

**After Completing Work:**
- Update documentation if you discovered new patterns
- Add new code snippets to `COMMON_OPERATIONS.md` if reusable
- Document any architectural changes
- Update `CURRENT_WORK_CONTEXT.md` with progress

### ❌ DON'T

**Avoid These Mistakes:**
- Don't skip updating `CURRENT_WORK_CONTEXT.md` - it's crucial for AI context
- Don't make architectural changes without updating relevant documentation
- Don't copy code patterns without understanding the underlying systems
- Don't ignore naming conventions - consistency is critical
- Don't add new dependencies without documenting in `BUILD_SETTINGS.md`

## 🔍 Troubleshooting Documentation Issues

### Common Problems and Solutions

#### **"AI doesn't understand my codebase"**
**Solution**: Ensure `CURRENT_WORK_CONTEXT.md` is updated and share relevant documentation sections with specific file references.

#### **"AI suggests code that doesn't fit project patterns"**  
**Solution**: Reference `NAMING_CONVENTIONS.md` and `COMMON_OPERATIONS.md` in your prompts to establish project standards.

#### **"AI recommendations break existing systems"**
**Solution**: Share `SYSTEM_INTERFACES.md` sections showing how systems communicate and integrate.

#### **"Code doesn't build after AI suggestions"**
**Solution**: Reference `BUILD_SETTINGS.md` to ensure suggested dependencies are available.

#### **"Documentation seems outdated"**
**Solution**: Update relevant files based on current codebase state and note changes in `CURRENT_WORK_CONTEXT.md`.

## 📊 Documentation Quality Metrics

### ✅ Quality Indicators

**Good Documentation State:**
- All files have been updated within their recommended frequency
- `CURRENT_WORK_CONTEXT.md` reflects actual active work
- Code examples in `COMMON_OPERATIONS.md` compile and work
- File references in `CODEBASE_MAP.md` match actual project structure
- Build information in `BUILD_SETTINGS.md` matches actual Build.cs configuration

### ⚠️ Warning Signs

**Documentation Needs Attention When:**
- `CURRENT_WORK_CONTEXT.md` hasn't been updated in >1 week
- File references point to non-existent files
- Code examples don't compile or use outdated APIs
- New systems exist but aren't documented in `CODEBASE_MAP.md`
- AI assistants consistently provide poor suggestions despite using documentation

## 🎮 Project-Specific Notes

### EROEOREOREOR Architecture Highlights

**Key Architectural Decisions:**
- **Composition over inheritance**: Modular component-based design
- **Data-driven combat**: CSV-based action definitions for rapid iteration
- **GAS integration**: Full Gameplay Ability System for abilities and attributes
- **Frame-accurate timing**: 60fps baseline for combat system precision
- **Performance-conscious**: Component caching and optimized update patterns

**Unique Systems:**
- **Dash-Bounce Combo System**: VelocitySnapshotComponent preserves momentum between abilities
- **Frame-Accurate Combat**: 60fps timing with cancel windows and priority interrupts  
- **Shape-Based Attacks**: Runtime hitbox generation with debug visualization
- **Data-Driven Balance**: CSV files for easy designer iteration without recompiling

**Development Philosophy:**
- **Rapid iteration**: Hot-reload support for data and code changes
- **Designer-friendly**: Extensive Blueprint exposure and data-driven design
- **Performance-aware**: Profile-guided optimizations and efficient patterns
- **Maintainable**: Clear separation of concerns and consistent conventions

---

## 📞 Support and Feedback

**For documentation issues:**
- Create issue in project repository
- Tag with `documentation` label
- Reference specific file and section

**For AI assistance improvement:**
- Update `CURRENT_WORK_CONTEXT.md` with better context
- Add missing patterns to `COMMON_OPERATIONS.md`
- Share successful AI interaction patterns with team

**For architecture questions:**
- Consult `CLASS_HIERARCHY.md` and `SYSTEM_INTERFACES.md`
- Review existing patterns in `CODEBASE_MAP.md`
- Ask senior developers to update documentation with clarifications

---

*Last Updated: August 30, 2025*  
*Generated by: AI Assistant (Cline)*  
*Maintainer: Development Team*
