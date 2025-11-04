# ant-kernel-filter

* A small project that filters and sends kernel messages

## Usage

The program requires superuser (root) privileges to load eBPF programs and kernel modules into the kernel.

```bash
sudo ./ant-kernel-filter
```

### General format:

```bash
sudo ./ant-kernel-filter <COMMAND> <SUBCOMMAND> [OPTIONS...]
```

### Help

Print help:

```bash
./ant-kernel-filter --help
```

## Requirements

* **Build Tools:**

* A C++23 compliant compiler (GCC 13+ or Clang 16+)
* `CMake` (version 3.23+)

* **Dependencies:**

* All dependencies are managed via `vcpkg` and are installed automatically during the build process

## Building the Project

**Clone the repository with submodules:**

```bash
git clone --recursive https://github.com/antilopinae/ant-kernel-filter
cd ant-kernel-filter
```

**Configure and build the project:**

The `utils/cmake-build.sh` script automates this process
```bash
bash ./utils/cmake-build.sh
cd build
```

Alternatively, you can run the commands manually:

```bash
# Configure the project, specifying the static triplet
cmake -B build -S .

# Build the project
cmake --build build -j$(nproc)
cd build
```