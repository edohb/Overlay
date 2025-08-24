# Overlay

A high-performance Windows overlay application built with DirectX 11 and ImGui, featuring compile-time string encryption and real-time FPS monitoring.

**Designed specifically for DMA Cheats who need a reliable black overlay solution.**

## Features

- **Real-time FPS monitoring** - Shows overlay rendering performance
- **Encrypted strings** - All text obfuscated at compile-time using ghostStr
- **Mouse controls** - Double-click anywhere to exit
- **Keyboard shortcuts** - ESC key to exit
- **Clean UI** - Minimalist design with ImGui dark theme

## Requirements

- **OS**: Windows 10/11 (x64)
- **Graphics**: DirectX 11 compatible GPU
- **Compiler**: Visual Studio 2022 / MSVC with C++20 support
- **Runtime**: Visual C++ Redistributable 2022

## External Dependencies

- **[ImGui](https://github.com/ocornut/imgui)** - Immediate mode GUI library
- **[ghostStr](https://github.com/edohb/ghostStr)** - Compile-time string encryption library

## Building

### Prerequisites

1. Install Visual Studio 2022 with C++ workload
2. Clone the repository:
   ```bash
   git clone https://github.com/edohb/Overlay.git
   cd Overlay
   ```

### Compilation

1. Open the project in Visual Studio 2022
2. Set build configuration:
   - **Configuration**: Release
   - **Platform**: x64
   - **C++ Standard**: C++20 (`/std:c++20`)
3. Build the solution (`Ctrl+Shift+B`)

## Usage

### Running the Overlay

1. Run the executable: `Overlay.exe`
2. The overlay will appear as a black full-screen window
3. You'll see:
   - FPS counter in the top-right corner
   - Discord link centered at the top
4. To exit:
   - Double-click anywhere on the screen, OR
   - Press the ESC key

### Security Features

All strings in the application are encrypted at compile-time using **ghostStr**:
- Window titles and class names
- Error messages  
- UI text elements
- Discord links

This prevents static analysis tools from extracting plain-text strings from the compiled binary.

## Configuration

### Modifying Text

To change the Discord link or other text elements:

```cpp
// In main.cpp
auto discord_text = ghostStr("your-new-link-here");
```

### FPS Update Rate

The FPS counter updates every 1000ms by default. To modify:

```cpp
// In UpdateOverlayFPS() function
if (duration.count() >= 500) { // Change to 500ms for faster updates
    // ...
}
```

## Development

### Adding New Features

1. **New UI Elements:**
   ```cpp
   // Add to the main rendering loop
   draw_list->AddText(ImVec2(x, y), color, ghostStr("New Text").scoped().data());
   ```

2. **New Encrypted Strings:**
   ```cpp
   auto my_string = ghostStr("sensitive text");
   auto view = my_string.scoped();
   // Use view.data() or view.c_str()
   ```

3. **Performance Monitoring:**
   ```cpp
   // The FPS counter automatically tracks overlay rendering performance
   // No additional code needed
   ```

## Contributing

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/amazing-feature`
3. Commit changes: `git commit -m 'feat: add amazing feature'`
4. Push to branch: `git push origin feature/amazing-feature`
5. Open a Pull Request

### Commit Convention

We follow conventional commits:
- `feat:` - New features
- `fix:` - Bug fixes  
- `perf:` - Performance improvements
- `refactor:` - Code refactoring
- `docs:` - Documentation changes
- `chore:` - Maintenance tasks

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- **[ocornut](https://github.com/ocornut)** - Creator of ImGui
- **[edohb](https://github.com/edohb)** - Creator of compile-time string encryption header-only library
- **Microsoft** - DirectX 11 API and development tools
- **Windows API** - Low-level system integration

## Support

- 📧 **Issues**: [GitHub Issues](https://github.com/edohb/Overlay/issues)
- 📖 **Wiki**: [Project Wiki](https://github.com/edohb/Overlay/wiki)

---

**⚠️ Note**: This overlay application is designed for DMA Cheats who require a reliable black overlay solution.
