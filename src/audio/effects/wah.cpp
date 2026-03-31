#include "audio/effects/wah.h"

namespace GuitarAmp {

WahPedal::WahPedal() {
    params_ = {
        {"Mode",        0.0f,  0.0f,   1.0f,  0.0f, "",   "0 = Manual sweep; 1 = Auto-wah driven by envelope follower."},
        {"Sweep",       0.5f,  0.0f,   1.0f,  0.5f, "",   "Manual wah sweep position. Heel-down (0) = low frequency; toe-down (1) = high frequency. Active in Manual mode."},
        {"Resonance",   3.5f,  1.0f,   8.0f,  3.5f, "Q",  "Bandpass filter Q factor. Higher values give a sharper, more vocal wah character."},
        {"Sensitivity", 0.5f,  0.0f,   1.0f,  0.5f, "",   "How strongly the input signal amplitude drives the sweep in Auto-wah mode."},
        {"Attack",      5.0f,  1.0f,  50.0f,  5.0f, "ms", "Envelope follower attack time. Faster values track transients more aggressively."},
        {"Release",   100.0f, 20.0f, 500.0f, 100.0f, "ms", "Envelope follower release time. Controls how quickly the filter falls back after a note dies."},
    };
}

void WahPedal::process(float* buffer, int num_samples) {
    if (!enabled_) return;

    bool is_auto   = (params_[0].value > 0.5f);
    float sweep    = params_[1].value;
    float q        = params_[2].value;
    float sens     = params_[3].value;
    float atk_ms   = params_[4].value;
    float rel_ms   = params_[5].value;

    float atk_coeff = std::exp(-1.0f / (sample_rate_ * atk_ms  * 0.001f));
    float rel_coeff = std::exp(-1.0f / (sample_rate_ * rel_ms  * 0.001f));

    // Damping factor for SVF (inverse of Q)
    float q_damp = 1.0f / q;

    // Sweep smoothing (~5 ms time constant — removes zipper noise on knob moves)
    float smooth_coeff = std::exp(-1.0f / (sample_rate_ * 0.005f));

    for (int i = 0; i < num_samples; ++i) {
        float dry = buffer[i];

        // --- Compute target sweep position ---
        float target_sweep;
        if (is_auto) {
            // Peak-tracking envelope follower
            float abs_in = std::abs(dry);
            float coeff  = (abs_in > envelope_) ? atk_coeff : rel_coeff;
            envelope_    = coeff * envelope_ + (1.0f - coeff) * abs_in;

            // Map envelope → sweep (sensitivity scales the detection range)
            target_sweep = clamp(envelope_ * sens * 4.0f, 0.0f, 1.0f);
        } else {
            target_sweep = sweep;
        }

        // Smooth sweep position to prevent discontinuities
        sweep_smooth_ += (1.0f - smooth_coeff) * (target_sweep - sweep_smooth_);

        // --- Map sweep 0→1 to centre frequency 350 Hz → 2500 Hz (exponential) ---
        constexpr float FREQ_LO = 350.0f;
        constexpr float FREQ_HI = 2500.0f;
        float fc = FREQ_LO * std::pow(FREQ_HI / FREQ_LO, sweep_smooth_);

        // --- Chamberlin state-variable filter ---
        // f_coeff = 2 * sin(pi * fc / sr)  (exact drive coefficient for SVF)
        float f_coeff = 2.0f * std::sin(PI * fc / static_cast<float>(sample_rate_));

        float hp    = dry - q_damp * svf_bp_ - svf_lp_;
        svf_bp_     = f_coeff * hp   + svf_bp_;
        svf_lp_     = f_coeff * svf_bp_ + svf_lp_;

        // Bandpass output boosted by Q for the classic resonant wah peak
        float wet = svf_bp_ * q * 0.5f;

        buffer[i] = dry * (1.0f - mix_) + wet * mix_;
    }
}

void WahPedal::reset() {
    svf_lp_      = 0.0f;
    svf_bp_      = 0.0f;
    envelope_    = 0.0f;
    sweep_smooth_ = 0.5f;
}

} // namespace GuitarAmp
