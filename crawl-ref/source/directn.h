/*
 *  File:       directn.h
 *  Summary:    Functions used when picking squares.
 *  Written by: Linley Henzell
 */


#ifndef DIRECT_H
#define DIRECT_H

#include "describe.h"
#include "externs.h"
#include "enum.h"
#include "ray.h"
#include "state.h"

class range_view_annotator
{
public:
    range_view_annotator(int range);
    virtual ~range_view_annotator();
};

// An object that modifies the behaviour of the direction prompt.
class targeting_behaviour
{
public:
    targeting_behaviour(bool just_looking = false);
    virtual ~targeting_behaviour();

    // Returns a keystroke for the prompt.
    virtual int get_key();
    virtual command_type get_command(int key = -1);

    // Should we force a redraw?
    virtual bool should_redraw() const { return false; }
    // Clear the bit set above.
    virtual void clear_redraw()  { return; }

    // Update the prompt shown at top.
    virtual void update_top_prompt(std::string* p_top_prompt) {}

 private:
    std::string prompt;

public:
    bool just_looking;
    bool compass;
};

// output from direction() function:
class dist
{
public:
    dist();

    bool isMe() const;

    bool isValid;       // valid target chosen?
    bool isTarget;      // target (true), or direction (false)?
    bool isEndpoint;    // Does the player want the attack to stop at target?
    bool isCancel;      // user cancelled (usually <ESC> key)
    bool choseRay;      // user wants a specific beam
    coord_def target;   // target x,y or logical extension of beam to map edge
    coord_def delta;    // delta x and y if direction - always -1,0,1
    ray_def ray;        // ray chosen if necessary
};

struct direction_chooser_args
{
    targeting_type restricts;
    targ_mode_type mode;
    int range;
    bool just_looking;
    bool needs_path;
    bool may_target_monster;
    bool may_target_self;
    const char *target_prefix;
    std::string top_prompt;
    targeting_behaviour *behaviour;
    bool cancel_at_self;
    bool show_floor_desc;

    direction_chooser_args() :
        restricts(DIR_NONE),
        mode(TARG_ANY),
        range(-1),
        just_looking(false),
        needs_path(true),
        may_target_monster(true),
        may_target_self(false),
        target_prefix(NULL),
        behaviour(NULL),
        cancel_at_self(false),
        show_floor_desc(false) {}
};

class direction_chooser
{
public:
    direction_chooser(dist& moves, const direction_chooser_args& args);
    bool choose_direction();

private:
    bool choose_again();        // Used when replaying keys
    bool choose_compass();      // Used when we only need to choose a direction

    bool do_main_loop();

    // Return the location where targeting should start.
    coord_def find_default_target() const;

    void handle_mlist_cycle_command(command_type key_command);
    void handle_wizard_command(command_type key_command, bool* loop_done);
    void handle_movement_key(command_type key_command, bool* loop_done);

    bool in_range(const coord_def& p) const;

    // Jump to the player.
    void move_to_you();

    // Cycle to the next (dir == 1) or previous (dir == -1) object.
    void object_cycle(int dir);

    // Cycle to the next (dir == 1) or previous (dir == -1) monster.
    void monster_cycle(int dir);

    // Cycle to the next feature of the given type.
    void feature_cycle_forward(int feature);

    // Set the remembered target to be the current target.
    void update_previous_target() const;

    // Finalise the current choice of target. Return true if
    // successful, false if failed (e.g. out of range.)
    bool select(bool allow_out_of_range, bool endpoint);
    bool select_compass_direction(const coord_def& delta);
    bool select_previous_target();

    // Return true if we need to abort targeting due to a signal.
    bool handle_signals();

    void reinitialize_move_flags();

    // Return or set the current target.
    const coord_def& target() const;
    void set_target(const coord_def& new_target);

    std::string build_targeting_hint_string() const;

    actor* targeted_actor() const;
    monsters* targeted_monster() const;

    // Functions which print things to the user.
    // Each one is commented with a sample output.

    // Whatever the caller defines. Typically something like:
    // Casting: Venom Bolt.
    // Can be modified by the targeting_behaviour.
    void print_top_prompt() const;

    // Press: ? - help, Shift-Dir - straight line, t - giant bat
    void print_key_hints() const;

    // Here: An orc wizard, wielding a glowing orcish dagger, and wearing
    // an orcish robe (miasma, silenced, almost dead)
    // OR:
    // Apport: A short sword.
    void print_target_description() const;

    // Helper functions for the above.
    void print_target_monster_description() const;
    void print_target_object_description() const;

    // You see 2 +3 dwarven bolts here.
    // There is something else lying underneath.
    void print_items_description() const;

    // Lava.
    //
    // If boring_too is false, then don't print anything on boring
    // terrain (i.e. floor.)
    void print_floor_description(bool boring_too) const;

    std::string target_interesting_terrain_description() const;
    std::string target_cloud_description() const;
    std::string target_sanctuary_description() const;
    std::string target_silence_description() const;
    std::vector<std::string> target_cell_description_suffixes() const;
    std::vector<std::string> monster_description_suffixes(
        const monsters* mon) const;

    void describe_cell() const;

    // Move the target to where the mouse pointer is (on tiles.)
    // Returns whether the move was valid, i.e., whether the mouse
    // pointer is in bounds.
    bool tiles_update_target();

    // Display the prompt when beginning targeting.
    void show_initial_prompt();

    void toggle_beam();

    void finalize_moves();
    command_type massage_command(command_type key_command) const;
    void draw_beam_if_needed();
    void do_redraws();

    // Whether the current target is you.
    bool looking_at_you() const;

    // Whether the current target is valid.
    bool move_is_ok() const;

    void cycle_targeting_mode();

    void describe_target();
    void show_help();

    // Parameters.
    dist& moves;                // Output.
    targeting_type restricts;   // What kind of target do we want?
    targ_mode_type mode;        // Hostiles or friendlies?
    int range;                  // Max range to consider
    bool just_looking;
    bool needs_path;            // Determine a ray while we're at it?
    bool may_target_monster;
    bool may_target_self;       // ?? XXX Used only for _init_mlist() currently
    const char *target_prefix;  // A string displayed before describing target
    std::string top_prompt;     // Shown at the top of the message window
    targeting_behaviour *behaviour; // Can be NULL for default
    bool cancel_at_self;        // Disallow self-targeting?
    bool show_floor_desc;       // Describe the floor of the current target

    // Internal data.
    ray_def beam;               // The (possibly invalid) beam.
    bool show_beam;             // Does the user want the beam displayed?
    bool have_beam;             // Is the currently stored beam valid?
    coord_def objfind_pos, monsfind_pos; // Cycling memory

    // What we need to redraw.
    bool need_beam_redraw;
    bool need_cursor_redraw;
    bool need_text_redraw;
    bool need_all_redraw;       // All of the above.

    bool show_items_once;       // Should we show items this time?
    bool target_unshifted;      // Do unshifted direction keys fire?

    // Default behaviour, saved across instances.
    static targeting_behaviour stock_behaviour;
    
};

void direction(dist &moves, const direction_chooser_args& args);

bool in_los_bounds(const coord_def& p);
bool in_viewport_bounds(int x, int y);
inline bool in_viewport_bounds(const coord_def& pos) {
  return in_viewport_bounds(pos.x, pos.y);
}
bool in_los(int x, int y);
bool in_los(const coord_def &pos);
bool in_vlos(int x, int y);
bool in_vlos(const coord_def &pos);

std::string thing_do_grammar(description_level_type dtype,
                             bool add_stop,
                             bool force_article,
                             std::string desc);

std::string get_terse_square_desc(const coord_def &gc);
void terse_describe_square(const coord_def &c, bool in_range = true);
void full_describe_square(const coord_def &c);
void get_square_desc(const coord_def &c, describe_info &inf,
                     bool examine_mons = false);

void describe_floor();
std::string get_monster_equipment_desc(const monsters *mon,
                                bool full_desc = true,
                                description_level_type mondtype = DESC_CAP_A,
                                bool print_attitude = false);

int dos_direction_unmunge(int doskey);

std::string feature_description(const coord_def& where, bool covering = false,
                                description_level_type dtype = DESC_CAP_A,
                                bool add_stop = true, bool base_desc = false);
std::string raw_feature_description(dungeon_feature_type grid,
                                    trap_type tr = NUM_TRAPS,
                                    bool base_desc = false);
std::string feature_description(dungeon_feature_type grid,
                                trap_type trap = NUM_TRAPS, bool bloody = false,
                                description_level_type dtype = DESC_CAP_A,
                                bool add_stop = true, bool base_desc = false, bool mold = false);

void set_feature_desc_short(dungeon_feature_type grid,
                            const std::string &desc);
void set_feature_desc_short(const std::string &base_name,
                            const std::string &desc);

void setup_feature_descs_short();

std::vector<dungeon_feature_type> features_by_desc(const base_pattern &pattern);

void full_describe_view(void);

extern const struct coord_def Compass[8];

#endif
