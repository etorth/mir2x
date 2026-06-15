#!/usr/bin/env python3
import argparse
import os
import platform
import shutil
import subprocess
import sys
from pathlib import Path


SOURCE_DIR = Path(__file__).resolve().parent
DEFAULT_LOCAL_BUILD_DIR = Path.cwd().resolve()
MIR2X_VCPKG_CHAINLOAD_TOOLCHAIN = SOURCE_DIR / "cmake/Mir2xVcpkgChainload.cmake"
MIR2X_RES_REPO_URL = "https://github.com/etorth/mir2x_res.git"
MSYSTEM_TRIPLETS = {
    "CLANGARM64": "arm64-mingw-static",
    "MINGWARM64": "arm64-mingw-static",
    "MINGW64": "x64-mingw-static",
    "UCRT64": "x64-mingw-static",
    "CLANG64": "x64-mingw-static",
}
MSYSTEM_32BIT = {"MINGW32", "CLANG32"}


def run(args, *, env=None):
    subprocess.run(args, check=True, env=env)


def run_batch_file(batch_file, *args):
    run([os.environ.get("COMSPEC") or "cmd", "/c", str(batch_file), *args])


def log(message):
    print(message, flush=True)


def fail_unsupported_32bit(reason):
    print(f"32-bit builds are not supported: {reason}", file=sys.stderr)
    sys.exit(1)


def is_windows_like(system=None):
    system = system or platform.system()
    return system == "Windows" or system.startswith(("MINGW", "MSYS", "CYGWIN"))


def default_mingw_triplet():
    msystem = os.environ.get("MSYSTEM", "").upper()
    msystem_chost = os.environ.get("MSYSTEM_CHOST", "").lower()
    has_msys2_environment = bool(
        msystem
        or msystem_chost
        or os.environ.get("MINGW_PREFIX")
        or os.environ.get("MSYSTEM_PREFIX")
    )

    if not has_msys2_environment and not platform.system().startswith(("MINGW", "MSYS")):
        return None

    if msystem in MSYSTEM_32BIT:
        fail_unsupported_32bit(f"MSYSTEM={msystem}")
    if msystem_chost.startswith("i686"):
        fail_unsupported_32bit(f"MSYSTEM_CHOST={msystem_chost}")
    if msystem in MSYSTEM_TRIPLETS:
        return MSYSTEM_TRIPLETS[msystem]
    if "aarch64" in msystem_chost or "arm64" in msystem:
        return "arm64-mingw-static"

    machine = platform.machine().lower()
    if machine in ("aarch64", "arm64"):
        return "arm64-mingw-static"
    if machine in ("i386", "i686", "x86"):
        fail_unsupported_32bit(f"platform.machine()={machine}")

    return "x64-mingw-static"


def default_triplet():
    mingw_triplet = default_mingw_triplet()
    if mingw_triplet:
        return mingw_triplet
    if is_windows_like():
        return "x64-windows-static"
    return "x64-linux"


def default_host_triplet(target_triplet):
    mingw_triplet = default_mingw_triplet()
    if mingw_triplet:
        return mingw_triplet
    if "mingw" in target_triplet:
        return target_triplet
    return None


def reject_unsupported_triplet(triplet, source):
    if triplet and triplet.startswith("x86-"):
        fail_unsupported_32bit(f"{source}={triplet}")


def positive_int(value):
    try:
        result = int(value)
    except ValueError as exc:
        raise argparse.ArgumentTypeError(f"invalid positive integer: {value}") from exc

    if result <= 0:
        raise argparse.ArgumentTypeError(f"expected a positive integer: {value}")
    return result


def parse_args():
    parser = argparse.ArgumentParser(description="Build mir2x with vcpkg.")
    parser.add_argument(
        "--vcpkg-prefix",
        type=Path,
        help="Use the vcpkg installation at this prefix instead of bootstrapping a local one.",
    )
    parser.add_argument(
        "--build-dir",
        type=Path,
        help="Use this directory as the local build root. Defaults to the current directory.",
    )
    parser.add_argument(
        "--fresh",
        action="store_true",
        help="Delete <build-dir>/build, including vcpkg_installed, before configuring.",
    )
    parser.add_argument("--triplet", help="VCPKG_TARGET_TRIPLET.")
    parser.add_argument("--host-triplet", help="VCPKG_HOST_TRIPLET.")
    parser.add_argument("--c-compiler", help="C compiler for vcpkg and mir2x.")
    parser.add_argument("--cxx-compiler", help="C++ compiler for vcpkg and mir2x.")
    parser.add_argument(
        "--build-type",
        default="Release",
        help="CMAKE_BUILD_TYPE.",
    )
    parser.add_argument(
        "--parallel",
        type=positive_int,
        help="Number of parallel jobs for vcpkg port builds and mir2x target builds.",
    )
    parser.add_argument(
        "--verbose",
        action="store_true",
        help="Show verbose CMake build/install output and vcpkg toolchain messages.",
    )
    parser.add_argument(
        "--install-prefix",
        type=Path,
        help="CMAKE_INSTALL_PREFIX. Defaults to <build-dir>/install.",
    )
    parser.add_argument(
        "--res-path",
        type=Path,
        help=f"Use an existing mir2x resource path. If omitted, CMake clones {MIR2X_RES_REPO_URL} during the build.",
    )
    parser.add_argument(
        "--target",
        action="append",
        help="Build this CMake target. Can be repeated. Defaults to all targets.",
    )
    parser.add_argument(
        "--no-install",
        action="store_true",
        help="Build only; do not run cmake --install.",
    )
    return parser.parse_args()


def is_executable(path):
    return path.is_file() and os.access(path, os.X_OK)


def vcpkg_toolchain(vcpkg_root):
    return vcpkg_root / "scripts/buildsystems/vcpkg.cmake"


def resolve_vcpkg_prefix(vcpkg_prefix):
    prefix = vcpkg_prefix.expanduser().resolve()
    if prefix.is_file():
        vcpkg = prefix
        vcpkg_root = prefix.parent
    else:
        vcpkg_root = prefix
        vcpkg = vcpkg_root / ("vcpkg.exe" if is_windows_like() else "vcpkg")
        if not is_executable(vcpkg):
            fallback = vcpkg_root / "vcpkg.exe"
            if is_executable(fallback):
                vcpkg = fallback

    return vcpkg, vcpkg_root


def bootstrap_local_vcpkg(local_vcpkg_dir):
    if not local_vcpkg_dir.exists():
        log(f"Installing vcpkg into {local_vcpkg_dir}")
        run(["git", "clone", "https://github.com/microsoft/vcpkg.git", str(local_vcpkg_dir)])

    vcpkg = local_vcpkg_dir / ("vcpkg.exe" if is_windows_like() else "vcpkg")
    if is_executable(vcpkg):
        return vcpkg, local_vcpkg_dir

    bootstrap_sh = local_vcpkg_dir / "bootstrap-vcpkg.sh"
    bootstrap_bat = local_vcpkg_dir / "bootstrap-vcpkg.bat"
    if is_windows_like() and bootstrap_bat.exists():
        run_batch_file(bootstrap_bat, "-disableMetrics")
    elif bootstrap_sh.exists():
        run([str(bootstrap_sh), "-disableMetrics"])
    elif bootstrap_bat.exists():
        run_batch_file(bootstrap_bat, "-disableMetrics")
    else:
        print(f"vcpkg is not ready and no bootstrap script was found in {local_vcpkg_dir}", file=sys.stderr)
        sys.exit(1)

    if is_executable(vcpkg):
        return vcpkg, local_vcpkg_dir

    fallback = local_vcpkg_dir / "vcpkg.exe"
    if is_executable(fallback):
        return fallback, local_vcpkg_dir

    print(f"vcpkg bootstrap finished, but no vcpkg executable was found in {local_vcpkg_dir}", file=sys.stderr)
    sys.exit(1)


def main():
    args = parse_args()
    vcpkg_triplet = args.triplet or os.environ.get("VCPKG_DEFAULT_TRIPLET", default_triplet())
    vcpkg_host_triplet = args.host_triplet or os.environ.get("VCPKG_DEFAULT_HOST_TRIPLET", default_host_triplet(vcpkg_triplet))
    reject_unsupported_triplet(vcpkg_triplet, "VCPKG_TARGET_TRIPLET")
    reject_unsupported_triplet(vcpkg_host_triplet, "VCPKG_HOST_TRIPLET")
    local_build_dir = args.build_dir.expanduser().resolve() if args.build_dir else DEFAULT_LOCAL_BUILD_DIR
    local_vcpkg_dir = local_build_dir / "vcpkg"
    cmake_build_dir = local_build_dir / "build"
    selected_vcpkg = resolve_vcpkg_prefix(args.vcpkg_prefix) if args.vcpkg_prefix else None
    install_prefix = (
        args.install_prefix.expanduser().resolve()
        if args.install_prefix
        else local_build_dir / "install"
    )
    local_build_dir.mkdir(parents=True, exist_ok=True)

    if selected_vcpkg:
        vcpkg, vcpkg_root = selected_vcpkg
    else:
        vcpkg, vcpkg_root = bootstrap_local_vcpkg(local_vcpkg_dir)

    log(f"Using vcpkg: {vcpkg}")
    if args.res_path:
        log(f"Using resource path: {args.res_path.expanduser().resolve()}")
    else:
        log("Using default CMake-managed resource clone")
    if vcpkg_host_triplet:
        log(f"Configuring mir2x for {vcpkg_triplet} with host triplet {vcpkg_host_triplet}")
    else:
        log(f"Configuring mir2x for {vcpkg_triplet}")

    if args.fresh:
        log(f"Configuring a fresh build in {cmake_build_dir}")
        shutil.rmtree(cmake_build_dir, ignore_errors=True)
    else:
        log(f"Configuring incremental build in {cmake_build_dir}")
    configure_env = os.environ.copy()
    use_chainload_toolchain = bool(args.c_compiler or args.cxx_compiler)
    if args.c_compiler:
        configure_env["CC"] = args.c_compiler
    if args.cxx_compiler:
        configure_env["CXX"] = args.cxx_compiler
    if args.parallel:
        configure_env["VCPKG_MAX_CONCURRENCY"] = str(args.parallel)
        log(f"Using parallel jobs: {args.parallel}")
    if args.verbose:
        log("Using verbose output")
    if use_chainload_toolchain:
        log(f"Using compiler chainload toolchain: {MIR2X_VCPKG_CHAINLOAD_TOOLCHAIN}")

    cmake_configure_args = [
        "cmake",
    ]
    if args.verbose:
        cmake_configure_args.append("--log-level=VERBOSE")
    cmake_configure_args.extend([
        "-S",
        str(SOURCE_DIR),
        "-B",
        str(cmake_build_dir),
        f"-DCMAKE_BUILD_TYPE={args.build_type}",
        f"-DCMAKE_TOOLCHAIN_FILE={vcpkg_toolchain(vcpkg_root)}",
        f"-DVCPKG_TARGET_TRIPLET={vcpkg_triplet}",
        f"-DCMAKE_INSTALL_PREFIX={install_prefix}",
    ])
    if vcpkg_host_triplet:
        cmake_configure_args.append(f"-DVCPKG_HOST_TRIPLET={vcpkg_host_triplet}")
    if use_chainload_toolchain:
        cmake_configure_args.append("-DI_AM_BUILD_PY=ON")
        cmake_configure_args.append(f"-DVCPKG_CHAINLOAD_TOOLCHAIN_FILE={MIR2X_VCPKG_CHAINLOAD_TOOLCHAIN}")
    if args.verbose:
        cmake_configure_args.append("-DCMAKE_VERBOSE_MAKEFILE=ON")
        cmake_configure_args.append("-DVCPKG_VERBOSE=ON")
    if args.res_path:
        cmake_configure_args.append(f"-DMIR2X_RES_REPO_PATH={args.res_path.expanduser().resolve()}")
    run(cmake_configure_args, env=configure_env)

    build_base_args = ["cmake", "--build", str(cmake_build_dir), "--config", args.build_type, "--parallel"]
    if args.parallel:
        build_base_args.append(str(args.parallel))
    if args.verbose:
        build_base_args.append("--verbose")
    if args.target:
        for target in args.target:
            log(f"Building target {target}")
            run([*build_base_args, "--target", target])
    else:
        log("Building")
        run(build_base_args)

    if not args.no_install:
        log(f"Installing into {install_prefix}")
        install_args = ["cmake", "--install", str(cmake_build_dir), "--config", args.build_type]
        if args.verbose:
            install_args.append("--verbose")
        run(install_args)


if __name__ == "__main__":
    main()
