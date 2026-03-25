#pragma once

#include "common.h"
#include "audio/audio_engine.h"
#include "gui/command_history.h"

namespace GuitarAmp {

class PedalWidget;

/**
 * @brief Visual representation of the audio effect signal chain.
 *
 * Renders a horizontal strip of PedalWidget instances, an "Add Pedal" menu,
 * and drag-and-drop reordering. All structural changes (add, remove, reorder)
 * are routed through CommandHistory for undo/redo support.
 */
class PedalBoard {
public:
    /**
     * @brief Construct the pedal board.
     * @param engine  Reference to the audio engine that owns the effect chain.
     * @param history Reference to the shared command history for undo/redo.
     */
    PedalBoard(AudioEngine& engine, CommandHistory& history);

    /** @brief Destructor. */
    ~PedalBoard();

    /** @brief Render the toolbar and signal chain each frame. */
    void render();

    /** @brief Recreate PedalWidget instances from the current engine effect list. */
    void rebuild_widgets();

    /** @brief Whether only enabled pedals are shown (default true). */
    bool show_active_only() const { return show_active_only_; }

private:
    /** @brief Render the "+ Add Pedal" button and its popup menu. */
    void render_add_pedal_menu();

    /** @brief Render the amp model selector dropdown. */
    void render_amp_selector();

    /** @brief Render the signal flow line, pedal widgets, and drag-and-drop targets. */
    void render_signal_chain();

    /** @brief Find the index of the current AmpSimulator in the effect chain (-1 if none). */
    int find_amp_index() const;

    AudioEngine& engine_;
    CommandHistory& history_;
    std::vector<std::unique_ptr<PedalWidget>> widgets_;
    bool show_active_only_ = true;
};

} // namespace GuitarAmp
