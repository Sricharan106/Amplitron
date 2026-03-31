#pragma once

#include "audio/effect.h"

namespace GuitarAmp {

class WahPedal : public Effect {
public:
    WahPedal();
    void process(float* buffer, int num_samples) override;
    void reset() override;
    const char* name() const override { return "Wah"; }
    std::vector<EffectParam>& params() override { return params_; }

private:
    std::vector<EffectParam> params_;

    // State-variable filter (Chamberlin topology) state
    float svf_lp_ = 0.0f;
    float svf_bp_ = 0.0f;

    // Envelope follower state (auto-wah)
    float envelope_ = 0.0f;

    // Smoothed sweep position (avoids zipper noise)
    float sweep_smooth_ = 0.5f;
};

} // namespace GuitarAmp
