#include "register_types.h"

#include "core/class_db.h"
#include "slicer.h"

void register_slicer_types() {
    ClassDB::register_class<Slicer>();
}

void unregister_slicer_types() {
}
