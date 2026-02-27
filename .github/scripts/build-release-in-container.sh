#!/usr/bin/env bash
set -euxo pipefail

: "${TARGET_ARCH:?TARGET_ARCH is required}"
: "${GITHUB_SHA:?GITHUB_SHA is required}"
: "${GITHUB_RUN_ID:?GITHUB_RUN_ID is required}"

BUILD_DIR="build/ci-release-tflite-${TARGET_ARCH}"
ARMHF_PROCESSOR="armv7"

apt-get update
apt-get install -y \
  ca-certificates \
  curl \
  git \
  ninja-build \
  g++ \
  gcc \
  make \
  pkg-config \
  python3 \
  python3-pip \
  libfmt-dev \
  libpaho-mqtt-dev

python3 -m pip install --break-system-packages "cmake>=3.30,<3.32"
export PATH="${HOME}/.local/bin:${PATH}"
cmake --version

git config --global --add safe.directory "/work"
git submodule sync -- 4diacFORTE
git -c protocol.version=2 submodule update --init --depth=1 4diacFORTE

mapfile -t format_files < <(grep -Rsl -- "#include <format>" 4diacFORTE || true)
for f in "${format_files[@]}"; do
  sed -i "s|#include <format>|#include <fmt/format.h>|g" "${f}"
done
mapfile -t format_to_files < <(grep -Rsl -- "std::format_to(" 4diacFORTE || true)
for f in "${format_to_files[@]}"; do
  sed -i "s/std::format_to(/fmt::format_to(/g" "${f}"
done
mapfile -t format_call_files < <(grep -Rsl -- "std::format(" 4diacFORTE || true)
for f in "${format_call_files[@]}"; do
  sed -i "s/std::format(/fmt::format(/g" "${f}"
done
grep -RsnE "#include <format>|std::format\(|std::format_to\(" 4diacFORTE || true

paho_include_dir="$(dpkg -L libpaho-mqtt-dev | grep "/MQTTAsync.h$" | head -n1 | sed "s#/MQTTAsync.h##")"
paho_lib="$(ldconfig -p | awk "/libpaho-mqtt3a\\.so/{print \$NF; exit}")"
if [ -z "${paho_lib}" ]; then
  paho_lib="$(ldconfig -p | awk "/libpaho-mqtt3as\\.so/{print \$NF; exit}")"
fi
test -n "${paho_include_dir}"
test -f "${paho_include_dir}/MQTTAsync.h"
test -n "${paho_lib}"
test -e "${paho_lib}"
paho_lib_dir="$(dirname "${paho_lib}")"

tflite_include_dir=""
tflite_library_path=""
tflite_flatbuffers_include=""
tflite_extra_libraries=""

if apt-cache show libtensorflow-lite-dev >/dev/null 2>&1; then
  apt-get install -y libtensorflow-lite-dev
  tflite_include_dir="$(dpkg -L libtensorflow-lite-dev | grep "/tensorflow/lite/interpreter.h$" | head -n1 | sed "s#/tensorflow/lite/interpreter.h##")"
  tflite_library_path="$(ldconfig -p | awk "/libtensorflowlite\\.so/{print \$NF; exit}")"
  if [ -z "${tflite_library_path}" ]; then
    tflite_library_path="$(ldconfig -p | awk "/libtensorflow-lite\\.so/{print \$NF; exit}")"
  fi
  if [ -f "${tflite_include_dir}/flatbuffers/flatbuffers.h" ]; then
    tflite_flatbuffers_include="${tflite_include_dir}"
  elif [ -f "/usr/include/flatbuffers/flatbuffers.h" ]; then
    tflite_flatbuffers_include="/usr/include"
  fi
fi

if [ -z "${tflite_include_dir}" ] || [ -z "${tflite_library_path}" ] || [ ! -f "${tflite_include_dir}/tensorflow/lite/interpreter.h" ] || [ ! -f "${tflite_library_path}" ]; then
  rm -rf third_party/tensorflow third_party/tflite-build third_party/flatbuffers third_party/cpuinfo
  git clone --depth=1 --branch v2.19.1 https://github.com/tensorflow/tensorflow.git third_party/tensorflow
  git clone --depth=1 --branch v24.3.25 https://github.com/google/flatbuffers.git third_party/flatbuffers

  tflite_cmake_args=(
    -DCMAKE_BUILD_TYPE=Release
    -DBUILD_SHARED_LIBS=ON
    -DTFLITE_ENABLE_XNNPACK=OFF
    -DFETCHCONTENT_SOURCE_DIR_FLATBUFFERS="/work/third_party/flatbuffers"
  )

  if [ "${TARGET_ARCH}" = "armhf" ]; then
    git clone https://github.com/pytorch/cpuinfo.git third_party/cpuinfo
    git -C third_party/cpuinfo checkout 8a1772a0c5c447df2d18edf33ec4603a8c9c04a6
    python3 - <<'PY'
from pathlib import Path

p = Path("third_party/cpuinfo/CMakeLists.txt")
text = p.read_text()
needle = 'SET(CPUINFO_TARGET_PROCESSOR "${CMAKE_SYSTEM_PROCESSOR}")\n'
insert = (
    'SET(CPUINFO_TARGET_PROCESSOR "${CMAKE_SYSTEM_PROCESSOR}")\n'
    '# Force processor detection from compiler target in armhf containers on arm64 hosts.\n'
    'EXECUTE_PROCESS(\n'
    '  COMMAND "${CMAKE_C_COMPILER}" -dumpmachine\n'
    '  OUTPUT_VARIABLE CPUINFO_C_DUMP_MACHINE\n'
    '  OUTPUT_STRIP_TRAILING_WHITESPACE\n'
    '  ERROR_QUIET\n'
    ')\n'
    'IF(CPUINFO_C_DUMP_MACHINE MATCHES "^(arm|armv[5-8]).*")\n'
    '  SET(CPUINFO_TARGET_PROCESSOR "armv7")\n'
    'ENDIF()\n'
)
if needle not in text:
    raise SystemExit("cpuinfo CMakeLists layout changed; needle missing")
text = text.replace(needle, insert, 1)
text = text.replace(
    'IF(CMAKE_SYSTEM_PROCESSOR MATCHES "^armv[5-8]")',
    'IF(CPUINFO_TARGET_PROCESSOR MATCHES "^armv[5-8]")',
)
text = text.replace(
    'ELSEIF(CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|arm64)$")',
    'ELSEIF(CPUINFO_TARGET_PROCESSOR MATCHES "^(aarch64|arm64)$")',
)
p.write_text(text)
PY
    tflite_cmake_args+=(
      -DCMAKE_SYSTEM_PROCESSOR="${ARMHF_PROCESSOR}"
      -DFETCHCONTENT_SOURCE_DIR_CPUINFO="/work/third_party/cpuinfo"
    )
  fi

  cmake -S third_party/tensorflow/tensorflow/lite \
    -B third_party/tflite-build \
    -G Ninja \
    "${tflite_cmake_args[@]}"

  if [ "${TARGET_ARCH}" = "armhf" ]; then
    grep -Eq "^CMAKE_SYSTEM_PROCESSOR:.*=${ARMHF_PROCESSOR}$" third_party/tflite-build/CMakeCache.txt
    if grep -q "/cpuinfo/src/arm/linux/aarch64-isa.c" third_party/tflite-build/build.ninja; then
      echo "armhf build incorrectly selected cpuinfo aarch64 source file"
      grep -n "/cpuinfo/src/arm/linux/aarch64-isa.c" third_party/tflite-build/build.ninja || true
      exit 1
    fi
  fi

  cmake --build third_party/tflite-build --target tensorflow-lite -j"$(nproc)"

  mapfile -t tflite_targets < <(ninja -C third_party/tflite-build -t targets all | cut -d: -f1)
  declare -A tflite_target_set=()
  for t in "${tflite_targets[@]}"; do
    tflite_target_set["${t}"]=1
  done
  required_absl_targets=(
    absl_log_internal_message
    absl_log_internal_format
    absl_log_internal_globals
    absl_log_internal_check_op
    absl_log_internal_conditions
    absl_log_internal_log_sink_set
    absl_log_internal_nullguard
    absl_log_internal_proto
    absl_log_entry
    absl_log_sink
    absl_log_flags
    absl_log_globals
    absl_log_initialize
    absl_log_severity
  )
  build_absl_targets=()
  for t in "${required_absl_targets[@]}"; do
    if [ "${tflite_target_set[${t}]+x}" = "x" ]; then
      build_absl_targets+=("${t}")
    fi
  done
  if [ "${#build_absl_targets[@]}" -gt 0 ]; then
    cmake --build third_party/tflite-build --target "${build_absl_targets[@]}" -j"$(nproc)"
  fi

  tflite_include_dir="/work/third_party/tensorflow"
  tflite_library_path="$(find /work/third_party/tflite-build -type f \( -name "libtensorflow-lite.so" -o -name "libtensorflowlite.so" \) | head -n1)"
  if [ -z "${tflite_library_path}" ]; then
    tflite_library_path="$(find /work/third_party/tflite-build -type f \( -name "libtensorflow-lite.a" -o -name "libtensorflowlite.a" \) | head -n1)"
  fi
  tflite_flatbuffers_include="/work/third_party/flatbuffers/include"

  mapfile -t absl_libs < <(find /work/third_party/tflite-build -type f \( -name "libabsl*.so*" -o -name "libabsl*.a" \) | sort -u)
  if [ "${#absl_libs[@]}" -gt 0 ]; then
    tflite_extra_libraries="$(printf "%s;" "${absl_libs[@]}")"
    tflite_extra_libraries="${tflite_extra_libraries%;}"
  fi
fi

test -n "${tflite_include_dir}"
test -n "${tflite_library_path}"
test -f "${tflite_include_dir}/tensorflow/lite/interpreter.h"
test -f "${tflite_library_path}"
if [ -n "${tflite_flatbuffers_include}" ]; then
  test -f "${tflite_flatbuffers_include}/flatbuffers/flatbuffers.h"
fi

cmake_args=(
  -DCMAKE_CXX_FLAGS=-DFMT_HEADER_ONLY
  -DCMAKE_BUILD_TYPE=Release
  -DBOOTCTRL_ENABLE_EDGEML=ON
  -DFORTE_TESTS=OFF
  -DFORTE_COM_PAHOMQTT=ON
  -DFORTE_COM_PAHOMQTT_INCLUDE_DIR="${paho_include_dir}"
  -DFORTE_COM_PAHOMQTT_LIB_DIR="${paho_lib_dir}"
  -DFORTE_COM_PAHOMQTT_LIB="${paho_lib}"
  -DFORTE_EDGEML_BACKEND_TFLITE=ON
  -DFORTE_EDGEML_TFLITE_INCLUDE_DIR="${tflite_include_dir}"
  -DFORTE_EDGEML_TFLITE_LIBRARY="${tflite_library_path}"
  -DFORTE_EDGEML_TFLITE_EXTRA_LIBRARIES="${tflite_extra_libraries}"
  -DFORTE_EDGEML_TFLITE_FLATBUFFERS_INCLUDE_DIR="${tflite_flatbuffers_include}"
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
)
if [ "${TARGET_ARCH}" = "armhf" ]; then
  cmake_args+=(-DCMAKE_SYSTEM_PROCESSOR="${ARMHF_PROCESSOR}")
fi

cmake -S . -B "${BUILD_DIR}" -G Ninja "${cmake_args[@]}"
if [ "${TARGET_ARCH}" = "armhf" ]; then
  grep -Eq "^CMAKE_SYSTEM_PROCESSOR:.*=${ARMHF_PROCESSOR}$" "${BUILD_DIR}/CMakeCache.txt"
fi
grep -q "FORTE_EDGEML_BACKEND_TFLITE_ACTIVE:INTERNAL=ON" "${BUILD_DIR}/CMakeCache.txt"
grep -q "FORTE_COM_PAHOMQTT:BOOL=ON" "${BUILD_DIR}/CMakeCache.txt"

cmake --build "${BUILD_DIR}" -j"$(nproc)"

package_name="bootctrl-edgeos-${TARGET_ARCH}-release-tflite"
package_root="${BUILD_DIR}/package/${package_name}"
artifact_dir="${BUILD_DIR}/artifacts"

rm -rf "${BUILD_DIR}/package"
mkdir -p "${package_root}/bin" "${package_root}/lib" "${package_root}/configurations" "${artifact_dir}"

cp "${BUILD_DIR}/4diacFORTE/forte" "${package_root}/bin/forte"
if [ -f "README.md" ]; then
  cp README.md "${package_root}/README.md"
else
  printf "%s\n" \
    "BootCtrl EdgeOS Release Artifact" \
    "README.md was not available in this CI checkout." \
    > "${package_root}/README.md"
fi
if [ -f "docs/DEPLOYMENT.md" ]; then
  cp docs/DEPLOYMENT.md "${package_root}/DEPLOYMENT.md"
else
  printf "%s\n" \
    "Deployment guide was not available in this CI checkout." \
    "Run ./run-forte.sh to start the runtime." \
    > "${package_root}/DEPLOYMENT.md"
fi
if [ -d "configurations/examples" ]; then
  cp -r configurations/examples "${package_root}/configurations/examples"
fi
if [ -d "configurations/templates" ]; then
  cp -r configurations/templates "${package_root}/configurations/templates"
fi
if [ -f "configurations/README.md" ]; then
  cp configurations/README.md "${package_root}/configurations/README.md"
fi

if [[ "${tflite_library_path}" == *.so* ]]; then
  tflite_lib_real="$(readlink -f "${tflite_library_path}")"
  cp "${tflite_lib_real}" "${package_root}/lib/"
  for candidate in "$(dirname "${tflite_lib_real}")"/libtensorflow-lite.so* "$(dirname "${tflite_lib_real}")"/libtensorflowlite.so*; do
    if [ -f "${candidate}" ] || [ -L "${candidate}" ]; then
      cp -P "${candidate}" "${package_root}/lib/" || true
    fi
  done
fi
if [ -n "${tflite_extra_libraries}" ]; then
  IFS=";" read -r -a extra_lib_array <<< "${tflite_extra_libraries}"
  for extra_lib in "${extra_lib_array[@]}"; do
    if [[ "${extra_lib}" == *.so* ]] && { [ -f "${extra_lib}" ] || [ -L "${extra_lib}" ]; }; then
      cp -P "${extra_lib}" "${package_root}/lib/" || true
    fi
  done
fi
if [ -d "/work/third_party/tflite-build" ]; then
  while IFS= read -r dep_so; do
    cp -P "${dep_so}" "${package_root}/lib/" || true
  done < <(find /work/third_party/tflite-build -type f -name "lib*.so*" | sort -u)
fi
for rt_lib in libstdc++.so.6 libgcc_s.so.1 libatomic.so.1; do
  rt_path="$(ldconfig -p | awk -v l="${rt_lib}" '$1==l {print $NF; exit}')"
  if [ -n "${rt_path}" ] && [ -e "${rt_path}" ]; then
    cp -P "${rt_path}" "${package_root}/lib/" || true
  fi
done

cat > "${package_root}/run-forte.sh" <<'RUNNER'
#!/usr/bin/env bash
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
export LD_LIBRARY_PATH="${SCRIPT_DIR}/lib:${LD_LIBRARY_PATH:-}"
exec "${SCRIPT_DIR}/bin/forte" "$@"
RUNNER
chmod +x "${package_root}/run-forte.sh"

cat > "${package_root}/BUILD_INFO.txt" <<INFO
GIT_SHA=${GITHUB_SHA}
BUILD_DATE_UTC=$(date -u +%Y-%m-%dT%H:%M:%SZ)
RUN_ID=${GITHUB_RUN_ID}
ARCH=${TARGET_ARCH}
BACKEND=TFLITE
CONFIG=Release
INFO

tar -C "${BUILD_DIR}/package" -czf "${artifact_dir}/${package_name}.tar.gz" "${package_name}"
sha256sum "${artifact_dir}/${package_name}.tar.gz" > "${artifact_dir}/${package_name}.tar.gz.sha256"
