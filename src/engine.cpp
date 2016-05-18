#include "os.h"
#include "lib.h"
#include "event.h"
#include "kernel.h"
#include "engine.h"
#include "texture.h"
#include "rectangle.h"
#include "exception.h"
#include "collidable.h"
#include "game_event.h"
#include "level_factory.h"
#include "events_translator.h"
#include "game_events_listener.h"

#include <list>
#include <memory>
#include <utility>

using namespace std;

namespace ijengine 
{
    static Kernel *kernel = nullptr;
    
    Engine::Engine()
    {
        auto files = os::list_files("libs");
        string path("");
        
        for (auto file : files)
            if (file.find("kernel") != string::npos) 
            {
                path = "libs/" + file;
                break;
            }
            
        if (path.size() == 0)
            throw Exception("Kernel not found in libs dir");
            
        m_kernel_lib = unique_ptr<Lib>(os::load_lib(path));
        
        if (not m_kernel_lib)
            throw Exception("Can't load kernel lib");
            
        auto sym = m_kernel_lib->symbol("create_kernel");
                          
        if (not sym)
            throw Exception("Invalid kernel: missing create_kernel()");
            
        Kernel * (*create)();
        *reinterpret_cast<void **>(&create) = sym;
        
        kernel = create();
        
        if (not kernel)
            throw Exception("Can't create the kernel");
    }
    
    Engine::~Engine()
    {
        resources::release_all();

        auto sym = m_kernel_lib->symbol("destroy_kernel");
                          
        if (not sym)
            throw Exception("Invalid kernel: missing destroy_kernel()");
            
        void (*destroy)(Kernel *);
        *reinterpret_cast<void **>(&destroy) = sym;
        
        destroy(kernel);
    }
    
    namespace video 
    {
        Window *
        create_window(const string& title, int w, int h)
        {
            return kernel->create_window(title, w, h);
        }
    }

    namespace event
    {
        static list<const EventsTranslator *> translators;
        static list<GameEventsListener *> listeners;

        void
        dispatch_pending_events(unsigned now)
        {
            auto events = kernel->pending_events(now);

            if (events.empty())
                return;

            list<game_event_t> game_events;

            for (auto translator : translators)
            {
                auto more = translator->translate(events);
                game_events.merge(more);

                if (events.empty())
                    break;
            }

            for (auto event : game_events)
            {
                GameEvent game_event = GameEvent::deserialize(event.second,
                    event.first);

                for (auto listener : listeners)
                    if (listener->on_event(game_event))
                        break;
            }
        }

        void
        register_translator(const EventsTranslator *translator)
        {
            if (translator) translators.push_back(translator);
        }

        void
        unregister_translator(const EventsTranslator *translator)
        {
            if (translator) translators.remove(translator);
        }

        void
        register_listener(GameEventsListener *listener)
        {
            if (listener) listeners.push_back(listener);
        }

        void unregister_listener(GameEventsListener *listener)
        {
            if (listener) listeners.remove(listener);
        }
    }

    namespace time
    {
        unsigned
        time_elapsed()
        {
            return kernel->time_elapsed();
        }

        void
        pause()
        {
            kernel->pause_timer();
        }

        void
        resume()
        {
            kernel->resume_timer();
        }
    }

    namespace level
    {
        static LevelFactory *level_factory = nullptr;

        void
        register_factory(LevelFactory *factory)
        {
            level_factory = factory;
        }

        void unregister_factory()
        {
            level_factory = nullptr;
        }

        Level *
        make(const string& level_id)
        {
            return level_factory ? level_factory->make_level(level_id) : nullptr;
        }

        void
        release(Level *level)
        {
            if (level_factory)
                level_factory->release(level);
        }
    }

    namespace resources {
        static string textures_dir_path { "." };
        static map<string, shared_ptr<Texture> > textures;
        static Canvas *canvas = nullptr;

        void
        set_textures_dir(const string& dir_path)
        {
            textures_dir_path = dir_path;
        }

        void
        set_canvas(Canvas *c)
        {
            canvas = c;
        }

        shared_ptr<Texture>
        get_texture(const string& name)
        {
            if (not canvas)
                throw Exception("Can't load textures with a null canvas\n");

            auto it = textures.find(name);

            if (it != textures.end())
                return it->second;

            string filepath = textures_dir_path + "/" + name;
            Texture *texture = kernel->load_texture(canvas, filepath);

            if (not texture)
                throw Exception("Can't load texture " + filepath);

            textures[name] = shared_ptr<Texture>(texture);

            return textures[name];
        }

        void
        release_all()
        {
            textures.clear();
        }
    }

    namespace physics {
        static Collidable *target = nullptr;
        static list<Collidable *> objects;
        static Mode collisions_mode = ALL_TO_ALL;

        Rectangle collision(Collidable *a, Collidable *b);
        void set_colision_mode(Mode mode, Collidable *c);

        void
        register_object(Collidable *c)
        {
            if (c)
                objects.push_back(c);
        }

        void
        unregister_object(Collidable *c)
        {
            objects.remove(c);

            if (target == c)
                target = nullptr;
        }
        
        Rectangle
        collision(Collidable *a, Collidable *b)
        {
            auto bbA = a->bounding_box();
            auto bbB = b->bounding_box();

            auto r = bbA.intersection(bbB);

            if (r.area() == 0)
                return r;

            for (auto hbA : a->hit_boxes())
            {
                for (auto hbB : b->hit_boxes())
                {
                    auto t = hbA.intersection(hbB);

                    if (t.area() > 0)
                        return t;
                }
            }

            return Rectangle(0, 0, 0, 0);
        }

        void
        do_collisions()
        {
            switch (collisions_mode) {
            case ONE_TO_ALL:
                for (auto obj : objects)
                {
                    if (obj == target)
                        continue;

                    auto r = collision(target, obj);

                    if (r.area() > 0)
                    {
                        target->on_collision(obj, r);
                        obj->on_collision(target, r);
                    }
                }

                break;

            case ALL_TO_ALL:
                for (auto a : objects)
                {
                    for (auto b : objects)
                    {
                        if (a == b)
                            continue;

                        auto r = collision(a, b);

                        if (r.area() > 0)
                        {
                            a->on_collision(b, r);
                            b->on_collision(a, r);
                        }
                    }
                }

                break;

            default:
                break;
            }
        }

        void
        set_colision_mode(Mode mode, Collidable *c)
        {
            collisions_mode = mode;

            if (c and mode == ONE_TO_ALL)
                target = c;
        }
    }
}