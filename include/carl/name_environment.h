#ifndef carl_name_environment_h
#define carl_name_environment_h

#include <memory>
#include <map>
#include <tuple>
#include <string>

namespace carl {

template <typename V, typename F>
class Environment {
   private:
    int id = 0;
    std::unique_ptr<Environment<V, F>> parent;
    std::map<std::string, V> variables;
    std::map<std::string, F> functions;

   public:
    Environment(std::unique_ptr<Environment> env) : parent(std::move(env)) {
        if (parent) {
            id = parent->id + 1; 
        } else {
            id = 0;
        }
    };

    Environment(std::map<std::string, V>&& v, std::map<std::string, F>&& f,
                std::unique_ptr<Environment> p, int id)
        : id(id), parent(std::move(p)), variables(v), functions(f){};

    void push() {
        parent = std::make_unique<Environment<V, F>>(
            std::move(variables),
            std::move(functions),
            std::move(parent),
            id
        );
        variables = std::map<std::string, V>();
        functions = std::map<std::string, F>();
        id += 1;
    }

    void pop() {
        variables.clear();
        functions.clear();

        id = parent->id;
        variables = std::move(parent->variables);
        functions = std::move(parent->functions);
        parent = std::move(parent->parent);
    }

    V& get_variable(const std::string& name) {
        if (variables.contains(name)) {
            return variables[name];
        } else {
            return parent->get_variable(name);
        }
    }

    bool can_set_variable(const std::string& name) {
        return !variables.contains(name);
    }

    void set_variable(const std::string& name) {
        variables[name] = nullptr;
    }

    void set_variable(const std::string& name, V v) {
        variables[name] = v;
    }

    bool has_variable(const std::string& name) {
        return variables.contains(name) || (parent && parent->has_variable(name));
    }

    F& get_function(const std::string& name) {
        if (functions.contains(name)) {
            return functions[name];
        } else {
            return parent->get_function(name);
        }
    }

    bool can_set_function(const std::string& name) {
        return !functions.contains(name);
    }

    void set_function(const std::string& name) {
        functions[name] = nullptr;
    }

    void set_function(const std::string& name, F f) {
        functions[name] = f;
    }

    bool has_function(const std::string& name) {
        return functions.contains(name) || (parent && parent->has_function(name));
    }
};

/** Calls push_env() in the constructor and pop_env() in the destructor. */
template <typename V, typename F>
struct UseNewEnv {
   private:
    Environment<V, F>* env;

   public:
    UseNewEnv(Environment<V, F>* env) {
        this->env = env;
        env->push();
    }

    ~UseNewEnv() {
        env->pop();
    }
};
}  // namespace carl

#endif