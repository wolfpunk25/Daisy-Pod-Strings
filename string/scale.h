#pragma once

#include <array>
#include <random>
#include "nocopy.h"
#include "config.h"

namespace synthux {

class Scale {
public:
  Scale();
  ~Scale() {}

  uint8_t ScalesCount() {
    return kStringScalesCount;
  }

  void SetScaleIndex(uint8_t index) {
    if (index != _scale_index) {
      _scale_index = index;
      _PrepareScale();
    }
  }

  float TransMult(const float value) {
    auto new_trans_index = static_cast<uint8_t>(value * (_trans.size() - 1));
    return _trans[new_trans_index];
  }

  float FreqAt(uint8_t idx) {
    return _scale[idx] * 0.5f;
  }

  float Random() {
    std::uniform_int_distribution<uint8_t> note_distribution(0, kStringScaleSize - 1);
    return FreqAt(note_distribution(_rand_engine));
  }

private:
  NOCOPY(Scale)

  void _PrepareScale() {
    auto transposition = _trans[_trans_index];
    auto& scale = kStringScales[_scale_index];
    for (uint8_t i = 0; i < kStringScaleSize; i++) {
      _scale[i] = scale[i] * transposition;
    }
  }

  uint8_t _scale_index;
  uint8_t _trans_index;
  std::default_random_engine _rand_engine;

  std::array<float, kStringScaleSize> _scale;

  std::array<float, 25> _trans = {
    0.5f,
    0.52973154717962f,
    0.56123102415466f,  0.594603557501335f,
    0.629960524947413f, 0.667419927084995f,
    0.707106781186527f, 0.749153538438323f,
    0.793700525984085f, 0.840896415253703f,
    0.890898718140331f, 0.943874312681689f,
    1.f,
    1.0594630943593f,   1.12246204830938f,
    1.18920711500273f,  1.25992104989489f,
    1.33483985417006f,  1.41421356237313f,
    1.49830707687673f,  1.58740105196826f,
    1.6817928305075f,   1.78179743628076f,
    1.88774862536348f,
    2.f
  };
};

};
