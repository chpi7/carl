#ifndef carl_vm_h
#define carl_vm_h

#include <cstdint>

typedef void* carl_pointer_t;
typedef void* carl_stackelem_t;

class VM {
   private:
    uint64_t stack_size;
    carl_stackelem_t* stack = nullptr;

    carl_pointer_t ip;
    carl_stackelem_t* sp;

   public:
    VM(uint64_t stack_size);
    ~VM();
    void load_binary(carl_pointer_t start, uint64_t size);
    carl_pointer_t get_stack_top();
};

#endif