# VulkanEngineCore

Core architecture and systems for a high-performance hybrid engine built with **.NET 8** and native **C++**.

This repository focuses on clean managed/native boundaries, high-performance interop, memory management, and modular design. It serves as the foundation for larger engine and tooling projects.

## Key Features

- **Hybrid .NET 8 + Native Architecture**  
  Clear separation between managed C# systems and native C++ performance-critical code.

- **High-Performance Interop**  
  Custom C#/C++ interop layer designed for low overhead and efficient data exchange.

- **Memory Management**  
  Memory pooling system aimed at reducing garbage collection pressure and improving runtime performance (estimated ~200 MB reduction in related projects).

- **Modular Design**  
  Emphasis on clean architecture, modular components, and maintainable system boundaries.

- **Cross-Platform Foundation**  
  Built with support for Windows, Linux, and Android in mind.

## Tech Stack

- **Managed**: C# / .NET 8
- **Native**: C++
- **Interop**: Custom DLLs, unsafe code, Marshal
- **Focus Areas**: Performance, memory management, modular architecture

## Related Projects

- [Vulkan Game Engine](https://github.com/ThomasDHZ/VulkanGameEngine) – Higher-level engine built on top of these core systems
- [EclipseEngine](https://github.com/ThomasDHZ/EclipseEngine) – Earlier graphics engine that informed this architecture

## Purpose

This project explores practical techniques for combining the productivity of .NET with the performance of native code. It demonstrates real-world patterns for interop, memory control, and clean system design in performance-sensitive applications.

## Status

Active development.
