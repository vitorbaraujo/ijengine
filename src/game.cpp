#include "game.h"
#include "level.h"
#include "window.h"
#include "canvas.h"
#include "engine.h"
#include "game_event.h"

#include <memory>

#include <SDL2/SDL.h>

#define FRAME_VALUES 10

using std::unique_ptr;


Uint32 frametimes[FRAME_VALUES];
Uint32 frametimelast;
Uint32 framecount;

float framespersecond;

void fpsinit() {
    memset(frametimes, 0, sizeof(frametimes));
    framecount = 0;
    framespersecond = 0;
    frametimelast = SDL_GetTicks();
}

void fpsthink() {
    Uint32 frametimesindex;
    Uint32 getticks;
    Uint32 count;
    Uint32 i;

    frametimesindex = framecount % FRAME_VALUES;
    getticks = SDL_GetTicks();
    frametimes[frametimesindex] = getticks - frametimelast;
    frametimelast = getticks;
    framecount++;

    if (framecount < FRAME_VALUES) {
        count = framecount;
    }
    else{
        count = FRAME_VALUES;
    }

    framespersecond = 0;

    for (i = 0; i < count; i++) {
        framespersecond += frametimes[i];
    }

    framespersecond /= count;
    framespersecond = 1000.f / framespersecond;
}

namespace ijengine {

    Game::Game(const string& title, int w, int h, double scale)
        : m_title(title), m_w(w), m_h(h), m_scale(scale), m_state(PAUSED)
    {
        event::register_listener(this);
    }

    Game::~Game()
    {
        event::unregister_listener(this);
    }

    int
    Game::run(const string& level_id)
    {
        auto test = video::create_window(m_title, m_w, m_h, m_scale);
        auto window = unique_ptr<Window>(test);
        
        if (not window)
            return -1;

        Canvas *canvas = window->canvas();
        resources::set_canvas(canvas);

        Level *current_level = level::make(level_id);
        unsigned last = time::time_elapsed();

        m_state = current_level ? RUNNING : QUIT;

        fpsinit();

        while (m_state != QUIT)
        {
            unsigned now = time::time_elapsed();

            event::dispatch_pending_events(now);

            current_level->update(now, last);
            current_level->draw(canvas, now, last);

            audio::play_audio_from_path(current_level->audio());

            physics::do_collisions(now, last);

            current_level->draw(canvas, now, last);
            canvas->update();

            if (current_level->done())
            {
                audio::stop_audio();
                string next = current_level->next();
                level::release(current_level);
                current_level = level::make(next);

                if (not current_level) m_state = QUIT;
            }

            last = now;

            fpsthink();
            printf("%f\n", framespersecond);
        }

        return 0;
    }

    bool
    Game::on_event(const GameEvent& event)
    {
        if (event.id() == game_event::QUIT)
        {
            m_state = QUIT;
            return true;
        }

        return false;
    }
}
