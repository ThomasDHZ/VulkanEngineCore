# VulkanEngineCore

Core architecture for a hybrid **.NET 8 + native C++** system: explicit DLL boundaries, interop, memory pools, and the C# wrappers the editor and runtime sit on.

This repo is the foundation. [VulkanGameEngine](https://github.com/ThomasDHZ/VulkanGameEngine) is the runtime built on it. [VulkanGameEngineLevelEditor](https://github.com/ThomasDHZ/VulkanGameEngineLevelEditor) is the WinForms tool that hosts it.

## Call path

```text
C# tool or script
  → C# wrapper (VulkanEngineCoreCS)
    → P/Invoke / explicit DLL export
      → native C++ (pools, renderer hooks, baker)
```

C# owns orchestration, config, and tooling. Native code owns tight loops and memory that should not live on the managed heap.

## What it provides

- Hybrid .NET 8 + native layout with a hard managed/native line
- Custom interop (P/Invoke, unsafe, Marshal, explicit exports)
- Memory pooling aimed at cutting GC pressure on hot buffers (on the order of ~200 MB in related projects)
- Modular systems the editor and runtime can load without copying the whole engine
- Cross-platform target: Windows, Linux (CMake/Ninja), Android NDK

## Tech stack

| Side | Tech |
|---|---|
| Managed | C# / .NET 8 |
| Native | C++ |
| Interop | Custom DLLs, unsafe, Marshal |
| Build | Visual Studio, CMake / Ninja |

## Related repos

- [VulkanGameEngine](https://github.com/ThomasDHZ/VulkanGameEngine) — runtime on top of this core
- [VulkanGameEngineLevelEditor](https://github.com/ThomasDHZ/VulkanGameEngineLevelEditor) — C# WinForms host
- [ListPtr](https://github.com/ThomasDHZ/ListPtr) — dense C# ↔ native transfer
- [MemoryLeakReporterDemo](https://github.com/ThomasDHZ/MemoryLeakReporterDemo) — native leak reporting from managed code
- [EclipseEngine](https://github.com/ThomasDHZ/EclipseEngine) — earlier C++ graphics work that informed the split

## Status

Active. Used by the editor and runtime repos above.
