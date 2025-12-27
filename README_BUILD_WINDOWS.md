## Build on Windows: 

#### Prerequisites

Before you begin, ensure you have the following installed:

*   **Git:** Necessary for cloning the repository.
*   **Visual Studio:** You must install the "Desktop development with C++" workload to compile packages with vcpkg.

#### Installation Steps

1.  **Open your Terminal** (PowerShell is recommended).

2.  **Clone the vcpkg repository** to a directory of your choice. A common location is `C:\dev\vcpkg`:

    ```powershell
    git clone https://github.com/microsoft/vcpkg.git C:\dev\vcpkg
    ```

3.  **Navigate into the newly created `vcpkg` directory:**

    ```powershell
    cd C:\dev\vcpkg
    ```

4.  **Run the bootstrap script** to compile the `vcpkg.exe` tool:

    ```powershell
    .\bootstrap-vcpkg.bat
    ```

---

### Using vcpkg with OhmFlux


```powershell
cd C:\dev\vcpkg
.\vcpkg install sdl3 glew opengl
```

In OhmFlux directory you can now do: 

```powershell
cmake -B build -S . `
  -DCMAKE_TOOLCHAIN_FILE="C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake" `
  -DVCPKG_TARGET_TRIPLET=x64-windows `
  -A x64
  
cmake --build build
```
