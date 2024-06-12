#pragma once
/**
 * This will be the future polymorphic type system if it ever gets that far :)
 */

#include <cassert>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#define TODO(msg) assert(false && msg)

namespace carl {
namespace polymorphic {

template <typename T>
using ref = std::shared_ptr<T>;

/* ==== AST for testing ==== */

namespace ast {

struct Expr {
    virtual void print(std::ostream &os) { os << "UNDEFINED"; };
    void println(std::ostream &os) {
        this->print(os);
        os << "\n";
    };

    template <typename T, typename... Args>
    static ref<T> make(Args &&...args) {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }
};
struct BoolConst : public Expr {
    const bool value = false;
    void print(std::ostream &os) { os << (value ? "true" : "false"); };
};
struct IntegerConst : public Expr {
    const uint64_t value = 0;
    IntegerConst(uint64_t value) : value(value){};
    void print(std::ostream &os) { os << value; };
};
struct Var : public Expr {
    const std::string id = "";
    Var(std::string id) : id(id){};
    void print(std::ostream &os) { os << id; };
};
struct Fn : public Expr {
    const std::string fname = "";
    const std::string argname = "";
    const ref<Expr> expr = nullptr;
    Fn(std::string fname, std::string argname, ref<Expr> expr)
        : fname(fname), argname(argname), expr(std::move(expr)){};

   public:
    void print(std::ostream &os) {
        os << fname << "(" << argname << ") = ";
        expr->print(os);
    };
};

struct Application : public Expr {
    const ref<Expr> a = nullptr;
    const ref<Expr> b = nullptr;
    Application(ref<Expr> a, ref<Expr> b) : a(std::move(a)), b(std::move(b)){};
    void print(std::ostream &os) {
        a->print(os);
        os << "(";
        b->print(os);
        os << ")";
    };
};
}  // namespace ast

/* type Type = Bool | Integer | Product (a,b) | Function (d,i) */
class Type {
   public:
    uint64_t eq_class;
    inline static uint32_t next_eq_class = 0;

   public:
    enum class BasicType { NotBasic, Integer, Bool };

   public:
    Type() : eq_class(next_eq_class++){};
    bool operator==(const Type &other) const { return this == &other; }
    virtual bool is_var() { return false; }
    virtual bool is_function() { return false; }
    virtual void print(std::ostream &os) = 0;
    virtual BasicType get_basic_type() { return BasicType::NotBasic; }

    /* Check if the basic type of this type is equal to that of the other type.
     * False if basic type is NotBasic */
    bool basic_type_equal(ref<Type> &other) {
        const BasicType this_basic_type = get_basic_type();
        if (this_basic_type == BasicType::NotBasic) {
            return false;
        }

        const BasicType other_basic_type = other->get_basic_type();
        return other_basic_type == this_basic_type;
    };

    void println(std::ostream &os) {
        this->print(os);
        os << "\n";
    };
};

static std::ostream &operator<<(std::ostream &os, const ref<Type> &t) {
    t->print(os);
    return os;
};

class TypeVar : public Type {
   private:
    inline static uint32_t next_id = 0;
    uint64_t id;

   public:
    TypeVar() : id(next_id++){};
    bool is_var() { return true; }
    void print(std::ostream &os) { os << "'c" << eq_class; };
    // void print(std::ostream &os) { os << "'" << id << "_" << eq_class; };
};

class TypeError : public Type {
    void print(std::ostream &os) { os << "ERROR"; };
};

struct Product : public Type {
    ref<Type> a;
    ref<Type> b;
    Product(ref<Type> a, ref<Type> b) : a(std::move(a)), b(std::move(b)){};
    void print(std::ostream &os) {
        os << "(";
        a->print(os);
        os << " x ";
        b->print(os);
        os << ")_" << eq_class;
    };
};

class Integer : public Type {
   public:
    inline static ref<Integer> instance = nullptr;

   private:
    void print(std::ostream &os) { os << "Integer"; };
    BasicType get_basic_type() { return BasicType::Integer; };
};

class Bool : public Type {
   public:
    inline static ref<Bool> instance = nullptr;

   private:
    void print(std::ostream &os) { os << "Bool"; };
    BasicType get_basic_type() { return BasicType::Bool; };
};

/** A mapping from the domain to the image type */
class Function : public Type {
   public:
    ref<Type> domain;
    ref<Type> image;
    Function(ref<Type> domain, ref<Type> image) : domain(std::move(domain)), image(std::move(image)){};
    void print(std::ostream &os) {
        domain->print(os);
        os << " ->_" << eq_class << " ";
        image->print(os);
    };
    bool is_function() { return true; }
};

struct Env {
    static constexpr bool debug = false;

    const Env *parent = nullptr;
    std::unordered_map<std::string, ref<Type>> mappings;

    void update_in_place(const std::string &name, ref<Type> t) {
        if (debug) {
            std::clog << "assign (in place) " << name << " = " << t << "\n";
        }
        mappings.insert(std::make_pair(name, t));
    }

    Env update(const std::string &name, ref<Type> &t) const {
        Env new_env{};
        if (debug) {
            std::clog << "assign " << name << " = " << t << "\n";
        }
        new_env.mappings.insert(std::make_pair(name, t));
        new_env.parent = this;

        return new_env;
    }

    std::optional<ref<Type>> lookup(const std::string &name) const {
        if (debug) {
            std::clog << "lookup(" << name << ")\n";
        }
        if (mappings.contains(name)) {
            return mappings.at(name);
        } else if (parent != nullptr) {
            if (debug) {
                std::clog << "checking in parent env\n";
            }
            return parent->lookup(name);
        } else {
            if (debug) {
                std::clog << "not found\n";
            }
            return std::nullopt;
        }
    }

    void print(std::ostream &os) const {
        for (const auto &kvp : mappings) {
            std::cout << kvp.first << ": ";
            kvp.second->println(std::cout);
        }
        if (parent != nullptr) {
            parent->print(os);
        }
    }
};

class TypeChecker {
    // Poor mans pattern matching:
#define MATCH(from, type, var)                        \
    auto var = std::dynamic_pointer_cast<type>(from); \
    var != nullptr

   private:
    static constexpr bool debug = true;

    std::unordered_map<uint64_t, ref<Type>> representative_types = {};
    std::unordered_map<uint64_t, std::vector<ref<Type>>> equivalence_classes = {};

   public:
    Env globals{};

   public:
    ref<Type> check(const ref<ast::Expr> &expr, Env &env, bool store_type = false) {
        if (debug) {
            std::clog << "check(";
            expr->print(std::clog);
            std::clog << ")\n";
        }

        if (MATCH(expr, ast::BoolConst, _)) {
            return make<Bool>();
        } else if (MATCH(expr, ast::IntegerConst, _)) {
            return make<Integer>();
        } else if (MATCH(expr, ast::Var, var)) {
            auto type = env.lookup(var->id);
            if (type.has_value()) {
                ref<Type> t = *type;
                if (debug) {
                    std::clog << "lookup result: " << t << " is_function=" << t->is_function() << "\n";
                }
                const bool is_var = t->is_function();
                const auto r = t->is_function() ? instantiate(t) : t;  // only instantiate functions (or non vars)
                if (is_var && debug) {
                    std::clog << "instantiate " << var->id << "(" << *type << ") as " << r << "\n";
                }
                return to_repr(r);
            } else {
                if (debug) {
                    std::clog << "new unknown identifier '" << var->id << "'\n";
                }
                ref<Type> t = make<TypeVar>();
                env.update(var->id, t);
                return to_repr(t);
            }
        } else if (MATCH(expr, ast::Fn, fn)) {
            ref<Type> dom_type = make<TypeVar>();
            ref<Type> img_type = make<TypeVar>();

            // Create a function type for future lookups IF fn has a name (which it has for sure atm).
            ref<Type> tf = make<Function>(dom_type, img_type);
            Env e1 = env.update(fn->fname, tf);

            Env e2 = e1.update(fn->argname, dom_type);
            ref<Type> expr_type = check(fn->expr, e2);

            unify(expr_type, img_type);

            if (store_type) {
                env.update_in_place(fn->fname, to_repr(tf));
            }

            return to_repr(tf);
        } else if (MATCH(expr, ast::Application, app)) {
            ref<Type> tf = representative_types[check(app->a, env)->eq_class];
            if (MATCH(tf, Function, tf2)) {
                ref<Type> targ = check(app->b, env);
                unify(targ, tf2->domain);  // unify actual and expected arg type
                if (debug) {
                    std::clog << "after unify targ = " << targ << " tf2->domain = " << tf2->domain << "\n";
                }
                return to_repr(tf2->image);
            } else {
                std::cerr << "Type Checking Error: Expected Function, found: " << tf << "\n";
                return make<TypeError>();
            }
        }
        return make<TypeError>();
    }

    template <typename T, typename... Args>
    ref<T> make(Args &&...args) {
        return init_type_ref(std::make_shared<T>(std::forward<Args>(args)...));
    }

    template <typename... Args>
    ref<Integer> make(Args &&...args) {
        if (Integer::instance == nullptr) {
            Integer::instance = init_type_ref(std::make_shared<Integer>(std::forward<Args>(args)...));
        }
        return Integer::instance;
    }

    template <typename... Args>
    ref<Bool> make(Args &&...args) {
        if (Bool::instance == nullptr) {
            Bool::instance = init_type_ref(std::make_shared<Bool>(std::forward<Args>(args)...));
        }
        return Bool::instance;
    }

   private:
    ref<Type> to_repr(const ref<Type> t) { return representative_types[t->eq_class]; }

    ref<Type> instantiate(const ref<Type> t, std::unordered_map<uint64_t, std::weak_ptr<Type>> &resolved) {
        // std::clog << "instantiate(" << t << ")\n";
        if (MATCH(t, TypeVar, v)) {
            if (resolved.contains(t->eq_class)) {
                return resolved[t->eq_class].lock();
            } else {
                ref<Type> new_type = make<TypeVar>();
                resolved.insert(std::make_pair(t->eq_class, new_type));
                return new_type;
            }
        } else if (MATCH(t, Function, f)) {
            ref<Type> dom = instantiate(f->domain, resolved);
            ref<Type> img = instantiate(f->image, resolved);
            return make<Function>(dom, img);
        } else if (MATCH(t, Product, p)) {
            ref<Type> a = instantiate(p->a, resolved);
            ref<Type> b = instantiate(p->b, resolved);
            return make<Product>(a, b);
        } else {
            // Bool, Int, TypeError: no child types.
            return t;
        }
    }

    ref<Type> instantiate(ref<Type> t) {
        std::unordered_map<uint64_t, std::weak_ptr<Type>> m{};
        return instantiate(t, m);
    }

    template <typename T>
    ref<T> init_type_ref(ref<T> t) {
        equivalence_classes[t->eq_class].push_back(t);
        representative_types.insert(std::make_pair(t->eq_class, t));
        return t;
    }

    /* Find the representative type for this type. */
    ref<Type> find(ref<Type> t) {
        if (debug) {
            std::clog << "find(" << t << ")";
        }

        assert(representative_types.contains(t->eq_class));
        const auto &result = representative_types[t->eq_class];

        if (debug) {
            std::clog << " => " << result << "\n";
        }

        return result;
    }

    void union_types(ref<Type> a, ref<Type> b) {
        if (debug) {
            std::clog << "union(" << a << ", " << b << ")\n";
        }

        // Merge class b into class a.
        const uint64_t new_class = a->eq_class;
        const uint64_t merged_class = b->eq_class;
        auto &class_a_types = equivalence_classes[a->eq_class];
        auto &class_b_types = equivalence_classes[b->eq_class];
        for (const auto &in_b : class_b_types) {
            in_b->eq_class = new_class;
        }
        class_a_types.insert(class_a_types.end(), class_b_types.begin(), class_b_types.end());

        if (!b->is_var() && a->is_var()) {
            representative_types[new_class] = b;
        } else {
            // Leave a as the representative.
        }

        // Clear the merged class of b.
        equivalence_classes.erase(merged_class);
        representative_types.erase(merged_class);
        if (debug) {
            std::clog << "Clearing eq class " << merged_class << "\n";
            std::clog << "New unified eq class " << new_class << "\n";
            print_type_checking_state();
        }
    }

    bool unify(ref<Type> a, ref<Type> b) {
        // Basically Ullman, Compilers 2nd edition, page 397
        if (debug) {
            std::clog << "unify(" << a << ", " << b << ")\n";
        }

        ref<Type> s = find(a);
        ref<Type> t = find(b);

        /* Types are the same. Nothing to unify. */
        if (s == t) {
            return true;
        } else if (s->basic_type_equal(t)) {
            return true;
        }

        ref<Function> f_s = std::dynamic_pointer_cast<Function>(s);
        ref<Function> f_t = std::dynamic_pointer_cast<Function>(t);
        ref<Product> p_s = std::dynamic_pointer_cast<Product>(s);
        ref<Product> p_t = std::dynamic_pointer_cast<Product>(t);

        if (f_s != nullptr && f_t != nullptr) {
            union_types(s, t);
            return unify(f_s->domain, f_t->domain) && unify(f_s->image, f_t->image);
        } else if (p_s != nullptr && p_t != nullptr) {
            union_types(s, t);
            return unify(p_s->a, p_t->a) && unify(p_s->b, p_t->b);
        } else if (s->is_var() || t->is_var()) {
            union_types(s, t);
            return true;
        }

        if (debug) {
            std::clog << "Unification failed: " << a << " and " << b << "\n";
        }
        return false;
    }

    void print_type_checking_state() {
        std::clog << " === Type State ===\n";
        std::clog << "Representative Types:\n";
        for (const auto &kvp : representative_types) {
            if (kvp.second != nullptr) {
                std::clog << kvp.first << "   " << kvp.second << "\n";
            } else {
                std::clog << kvp.first << "   "
                          << "EMPTY"
                          << "\n";
            }
        }

        std::clog << "Equivalence Classes:\n";
        for (const auto &kvp : equivalence_classes) {
            std::clog << kvp.first << "   ";
            for (const auto &t : kvp.second) {
                std::clog << t << ", ";
            }
            std::clog << "\n";
        }
    }
#undef MATCH
};

/* ------------------- TYPE CHECKING ------------------- */

}  // namespace polymorphic
}  // namespace carl
