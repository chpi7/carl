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

/* type Type = Bool | Integer | Product (a,b) | Function (d,i) */
class Type {
   public:
    uint64_t eq_class;
    inline static uint32_t next_eq_class = 0;
    inline static std::unordered_map<uint64_t, ref<Type>> representative_types =
        {};
    inline static std::unordered_map<uint64_t, std::vector<ref<Type>>>
        equivalence_classes = {};

   public:
    enum class BasicType { NotBasic, Integer, Bool };

   public:
    Type() : eq_class(next_eq_class++){};
    bool operator==(const Type &other) const { return this == &other; }
    virtual bool is_var() { return false; }
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

static void print_type_checking_state() {
    std::clog << " === Type State ===\n";
    std::clog << "Representative Types:\n";
    for (const auto &kvp : Type::representative_types) {
        if (kvp.second != nullptr) {
            std::clog << kvp.first << "   " << kvp.second << "\n";
        } else {
            std::clog << kvp.first << "   "
                      << "EMPTY"
                      << "\n";
        }
    }

    std::clog << "Equivalence Classes:\n";
    for (const auto &kvp : Type::equivalence_classes) {
        std::clog << kvp.first << "   ";
        for (const auto &t : kvp.second) {
            std::clog << t << ", ";
        }
        std::clog << "\n";
    }
}

class TypeVar : public Type {
   private:
    inline static uint32_t next_id = 0;
    uint64_t id;

   public:
    TypeVar() : id(next_id++){};
    bool is_var() { return true; }
    void print(std::ostream &os) { os << "'" << id << "_" << eq_class; };
};

class TypeError : public Type {
    void print(std::ostream &os) { os << "ERROR"; };
};

struct Product : public Type {
    ref<Type> a;
    ref<Type> b;
};

class Integer : public Type {
    void print(std::ostream &os) { os << "Integer"; };
    BasicType get_basic_type() { return BasicType::Integer; };
};
class Bool : public Type {
    void print(std::ostream &os) { os << "Bool"; };
    BasicType get_basic_type() { return BasicType::Bool; };
};
/** A mapping from the domain to the image type */
class Function : public Type {
   public:
    ref<Type> domain;
    ref<Type> image;
    Function(ref<Type> domain, ref<Type> image)
        : domain(std::move(domain)), image(std::move(image)){};
    void print(std::ostream &os) {
        domain->print(os);
        os << " ->_" << eq_class << " ";
        image->print(os);
    };
};

template <typename T>
static ref<T> init_type_ref(ref<T> t) {
    Type::equivalence_classes[t->eq_class].push_back(t);
    Type::representative_types.insert(std::make_pair(t->eq_class, t));
    return t;
}

#define CREATE_MAKE_FN(name)                                      \
    template <typename... Args>                                   \
    static ref<name> get_##name(Args &&...args) {                 \
        return init_type_ref(                                     \
            std::make_shared<name>(std::forward<Args>(args)...)); \
    }

CREATE_MAKE_FN(Bool);
CREATE_MAKE_FN(Integer);
CREATE_MAKE_FN(Product);
CREATE_MAKE_FN(Function);
CREATE_MAKE_FN(TypeVar);
CREATE_MAKE_FN(TypeError);

#undef CREATE_MAKE_FN

struct Expr {
    virtual void print(std::ostream &os) { os << "UNDEFINED"; };
    void println(std::ostream &os) {
        this->print(os);
        os << "\n";
    };
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

#define CREATE_MAKE_FN(name)                                        \
    template <typename... Args>                                     \
    static ref<name> get_##name(Args &&...args) {                   \
        return std::make_shared<name>(std::forward<Args>(args)...); \
    }

CREATE_MAKE_FN(BoolConst);
CREATE_MAKE_FN(IntegerConst);
CREATE_MAKE_FN(Var);
CREATE_MAKE_FN(Fn);
CREATE_MAKE_FN(Application);

#undef CREATE_MAKE_FN

/* ------------------- TYPE CHECKING ------------------- */

struct Env {
    Env *parent = nullptr;
    std::unordered_map<std::string, ref<Type>> mappings;

    void update_in_place(const std::string &name, ref<Type> &t) {
        std::clog << "assign (in place) " << name << " = " << t << "\n";
        mappings.insert(std::make_pair(name, t));
    }

    Env update(const std::string &name, ref<Type> &t) {
        Env new_env{};

        std::clog << "assign " << name << " = " << t << "\n";
        mappings.insert(std::make_pair(name, t));
        new_env.parent = this;

        return new_env;
    }

    std::optional<ref<Type>> lookup(const std::string &name) const {
        std::clog << "lookup(" << name << ")\n";
        if (mappings.contains(name)) {
            return mappings.at(name);
        } else if (parent != nullptr) {
            std::clog << "checking in parent env\n";
            return parent->lookup(name);
        } else {
            std::clog << "not found\n";
            return std::nullopt;
        }
    }

    void print(std::ostream &os) {
        for (const auto &kvp : mappings) {
            std::cout << kvp.first << ": ";
            kvp.second->println(std::cout);
        }
        if (parent != nullptr) {
            parent->print(os);
        }
    }
};

/* Find the representative type for this type. */
static ref<Type> find(ref<Type> t) {
    std::clog << "find(" << t << ")";

    assert(Type::representative_types.contains(t->eq_class));
    const auto &result = Type::representative_types[t->eq_class];

    std::clog << " => " << result << "\n";

    return result;
}

static void union_types(ref<Type> a, ref<Type> b) {
    std::clog << "union(" << a << ", " << b << ")\n";

    // Merge class b into class a.
    const uint64_t new_class = a->eq_class;
    const uint64_t merged_class = b->eq_class;
    auto &class_a_types = Type::equivalence_classes[a->eq_class];
    auto &class_b_types = Type::equivalence_classes[b->eq_class];
    for (const auto &in_b : class_b_types) {
        in_b->eq_class = new_class;
    }
    class_a_types.insert(class_a_types.end(), class_b_types.begin(),
                         class_b_types.end());

    if (!b->is_var() && a->is_var()) {
        Type::representative_types[new_class] = b;
    } else {
        // Leave a as the representative.
    }

    // Clear the merged class of b.
    std::clog << "Clearing eq class " << merged_class << "\n";
    Type::equivalence_classes[merged_class].clear();
    Type::representative_types[merged_class] = nullptr;
    std::clog << "New unified eq class " << new_class << "\n";

    print_type_checking_state();
}

static bool unify(ref<Type> a, ref<Type> b) {
    // Basically Ullman, Compilers 2nd edition, page 397
    std::clog << "unify(" << a << ", " << b << ")\n";

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

    std::clog << "Unification failed: " << a << " and " << b << "\n";
    return false;
}

static ref<Type> check(const ref<Expr> &expr, Env &env) {
    std::clog << "check(";
    expr->print(std::clog);
    std::clog << ")\n";
    // Poor mans pattern matching:
#define MATCH(from, type, var)                        \
    auto var = std::dynamic_pointer_cast<type>(from); \
    var != nullptr

    if (MATCH(expr, BoolConst, _)) {
        return get_Bool();
    } else if (MATCH(expr, IntegerConst, _)) {
        return get_Integer();
    } else if (MATCH(expr, Var, var)) {
        auto type = env.lookup(var->id);
        if (type.has_value()) {
            return *type;
        } else {
            std::clog << "new unknown identifier '" << var->id << "'\n";
            ref<Type> t = get_TypeVar();
            env.update(var->id, t);
            return t;
        }
    } else if (MATCH(expr, Fn, fn)) {
        TODO(
            "implement instantiate to make fresh variables. NOT JUST HERE, but "
            "for anything function like, check book");

        ref<Type> t1 = get_TypeVar();
        ref<Type> t2 = get_TypeVar();

        // if fn has a name (which it has for sure atm)
        ref<Type> tf = get_Function(t1, t2);
        Env e1 = env.update(fn->fname, tf);

        Env e2 = e1.update(fn->argname, t1);
        ref<Type> te = check(fn->expr, e2);

        unify(te, t2);

        return tf;
    } else if (MATCH(expr, Application, app)) {
        ref<Type> tf = check(app->a, env);
        if (MATCH(tf, Function, tf2)) {
            ref<Type> targ = check(app->b, env);
            unify(targ, tf2->domain);  // unify actual and expected arg type
            return tf2->image;
        } else {
            std::cerr << "Type Checking Error: Expected Function, found: " << tf
                      << "\n";
            return get_TypeError();
        }
    }
    return get_TypeError();

#undef MATCH
}
}  // namespace polymorphic
}  // namespace carl
