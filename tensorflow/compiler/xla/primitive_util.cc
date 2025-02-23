/* Copyright 2017 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "tensorflow/compiler/xla/primitive_util.h"

#include <limits>
#include <string>

#include "absl/container/flat_hash_map.h"
#include "absl/strings/ascii.h"
#include "tensorflow/compiler/xla/types.h"
#include "tensorflow/compiler/xla/util.h"
#include "tensorflow/compiler/xla/xla_data.pb.h"
#include "tensorflow/tsl/platform/logging.h"

namespace xla {
namespace primitive_util {

int SignificandWidth(PrimitiveType type) {
  switch (type) {
    case F32:
      return std::numeric_limits<float>::digits;
    case F64:
      return std::numeric_limits<double>::digits;
    case BF16:
      return std::numeric_limits<bfloat16>::digits;
    case F16:
      return std::numeric_limits<half>::digits;
    case F8E5M2:
      return std::numeric_limits<tsl::float8_e5m2>::digits;
    case F8E4M3FN:
      return std::numeric_limits<tsl::float8_e4m3fn>::digits;
    case F8E4M3B11FNUZ:
      return std::numeric_limits<tsl::float8_e4m3b11>::digits;
    case F8E5M2FNUZ:
      return std::numeric_limits<tsl::float8_e5m2fnuz>::digits;
    case F8E4M3FNUZ:
      return std::numeric_limits<tsl::float8_e4m3fnuz>::digits;
    default:
      LOG(FATAL) << "Not a floating data type " << type;
  }
}

int ExponentWidth(PrimitiveType type) {
  // Per the IEEE-754 standard: a floating point type is stored as a sign bit, a
  // biased exponent and a trailing significand field.
  int total_bit_width = BitWidth(type);
  // This field contains all bits in the significand other than the leading
  // digit which is implied by the exponent.
  int trailing_significand_field_width = SignificandWidth(type) - 1;
  // The sign is encoded with a single bit.
  int kSignBitWidth = 1;
  // The remaining bits are used for encoding the biased exponent.
  return total_bit_width - (trailing_significand_field_width + kSignBitWidth);
}

int UnderflowExponent(PrimitiveType type) {
  // |std::numeric_limits<float>::min_exponent| is defined as: "minimum negative
  // integer such that radix raised to the power one less than that integer is a
  // normalized floating-point number." as such it does not actually yield the
  // minimum exponent but one above the minimum exponent that a normalized
  // number can have.
  switch (type) {
    case F32:
      return std::numeric_limits<float>::min_exponent;
    case F64:
      return std::numeric_limits<double>::min_exponent;
    case BF16:
      return std::numeric_limits<bfloat16>::min_exponent;
    case F16:
      return std::numeric_limits<half>::min_exponent;
    case F8E5M2:
      return std::numeric_limits<tsl::float8_e5m2>::min_exponent;
    case F8E4M3FN:
      return std::numeric_limits<tsl::float8_e4m3fn>::min_exponent;
    case F8E4M3B11FNUZ:
      return std::numeric_limits<tsl::float8_e4m3b11>::min_exponent;
    case F8E5M2FNUZ:
      return std::numeric_limits<tsl::float8_e5m2fnuz>::min_exponent;
    case F8E4M3FNUZ:
      return std::numeric_limits<tsl::float8_e4m3fnuz>::min_exponent;
    default:
      LOG(FATAL) << "Not a floating data type " << type;
  }
}

int OverflowExponent(PrimitiveType type) {
  // |std::numeric_limits<float>::max_exponent| is defined as: "Maximum positive
  // integer such that radix raised to the power one less than that integer is a
  // representable finite floating-point number." as such it does not actually
  // yield the maximum exponent but the exponent of the first integer which
  // overflows.
  switch (type) {
    case F32:
      return std::numeric_limits<float>::max_exponent;
    case F64:
      return std::numeric_limits<double>::max_exponent;
    case BF16:
      return std::numeric_limits<bfloat16>::max_exponent;
    case F16:
      return std::numeric_limits<half>::max_exponent;
    case F8E5M2:
      return std::numeric_limits<tsl::float8_e5m2>::max_exponent;
    case F8E4M3FN:
      return std::numeric_limits<tsl::float8_e4m3fn>::max_exponent;
    case F8E4M3B11FNUZ:
      return std::numeric_limits<tsl::float8_e4m3b11>::max_exponent;
    case F8E5M2FNUZ:
      return std::numeric_limits<tsl::float8_e5m2fnuz>::max_exponent;
    case F8E4M3FNUZ:
      return std::numeric_limits<tsl::float8_e4m3fnuz>::max_exponent;
    default:
      LOG(FATAL) << "Not a floating data type " << type;
  }
}

int ExponentBias(PrimitiveType type) {
  switch (type) {
    case F32:
    case BF16:
    case F16:
    case F64:
    case F8E5M2:
    case F8E4M3FN:
      return (1 << (ExponentWidth(type) - 1)) - 1;
    case F8E4M3B11FNUZ:
      return 11;
    case F8E4M3FNUZ:
      return 8;
    case F8E5M2FNUZ:
      return 16;
    default:
      LOG(FATAL) << "Not a floating data type " << type;
  }
}

bool HasInfinity(PrimitiveType type) {
  switch (type) {
    case F32:
      return std::numeric_limits<float>::has_infinity;
    case F64:
      return std::numeric_limits<double>::has_infinity;
    case BF16:
      return std::numeric_limits<bfloat16>::has_infinity;
    case F16:
      return std::numeric_limits<half>::has_infinity;
    case F8E5M2:
      return std::numeric_limits<tsl::float8_e5m2>::has_infinity;
    case F8E4M3FN:
      return std::numeric_limits<tsl::float8_e4m3fn>::has_infinity;
    case F8E4M3B11FNUZ:
      return std::numeric_limits<tsl::float8_e4m3b11>::has_infinity;
    case F8E5M2FNUZ:
      return std::numeric_limits<tsl::float8_e5m2fnuz>::has_infinity;
    case F8E4M3FNUZ:
      return std::numeric_limits<tsl::float8_e4m3fnuz>::has_infinity;
    // Assumes types not enumerated are non-floating point types without an
    // infinity.
    default:
      return false;
  }
}

xla::PrimitiveType SignedIntegralTypeForBitWidth(int64_t src_bitwidth) {
  switch (src_bitwidth) {
    case 4:
      return xla::S4;
    case 8:
      return xla::S8;
    case 16:
      return xla::S16;
    case 32:
      return xla::S32;
    case 64:
      return xla::S64;
    default:
      return xla::PRIMITIVE_TYPE_INVALID;
  }
}

// Class to memoize the computation of
//   absl::AsciiStrToLower(PrimitiveType_Name(p))
// for all PrimitiveType values "p"
//
// xla::OPAQUE_TYPE canonically maps to the string "opaque" -- the only reason
// it's called OPAQUE_TYPE is to avoid clashing with a windows.h macro.
class PrimitiveTypeNameGenerator {
 public:
  PrimitiveTypeNameGenerator() {
    for (int i = 0; i < PrimitiveType_ARRAYSIZE; i++) {
      if (i == static_cast<int>(OPAQUE_TYPE)) {
        lowercase_name_[i] = "opaque";
      } else if (PrimitiveType_IsValid(i)) {
        lowercase_name_[i] = absl::AsciiStrToLower(
            PrimitiveType_Name(static_cast<PrimitiveType>(i)));
      }
    }
  }
  const std::string& LowercaseName(PrimitiveType t) {
    CHECK_LT(t, PrimitiveType_ARRAYSIZE);
    return lowercase_name_[static_cast<int>(t)];
  }

 private:
  std::string lowercase_name_[PrimitiveType_ARRAYSIZE];
};

const std::string& LowercasePrimitiveTypeName(PrimitiveType s) {
  static auto* gen = new PrimitiveTypeNameGenerator();
  return gen->LowercaseName(s);
}

namespace {

// Returns a map from lower-case primitive type name to primitive type.
//
// Due to Postel's Law considerations, both "opaque" and "opaque_type" map to
// the xla::OPAQUE_TYPE enumerator.
const absl::flat_hash_map<std::string, PrimitiveType>&
GetPrimitiveTypeStringMap() {
  static absl::flat_hash_map<std::string, PrimitiveType>* name_to_type = [] {
    static auto* map = new absl::flat_hash_map<std::string, PrimitiveType>;
    for (int i = 0; i < PrimitiveType_ARRAYSIZE; i++) {
      if (PrimitiveType_IsValid(i) && i != PRIMITIVE_TYPE_INVALID) {
        auto value = static_cast<PrimitiveType>(i);
        (*map)[LowercasePrimitiveTypeName(value)] = value;
      }
    }
    (*map)["opaque"] = OPAQUE_TYPE;
    return map;
  }();
  return *name_to_type;
}

}  // namespace

StatusOr<PrimitiveType> StringToPrimitiveType(absl::string_view name) {
  const auto& map = GetPrimitiveTypeStringMap();
  auto found = map.find(name);
  if (found == map.end()) {
    return InvalidArgument("Invalid element type string: \"%s\".", name);
  }
  return found->second;
}

bool IsPrimitiveTypeName(absl::string_view name) {
  const auto& map = GetPrimitiveTypeStringMap();
  auto found = map.find(name);
  return found != map.end();
}

}  // namespace primitive_util
}  // namespace xla
