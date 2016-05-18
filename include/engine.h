#ifndef IJENGINE_ENGINE_H
#define IJENGINE_ENGINE_H

#include <string>
#include <memory>

using std::string;
using std::unique_ptr;
using std::shared_ptr;

namespace ijengine {
    
    class Lib;
    class Level;
    class Window;
    class Canvas;
    class Texture;
    class Collidable;
    class LevelFactory;
    class EventsTranslator;
    class GameEventsListener;
    
    class Engine {
    public:
        Engine();
        ~Engine();    
        
    private:
        unique_ptr<Lib> m_kernel_lib;
    };
    
    namespace video {
        Window * create_window(const string& title, int w, int h);
    }

    namespace event {
        void dispatch_pending_events(unsigned now);
        void register_translator(const EventsTranslator *translator);
        void unregister_translator(const EventsTranslator *translator);
        void register_listener(GameEventsListener *listener);
        void unregister_listener(GameEventsListener *listener);
    }

    namespace time {
        unsigned time_elapsed();
        void pause();
        void resume();
    }

    namespace level {
        void register_factory(LevelFactory *factory);
        void unregister_factory();
        Level * make(const string& level_id);
        void release(Level *level);
    }

    namespace resources {
        void set_textures_dir(const string& dir_path);
        void set_canvas(Canvas *canvas);
        shared_ptr<Texture> get_texture(const string& name);
        void release_all();
    }

    namespace physics {
        typedef enum { ONE_TO_ALL, ALL_TO_ALL, NONE } Mode;

        void register_object(Collidable *c);
        void unregister_object(Collidable *c);
        void do_collisions();
        void set_collision_mode(Mode mode, Collidable *c = nullptr);
    }
}

#endif