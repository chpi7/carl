#ifndef carl_name_environment_h
#define carl_name_environment_h

#include <memory>
#include <map>
#include <tuple>
#include <string>

namespace carl {

template <typename V>
class Environment {
   private:
    int id = 0;
    std::unique_ptr<Environment<V>> parent;
    std::map<std::string, V> values;

   public:
    Environment(std::unique_ptr<Environment> env) : parent(std::move(env)) {
        if (parent) {
            id = parent->id + 1; 
        } else {
            id = 0;
        }
    };

    Environment(std::map<std::string, V>&& v, std::unique_ptr<Environment> p,
                int id)
        : id(id), parent(std::move(p)), values(v){};

    void push() {
        parent = std::make_unique<Environment<V>>(
            std::move(values),
            std::move(parent),
            id
        );
        values = std::map<std::string, V>();
        id += 1;
    }

    void pop() {
        values.clear();
        id = parent->id;
        values = std::move(parent->values);
        parent = std::move(parent->parent);
    }

    V& get_variable(const std::string& name) {
        if (values.contains(name)) {
            return values.at(name);
        } else {
            return parent->get_variable(name);
        }
    }

    bool can_set_variable(const std::string& name) {
        return !values.contains(name);
    }

    void set_variable(const std::string& name, V v) {
        values.insert_or_assign(name, v);
    }

    bool has_variable(const std::string& name, int min_id = 0) {
        if (id < min_id) return false;
        return values.contains(name) || (parent && parent->has_variable(name, min_id));
    }

    int get_id() {
        return id;
    }
};

/** Calls push_env() in the constructor and pop_env() in the destructor. */
template <typename V>
struct UseNewEnv {
   private:
    Environment<V>* env;

   public:
    UseNewEnv(Environment<V>* env) {
        this->env = env;
        env->push();
    }

    ~UseNewEnv() {
        env->pop();
    }
};
}  // namespace carl

#endif